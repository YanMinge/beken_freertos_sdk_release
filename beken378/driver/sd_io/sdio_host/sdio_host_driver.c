// Copyright 2020-2021 Beken
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "include.h"
#include "mem_pub.h"
#include "gpio_pub.h"
#include "intc_pub.h"
#include "sdio_config.h"
#include "sdio_host.h"
#include "sdio_host_driver.h"
#include "sdio_host_hal.h"
#include "drv_model_pub.h"
#include "icu.h"
#include "icu_pub.h"

#if (!CONFIG_SYSTEM_CTRL)
#include "bk_sys_ctrl.h"
#endif

#if CONFIG_SDIO_PM_CB_SUPPORT
#include <pm.h>
#endif

#if ((CONFIG_SDIO_V2P0) && (CONFIG_SDIO_GDMA_EN))
#include "general_dma_pub.h"
#endif

#if (CONFIG_TASK_WDT)
#include "bk_wdt.h"
#endif

#if (CONFIG_SDIO_V2P0)
/*
 * TODO: until now(2022-08-20),we use specific value for timeout.
 *       The right way is read the SDCARD speed mode and SDIO clock frequency, and then set a suit value.
 *       But sometimes, the special SDCARD maybe at busy status(programming data to memory) too much time.
 */
#define SDIO_MAX_TX_WAIT_TIME (512)
#define SDIO_MAX_RX_WAIT_TIME (512)
#define SDIO_CMD_WAIT_TIME    (4)	//one cycle:try 128 counts
#define SDIO_BLOCK_SIZE (0x200)
#define APP_RSP_CMD               63
#endif
typedef struct {
	sdio_host_hal_t hal;
	uint32_t int_status;
	beken_queue_t irq_cmd_msg;	//hasn't event in freertos
	bool is_tx_blocked;
	beken_semaphore_t tx_sema;
	beken_semaphore_t rx_sema;
#if CONFIG_SDIO_PM_CB_SUPPORT
	uint32_t pm_backup[SDIO_PM_BACKUP_REG_NUM];
	uint8_t pm_backup_is_valid;
#endif

#if CONFIG_SDIO_GDMA_EN
	uint8_t dma_tx_en;
	uint8_t dma_tx_id;
	uint32_t tx_start_addr;
	uint32_t tx_total_len;	//how many bytes data will be write to FIFO:before APP start to write, it sets this value
	uint32_t tx_transfered_len;	//used by driver to record has transfered how many bytes
	uint32_t tx_cur_trans_len;
	uint8_t dma_rx_en;
	uint8_t dma_rx_id;
	uint32_t rx_start_addr;
	uint32_t rx_total_len;
	uint32_t rx_transfered_len;
	uint32_t rx_cur_trans_len;
#endif
} sdio_host_driver_t;

#define SDIO_HOST_RETURN_ON_NOT_INIT() do {\
	if (!s_sdio_host_driver_is_init) {\
		SDIO_HOST_LOGE("sdio host driver not init\r\n");\
		return BK_ERR_SDIO_HOST_NOT_INIT;\
	}\
} while(0)

static sdio_host_driver_t s_sdio_host = {0};
static bool s_sdio_host_driver_is_init = false;

static void sdio_host_isr(void);
void bk_sdio_clock_en(uint32_t enable);
void delay_us(UINT32 us_count);

#if (!CONFIG_GPIO_DEFAULT_SET_SUPPORT)
static void sdio_host_init_gpio(void)
{
	uint32_t param = GFUNC_MODE_SD_HOST;

	gpio_ctrl(CMD_GPIO_ENABLE_SECOND, &param);
}
#endif

#if CONFIG_SDIO_GDMA_EN
static uint32_t s_dma_transfer_size_check = 0;

static void sdio_dma_tx_start(dma_id_t id)
{
	if(id != s_sdio_host.dma_tx_id)
	{
		SDIO_HOST_LOGE("%s:id=%d,sdio_id=%d\r\n", __func__, id, s_sdio_host.dma_tx_id);
		return;
	}

	if(s_sdio_host.tx_transfered_len >= s_sdio_host.tx_total_len)
	{
		SDIO_HOST_LOGD("%s:id=%d,total_len=%d,transfered_len=%d\r\n", __func__, id, s_sdio_host.tx_total_len, s_sdio_host.tx_transfered_len);
		return;
	}

	uint32_t remain_len = s_sdio_host.tx_total_len - s_sdio_host.tx_transfered_len;
	uint32_t next_trans_len = remain_len < bk_dma_get_transfer_len_max(id) ? remain_len : bk_dma_get_transfer_len_max(id);

	if(next_trans_len)
	{
		s_sdio_host.tx_cur_trans_len = next_trans_len;
		bk_dma_set_transfer_len(id, next_trans_len);
		bk_dma_set_src_addr(id, (uint32_t)(s_sdio_host.tx_start_addr + s_sdio_host.tx_transfered_len), (uint32_t)((s_sdio_host.tx_start_addr + s_sdio_host.tx_transfered_len)+next_trans_len));
		bk_dma_start(id);
	}
}

void sdio_dma_tx_finish(dma_id_t id)
{
	/* Current round finish, add the transfered len*/
	s_sdio_host.tx_transfered_len += s_sdio_host.tx_cur_trans_len;
	sdio_dma_tx_start(id);
}

static bk_err_t sdio_dma_tx_init(void)
{
    GDMACFG_TPYES_ST cfg = {0};

    cfg.dstdat_width = 32;
    cfg.srcdat_width = 32;

    cfg.dstptr_incr = 0;
    cfg.srcptr_incr = 1;

    /*This is shared memory address,and it will be reconfigured later*/
    cfg.src_start_addr = (void*)NULL;
    cfg.dst_start_addr = (void*)SDIO_REG0XF_ADDR;

    cfg.channel = GDMA_CHANNEL_2;
    cfg.prio = 0;
    cfg.u.type5.dst_loop_start_addr = (void*)SDIO_REG0XF_ADDR;
    cfg.u.type5.dst_loop_end_addr = (void*)SDIO_REG0XF_ADDR;

    cfg.half_fin_handler = NULL;
    cfg.fin_handler = (DMA_ISR_FUNC)sdio_dma_tx_finish;

    cfg.src_module = GDMA_X_SRC_DTCM_WR_REQ;
    cfg.dst_module = GDMA_X_DST_SDIO_RX_REQ;
    gdma_ctrl(CMD_GDMA_CFG_TYPE5, &cfg);

    SDIO_HOST_LOGI("dst start addr=0x%8x\n", (uint32_t) SDIO_REG0XF_ADDR);
    if(s_sdio_host.dma_tx_id != DMA_ID_MAX)
    SDIO_HOST_LOGI("s_sdio_host.dma_tx_id = %d.\n", (dma_id_t)s_sdio_host.dma_tx_id);
    s_sdio_host.dma_tx_id = GDMA_CHANNEL_2;/* bk_dma_alloc(DMA_DEV_SDIO)*/
    os_printf("[Attention:fix dma channel]s_sdio_host.dma_tx_id = %d.\n", (dma_id_t)s_sdio_host.dma_tx_id);
    if(DMA_ID_MAX == (dma_id_t)s_sdio_host.dma_tx_id)
    {
        return BK_ERR_DMA_ID;
    }

    BK_LOG_ON_ERR(bk_dma_register_isr((dma_id_t)s_sdio_host.dma_tx_id, NULL, sdio_dma_tx_finish));
    BK_LOG_ON_ERR(bk_dma_enable_finish_interrupt((dma_id_t)s_sdio_host.dma_tx_id));

    return BK_OK;
}

