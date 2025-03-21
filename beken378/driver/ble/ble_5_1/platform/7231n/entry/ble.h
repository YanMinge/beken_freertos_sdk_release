
#ifndef __BLE_H_
#define __BLE_H_

#define BLE_CONN_IDX_MAX             (0x01)
#define BLE_CHAR_DATA_LEN            (128)

#define BLE_MSG_QUEUE_COUNT          (20)
#define BLE_STACK_SIZE               (2048)

// ble dut uart port
#define PORT_UART1                   (1)
#define PORT_UART2                   (2)
#define BLE_DUT_UART_PORT            PORT_UART2  // PORT_UART2

enum
{
	BLE_MSG_POLL = 0,
	BLE_MSG_DUT,
	BLE_DUT_START,
	BLE_DUT_EXIT,
	BLE_MSG_SLEEP,
	BLE_MSG_NULL,
	BLE_THREAD_EXIT,
};

enum system_run_mode {
	NORMAL_MODE = 0,
	DUT_FCC_MODE = (0x01 << 0),
};

typedef struct ble_message {
    uint32_t data;
} BLE_MSG_T;

uint8_t ble_get_sys_mode(void);
void ble_send_msg(UINT32 data);
UINT32 ble_ctrl( UINT32 cmd, void *param );
void ble_switch_rf_to_wifi(void);
void ble_set_ext_wkup(uint8_t enable);
bool ble_thread_is_up(void);
void ble_entry(void);
#if (CFG_SUPPORT_MATTER == 0)
// should not define in .h, if matter is open
void ble_thread_exit(void);
#endif
void ble_coex_set_pta(bool enable);
bool ble_coex_pta_is_on(void);
#endif

