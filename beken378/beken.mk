NAME := beken

$(NAME)_TYPE := kernel

-include $(SOURCE_ROOT)/platform/mcu/$(HOST_MCU_FAMILY)/.config

WPA_VERSION := wpa_supplicant_2_9

$(NAME)_INCLUDES := app/standalone-ap \
					app/standalone-station \
					app/video_work \
					app/net_work   \
					app/http       \
                                        app            \
					driver/sdio    \
					driver/uart    \
					driver/sys_ctrl\
					driver/gpio    \
					driver/common/reg \
					driver/dma \
					driver/intc \
					driver/phy \
					driver/rc_beken \
					driver/general_dma \
					driver/spidma \
					driver/rw_pub \
					driver/icu \
					driver/i2c \
					driver/jpeg \
					driver/calendar \
					driver/pwm \
					func/sdio_intf \
					func/power_save \
					func/temp_detect \
					func/saradc_intf \
					func/spidma_intf \
					func/ethernet_intf \
					func/rwnx_intf \
					func/rf_test \
					func/rf_use \
					func/misc \
					func/camera_intf \
					func/video_transfer \
					func/user_driver \
					func/force_sleep \
					func/ble_wifi_exchange \
					driver/ble/ble_5_1/platform/7231n/entry

$(NAME)_INCLUDES += ip/ke \
					ip/mac \
					ip/lmac/src/hal \
					ip/lmac/src/mm \
					ip/lmac/src/ps \
					ip/lmac/src/rd \
					ip/lmac/src/rwnx \
					ip/lmac/src/rx \
					ip/lmac/src/scan \
					ip/lmac/src/sta \
					ip/lmac/src/tx \
					ip/lmac/src/vif \
					ip/lmac/src/rx/rxl \
					ip/lmac/src/tx/txl \
					ip/lmac/src/tpc \
					ip/lmac/src/tdls \
					ip/lmac/src/p2p \
					ip/lmac/src/chan \
					ip/lmac/src/td \
					ip/umac/src/bam \
					ip/umac/src/llc \
					ip/umac/src/me \
					ip/umac/src/rxu \
					ip/umac/src/scanu \
					ip/umac/src/sm \
					ip/umac/src/txu \
					ip/umac/src/apm \
					ip/umac/src/mesh \
					ip/umac/src/rc

$(NAME)_SOURCES :=  app/app.c \
					app/config/param_config.c \
					app/standalone-ap/sa_ap.c \
					app/standalone-station/sa_station.c \
					app/video_work/video_transfer_tcp.c \
					app/video_work/video_transfer_udp.c \
					app/video_work/video_buffer.c \
					app/net_work/video_demo_main.c \
					app/net_work/video_demo_station.c \
					app/net_work/video_demo_softap.c \
					app/http/utils_httpc.c \
					app/http/utils_net.c \
					app/http/utils_timer.c \
					app/http/lite-log.c \
					driver/common/dd.c \
					driver/common/drv_model.c \
					driver/dma/dma.c \
					driver/driver.c \
					driver/flash/flash.c \
					driver/general_dma/general_dma.c \
					driver/gpio/gpio.c \
					driver/i2c/i2c1.c \
					driver/i2c/i2c2.c \
					driver/jpeg/jpeg_encoder.c \
					driver/icu/icu.c \
					driver/irda/irda.c \
					driver/macphy_bypass/mac_phy_bypass.c \
					driver/phy/phy_trident.c \
					driver/pwm/pwm.c \
					driver/pwm/pwm_bk7231n.c \
					driver/pwm/bk_timer.c \
					driver/pwm/bk_timer_extense.c \
					driver/pwm/mcu_ps_timer.c \
					driver/saradc/saradc.c \
					driver/sdio/sdio.c \
					driver/sdio/sdma.c \
					driver/sdio/sutil.c \
					driver/spidma/spidma.c \
					driver/sys_ctrl/sys_ctrl.c \
					driver/uart/Retarget.c \
					driver/uart/uart.c \
					driver/wdt/wdt.c \
					driver/rw_pub/rw_platf_pub.c \
					driver/intc/intc.c \
					driver/calendar/calendar.c \
					func/func.c \
					func/hostapd_intf/hostapd_intf.c \
					func/misc/fake_clock.c \
					func/misc/target_util.c \
					func/misc/start_type.c \
					func/rwnx_intf/rw_ieee80211.c \
					func/rwnx_intf/rw_msdu.c \
					func/rwnx_intf/rw_tx_buffering.c \
					func/rwnx_intf/rw_msg_rx.c \
					func/rwnx_intf/rw_msg_tx.c \
					func/sdio_intf/sdio_intf.c \
					func/sdio_trans/sdio_trans.c \
					func/sim_uart/gpio_uart.c \
					func/sim_uart/pwm_uart.c \
					func/spidma_intf/spidma_intf.c \
					func/temp_detect/temp_detect.c \
					func/saradc_intf/saradc_intf.c \
					func/power_save/power_save.c \
					func/power_save/mcu_ps.c \
					func/power_save/manual_ps.c \
                                        func/power_save/low_voltage_ps.c \
                                        func/power_save/low_voltage_compensation.c \
					func/power_save/ap_idle.c \
					func/wlan_ui/wlan_ui.c \
					func/net_param_intf/net_param.c \
					func/lwip_intf/dhcpd/dhcp-server-main.c \
					func/lwip_intf/dhcpd/dhcp-server.c \
					func/camera_intf/camera_intf.c \
					func/camera_intf/camera_intf_gc2145.c \
					func/video_transfer/video_transfer.c \
					func/force_sleep/force_mac_ps.c \
					func/force_sleep/force_mcu_ps.c \
					func/ble_wifi_exchange/ble_wifi_port.c \
					func/bk7011_cal/bk_cal_pub.c