static void sdio_dma_rx_start(dma_id_t id)
{
	if(id != s_sdio_host.dma_rx_id)
	{
		SDIO_HOST_LOGE("%s:id=%d,sdio_id=%d\r\n", __func__, id, s_sdio_host.dma_rx_id);
		return;
	}

	if(s_sdio_host.rx_transfered_len >= s_sdio_host.rx_total_len)
	{
		SDIO_HOST_LOGD("%s:id=%d,total_len=%d,transfered_len=%d\r\n", __func__, id, s_sdio_host.rx_total_len, s_sdio_host.rx_transfered_len);
		return;
	}

	uint32_t remain_len = s_sdio_host.rx_total_len - s_sdio_host.rx_transfered_len;
	uint32_t next_trans_len = remain_len < bk_dma_get_transfer_len_max(id) ? remain_len : bk_dma_get_transfer_len_max(id);

	if(next_trans_len)
	{
		s_sdio_host.rx_cur_trans_len = next_trans_len;
		bk_dma_set_transfer_len(id, next_trans_len);
		bk_dma_set_dest_addr(id, (uint32_t)(s_sdio_host.rx_start_addr + s_sdio_host.rx_transfered_len), (uint32_t)((s_sdio_host.rx_start_addr + s_sdio_host.rx_transfered_len)+next_trans_len));
		bk_dma_start(id);
	}
}

void sdio_dma_rx_finish(dma_id_t id)
{
	static uint32_t s_dma_rx_finish_cnt = 0;

	s_sdio_host.rx_transfered_len += s_sdio_host.rx_cur_trans_len;	//Current round finish, add the transfered len
	sdio_dma_rx_start(id);

	if(s_sdio_host.rx_total_len <= s_sdio_host.rx_transfered_len)
	{
		rtos_set_semaphore(&s_sdio_host.rx_sema);
		s_dma_rx_finish_cnt++;
		SDIO_HOST_LOGD("%s:s_dma_rx_finish_cnt=%d\r\n", __func__, s_dma_rx_finish_cnt);
	}
}

static bk_err_t sdio_dma_rx_init(void)
{
    GDMACFG_TPYES_ST cfg = {0};

    cfg.dstdat_width = 32;
    cfg.srcdat_width = 32;

    cfg.dstptr_incr = 1;
    cfg.srcptr_incr = 0;

    /*This is shared memory address,and it will be reconfigured later*/
    cfg.src_start_addr = (void*)SDIO_REG0XF_ADDR;
    cfg.dst_start_addr = NULL;

    cfg.channel = GDMA_CHANNEL_3;
    cfg.prio = 0;
    cfg.u.type5.dst_loop_start_addr = NULL;
    cfg.u.type5.dst_loop_end_addr = NULL;

    cfg.half_fin_handler = NULL;
    cfg.fin_handler = (DMA_ISR_FUNC)sdio_dma_rx_finish;

    cfg.src_module = GDMA_X_SRC_SDIO_TX_REQ;
    cfg.dst_module = GDMA_X_DST_DTCM_WR_REQ;
    gdma_ctrl(CMD_GDMA_CFG_TYPE5, &cfg);

    s_sdio_host.dma_rx_id = GDMA_CHANNEL_3;/* bk_dma_alloc(DMA_DEV_SDIO_RX);*/
    os_printf("[Attention:fix sdio rx dma channel]s_sdio_host.dma_rx_id = %d.\n", s_sdio_host.dma_rx_id);
    if(DMA_ID_MAX == s_sdio_host.dma_rx_id)
    {
        return BK_ERR_DMA_ID;
    }

    BK_LOG_ON_ERR(bk_dma_register_isr(s_sdio_host.dma_rx_id, NULL, sdio_dma_rx_finish));
    BK_LOG_ON_ERR(bk_dma_enable_finish_interrupt(s_sdio_host.dma_rx_id));

    return BK_OK;
}
#endif

/* 1. power up sdio host
 * 2. set clock
 * 3. enable sdio host interrupt
 * 4. map gpio
 */
static void sdio_host_init_common(void)
{
#if (CONFIG_SYSTEM_CTRL)
	uint32_t param = PWD_SDIO_CLK;
	icu_ctrl(CMD_CLK_PWR_UP, &param); /* sdio power up:sys_drv_dev_clk_pwr_up(CLK_PWR_ID_SDIO, CLK_PWR_CTRL_PWR_UP);*/

	// set sdio source clock as XTAL 26M
	param = PCLK_POSI_SDIO;
	icu_ctrl(CMD_CONF_PCLK_26M, &param); /* sys_hal_set_sdio_clk_sel(SDIO_CLK_XTL);*/

#if (CONFIG_SOC_BK7236XX || CONFIG_SOC_BK7239XX || CONFIG_SOC_BK7286XX)
	sdio_host_hal_write_blk_en(&s_sdio_host.hal, 1);
	sdio_host_hal_set_fifo_send_cnt(&s_sdio_host.hal, 128);
#endif

	param = IRQ_SDIO_BIT;
	icu_ctrl(CMD_ICU_INT_ENABLE, &param); /* sys_drv_int_enable(SDIO_INTERRUPT_CTRL_BIT);*/

#if CONFIG_SDIO_V2P0
	sdio_host_hal_enable_all_mask(&s_sdio_host.hal);
#endif
#else
	power_sdio_pwr_up();
	clk_set_sdio_clk_26m();
	icu_enable_sdio_interrupt();
#endif

// Temp code : if not set this, bk7271 sdio cmd line will send CMD73 forever
#if (CONFIG_SOC_BK7271)
	UINT32 param;

	param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
	sddev_control(DD_DEV_TYPE_SCTRL, CMD_SCTRL_BLK_ENABLE, &param);

	param = PSRAM_VDD_3_3V;
	sddev_control(DD_DEV_TYPE_SCTRL, CMD_QSPI_VDDRAM_VOLTAGE, &param);
#endif

#if CONFIG_GPIO_DEFAULT_SET_SUPPORT
	/*
	 * GPIO info is setted in GPIO_DEFAULT_DEV_CONFIG and inited in bk_gpio_driver_init->gpio_hal_default_map_init.
	 * If needs to re-config GPIO, can deal it here.
	 */
#else
	/* config sdio host gpio */
	sdio_host_init_gpio();
#endif
}

static void sdio_host_deinit_common(void)
{
	sdio_host_hal_reset_config_to_default(&s_sdio_host.hal);

#if (CONFIG_SYSTEM_CTRL)
	uint32_t param = PWD_SDIO_CLK;
	icu_ctrl(CMD_CLK_PWR_DOWN, &param); /* sdio power down: sys_drv_dev_clk_pwr_up(CLK_PWR_ID_SDIO, CLK_PWR_CTRL_PWR_DOWN);*/
#else
	power_sdio_pwr_down();
	icu_disable_sdio_interrupt();
#endif
}

static void sdio_host_set_clock_freq(uint32_t clk_src, uint32_t clk_div)
{
	sddev_control(ICU_DEV_NAME, CMD_SDIO_CLK_SEL, &clk_src);
	sddev_control(ICU_DEV_NAME, CMD_SDIO_CLK_DIV, &clk_div);
}

#if (CONFIG_SDIO_PM_CB_SUPPORT)
#define SDIO_PM_CHECK_RESTORE(id) do {\
	if (bk_pm_module_lv_sleep_state_get(PM_DEV_ID_SDIO)) {\
		bk_pm_module_vote_power_ctrl(PM_POWER_SUB_MODULE_NAME_BAKP_SDIO, PM_POWER_MODULE_STATE_ON);\
		bk_sdio_restore(0, (void *)id);\
		bk_pm_module_lv_sleep_state_clear(PM_DEV_ID_SDIO); \
	}\
} while(0)

static int bk_sdio_backup(uint64_t sleep_time, void *args)
{
	SDIO_HOST_RETURN_ON_NOT_INIT();
	if (!s_sdio_host.pm_backup_is_valid)
	{
		s_sdio_host.pm_backup_is_valid = 1;
		sdio_hal_backup(&s_sdio_host.hal, &s_sdio_host.pm_backup[0]);
		bk_sdio_clock_en(false);
	}
	return BK_OK;
}

static int bk_sdio_restore(uint64_t sleep_time, void *args)
{
	SDIO_HOST_RETURN_ON_NOT_INIT();
	if (s_sdio_host.pm_backup_is_valid)
	{
		bk_sdio_clock_en(true);
		sdio_hal_restore(&s_sdio_host.hal, &s_sdio_host.pm_backup[0]);
		s_sdio_host.pm_backup_is_valid = 0;
	}
	return BK_OK;
}
#else
#define SDIO_PM_CHECK_RESTORE(id)
#endif

bk_err_t bk_sdio_host_driver_init(void)
{
	bk_err_t ret = BK_OK;
	if (s_sdio_host_driver_is_init) {
		SDIO_HOST_LOGD("bk_sdio_host_driver_init has inited\r\n");
		return BK_OK;
	}

	os_memset(&s_sdio_host, 0, sizeof(s_sdio_host));
#if (CONFIG_SDIO_PM_CB_SUPPORT)
	bk_pm_module_vote_power_ctrl(PM_POWER_SUB_MODULE_NAME_BAKP_SDIO, PM_POWER_MODULE_STATE_ON);
#endif
	sdio_host_hal_init(&s_sdio_host.hal);
#if (CONFIG_SDIO_PM_CB_SUPPORT)
	int id = 0;
	pm_cb_conf_t enter_config = {bk_sdio_backup, (void *)id};
	pm_cb_conf_t exit_config = {bk_sdio_restore, (void *)id};
	bk_pm_module_vote_power_ctrl(PM_POWER_SUB_MODULE_NAME_BAKP_SDIO, PM_POWER_MODULE_STATE_ON);
	bk_pm_sleep_register_cb(PM_MODE_LOW_VOLTAGE, PM_DEV_ID_SDIO, &enter_config, &exit_config);
#endif

    intc_service_register(IRQ_SDIO, PRI_IRQ_SDIO, sdio_host_isr); /* bk_int_isr_register(INT_SRC_SDIO, sdio_host_isr, NULL);*/

	if (s_sdio_host.irq_cmd_msg == NULL) {
		ret = rtos_init_queue(
								&s_sdio_host.irq_cmd_msg,
								"sdio_host_queue",
								4,
								16
							);
		BK_ASSERT(kNoErr == ret); /* ASSERT VERIFIED */
	}
	if (s_sdio_host.tx_sema == NULL) {
		int ret = rtos_init_semaphore(&(s_sdio_host.tx_sema), 1);
		BK_ASSERT(kNoErr == ret); /* ASSERT VERIFIED */
	}
	s_sdio_host.is_tx_blocked = false;

	if (s_sdio_host.rx_sema == NULL) {
		int ret = rtos_init_semaphore(&(s_sdio_host.rx_sema), 1);
		BK_ASSERT(kNoErr == ret); /* ASSERT VERIFIED */
	}
	s_sdio_host_driver_is_init = true;
	return BK_OK;
}

bk_err_t bk_sdio_host_driver_deinit(void)
{
	bk_err_t ret = BK_OK;
	if (!s_sdio_host_driver_is_init) {
		return BK_OK;
	}
	sdio_host_deinit_common();
#if (CONFIG_SDIO_PM_CB_SUPPORT)
	bk_pm_module_vote_power_ctrl(PM_POWER_SUB_MODULE_NAME_BAKP_SDIO, PM_POWER_MODULE_STATE_OFF);
#endif
	intc_service_unregister(IRQ_SDIO);/* bk_int_isr_unregister(INT_SRC_SDIO);*/
#if (CONFIG_SDIO_PM_CB_SUPPORT)
	bk_pm_sleep_unregister_cb(PM_MODE_LOW_VOLTAGE, PM_DEV_ID_SDIO, true, true);
	bk_pm_module_vote_power_ctrl(PM_POWER_SUB_MODULE_NAME_BAKP_SDIO, PM_POWER_MODULE_STATE_OFF);
#endif

	if (s_sdio_host.irq_cmd_msg) {
		ret = rtos_deinit_queue(&s_sdio_host.irq_cmd_msg);
		BK_ASSERT(kNoErr == ret); /* ASSERT VERIFIED */

		s_sdio_host.irq_cmd_msg = NULL;
	}

	if (s_sdio_host.tx_sema) {
		int ret = rtos_deinit_semaphore(&(s_sdio_host.tx_sema));
		BK_ASSERT(kNoErr == ret); /* ASSERT VERIFIED */

		s_sdio_host.tx_sema = NULL;
	}
	if (s_sdio_host.rx_sema) {
		int ret = rtos_deinit_semaphore(&(s_sdio_host.rx_sema));
		BK_ASSERT(kNoErr == ret); /* ASSERT VERIFIED */

		s_sdio_host.rx_sema = NULL;
	}

	s_sdio_host_driver_is_init = false;
	return BK_OK;
}

bk_err_t bk_sdio_host_init(const sdio_host_config_t *config)
{
	BK_RETURN_ON_NULL(config);
	SDIO_HOST_RETURN_ON_NOT_INIT();
#if (CONFIG_SDIO_PM_CB_SUPPORT)
	uint32_t id = 0;
	pm_cb_conf_t enter_config = {bk_sdio_backup, (void *)id};
	bk_pm_module_lv_sleep_state_clear(PM_DEV_ID_SDIO);
	bk_pm_module_vote_power_ctrl(PM_POWER_SUB_MODULE_NAME_BAKP_SDIO, PM_POWER_MODULE_STATE_ON);
	bk_pm_sleep_register_cb(PM_MODE_LOW_VOLTAGE, PM_DEV_ID_SDIO, &enter_config, NULL);
#endif

#if (CONFIG_SDIO_V2P0)
	/* reset sdio host register */
	sdio_host_hal_reset_config_to_default(&s_sdio_host.hal);

	sdio_host_init_common();
#else
	sdio_host_init_common();

	/* reset sdio host register */
	sdio_host_hal_reset_config_to_default(&s_sdio_host.hal);
#endif

#if (CONFIG_SDIO_V2P0)
	bk_sdio_host_set_clock_freq(config->clock_freq); // wangzhilei
#else
	/* set sdio host clock frequence */
	sdio_host_hal_set_clk_freq(&s_sdio_host.hal, config->clock_freq);
#endif

#if CONFIG_SDIO_GDMA_EN
	//SDIO DMA TX init
	s_sdio_host.dma_tx_en = config->dma_tx_en;
	if(s_sdio_host.dma_tx_en)
	{
		if(sdio_dma_tx_init() != BK_OK)
		{
			SDIO_HOST_LOGI("sdio tx dma enable failed\r\n");
			s_sdio_host.dma_tx_en = 0;
		}

		s_sdio_host.tx_start_addr = 0;
		s_sdio_host.tx_total_len = 0;
		s_sdio_host.tx_transfered_len = 0;
	}

	//SDIO DMA RX init
	s_sdio_host.dma_rx_en = config->dma_rx_en;
	if(s_sdio_host.dma_rx_en)
	{
		if(sdio_dma_rx_init() != BK_OK)
		{
			SDIO_HOST_LOGI("sdio rx dma enable failed\r\n");
			s_sdio_host.dma_rx_en = 0;
		}

		s_sdio_host.rx_start_addr = 0;
		s_sdio_host.rx_total_len = 0;
		s_sdio_host.rx_transfered_len = 0;
	}
#endif

	/* set sdio host bus width */
	sdio_host_hal_set_bus_width(&s_sdio_host.hal, config->bus_width);

	return BK_OK;
}