ifeq ($(CFG_SOC_NAME), 2)
$(NAME)_SOURCES +=  driver/spi/spi.c \
					driver/spi/spi_master.c \
					driver/spi/spi_slave.c
endif
ifeq ($(CFG_SOC_NAME), 5)
$(NAME)_SOURCES +=  driver/spi/spi_bk7231n.c \
					driver/spi/spi_master_bk7231n.c \
					driver/spi/spi_slave_bk7231n.c
endif
ifeq ($(CFG_SOC_NAME), 8)
$(NAME)_SOURCES +=  driver/rtc/rtc_reg.c \
					driver/irda/irda_bk7252n.c \
					driver/i2s/i2s_bk7252n.c \
					driver/sdcard/sdcard.c \
					driver/sdcard/sdio_driver \
					func/sd_music/sdcard_test.c \
					driver/charge/charge.c \
					driver/hpm/hpm.c \
					driver/audio/audio.c \
					driver/la/la.c \
					driver/general_dma/general_dma_bk7252n.c \
					driver/qspi/qspi_bk7252n.c \
					driver/jpeg/jpeg.c \
					driver/yuv_buf/yuv_buf.c \
					driver/i2c/i2c1_bk7252n.c \
					driver/ipchksum/ipchksum.c
$(NAME)_INCLUDES += driver/rtc \
					driver/sdcard \
					func/sd_music \
					driver/audio \
					driver/yuv_buf \
					driver/ipchksum
endif

ifeq ($(CFG_WRAP_LIBC), 1)
$(NAME)_INCLUDES += func/libc
$(NAME)_SOURCES +=  func/libc/errno/lib_errno.c \
					func/libc/math/lib_exp.c \
					func/libc/math/lib_libexpi.c \
					func/libc/math/lib_log.c \
					func/libc/math/lib_pow.c \
					func/libc/stdio/lib_libvscanf.c \
					func/libc/stdio/lib_memsistream.c \
					func/libc/stdio/lib_meminstream.c \
					func/libc/stdio/lib_sscanf.c \
					func/libc/stdio/lib_vsscanf.c \
					func/libc/stdlib/lib_checkbase.c \
					func/libc/stdlib/lib_strtod.c \
					func/libc/stdlib/lib_qsort.c \
					func/libc/stdlib/lib_srand.c \
					func/libc/stdlib/lib_strtol.c \
					func/libc/stdlib/lib_strtoll.c \
					func/libc/stdlib/lib_strtoul.c \
					func/libc/stdlib/lib_strtoull.c \
					func/libc/string/lib_isbasedigit.c
endif


$(NAME)_INCLUDES += func/$(WPA_VERSION)/src \
					func/$(WPA_VERSION)/src/ap \
					func/$(WPA_VERSION)/src/utils \
					func/$(WPA_VERSION)/src/common \
					func/$(WPA_VERSION)/src/drivers \
					func/$(WPA_VERSION)/bk_patch \
					func/$(WPA_VERSION)/hostapd \
					func/$(WPA_VERSION)/wpa_supplicant

$(NAME)_SOURCES +=                      alios/lwip-2.0.2/port/ethernetif.c \
					alios/lwip-2.0.2/port/net.c \
					alios/lwip-2.0.2/apps/ping/ping.c \
					alios/lwip-2.0.2/apps/iperf/iperf.c \
					alios/os/mem_arch.c \
					alios/os/str_arch.c \
					alios/flash_hal.c \
					alios/entry/arch_main.c