bk_err_t bk_sdio_host_deinit(void)
{
	SDIO_HOST_RETURN_ON_NOT_INIT();

	sdio_host_deinit_common();

#if CONFIG_SDIO_GDMA_EN
	if(s_sdio_host.dma_tx_en)
	{
		//bk_dma_free(DMA_DEV_SDIO, s_sdio_host.dma_tx_id);
		s_sdio_host.dma_tx_id = DMA_ID_MAX;
		s_sdio_host.dma_tx_en = 0;
	}

	if(s_sdio_host.dma_rx_en)
	{
		//bk_dma_free(DMA_DEV_SDIO_RX, s_sdio_host.dma_rx_id);
		s_sdio_host.dma_rx_id = DMA_ID_MAX;
		s_sdio_host.dma_rx_en = 0;
	}
#endif

#if (CONFIG_SDIO_PM_CB_SUPPORT)
	bk_pm_sleep_unregister_cb(PM_MODE_LOW_VOLTAGE, PM_DEV_ID_SDIO, true, true);
	bk_pm_module_vote_power_ctrl(PM_POWER_SUB_MODULE_NAME_BAKP_SDIO, PM_POWER_MODULE_STATE_OFF);
#endif

	return BK_OK;
}

#if CONFIG_SDIO_V2P0
/*
 * WARNING:
 * Reset the sdio states:TX FIFO, RX FIFO, SDIO state.
 * TX FIFO/RX FIFO effects the clock gate function.
 * Just be called in SDK,APP don't use it.
 * If error called this API maybe cause error clock output to sdcard,
 * because sdcard read/write data and command response bases on clock from BK7256.
 */
void bk_sdio_host_reset_sd_state(void)
{
	SDIO_PM_CHECK_RESTORE(0);
	sdio_host_hal_reset_sd_state(&s_sdio_host.hal);
}

/*
 * WARNING: CLOCK(Enable/Disbale/Gate) API are only called in SDK,not for APP.
 * BK7256 Clock scheme:
 * 1.bk_sdio_clock_en: sdio asic module clock enable
 * 2.bk_sdio_clk_gate_config:
 *   a. enable means the sdio clock always on, not gated;
 *   b. disable means the clock will be controlled by sdio fifo status(read/write).
 * 3.bk_sdio_tx_fifo_clk_gate_config
 *   a. at WRITE status(bk7256 write data to sdio fifo),if the fifo is empty,the clock will be gated
 *      clock doesn't output to sdcard
 * 4.If at READ status, the clock gate will be controlled by sdio asic, SW can't control it.
 *   a. FIFO full or read data finish,the clock will be gated, else clock output to sdcard
 */
void bk_sdio_clock_en(uint32_t enable)
{
	uint32_t cmd;
	uint32_t param = PWD_SDIO_CLK;

	SDIO_PM_CHECK_RESTORE(0);

	/* sys_drv_dev_clk_pwr_up(CLK_PWR_ID_SDIO, enable);*/
	if(enable){
		cmd = CMD_CLK_PWR_UP;
	}else{
		cmd = CMD_CLK_PWR_DOWN;
	}

	icu_ctrl(cmd, &param);
}

/*
 * WARNING: CLOCK(Enable/Disbale/Gate) API are only called in SDK,not for APP.
 * BK7256 Clock scheme:
 * 1.bk_sdio_clock_en: sdio asic module clock enable
 * 2.bk_sdio_clk_gate_config:
 *   a. enable means the sdio clock always on, not gated;
 *   b. disable means the clock will be controlled by sdio fifo status(read/write).
 * 3.bk_sdio_tx_fifo_clk_gate_config
 *   a. at WRITE status(bk7256 write data to sdio fifo),if the fifo is empty,the clock will be gated
 *      clock doesn't output to sdcard
 * 4.If at READ status, the clock gate will be controlled by sdio asic, SW can't control it.
 *   a. FIFO full or read data finish,the clock will be gated, else clock output to sdcard
 */
void bk_sdio_clk_gate_config(uint32_t enable)
{
	SDIO_PM_CHECK_RESTORE(0);
	sdio_host_hal_set_clock_gate(&s_sdio_host.hal, enable);
}

/*
 * WARNING: CLOCK(Enable/Disbale/Gate) API are only called in SDK,not for APP.
 * BK7256 Clock scheme:
 * 1.bk_sdio_clock_en: sdio asic module clock enable
 * 2.bk_sdio_clk_gate_config:
 *   a. enable means the sdio clock always on, not gated;
 *   b. disable means the clock will be controlled by sdio fifo status(read/write).
 * 3.bk_sdio_tx_fifo_clk_gate_config
 *   a. at WRITE status(bk7256 write data to sdio fifo),if the fifo is empty,the clock will be gated
 *      clock doesn't output to sdcard
 * 4.If at READ status, the clock gate will be controlled by sdio asic, SW can't control it.
 *   a. FIFO full or read data finish,the clock will be gated, else clock output to sdcard
 */
void bk_sdio_tx_fifo_clk_gate_config(uint32_t enable)
{
	SDIO_PM_CHECK_RESTORE(0);
	sdio_hal_host_set_tx_fifo_need_write_mask_cg(&s_sdio_host.hal, enable);
}

bk_err_t bk_sdio_host_set_clock_freq(sdio_host_clock_freq_t clock_freq)
{
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);
	sdio_host_hal_set_clk_freq(&s_sdio_host.hal, clock_freq);

	switch (clock_freq) {
	case SDIO_HOST_CLK_XTAL_1DIV:
		sdio_host_set_clock_freq(SDIO_CLK_MUX_XTAL, ICU_SDIO_CLK_DIV_1);
		break;
	case SDIO_HOST_CLK_XTAL_2DIV:
		sdio_host_set_clock_freq(SDIO_CLK_MUX_XTAL, ICU_SDIO_CLK_DIV_2);
		break;
	case SDIO_HOST_CLK_XTAL_4DIV:
		sdio_host_set_clock_freq(SDIO_CLK_MUX_XTAL, ICU_SDIO_CLK_DIV_4);
		break;
	case SDIO_HOST_CLK_XTAL_128DIV:
		sdio_host_set_clock_freq(SDIO_CLK_MUX_XTAL, ICU_SDIO_CLK_DIV_128);
		break;
	case SDIO_HOST_CLK_DCO_1DIV:
		sdio_host_set_clock_freq(SDIO_CLK_MUX_DCO, ICU_SDIO_CLK_DIV_1);
		break;
	case SDIO_HOST_CLK_DCO_2DIV:
		sdio_host_set_clock_freq(SDIO_CLK_MUX_DCO, ICU_SDIO_CLK_DIV_2);
		break;
	case SDIO_HOST_CLK_DCO_4DIV:
		sdio_host_set_clock_freq(SDIO_CLK_MUX_DCO, ICU_SDIO_CLK_DIV_4);
		break;
	case SDIO_HOST_CLK_DCO_128DIV:
		sdio_host_set_clock_freq(SDIO_CLK_MUX_DCO, ICU_SDIO_CLK_DIV_128);
		break;
	default:
		break;
	}

	return BK_OK;
}

#else
bk_err_t bk_sdio_host_set_clock_freq(sdio_host_clock_freq_t clock_freq)
{
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	sdio_host_hal_set_clk_freq(&s_sdio_host.hal, clock_freq);

	return BK_OK;
}
#endif

static void sdio_dump_cmd_info(const sdio_host_cmd_cfg_t *command)
{
#if 0
	SDIO_HOST_LOGI("cmd_index=%d,argument=0x%08x,response=0x%08x\r\n", command->cmd_index, command->argument, command->response );
	SDIO_HOST_LOGI("wait_rsp_timeout=%d, crc_check=%d\r\n", command->wait_rsp_timeout, command->crc_check);
#endif
}

static uint32_t s_sdio_cmd_index = 0;
bk_err_t bk_sdio_host_send_command(const sdio_host_cmd_cfg_t *command)
{
	BK_RETURN_ON_NULL(command);
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	sdio_dump_cmd_info(command);
	s_sdio_cmd_index = command->cmd_index;

	//default:need crc check, if no need crc check, please set it in command.
	//sdio_host_hal_set_cmd_crc_check(&s_sdio_host.hal, true);
	BK_RETURN_ON_ERR(sdio_host_hal_init_commad(&s_sdio_host.hal, command));
	sdio_host_hal_start_send_command(&s_sdio_host.hal);
	return BK_OK;
}

bk_err_t bk_sdio_host_wait_cmd_response(uint32_t cmd_index)
{
#if CONFIG_SDIO_V2P0
	bk_err_t ret = BK_OK;
	uint32_t msg = 0;
#endif

	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	uint32_t int_status = sdio_host_hal_get_interrupt_status(&s_sdio_host.hal);

#if CONFIG_SDIO_V2P0
	{
		//TODO:Timeout and CRC fail should use different case
		//FreeRTOS doesn't use EVENT,so use msg queue.
		ret = rtos_pop_from_queue(&s_sdio_host.irq_cmd_msg, &msg, SDIO_CMD_WAIT_TIME);
		if(ret)
		{
			SDIO_HOST_LOGD("sdio wait slave CMD%d timeout, int_status=0x%x, ret=%d\r\n", cmd_index, int_status, ret);
			return BK_ERR_SDIO_HOST_CMD_RSP_TIMEOUT;
		}
	}
#else
	/* wait until cmd response */
	BK_WHILE (!sdio_host_hal_is_cmd_rsp_interrupt_triggered(&s_sdio_host.hal, int_status)) {
		int_status = sdio_host_hal_get_interrupt_status(&s_sdio_host.hal);
	}

	/* clear command response int status */
	sdio_host_hal_clear_cmd_rsp_interrupt_status(&s_sdio_host.hal, int_status);

	if (sdio_host_hal_is_cmd_rsp_timeout_interrupt_triggered(&s_sdio_host.hal, int_status)) {
		if (cmd_index != SEND_OP_COND) {
			SDIO_HOST_LOGW("sdio wait slave CMD%d timeout\r\n", cmd_index);
			return BK_ERR_SDIO_HOST_CMD_RSP_TIMEOUT;
		}
	}

	if (sdio_host_hal_is_cmd_rsp_crc_fail_interrupt_triggered(&s_sdio_host.hal, int_status)) {
		if ((cmd_index != SD_APP_OP_COND) &&
			(cmd_index != ALL_SEND_CID) &&
			(cmd_index != SEND_CSD) &&
			(cmd_index != SEND_OP_COND)) {
			SDIO_HOST_LOGW("sdio receive slave CMD%d crc fail\r\n", cmd_index);
			return BK_ERR_SDIO_HOST_CMD_RSP_CRC_FAIL;
		}
	}
#endif
	(void)int_status;

	return BK_OK;
}

uint32_t bk_sdio_host_get_cmd_rsp_argument(sdio_host_response_t argument_index)
{
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	return sdio_host_hal_get_cmd_rsp_argument(&s_sdio_host.hal, argument_index);
}

#if (CONFIG_SDIO_V2P0)
bk_err_t bk_sdio_host_config_data(const sdio_host_data_config_t *data_config)
{
	BK_RETURN_ON_NULL(data_config);
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	if (data_config->data_dir == SDIO_HOST_DATA_DIR_RD) {
		/* 1) set data timer
		 * 2) clear int status(REG9)
		 * 3) set rx fifo threshold
		 * 4) reset read fifo , reset sdcard command and data state
		 * 5) set data register
 		 */
		//TODO:Interrupt status should only be cleared in ISR
		//sdio_host_hal_clear_interrupt_status(&s_sdio_host.hal);
		sdio_host_hal_set_rx_fifo_threshold(&s_sdio_host.hal, 0x1);

		sdio_host_hal_set_read_multi_block_data(&s_sdio_host.hal, data_config->data_block_size);
		sdio_host_hal_enable_rx_end_mask(&s_sdio_host.hal);
		//TODO:disable it or set a big value of RX threshhold?
		//sdio_host_hal_enable_rx_need_read_mask(&s_sdio_host.hal);
		sdio_host_hal_start_receive_data(&s_sdio_host.hal);

		sdio_host_hal_set_data_timeout(&s_sdio_host.hal, data_config->data_timeout);
	} else {
		//TODO:Interrupt status should only be cleared in ISR
 		//sdio_host_hal_clear_interrupt_status(&s_sdio_host.hal);
		sdio_host_hal_set_tx_fifo_threshold(&s_sdio_host.hal, 0x1);
		sdio_host_hal_set_write_multi_block_data(&s_sdio_host.hal, data_config->data_block_size);
	}
	return BK_OK;
}
#else
bk_err_t bk_sdio_host_config_data(const sdio_host_data_config_t *data_config)
{
	BK_RETURN_ON_NULL(data_config);
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	if (data_config->data_dir == SDIO_HOST_DATA_DIR_RD) {
		/* 1) set data timer
		 * 2) clear int status(REG9)
		 * 3) set rx fifo threshold
		 * 4) reset read fifo , reset sdcard command and data state
		 * 5) set data register
 		 */
 		sdio_host_hal_clear_interrupt_status(&s_sdio_host.hal);
		sdio_host_hal_set_rx_fifo_threshold(&s_sdio_host.hal, 0x1);
		sdio_host_hal_set_read_data(&s_sdio_host.hal, data_config->data_block_size);
		sdio_host_hal_start_receive_data(&s_sdio_host.hal);
	} else {
 		sdio_host_hal_clear_interrupt_status(&s_sdio_host.hal);
		sdio_host_hal_set_tx_fifo_threshold(&s_sdio_host.hal, 0x1);
		sdio_host_hal_set_write_data(&s_sdio_host.hal, data_config->data_block_size);
		//sdio_host_hal_start_send_data(&s_sdio_host.hal);
	}
	sdio_host_hal_set_data_timeout(&s_sdio_host.hal, data_config->data_timeout);
	return BK_OK;
}

#endif
bk_err_t bk_sdio_host_set_data_timeout(uint32_t timeout)
{
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	sdio_host_hal_set_data_timeout(&s_sdio_host.hal, timeout);
	return BK_OK;
}

#if CONFIG_SDIO_V2P0
#if CONFIG_SDIO_GDMA_EN
static bk_err_t sdio_dma_write_fifo(const uint8_t *write_data, uint32_t data_size)
{
	bk_err_t error_state = BK_FAIL;

	rtos_get_semaphore(&s_sdio_host.tx_sema, 0);	//assume last time tx_sema finish.
	s_sdio_host.tx_total_len = data_size;
	s_sdio_host.tx_transfered_len = 0;
	s_sdio_host.tx_start_addr = (uint32_t)write_data;

	s_dma_transfer_size_check = 0;

	//dma start
	sdio_dma_tx_start(s_sdio_host.dma_tx_id);

	//sdio start
	bk_sdio_tx_fifo_clk_gate_config(1);
	sdio_host_hal_start_send_data(&s_sdio_host.hal);

	//wait SDIO last one or two block finish(DMA copy data to SDIO FIFO,the last one or two blocks are still ongoing on SDIO wires.)
	error_state = rtos_get_semaphore(&(s_sdio_host.tx_sema), SDIO_MAX_TX_WAIT_TIME);
	if(error_state)
	{
		s_sdio_host.is_tx_blocked = false;
		SDIO_HOST_LOGE("TODO:sdio write data timeout,write_data=0x%x,data_size=%d\r\n", write_data, data_size);

		//special abnormal case:If the write_data pointer is error,which cause DMA can't copy data from the source, then it cause DMA err.
		if(s_sdio_host.tx_total_len != s_sdio_host.tx_transfered_len)
		{
			bk_dma_stop(s_sdio_host.dma_tx_id);
		}
	}

	//maybe DMA finish, but SDIO doesn't finish
	if(s_dma_transfer_size_check != s_sdio_host.tx_total_len)	//we can't confirm the write end hit counts not lost, because maybe CPU-ISR is disabled by some APP, or other case cause software lost write end int.
	{
		rtos_get_semaphore(&(s_sdio_host.tx_sema), 2);	//Default:after DMA finish ISR,there are 512+16 bytes still in the SDIO FIFO.so here wait more 2ms
		SDIO_HOST_LOGD("%s:s_dma_transfer_size_check=%d,total_len=%d\r\n", __func__, s_dma_transfer_size_check, s_sdio_host.tx_total_len);
	}

	return error_state;
}
#endif
static bk_err_t sdio_host_cpu_write_fifo(const uint8_t *write_data, uint32_t data_size)
{
	BK_RETURN_ON_NULL(write_data);
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	sdio_host_hal_t *hal = &s_sdio_host.hal;
	uint32_t index = 0;
#if (CONFIG_SDIO_DEBUG_SUPPORT || CONFIG_SOC_BK7256XX)
	uint32 data_tmp = 0;
#endif
	bk_err_t error_state = BK_OK;
	uint32_t i = 0;

	while ((index < data_size)) {
		//confirm can write data to fifo(fifo isn't at busy/full status)
		while((sdio_host_hal_is_tx_fifo_write_ready(hal)) == 0)
		{
			i++;
			if(i % 0x400000 == 0)
				SDIO_HOST_LOGE("FIFO can't write i=0x%08x", i);

			//avoid dead in while
			if(i == 0x1000000 * 8) {
				SDIO_HOST_LOGE("FIFO write fail,the write data is invalid");
				error_state = BK_ERR_SDIO_HOST_DATA_TIMEOUT;
				break;
			}
		}

//NOTES:SDIO V1P0:write data should be reverted sequence by software, and read data should revert by ASIC of "SD_BYTE_SEL"
//or the endian isn't match with windows system.
//From BK7258XX, the write data sequence is reverted by ASIC hardware,without any config of "SD_BYTE_SEL"
#if CONFIG_SOC_BK7256XX	//after BK7258 V5 chip, ASIC chip TX no needs software revert data.
		//switch byte sequence, as the SDIO transfer data with big-endian
		data_tmp = ((write_data[index] << 24) | (write_data[index + 1] << 16) | (write_data[index + 2] << 8) | write_data[index + 3]);
		sdio_host_hal_write_fifo(hal, data_tmp);
#else
#if (CONFIG_SDIO_DEBUG_SUPPORT)
		data_tmp = *(uint32_t *)&write_data[index];
#endif
		sdio_host_hal_write_fifo(hal, *(uint32_t *)&write_data[index]);
#endif

		index += 4;

#if (CONFIG_SDIO_DEBUG_SUPPORT)
		if(index < 16)
		{
			SDIO_HOST_LOGD("data_tmp=0x%08x", data_tmp);
		}
		if(index == 16)
		{
			SDIO_HOST_LOGD("0x%08x\r\n", data_tmp);
		}
#endif
		//first block finish, enable tx fifo clock gate and then start write data to sdio wires(sdcard)
		if((index == SDIO_BLOCK_SIZE))
		{
			bk_sdio_tx_fifo_clk_gate_config(1);
			sdio_host_hal_start_send_data(&s_sdio_host.hal);
		}

		//one block write to sdcard fifo finish
		if((index % SDIO_BLOCK_SIZE == 0) || (index == data_size))	//maybe isn't a full block(though sdcard always use full block)
		{
			/*
			 * wait write end int which means the data has sent to sdcard and get the sdcard's response.
			 * empty mask just mean host FIFO is empty but can't indicate that sdcard has dealed data finish.
			 * write status fail will not set semaphore yet, so it cause here timeout, here set it as tx fail.
			 */
			s_sdio_host.is_tx_blocked = true;
			error_state = rtos_get_semaphore(&(s_sdio_host.tx_sema), SDIO_MAX_TX_WAIT_TIME);
			if(error_state)
			{
				s_sdio_host.is_tx_blocked = false;
				SDIO_HOST_LOGE("TODO:sdio tx data fail:index=%d!\r\n", index);
			}
		}
	}

	return error_state;
}

bk_err_t bk_sdio_host_write_fifo(const uint8_t *write_data, uint32_t data_size)
{
#if CONFIG_SDIO_GDMA_EN
	if(s_sdio_host.dma_tx_en)
		return sdio_dma_write_fifo(write_data, data_size);
	else
		return sdio_host_cpu_write_fifo(write_data, data_size);
#else
	return sdio_host_cpu_write_fifo(write_data, data_size);
#endif
}

/*
 * Internal API only for SD-CARD Driver.
 * SDIO has pre-read feature, it cause SD-CARD driver gets one more sema if the read block
 * isn't continious addr with previous block address.
 * So add one special API for SD-CARD to discard the previous rx data sema.
 */
void bk_sdio_host_discard_previous_receive_data_sema(void)
{
	SDIO_PM_CHECK_RESTORE(0);

	rtos_get_semaphore(&(s_sdio_host.rx_sema), 0);
}

bk_err_t bk_sdio_host_wait_receive_data(void)
{
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);
	bk_err_t error_state = BK_FAIL;

	//CRC err will not set semaphore yet, so it cause here timeout, here set it as rx fail.
	error_state = rtos_get_semaphore(&(s_sdio_host.rx_sema), SDIO_MAX_RX_WAIT_TIME);
	if(error_state != BK_OK)
	{
		SDIO_HOST_LOGI("rx fail\r\n");
	}

	return error_state;
}
#else
bk_err_t bk_sdio_host_write_fifo(const uint8_t *write_data, uint32_t data_size)
{
	BK_RETURN_ON_NULL(write_data);
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	sdio_host_hal_t *hal = &s_sdio_host.hal;
	uint32_t index = 0;
	uint32 data_tmp = 0;
	uint32_t int_level = 0;
	bk_err_t error_state = BK_OK;

	while ((sdio_host_hal_is_tx_fifo_write_ready(hal)) && (index < data_size)) {
		data_tmp = ((write_data[index] << 24) | (write_data[index + 1] << 16) | (write_data[index + 2] << 8) | write_data[index + 3]);
		index += 4;
		sdio_host_hal_write_fifo(hal, data_tmp);
	}

	int_level = rtos_disable_int();
	sdio_host_hal_enable_tx_fifo_empty_mask(hal);
	sdio_host_hal_start_send_data(&s_sdio_host.hal);
	s_sdio_host.is_tx_blocked = true;
	rtos_enable_int(int_level);

	rtos_get_semaphore(&(s_sdio_host.tx_sema), BEKEN_NEVER_TIMEOUT);

	int_level = rtos_disable_int();
	sdio_host_hal_disable_tx_fifo_empty_mask(hal);
	if (sdio_host_hal_is_data_crc_fail_int_triggered(hal, s_sdio_host.int_status)) {
		error_state = BK_ERR_SDIO_HOST_DATA_CRC_FAIL;
	}
	if (sdio_host_hal_is_data_timeout_int_triggered(hal, s_sdio_host.int_status)) {
		error_state = BK_ERR_SDIO_HOST_DATA_TIMEOUT;
	}
	rtos_enable_int(int_level);

	return error_state;
}

bk_err_t bk_sdio_host_wait_receive_data(void)
{
	SDIO_HOST_RETURN_ON_NOT_INIT();
	SDIO_PM_CHECK_RESTORE(0);

	bk_err_t error_state = BK_FAIL;
	uint32_t int_status = 0;
	sdio_host_hal_t *hal = &s_sdio_host.hal;

	/* wait for recv data interrupt triggered */
	do {
		int_status = sdio_host_hal_get_interrupt_status(hal);
		sdio_host_hal_clear_data_interrupt_status(hal, int_status);
		SDIO_HOST_LOGI("int_status:%x\r\n", int_status);
	} while (!sdio_host_hal_is_recv_data_interrupt_triggered(hal, int_status));

	if (sdio_host_hal_is_data_recv_end_int_triggered(hal, int_status)) {
		if (sdio_host_hal_is_data_crc_fail_int_triggered(hal, int_status)) {
			error_state = BK_ERR_SDIO_HOST_DATA_CRC_FAIL;
		} else {
			error_state = BK_OK;
		}
	} else {
		if (sdio_host_hal_is_data_timeout_int_triggered(hal, int_status)) {
			error_state = BK_ERR_SDIO_HOST_DATA_TIMEOUT;
		}
	}

	return error_state;
}
#endif

void bk_sdio_host_start_read(void)
{
	SDIO_PM_CHECK_RESTORE(0);

	sdio_host_hal_start_receive_data(&s_sdio_host.hal);
}

#if CONFIG_SDIO_GDMA_EN
static bk_err_t sdio_dma_read_fifo(uint8_t *read_data, uint32_t data_size)
{
	bk_err_t error_state = BK_FAIL;

	s_sdio_host.rx_total_len = data_size;
	s_sdio_host.rx_transfered_len = 0;
	s_sdio_host.rx_start_addr = (uint32_t)read_data;

	sdio_dma_rx_start((dma_id_t)s_sdio_host.dma_rx_id);

	//wait finish
	error_state = rtos_get_semaphore(&(s_sdio_host.rx_sema), SDIO_MAX_RX_WAIT_TIME * 10);
	if(error_state)
	{
		SDIO_HOST_LOGE("read data fail,request cnt=%d\r\n", data_size);
	}

	return error_state;
}
#endif

bk_err_t bk_sdio_host_read_fifo(uint32_t *save_v_p)
{
#define SDIO_READ_FIFO_TIMEOUT (400)	//us, one word transfer in 13M clock nearly 10us with single wire
	uint32_t i = 0;
	SDIO_PM_CHECK_RESTORE(0);

	while (!sdio_host_hal_is_rx_fifo_read_ready(&s_sdio_host.hal))
	{
		i++;
		delay_us(1);
		if(i == SDIO_READ_FIFO_TIMEOUT) {
			SDIO_HOST_LOGE("FIFO read fail,the return data is invalid\r\n");
			return BK_ERR_SDIO_HOST_READ_DATA_FAIL;
		}
	}

	*save_v_p = sdio_host_hal_read_fifo(&s_sdio_host.hal);

	return BK_OK;
}

static bk_err_t sdio_host_cpu_read_blks_fifo(uint8_t *data, uint32_t blk_cnt)
{
	bk_err_t error_state = BK_OK;
	uint32_t read_data = 0, index = 0;
	uint32_t data_size = blk_cnt * 512;

	/* Read data from SDIO to buffer*/
	while (bk_sdio_host_wait_receive_data() == BK_OK) {
		do {
			/* Read data from SDIO Rx fifo */
			error_state = bk_sdio_host_read_fifo(&read_data);
			if(error_state == BK_OK)
			{
				data[index++] = read_data & 0xff;
				data[index++] = (read_data >> 8) & 0xff;
				data[index++] = (read_data >> 16) & 0xff;
				data[index++] = (read_data >> 24) & 0xff;
				//SDIO_HOST_LOGD("read_data:%x, index:%d\r\n", read_data, index);
			}
			else
			{
				SDIO_HOST_LOGW("Read FIFO fail,index=%d\r\n", index);
				break;
			}
		} while ((index % SDIO_BLOCK_SIZE) != 0);

		if (index >= data_size) {
			SDIO_HOST_LOGD("rx data finish bytes:%d\r\n", index);
			break;
		}

//maybe some files in SDCARD and read it takes too much time, then cause WDT timeout
#if (CONFIG_TASK_WDT)
		bk_task_wdt_feed();
#endif
	}

	//CRC fail or some of block timeout and so on
	if (index != data_size)
	{
		error_state = BK_ERR_SDIO_HOST_DATA_CRC_FAIL;
		SDIO_HOST_LOGE("read data fail,rx real cnt=%d,request cnt=%d\r\n", index, data_size);
	}

	return BK_OK;
}

bk_err_t bk_sdio_host_read_blks_fifo(uint8_t *read_data, uint32_t blk_cnt)
{
#if CONFIG_SDIO_GDMA_EN
	if(s_sdio_host.dma_rx_en)
	{
		uint32_t data_size = blk_cnt * 512;
		return sdio_dma_read_fifo(read_data, data_size);
	}
	else
		return sdio_host_cpu_read_blks_fifo(read_data, blk_cnt);
#else
	return sdio_host_cpu_read_blks_fifo(read_data, blk_cnt);
#endif
}

#if (CONFIG_SDIO_V2P0)
static void sdio_host_isr(void)
{
	sdio_host_hal_t *hal = &s_sdio_host.hal;
	uint32_t int_status = sdio_host_hal_get_interrupt_status(&s_sdio_host.hal);
	uint32_t cmd_index = sdio_host_hal_get_cmd_index_interrupt_status(&s_sdio_host.hal, int_status);
	s_sdio_host.int_status = int_status;

	//TODO:WARNING:sdio_host_hal_is_data_crc_fail_int_triggered should check
	SDIO_HOST_LOGD("sdio isr, cmd_index=%d,int_status:%x\r\n", cmd_index, int_status);

	//CMD:RESP, NO RESP, TIMEOUT
	if(sdio_host_hal_is_cmd_rsp_interrupt_triggered(hal, int_status))
	{
		//CMD END(Some CMD has no response, so has not CRC)
		if(sdio_host_hal_is_cmd_end_interrupt_triggered(hal, int_status))
		{
			uint32_t cmd_rsp_ok = true;
			if(sdio_host_hal_is_cmd_rsp_crc_ok_interrupt_triggered(hal, int_status))	//CRC OK
			{
			}
			else if(sdio_host_hal_is_cmd_rsp_crc_fail_interrupt_triggered(hal, int_status))	//CRC Fail
			{
				//SDIO Host driver no need to care about SDCARD CMD Index,SDCARD driver needs to set
				//the speific CMD Index whether needs CRC check.
				 {
					SDIO_HOST_LOGW("sdio receive CMD%d crc fail\r\n", cmd_index);
					cmd_rsp_ok = false;
				}
			}
			else	//some CMD has no response,so has no CRC check.
			{
			}

			if (/*s_sdio_host.is_cmd_blocked &&*/ cmd_rsp_ok) {
				if(rtos_push_to_queue(&s_sdio_host.irq_cmd_msg, &int_status, 0)){
					SDIO_HOST_LOGE("sdio push cmd msg fail\r\n");
				}
			}
		}
		else if(sdio_host_hal_is_cmd_rsp_timeout_interrupt_triggered(hal, int_status))	//timeout
		{
			if ((cmd_index != SEND_OP_COND) && (s_sdio_cmd_index != 6)) {
				SDIO_HOST_LOGD("isr sdio wait CMD RSP timeout, int_status=0x%x,cmd_index=%d,s_sdio_cmd_index=%d\r\n", int_status,cmd_index,s_sdio_cmd_index);
			}
		}

		//clear cmd int status
		sdio_host_hal_clear_cmd_rsp_interrupt_status(hal, int_status);
	}
	//TX(host write) DATA
	else if(sdio_host_hal_is_data_write_end_int_triggered(hal, int_status))
	{
		//TODO:check write status
		uint32_t tmp = sdio_host_hal_get_wr_status(&s_sdio_host.hal);
		if ( tmp != 2) {
			SDIO_HOST_LOGE("TODO:write data error!!!\r\n");
		}
		else
		{
			SDIO_HOST_LOGD("write blk end\r\n");
#if CONFIG_SDIO_GDMA_EN
			if(s_sdio_host.dma_tx_en)	//WARNING:DMA enable: SDIO,DMA,Software at a-sync status,maybe software loses end isr.
			{
				s_dma_transfer_size_check += 512;	//can't use this value to check whether tx finish:because if CPU-INT is disable or CPU response too slow, then it lost ISR handler.
				if(s_sdio_host.tx_total_len <= s_sdio_host.tx_transfered_len)	//after DMA finish and then set sema to avoid too much sema clear.
				{
					rtos_set_semaphore(&s_sdio_host.tx_sema);
				}
			}
			else
				rtos_set_semaphore(&s_sdio_host.tx_sema);
#else
			rtos_set_semaphore(&s_sdio_host.tx_sema);
#endif
		}
#if 0	//(CONFIG_SDIO_HOST_CLR_WRITE_INT)
		bk_sdio_tx_fifo_clk_gate_config(1);
		sdio_host_hal_clear_write_data_interrupt_status(hal, int_status);
		sdio_host_hal_clear_write_data_interrupt_status(hal, int_status);
		sdio_host_hal_clear_write_data_interrupt_status(hal, int_status);
		bk_sdio_tx_fifo_clk_gate_config(0);//clr 3 times to wait sd_card a cycle;
#else
		sdio_host_hal_clear_write_data_interrupt_status(hal, int_status);
#endif
	}
	//RX(host read) DATA
	else if(sdio_host_hal_is_data_recv_end_int_triggered(hal, int_status) ||
		/* sdio_host_hal_is_data_recv_need_read_int_triggered(hal, int_status) ||
		sdio_host_hal_is_data_recv_overflow_int_triggered(hal, int_status) || */
		sdio_host_hal_is_data_timeout_int_triggered(hal, int_status))
	{
		if(sdio_host_hal_is_data_crc_ok_int_triggered(hal, int_status)) //CRC OK
		{
#if CONFIG_SDIO_GDMA_EN
			if(s_sdio_host.dma_rx_en)
				SDIO_HOST_LOGD("DMA read blk end\r\n");	//do nothing
			else
				rtos_set_semaphore(&s_sdio_host.rx_sema);
#else
			rtos_set_semaphore(&s_sdio_host.rx_sema);
#endif
			sdio_host_hal_clear_read_data_interrupt_status(hal, int_status);
		}
		else if(sdio_host_hal_is_data_crc_fail_int_triggered(hal, int_status))	//CRC Fail
		{
			//multi-block read:the sdio has pre-read feature(after read finish, the sdio still reads one block data),
			//so it always read more data then SW requires,
			//but the data is invalid,so reset the sd state if CRC fail.

			bk_sdio_host_reset_sd_state();

			//TODO:If the data is really CRC fail, should notify APP the data received is error.
			SDIO_HOST_LOGE("TODO:read data crc error!!!\r\n");
#if 0	//just not set sema cause rx data timeout, which cause rx fail.
			rtos_set_semaphore(&s_sdio_host.rx_sema);
#endif
			sdio_host_hal_clear_read_data_interrupt_status(hal, int_status);
		}
		else if(sdio_host_hal_is_data_timeout_int_triggered(hal, int_status))	//timeout
		{
			SDIO_HOST_LOGE("TODO:read data timeout error!!!\r\n");
			sdio_host_hal_clear_read_data_timeout_interrupt_status(hal, int_status);
		}
		/*
		 * !!!NOTES!!! SDIO pre-read feature time-sequence issue!!!
		 * If read data finish,SW will clear FIFO(bk_sdio_host_reset_sd_state) when stop read,
		 * but maybe the ASIC have pre-read one block data finished but SDIO ISR doesn't response.
		 * After FIFO reset, ISR response found the CRC isn't OK and isn't ERROR,
		 * which cause ISR status isn't cleared, so the SDIO ISR will entry and exit forever
		 * cause system abnormall(watchdog timeout reset)
		 */
		else
		{
			SDIO_HOST_LOGE("read data status err:int_status=0x%08x!!!\r\n", int_status);
			sdio_host_hal_clear_read_data_interrupt_status(hal, int_status);
		}
	}
	else {
		SDIO_HOST_LOGD("sdio isr no deal:cmd_index=%d,int_status=%x\r\n", cmd_index, int_status);
	}
}

#else
static void sdio_host_isr(void)
{
	sdio_host_hal_t *hal = &s_sdio_host.hal;
	uint32_t int_status = sdio_host_hal_get_interrupt_status(hal);
	s_sdio_host.int_status = int_status;

	SDIO_HOST_LOGD("enter sdio_host isr, int_status:%x\r\n", int_status);
	sdio_host_hal_clear_data_interrupt_status(hal, int_status);

	if (sdio_host_hal_is_data_write_end_int_triggered(hal, int_status) ||
		sdio_host_hal_is_fifo_empty_int_triggered(hal, int_status) ||
		sdio_host_hal_is_data_crc_fail_int_triggered(hal, int_status) ||
		sdio_host_hal_is_data_timeout_int_triggered(hal, int_status)) {
		/* need clear tx fifo empty mask, otherwise sdio_isr will triggered forever */
		sdio_host_hal_disable_tx_fifo_empty_mask(hal);
		if (s_sdio_host.is_tx_blocked) {
			rtos_set_semaphore(&s_sdio_host.tx_sema);
			s_sdio_host.is_tx_blocked = false;
		}
	}
}
#endif
