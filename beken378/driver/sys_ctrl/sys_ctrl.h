#ifndef _SCTRL_H_
#define _SCTRL_H_

#define SCTRL_DEBUG    0

#if SCTRL_DEBUG
#define SCTRL_PRT      os_printf
#else
#define SCTRL_PRT      os_null_printf
#endif

#define DPLL_DELAY_EN                         1
#define DPLL_DELAY_TIME_10US	              120
#define DPLL_DELAY_TIME_200US	              3400

#define SCTRL_BASE                            (0x00800000)

#define SCTRL_CHIP_ID                         (SCTRL_BASE + 00 * 4)
#define CHIP_ID_DEFAULT                          (0x7111)
#define CHIP_ID_BK7231N                          (0x7231C)
#define CHIP_ID_BK7251                           (0x7221a)
#define CHIP_ID_BK7231U                          (0x7231a)
#define CHIP_ID_BK7238                           (0x7238)
#define CHIP_ID_BK7252N                          (0x7252a)

#define SCTRL_DEVICE_ID                       (SCTRL_BASE + 01 * 4)
#define DEVICE_ID_MASK                           (0xFFFF0000)

//Device ID of BK7231N
#define DEVICE_ID_DEFAULT                        (0x20150414) //UMC_TW
#define DEVICE_ID_BK7231N_A                      (0x20521023) //UMC_TW
#define DEVICE_ID_BK7231N_B                      (0x20521024) //UMC_TW
#define DEVICE_ID_BK7231N_C                      (0x20521025) //UMC_TW
#define DEVICE_ID_BK7231N_D                      (0x20521026) //UMC_TW
#define DEVICE_ID_BK7231N_E                      (0x20521027) //UMC_TW
#define DEVICE_ID_BK7231N_F                      (0x20521028) //UMC_TW
#define DEVICE_ID_BK7231N_G                      (0x20521029) //UMC_TW
#define DEVICE_ID_BK7231N_N                      (0x20521010) //UMC_XM
#define DEVICE_ID_BK7231N_O                      (0x20521011) //UMC_XM

#define DEVICE_ID_BK7231N_P                      (0x20A21020) //7231P_A, SMIC
#define DEVICE_ID_BK7231N_P_B                    (0x20A21021) //7231P_B, SMIC

//Device ID of BK7251
#define DEVICE_ID_BK7251_A                       (0x18221020) //UMC_TW

//Device ID of BK7231U
#define DEVICE_ID_BK7231U_A                      (0x18521020) //UMC_TW

//Device ID of BK7238
#define DEVICE_ID_BK7238_MPW                     (0x21128000)

//Device ID of BK7238
#define DEVICE_ID_BK7238_A                       (0x22068000)

//Device ID of BK7252N
#define DEVICE_ID_BK7252N_MPW                    (0x23A18000)
#define DEVICE_ID_BK7252N_MP                     (0x24318000)

#define SCTRL_CONTROL                         (SCTRL_BASE + 02 * 4)
#if (CFG_SOC_NAME == SOC_BK7221U)
#define EFUSE_VDD25_EN                           (1 << 23)
#define FLASH_SPI_MUX_BIT                        (1 << 22)
#define PSRAM_VDDPAD_VOLT_POSI                   (20)
#define PSRAM_VDDPAD_VOLT_MASK                   (0x3)
#define FLASH_SCK_IOCAP_POSI                     (18)
#define FLASH_SCK_IOCAP_MASK                     (0x3)
#define QSPI_IO_VOLT_POSI                        (16)
#define QSPI_IO_VOLT_MASK                        (0x3)
#define BLE_RF_EN_BIT                            (1 << 15)
#elif (CFG_SOC_NAME == SOC_BK7231U)
#define EFUSE_VDD25_EN                           (1 << 23)
#define PSRAM_VDDPAD_VOLT_POSI                   (20)
#define PSRAM_VDDPAD_VOLT_MASK                   (0x3)
#define FLASH_SCK_IOCAP_POSI                     (18)
#define FLASH_SCK_IOCAP_MASK                     (0x3)
#define QSPI_IO_VOLT_POSI                        (16)
#define QSPI_IO_VOLT_MASK                        (0x3)
#define BLE_RF_EN_BIT                            (1 << 15)
#elif (CFG_SOC_NAME == SOC_BK7231N)
#define BLE_RF_PTA_EN_BIT                        (1 << 24)
#define EFUSE_VDD25_EN                           (1 << 23)
#define FLASH_SPI_MUX_BIT                        (1 << 22)
#define FLASH_SCK_IOCAP_POSI                     (18)
#define FLASH_SCK_IOCAP_MASK                     (0x3)
#define BLE_RF_EN_BIT                            (1 << 15)
#elif (CFG_SOC_NAME == SOC_BK7238)
#define BLE_RF_PTA_EN_BIT                        (1 << 24)
#define EFUSE_VDD25_EN                           (1 << 23)
#define FLASH_SPI_MUX_BIT                        (1 << 22)
#define PWD_ARMCLK_IN_WFI_BIT                    (1 << 21)
#define PWD_AHBCLK_IN_SLEEP_BIT                  (1 << 20)
#define FLASH_SCK_IOCAP_POSI                     (18)
#define FLASH_SCK_IOCAP_MASK                     (0x3)
#define BLE_RF_EN_BIT                            (1 << 15)
#elif (CFG_SOC_NAME == SOC_BK7252N)
#define FORCE_RFADC_TO_ZERO                      (1 << 26)
#define PWD_FLASH_CLK_IN_WIFI_BIT                (1 << 25)
#define BLE_RF_PTA_EN_BIT                        (1 << 24)
#define EFUSE_VDD25_EN                           (1 << 23)
#define FLASH_SPI_MUX_BIT                        (1 << 22)
#define PWD_ARMCLK_IN_WFI_BIT                    (1 << 21)
#define PWD_AHBCLK_IN_SLEEP_BIT                  (1 << 20)
#define FLASH_SCK_IOCAP_POSI                     (18)
#define FLASH_SCK_IOCAP_MASK                     (0x3)
#define BLE_RF_EN_BIT                            (1 << 15)
#endif
#define DPLL_CLKDIV_RESET_BIT                    (1 << 14)
#define SDIO_CLK_INVERT_BIT                      (1 << 13)
#define MPIF_CLK_INVERT_BIT                      (1 << 12)
#define MAC_CLK480M_PWD_BIT                      (1 << 11)
#define MODEM_CLK480M_PWD_BIT                    (1 << 10)
#define HCLK_DIV2_EN_BIT                         (1 << 9)
#define FLASH_26M_MUX_BIT                        (1 << 8)

#define MCLK_DIV_MASK                            (0x0F)
#define MCLK_DIV_POSI                            (4)
#define MCLK_DIV_0                               (0)
#define MCLK_DIV_1                               (1)
#define MCLK_DIV_2                               (2)
#define MCLK_DIV_3                               (3)
#define MCLK_DIV_5                               (5)
#define MCLK_DIV_6                               (6)
#define MCLK_DIV_7                               (7)
#define MCLK_DIV_10                              (10)
#define MCLK_DIV_16                              (16)

#define MCLK_MUX_MASK                            (0x03)
#define MCLK_MUX_POSI                            (0)
#define MCLK_FIELD_DCO                           (0x0)
#define MCLK_FIELD_26M_XTAL                      (0x1)
#define MCLK_FIELD_DPLL                          (0x2)
#define MCLK_FIELD_LPO                           (0x3)

#define SCTRL_CLK_GATING                      (SCTRL_BASE + 3 * 4)
#define MAC_PI_CLKGATING_BIT                     (1 << 8)
#define MAC_PI_RX_CLKGATING_BIT                  (1 << 7)
#define MAC_PI_TX_CLKGATING_BIT                  (1 << 6)
#define MAC_PRI_CLKGATING_BIT                    (1 << 5)
#define MAC_CRYPT_CLKGATING_BIT                  (1 << 4)
#define MAC_CORE_TX_CLKGATING_BIT                (1 << 3)
#define MAC_CORE_RX_CLKGATING_BIT                (1 << 2)
#define MAC_WT_CLKGATING_BIT                     (1 << 1)
#define MAC_MPIF_CLKGATING_BIT                   (1 << 0)

#define SCTRL_RESET                           (SCTRL_BASE + 4 * 4)
#define SCTRL_RESET_MASK                         (0x7F)
#if (CFG_SOC_NAME != SOC_BK7231)
#define BLE_SUBSYS_RESET                         (1 << 8)
#endif // (CFG_SOC_NAME != SOC_BK7231)
#define MAC_WAKEUP_ARM                           (1 << 7)
#define MODEM_CORE_RESET_BIT                     (1 << 6)
#if (CFG_SOC_NAME == SOC_BK7231N) || (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define BOOT_MODE_BIT                            (1 << 9)
#define MAC_SUBSYS_RESET_BIT                     (1 << 2)
#else
#define TL410_EXT_WAIT_BIT                       (1 << 5)
#define USB_SUBSYS_RESET_BIT                     (1 << 4)
#define TL410_BOOT_BIT                           (1 << 3)
#define MAC_SUBSYS_RESET_BIT                     (1 << 2)
#define DSP_SUBSYS_RESET_BIT                     (1 << 1)
#endif
#define MODEM_SUBCHIP_RESET_BIT                  (1 << 0)

#if (CFG_SOC_NAME == SOC_BK7252N)
#define SCTRL_MODULE_POWE                     (SCTRL_BASE + 6 * 4)
#define MEM_RETENT_EN_POSI                       (16)
#define MEM_RETENT_EN_MASK                       (0x7F)
#define POWER_DOWN_POSI                          (0)
#define POWER_DOWN_MASK                          (0x1FF)
#define PERI_POSI                                (8)
#define DTCM_POSI                                (7)
#define SMEM1_POSI                               (6)
#define SMEM0_POSI                               (5)
#define FUNC_POSI                                (4)
#define WIFI_OFDM_POSI                           (3)
#define BLE_POSI                                 (2)
#define WIFI_PHY_POSI                            (1)
#define WIFI_MAC_POSI                            (0)
#define POWER_CTRL                               (0x1)
#endif

#define SCTRL_FLASH_DELAY                     (SCTRL_BASE + 7 * 4)
#define FLASH_DELAY_CYCLE_POSI                   (0)
#define FLASH_DELAY_CYCLE_MASK                   (0xFFF)

#define SCTRL_MODEM_SUBCHIP_RESET_REQ         (SCTRL_BASE + 8 * 4)
#define MODEM_SUBCHIP_RESET_WORD                 (0x7111e802)

#define SCTRL_MAC_SUBSYS_RESET_REQ            (SCTRL_BASE + 9 * 4)
#define MAC_SUBSYS_RESET_WORD                    (0x7111C802)

#define SCTRL_USB_SUBSYS_RESET_REQ            (SCTRL_BASE + 10 * 4)
#define USB_SUBSYS_RESET_WORD                    (0x7111C12B)

#define SCTRL_DSP_SUBSYS_RESET_REQ            (SCTRL_BASE + 11 * 4)
#define DSP_SUBSYS_RESET_WORD                    (0x7111C410)

#define SCTRL_MODEM_CORE_RESET_PHY_HCLK       (SCTRL_BASE + 12 * 4)
#define MODEM_CORE_RESET_POSI                    (16)
#define MODEM_CORE_RESET_MASK                    (0xFFFF)
#define MODEM_CORE_RESET_WORD                    (0xE802U)
#define MAC_HCLK_EN_BIT                          (1 << 1)
#define PHY_HCLK_EN_BIT                          (1 << 0)

#define SCTRL_DEBUG13                         (SCTRL_BASE + 13 * 4)
#define DBGMACCLKSEL_POSI                        (20)
#define DBGMACCLKSEL_MASK                        (0x1)
#define SMPSOURCE_POSI                           (16)
#define SMPSOURCE_MASK                           (0xF)
#define DBGMDMBANK1MUX_POSI                      (8)
#define DBGMDMBANK1MUX_MASK                      (0x1F)
#define DBGMDMBANK0MUX_POSI                      (02)
#define DBGMDMBANK0MUX_MASK                      (0x1F)

#define SCTRL_DEBUG14_MODEM_BANK              (SCTRL_BASE + 14 * 4)
#define SCTRL_DEBUG15_MAC_BANK                (SCTRL_BASE + 15 * 4)


#define SCTRL_ANALOG_SPI                      (SCTRL_BASE + 16 * 4)
#define ANA_SPI_FREQ_DIV_POSI                    (26)
#define ANA_SPI_FREQ_DIV_MASK                    (0x3F)
#if (CFG_SOC_NAME == SOC_BK7231)
#define ANA_SPI_STATE_POSI                       (0)
#define ANA_SPI_STAET_MASK                       (0x1F)
#elif (CFG_SOC_NAME == SOC_BK7231U) || (SOC_BK7231N == CFG_SOC_NAME) || (CFG_SOC_NAME == SOC_BK7238)
#define ANA_SPI_STATE_POSI                       (0)
#define ANA_SPI_STAET_MASK                       (0x7F)
#elif (CFG_SOC_NAME == SOC_BK7221U)
#define ANA_SPI_STATE_POSI                       (0)
#define ANA_SPI_STAET_MASK                       (0x7FF)
#elif (CFG_SOC_NAME == SOC_BK7252N)
#define ANA_SPI_STATE_POSI                       (0)
#define ANA_SPI_STAET_MASK                       (0xFFFF)
#endif

#if (CFG_SOC_NAME != SOC_BK7231N) && (CFG_SOC_NAME != SOC_BK7238) && (CFG_SOC_NAME != SOC_BK7252N)
#define SCTRL_LA_SAMPLE                       (SCTRL_BASE + 18 * 4)
#define LA_SMP_LEN_POSI                          (16)
#define LA_SMP_LEN_MASK                          (0xFFFF)
#define LA_SMP_FINISH_BIT                        (1 << 3)
#define LA_SMP_CLK_INVERT                        (1 << 2)
#define LA_SMP_MODE_POSI                         (0)
#define LA_SMP_MODE_MASK                         (0x3)

#define SCTRL_LA_SAMPLE_VALUE                 (SCTRL_BASE + 19 * 4)
#define SCTRL_LA_SAMPLE_MASK                  (SCTRL_BASE + 20 * 4)
#define SCTRL_LA_SAMPLE_DMA_START_ADDR        (SCTRL_BASE + 21 * 4)
#endif

#define SCTRL_ANALOG_CTRL0                    (SCTRL_BASE + 22 * 4)
#if (CFG_SOC_NAME == SOC_BK7231N) || (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define LPEN_DPLL                                (1 << 27)
#endif
#define SPI_TRIG_BIT                             (1 << 19)
#define SPI_DET_EN                               (1 << 4)

#define SCTRL_ANALOG_CTRL1                    (SCTRL_BASE + 23 * 4)
#if (CFG_SOC_NAME == SOC_BK7231N)
#define ROSC_TSTEN_BIT                           (1 << 31)
#define DCO_TSTEN_BIT                            (1 << 30)
#define DCO_DIV_POSI                             (27)
#define DCO_DIV_MASK                             (0x7U)
#define DCO_AMSEL_BIT                            (1 << 26)
#define SPI_RST_BIT                              (1 << 25)
#define DCO_CNTI_POSI                            (16)
#define DCO_CNTI_MASK                            (0x1FFU)
#define DCO_TRIG_BIT                             (1 << 15)
#define DCO_ICTRL_POSI                           (11)
#define DCO_ICTRL_MASK                           (0x7U)
#define DCO_MSW_POSI                             (2)
#define DCO_MSW_MASK                             (0x1FFU)
#define DPLL_VREFSEL_BIT                         (1 << 1)
#elif(CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define ROSC_TSTEN_BIT                           (1 << 31)
#define DCO_TSTEN_BIT                            (1 << 30)
#define DCO_DIV_POSI                             (27)
#define DCO_DIV_MASK                             (0x7U)
#define DCO_AMSEL_BIT                            (1 << 26)
#define SPI_RST_BIT                              (1 << 25)
#define DCO_CNTI_POSI                            (16)
#define DCO_CNTI_MASK                            (0x1FFU)
#define DCO_TRIG_BIT                             (1 << 15)
#define DCO_ICTRL_POSI                           (11)
#define DCO_ICTRL_MASK                           (0x7U)
#define DCO_MSW_POSI                             (2)
#define DCO_MSW_MASK                             (0x1FFU)
#define ROSC_MODIFY_AUTO                         (1 << 1)
#define ROSC_CALIB_AUTO                          (1 << 0)
#else
#define DIV_BYPASS_BIT                           (1 << 31)
#define SPI_RST_BIT                              (1 << 25)
#define DCO_CNTI_POSI                            (16)
#define DCO_CNTI_MASK                            (0x1FFU)
#define DCO_TRIG_BIT                             (1 << 15)
#define DCO_DIV_POSI                             (12)
#define DCO_DIV_MASK                             (0x3U)
#endif

#define SCTRL_ANALOG_CTRL2                    (SCTRL_BASE + 24 * 4)
#define CENTRAL_BAIS_ENABLE_BIT                  (1 << 13)
#if (CFG_SOC_NAME == SOC_BK7231N)
#define XTALH_CTUNE_POSI                         (2)
#define XTALH_CTUNE_MASK                         (0x7FU)
#define BANDGAP_CAL_MANUAL_POSI                  (23)
#define BANDGAP_CAL_MANUAL_MASK                  (0x3FU)
#elif (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define XTALH_CTUNE_POSI                         (1)
#define XTALH_CTUNE_MASK                         (0xFFU)
#define BANDGAP_CAL_MANUAL_POSI                  (23)
#define BANDGAP_CAL_MANUAL_MASK                  (0x3FU)
#define ROSC_CAL_MODE_BIT                        (1 << 14) //1:32K,0:31.25K
#define ROSC_SPI_TRIG_BIT                        (1 << 12)
#define VREF_SCALE_BIT                           (1 << 0)
#elif (CFG_SOC_NAME != SOC_BK7231)
#define XTALH_CTUNE_POSI                         (2)
#define XTALH_CTUNE_MASK                         (0x3FU)
#endif // (CFG_SOC_NAME == SOC_BK7231)
#define TRXT_TST_ENABLE_BIT                      (1 << 12)

#define SCTRL_ANALOG_CTRL3                    (SCTRL_BASE + 25 * 4)
#if (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define MANU_EN_32K                              (1 << 7)
#define ANALOG_BUF_SEL_RX_BIT                    (1 << 10)
#define AMPBIAS_OUTEN_BIT                        (1 << 11)
#elif (CFG_SOC_NAME != SOC_BK7231N)
#define CHARGE_LC2CVDLYLV_MASK                   (0x7)
#define CHARGE_LC2CVDLYLV_POS                    (29)
#define CHARGE_VLCSWLV_MASK                      (0xF)
#define CHARGE_VLCSWLV_POS                       (23)
#define CHARGE_LCP_MASK                          (0x1F)
#define CHARGE_LCP_POS                           (8)
#endif

#define SCTRL_ANALOG_CTRL4                    (SCTRL_BASE + 26 * 4)
#if (CFG_SOC_NAME == SOC_BK7221U)
#define VSEL_SYS_LDO_POSI                        (27)
#define VSEL_SYS_LDO_MASK                        (0x3)
#define CHARGE_MANMODE_MASK                      (0x7)
#define CHARGE_MANMODE_POS                       (16)
#elif (CFG_SOC_NAME == SOC_BK7231N)
#define VREF_SEL_BIT                             (1 << 30)
#define GADC_CAL_SEL_POSI                        (19)
#define GADC_CAL_SEL_MASK                        (0x3)
#elif (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define BGCAL_EN_BIT                             (1 << 31)
#define VREF_SEL_BIT                             (1 << 30)
#define GADC_CAL_SEL_POSI                        (24)
#define GADC_CAL_SEL_MASK                        (0xF)
#define ANALOG_BUF_SEL_TX_BIT                    (1 << 0)
#else
#endif

#define SCTRL_ANALOG_CTRL5                    (SCTRL_BASE + 0x1B*4)
#if (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define ROSC_TRIG_EN_BIT                         (1 << 29)
#define ROSC_CAL_XTAL_POSI                       (16)
#define ROSC_CAL_XTAL_MASK                       (0xF)
#define ROSC_CAL_MODIFY_INTVAL_POSI              (10)
#define ROSC_CAL_MODIFY_INTVAL_MASK              (0x3F)
#define ROSC_CAL_INTERVAL_POSI                   (0)
#define ROSC_CAL_INTERVAL_MASK                   (0x3FF)
#endif

#if (CFG_SOC_NAME == SOC_BK7252N)
#define SCTRL_ANALOG_CTRL6                    (SCTRL_BASE + 0x1C*4)
#define IOLDO_TRIM_POS                           (29)
#define IOLDO_TRIM_MASK                          (0x7)
#define IOLDO_CUR_LIM_ENABLE                     (28)
#define TRICK_CUR_SELECT                         (27)
#define VUSB_4VPOR_SELECT_POS                    (25)
#define VUSB_4VPOR_SELECT_MASK                   (0x3)
#define IOLDO_BYPASS_ENABLE                      (24)
#define CHG_CC_MANUAL_ENABLE                     (13)
#define CHG_CV_MANUAL_ENABLE                     (12)
#define CHG_VOUT_SELECT_POS                      (8)
#define CHG_VOUT_SELECT_MASK                     (0xF)
#define CHG_VOUT_COMPENSATION_POS                (5)
#define CHG_VOUT_COMPENSATION_MASK               (0x7)
#define CHG_CC_TOCV_DELAY_ENABLE                 (4)
#define CHG_CC_TO_CV_DELAY_TIME_POS              (1)
#define CHG_CC_TO_CV_DELAY_TIME_MASK             (0x7)
#define CHG_VUSB_DETECTOR_ENABLE                 (0)

#elif (CFG_SOC_NAME != SOC_BK7231)
#define SCTRL_ANALOG_CTRL6                    (SCTRL_BASE + 0x1C*4)
#define DPLL_CLK_FOR_AUDIO_EN                    (1 << 31)
#define DPLL_CLK_FOR_USB_EN                      (1 << 30)
#define DPLL_DIVIDER_CLK_SEL                     (1 << 29)
#define DPLL_DIVIDER_CTRL_MASK                   (0x7)
#define DPLL_DIVIDER_CTRL_POSI                   (26)
#define DPLL_CLKOUT_PAD_EN                       (1 << 25)
#define DPLL_XTAL26M_CLK_AUDIO_EN                (1 << 24)
#define DPLL_REF_CLK_SELECT                      (1 << 23)
#define DPLL_CHARGE_PUMP_CUR_CTRL_MASK           (0x7)
#define DPLL_CHARGE_PUMP_CUR_CTRL_POSI           (20)
#define DPLL_DPLL_VCO_BAND_MANUAL_EN             (1 << 19)
#define DPLL_SPI_TRIGGER                         (1 << 18)
#define DPLL_CLK_REF_LOOP_SEL                    (1 << 17)
#define DPLL_KVCO_CTRL_MASK                      (0x3)
#define DPLL_KVCO_CTRL_POSI                      (15)
#define DPLL_VSEL_CAL                            (1 << 14)
#define DPLL_RP_CTRL_LPF_MASK                    (0x7)
#define DPLL_RP_CTRL_LPF_POSI                    (11)
#define DPLL_RESET                               (1 << 3)
#endif

#if (CFG_SOC_NAME != SOC_BK7231)
#define SCTRL_EFUSE_CTRL                      (SCTRL_BASE + 0x1D*4)
#define EFUSE_OPER_EN                            (1 << 0)
#define EFUSE_OPER_DIR                           (1 << 1)
#define EFUSE_OPER_ADDR_POSI                     (8)
#define EFUSE_OPER_ADDR_MASK                     (0x1F)
#define EFUSE_OPER_WR_DATA_POSI                  (16)
#define EFUSE_OPER_WR_DATA_MASK                  (0xFF)

#define SCTRL_EFUSE_OPTR                      (SCTRL_BASE + 0x1E*4)
#define EFUSE_OPER_RD_DATA_POSI                  (0)
#define EFUSE_OPER_RD_DATA_MASK                  (0xFF)
#define EFUSE_OPER_RD_DATA_VALID                 (1 << 8)


#define SCTRL_DMA_PRIO_VAL                    (SCTRL_BASE + 0x1F*4)

#define SCTRL_BLE_SUBSYS_RESET_REQ            (SCTRL_BASE + 0x20*4)
#if (CFG_SOC_NAME == SOC_BK7221U)
#define SCTRL_CHARGE_STATUS                   (SCTRL_BASE + 0x21*4)
#define CHARGE_VCAL_MASK                         (0x3F)
#define CHARGE_VCAL_POS                          (5)
#define CHARGE_LCAL_MASK                         (0x1F)
#define CHARGE_LCAL_POS                          (11)
#define CHARGE_VCVCAL_MASK                       (0x1F)
#define CHARGE_VCVCAL_POS                        (0)

#define SCTRL_ANALOG_CTRL7                    (SCTRL_BASE + 0x22*4)

#define SCTRL_ANALOG_CTRL8                    (SCTRL_BASE + 0x23*4)
#define LINE_IN_EN                               (1 << 24)
#define LINE_IN_GAIN_MASK                        (0x3)
#define LINE_IN_GAIN_POSI                        (22)
#define SPI_PWD_AUD_ADC_L                        (1 << 21)
#define SPI_PWD_AUD_ADC_R                        (1 << 20)
#define AUD_DAC_GAIN_MASK                        (0x1F)
#define AUD_DAC_GAIN_POSI                        (2)
#define AUD_DAC_MUTE_EN                          (1 << 0)

#define SCTRL_ANALOG_CTRL9                    (SCTRL_BASE + 0x24*4)
#define DAC_DIFF_EN                              (1 << 31)
#define EN_AUD_DAC_L                             (1 << 30)
#define EN_AUD_DAC_R                             (1 << 29)
#define DAC_PA_OUTPUT_EN                         (1 << 24)
#define DAC_DRIVER_OUTPUT_EN                     (1 << 23)
#define AUD_DAC_DGA_EN                           (1 << 6)

#define SCTRL_ANALOG_CTRL10                   (SCTRL_BASE + 0x25*4)
#define DAC_N_END_OUPT_L                         (1 << 8)
#define DAC_N_END_OUPT_R                         (1 << 7)
#define DAC_VSEL_MASK                            (0x3)
#define DAC_VSEL_POSI                            (1)
#elif (CFG_SOC_NAME == SOC_BK7231N) || (CFG_SOC_NAME == SOC_BK7238)
#define SCTRL_ANALOG_STATE                    (SCTRL_BASE + 0x21*4)
#define ANALOG_STATE_MASK                        (0xFF)
#define ANALOG_STATE_POS                         (0)
#define DPLL_UNLOCK_STATE_BIT                    (1 << 15)
#elif (CFG_SOC_NAME == SOC_BK7252N)
#define SCTRL_ANALOG_STATE                    (SCTRL_BASE + 0x21*4)
#define USB_RD                                   (1 << 0)
#define DAC_CALL_POS                             (1)
#define DAC_CALL_MASK                            (0xFF)
#define IFADC_CAL_POS                            (9)
#define IFADC_CAL_MASK                           (0x3F)
#define DPLL_UNLOCK_STATE_BIT                    (1 << 15)
#define CHG_ICAL_POS                             (16)
#define CHG_ICAL_MASK                            (0x3F)
#define CHG_VCAL_POS                             (22)
#define CHG_VCAL_MASK                            (0xFF)
#define CHG_CCFAST_POS                           (30)
#define CHG_TERM_POS                             (31)

#define SCTRL_ANALOG_CTRL7                    (SCTRL_BASE + 0x22*4)
#define CHG_VCAL_MANUAL_CONTROL_POS            (0)
#define CHG_VCAL_MANUAL_CONTROL_MASK           (0xFF)
#define CHG_VCAL_TRIG                          (1 << 8)
#define CHG_VCAL_SELECT                        (1 << 9)
#define CHG_ICAL_MANUAL_CONTROL_POS            (10)
#define CHG_ICAL_MANUAL_CONTROL_MASK           (0x1F)
#define CHG_ICAL_SELECT                        (1 << 16)
#define CHG_ICAL_TRIG                          (1 << 17)
#define CHG_CURRENT_CONTROL_POS                (18)
#define CHG_CURRENT_CONTROL_MASK               (0x3F)
#define CHG_CV_END_VOLTAGE_CONTROL_POS         (25)
#define CHG_CV_END_VOLTAGE_CONTROL_MASK        (0x3)
#define CHG_LDO_ENABLE                         (1 << 27)
#define CHG_TERM_MANUAL_ENABLE                 (1 << 28)
#define CHG_TRICK_MANUAL_ENABLE                (1 << 29)
#define CHG_MODE_SELECT                        (1 << 30)
#define CHG_ENABLE                             (1 << 31)


#define SCTRL_ANALOG_CTRL8                    (SCTRL_BASE + 0x23*4)
#define AUD_BIAS_SELEC                        (1 << 0)
#define AUD_CLK_RELATCH_EN                    (1 << 1)
#define AUD_LATCH_CLK_INVERSION_EN            (1 << 2)
#define AUD_BIAS_EN                           (1 << 3)
#define AUD_ADC_BIAS_EN                       (1 << 4)
#define AUD_MIC_BIAS_EN                       (1 << 5)
#define AUD_ADC_CLK_INPUT_INVERSION_EN        (1 << 6)
#define AUD_ANA_REG8_NC_0_POS                 (7)
#define AUD_ANA_REG8_NC_0_MASK                (0x3)
#define AUD_MIC_BIAS_TRIM_POS                 (9)
#define AUD_MIC_BIAS_TRIM_MASK                (0x3)
#define AUD_MIC_BIAS_VOC_POS                  (11)
#define AUD_MIC_BIAS_VOC_MASK                 (0x1F)
#define AUD_MIC_BIAS_RSEL                     (1 << 16)
#define AUD_CAP_TRIM_VALUE_POS                (17)
#define AUD_CAP_TRIM_VALUE_MASK               (0x1F)
#define AUD_ADC_REF_SELECT_POS                (22)
#define AUD_ADC_REF_SELECT_MASK               (0x3)
#define AUD_LDO_SEL_POS                       (28)
#define AUD_LDO_SEL_MASK                      (0x3)
#define AUD_DAC_CLK_INVERSION_SEL             (1 << 31)

#define SCTRL_ANALOG_CTRL9                    (SCTRL_BASE + 0x24*4)
#define AUD_ADC_BIAS_TRIM_POS                 (0)
#define AUD_ADC_BIAS_TRIM_MASK                (0x3)
#define AUD_MIC_INPUT_RES_SEL                 (1 << 2)
#define AUD_MIC_DAC_LOUT_POS                  (3)
#define AUD_MIC_DAC_LOUT_MASK                 (0x3)
#define AUD_MIC_DAC_CTRL_POS                  (5)
#define AUD_MIC_DAC_CTRL_MASK                 (0xFF)
#define AUD_MIC_SINGLE_EN                     (1 << 13)
#define AUD_MIC_SINGLE_MODE                   (1 << 14)
#define AUD_MIC_GAIN_POS                      (15)
#define AUD_MIC_GAIN_MASK                     (0xF)
#define AUD_MIC_LINE_IN_EN                    (1 << 19)
#define AUD_MIC_SINGLE_OPEN_LOOP_CALI_EN      (1 << 21)
#define AUD_MIC_DCOC_N_EN                     (1 << 22)
#define AUD_MIC_DCOC_P_EN                     (1 << 23)
#define AUD_DWA_MODE_SEL                      (1 << 24)
#define AUD_R2R_EN                            (1 << 25)
#define AUD_MIC_DCOC_RANGE_CTRL_POS           (26)
#define AUD_MIC_DCOC_RANGE_CTRL_MASK          (0x3)
#define AUD_MIC_MODE_EN                       (1 << 28)
#define AUD_ADC_RESET_EN                      (1 << 29)
#define AUD_ADC_DWA_EN                        (1 << 30)
#define AUD_HCEN_1STG                         (1 << 31)

#define SCTRL_ANALOG_CTRL10                   (SCTRL_BASE + 0x25*4)
#define AUD_HP_DAC                            (1 << 0)
#define AUD_CALCON_SEL                        (1 << 1)
#define AUD_THD_CURRENT_POS                   (2)
#define AUD_THD_CURRENT_MASK                  (0x3)
#define AUD_OVER_CURRENT_PROTECT_EN           (1 << 4)
#define AUD_VCM_SEL                           (1 << 5)
#define AUD_DAC_REF_VOL_ADJUST_POS            (6)
#define AUD_DAC_REF_VOL_ADJUST_MASK           (0x1F)
#define AUD_DCOC_HIGH_GAIN_SEL_POS            (11)
#define AUD_DCOC_HIGH_GAIN_SEL_MASK           (0x3)
#define AUD_DAC_DIFF_EN                       (1 << 13)
#define AUD_DAC_CAL_EN                        (1 << 14)
#define AUD_L_DAC_DCOC_EN                     (1 << 16)
#define AUD_L_COMMON_MODE_EN                  (1 << 18)
#define AUD_DAC_OUTPUT_DRIVER_EN              (1 << 19)
#define AUD_L_DAC_EN                          (1 << 21)
#define AUD_DAC_GAIN_SET_POS                  (22)
#define AUD_DAC_GAIN_SET_MASK                 (0xF)
#define AUD_DAC_MUTE_EN                       (1 << 26)
#define AUD_DAC_DWA_SEL                       (1 << 27)
#define AUD_DAC_REF_SEL_POS                   (28)
#define AUD_DAC_REF_SEL_MASK                  (0xF)

#define SCTRL_ANALOG_CTRL11                   (SCTRL_BASE + 0x26*4)
#define AUD_LMDC_IN_POS                       (0)
#define AUD_LMDC_IN_MASK                      (0xFF)
#define AUD_DAC_OVC_CTRL_RST_EN               (1 << 16)
#define AUD_L_IDAC_EN                         (1 << 18)
#define AUD_DAC_3STG_TRIM                     (1 << 19)
#define AUD_DAC_CURRENT_CTRL                  (1 << 20)
#define AUD_DAC_FEEDBACK_CTRL                 (1 << 21)
#define AUD_COMBIND_VCM_SEL                   (1 << 22)
#define AUD_DAC_BIAS_EN                       (1 << 23)
#define AUD_CAL_CLK_SEL                       (1 << 24)
#define AUD_BYPASS_DAC_DWA                    (1 << 25)
#define AUD_DAC_LOOP_RST                      (1 << 26)
#define AUD_OVER_CURRENT_DELAY_TIME_POS       (27)
#define AUD_OVER_CURRENT_DELAY_TIME_MASK      (0x3)
#define AUD_SHORT_OUTPUT                      (1 << 29)
#define AUD_DAC_CURRENT_TRIM_POS              (30)
#define AUD_DAC_CURRENT_TRIM_MASK             (0x3)
#endif // (CFG_SOC_NAME == SOC_BK7221U)
#endif // (CFG_SOC_NAME != SOC_BK7221U)

#define SCTRL_LOW_PWR_CLK                     (SCTRL_BASE + 64 * 4)
#define LPO_CLK_MUX_POSI                         (0)
#define LPO_CLK_MUX_MASK                         (0x3)
#define LPO_SRC_ROSC                             (0x0)
#define LPO_SRC_32K_XTAL                         (0x1)
#define LPO_SRC_32K_DIV                          (0x2)

#define DEEP_SLEEP_LPO_SRC        LPO_SRC_32K_XTAL

#define SCTRL_SLEEP                           (SCTRL_BASE + 65 * 4)
#define PROCORE_DLY_POSI                         (20)
#define PROCORE_DLY_MASK                         (0xF)
#if (CFG_SOC_NAME == SOC_BK7231N)
#define BOOT_SKIP_ENABLE_BIT                     (1 << 24)
#define GPIO_SLEEP_SWITCH_BIT                    (1 << 19)
#define DCO_PWD_SLEEP_BIT                        (1 << 17)
#define FLASH_PWD_SLEEP_BIT                      (1 << 18)
#elif (CFG_SOC_NAME == SOC_BK7238)
#define MEM_DS_FOR_LVSLEEP_BIT                   (1 << 25)
#define BOOT_SKIP_BIT                            (1 << 24)
#define GPIO_SLEEP_SWITCH_BIT                    (1 << 19)
#define DCO_PWD_SLEEP_BIT                        (1 << 17)
#define FLASH_PWD_SLEEP_BIT                      (1 << 18)
#elif (CFG_SOC_NAME == SOC_BK7252N)
#define BLE_AON_ISOLATE_BIT                      (1 << 28)
#define MAC_AON_ISOLATE_BIT                      (1 << 27)
#define MEM_RET_FOR_LVSLEEP_BIT                  (1 << 26)
#define CPU_PWD_FOR_LVSLEEP_BIT                  (1 << 25)
#define BOOT_SKIP_BIT                            (1 << 24)
#define PORCORE_DLY_POSI                         (20)
#define PORCORE_DLY_MASK                         (0xF)
#define GPIO_ISOLATE_BIT                         (1 << 19)
#define DCO_PWD_SLEEP_BIT                        (1 << 17)
#define FLASH_PWD_SLEEP_BIT                      (1 << 18)
#else
#define DCO_PWD_SLEEP_BIT                        (1 << 18)
#define FLASH_PWD_SLEEP_BIT                      (1 << 17)
#endif
#define ROSC_PWD_DEEPSLEEP_BIT                   (1 << 16)
#define SLEEP_MODE_POSI                          (0)
#define SLEEP_MODE_MASK                          (0xFFFF)
#define SLEEP_MODE_CFG_NORMAL_VOL_WORD           (0x4F89)
#define SLEEP_MODE_CFG_LOW_VOL_WORD              (0xB706)
#define SLEEP_MODE_CFG_DEEP_WORD                 (0xADC1)

#define SCTRL_DIGTAL_VDD                      (SCTRL_BASE + 66 * 4)
#if (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define AON_VDD_ACTIVE_POSI                      (12)
#define AON_VDD_ACTIVE_MASK                      (0xF)
#define AON_VDD_SLEEP_POSI                       (8)
#define AON_VDD_SLEEP_MASK                       (0xF)
#define DIG_VDD_ACTIVE_POSI                      (4)
#define DIG_VDD_ACTIVE_MASK                      (0xF)
#define DIG_VDD_SLEEP_POSI                       (0)
#define DIG_VDD_SLEEP_MASK                       (0xF)
#else
#define DIG_VDD_ACTIVE_POSI                      (4)
#define DIG_VDD_ACTIVE_MASK                      (0x7)
#define DIG_VDD_SLEEP_POSI                       (0)
#define DIG_VDD_SLEEP_MASK                       (0x7)
#endif
#define VDD_1_DOT_51                             (0x7)
#define VDD_1_DOT_38                             (0x6)
#define VDD_1_DOT_25                             (0x5)
#define VDD_1_DOT_12                             (0x4)
#define VDD_0_DOT_99                             (0x3)
#define VDD_0_DOT_86                             (0x2)
#define VDD_0_DOT_73                             (0x1)
#define VDD_0_DOT_60                             (0x0)

#if !(CFG_SOC_NAME == SOC_BK7252N)
#define SCTRL_PWR_MAC_MODEM                   (SCTRL_BASE + 67 * 4)
#define MAC_PWD_POSI                             (16)
#define MAC_PWD_MASK                             (0xFFFFU)
#define MODEM_PWD_POSI                           (0)
#define MODEM_PWD_MASK                           (0xFFFFU)
#define MAC_PWD                                  (0xD802U)
#define MODEM_PWD                                (0xD802U)
#define MAC_PWU                                  (0x0001U)
#define MODEM_PWU                                (0x0001U)
#else
#define SCTRL_PWR_MAC_BLE                     (SCTRL_BASE + 67 * 4)
#define POWER_CTRL                               (0x1)
#define WIFI_MAC_ALWAYS_ON_POSI                  (0)
#define BLE_ALWAYS_ON_POSI                       (1)
#define PWD_ALWAYS_ON_MAGIC                      (0xAACC)
#endif

#if !(CFG_SOC_NAME == SOC_BK7252N)
#define SCTRL_DSP_PWR                         (SCTRL_BASE + 68 * 4)
#define DSP_PWD_POSI                             (0)
#define DSP_PWD_MASK                             (0xFFFF)
#define DSP_PWD                                  (0xD410)
#define DSP_PWU                                  (0x0001)
#else
#define SCTRL_SYS_WKUP                        (SCTRL_BASE + 68 * 4)
#define SYS_WKUP_SRC_POSI                        (8)
#define SYS_WKUP_SRC_MASK                        (0x1F)
#define SYS_WKUP_SRC_RTC_BIT                     (1 << (SYS_WKUP_SRC_POSI + 0))
#define SYS_WKUP_SRC_GPIO_BIT                    (1 << (SYS_WKUP_SRC_POSI + 1))
#define SYS_WKUP_SRC_WIFI_BIT                    (1 << (SYS_WKUP_SRC_POSI + 2))
#define SYS_WKUP_SRC_BLE_BIT                     (1 << (SYS_WKUP_SRC_POSI + 3))
#define SYS_WKUP_SRC_CHARGE_BIT                  (1 << (SYS_WKUP_SRC_POSI + 4))
#define SYS_WKUP_EN_POSI                         (0)
#define SYS_WKUP_EN_MASK                         (0x1F)
#define SYS_WKUP_EN_RTC_BIT                      (1 << (SYS_WKUP_EN_POSI + 0))
#define SYS_WKUP_EN_GPIO_BIT                     (1 << (SYS_WKUP_EN_POSI + 1))
#define SYS_WKUP_EN_WIFI_BIT                     (1 << (SYS_WKUP_EN_POSI + 2))
#define SYS_WKUP_EN_BLE_BIT                      (1 << (SYS_WKUP_EN_POSI + 3))
#define SYS_WKUP_EN_CHARGE_BIT                   (1 << (SYS_WKUP_EN_POSI + 4))
#endif

#if !(CFG_SOC_NAME == SOC_BK7252N)
#define SCTRL_USB_PWR                         (SCTRL_BASE + 69 * 4)
#define BLE_PWD_POSI                             (16)
#define BLE_PWD_MASK                             (0xFFFF)
#define BLE_PWD                                  (0xDB1E)
#define BLE_PWU                                  (0x0001)
#if (CFG_SOC_NAME == SOC_BK7238)
/* write 0xd5e5 to power down dtcm(final 128KB) */
#define MEM_PWD_POSI                             (0)
#define MEM_PWD_MASK                             (0xFFFF)
#define MEM_PWD                                  (0xD5E5)
#define MEM_PWU                                  (0x0001)
#elif (CFG_SOC_NAME != SOC_BK7231N)
#define USB_PWD_POSI                             (0)
#define USB_PWD_MASK                             (0xFFFF)
#define USB_PWD                                  (0xD12B)
#define USB_PWU                                  (0x0001)
#endif

#define SCTRL_PMU_STATUS                      (SCTRL_BASE + 70 * 4)
#if (CFG_SOC_NAME == SOC_BK7231N)
#define PMU_BLE_POWER_DOWN_BIT                   (1 << 4)
#define PMU_MAC_POWER_DOWN_BIT                   (1 << 3)
#define PMU_MODEM_POWER_DOWN_BIT                 (1 << 2)
#elif (CFG_SOC_NAME == SOC_BK7238)
#define PMU_BLE_POWER_DOWN_BIT                   (1 << 4)
#define PMU_MAC_POWER_DOWN_BIT                   (1 << 3)
#define PMU_MODEM_POWER_DOWN_BIT                 (1 << 2)
#else
#define PMU_MAC_POWER_DOWN_BIT                   (1 << 3)
#define PMU_MODEM_POWER_DOWN_BIT                 (1 << 2)
#define PMU_DSP_POWER_DOWN_BIT                   (1 << 1)
#define PMU_USB_POWER_DOWN_BIT                   (1 << 0)
#endif

#define SCTRL_ROSC_TIMER                      (SCTRL_BASE + 71 * 4)
#define ROSC_TIMER_PERIOD_POSI                   (16)
#define ROSC_TIMER_PERIOD_MASK                   (0xFFFF)
#define ROSC_TIMER_INT_STATUS_BIT                (1 << 8)
#define ROSC_TIMER_ENABLE_BIT                    (1 << 0)

#define SCTRL_GPIO_WAKEUP_EN                  (SCTRL_BASE + 72 * 4)
#define SCTRL_GPIO_WAKEUP_TYPE                (SCTRL_BASE + 73 * 4)
#define SCTRL_GPIO_WAKEUP_INT_STATUS          (SCTRL_BASE + 74 * 4)
#define GPIO_WAKEUP_ENABLE_FLAG                  (1)
#define GPIO_WAKEUP_TYPE_POSITIVE_EDGE           (0)
#define GPIO_WAKEUP_TYPE_NEGATIVE_EDGE           (1)
#else
#define SCTRL_GPIO_WAKEUP_EN                  (SCTRL_BASE + 71 * 4)
#define SCTRL_GPIO_WAKEUP_TYPE_L              (SCTRL_BASE + 72 * 4)
#define SCTRL_GPIO_WAKEUP_TYPE_M              (SCTRL_BASE + 73 * 4)
#define SCTRL_GPIO_WAKEUP_INT_STATUS          (SCTRL_BASE + 74 * 4)
#endif

enum
{
    SYSCTRL_GPIO00_ID = 0,
    SYSCTRL_GPIO01_ID,
    SYSCTRL_GPIO02_ID,
    SYSCTRL_GPIO03_ID,
    SYSCTRL_GPIO04_ID,
    SYSCTRL_GPIO05_ID,
    SYSCTRL_GPIO06_ID,
    SYSCTRL_GPIO07_ID,
    SYSCTRL_GPIO08_ID,
    SYSCTRL_GPIO09_ID,
    SYSCTRL_GPIO10_ID,
    SYSCTRL_GPIO11_ID,
    SYSCTRL_GPIO12_ID,
    SYSCTRL_GPIO13_ID,
    SYSCTRL_GPIO14_ID,
    SYSCTRL_GPIO15_ID,
    SYSCTRL_GPIO16_ID,
    SYSCTRL_GPIO17_ID,
    SYSCTRL_GPIO18_ID,
    SYSCTRL_GPIO19_ID,
    SYSCTRL_GPIO20_ID,
    SYSCTRL_GPIO21_ID,
    SYSCTRL_GPIO22_ID,
    SYSCTRL_GPIO23_ID,
    SYSCTRL_GPIO24_ID,
    SYSCTRL_GPIO25_ID,
    SYSCTRL_GPIO26_ID,
    SYSCTRL_GPIO27_ID,
    SYSCTRL_GPIO28_ID,
    SYSCTRL_GPIO29_ID,
    SYSCTRL_GPIO30_ID,
    SYSCTRL_GPIO31_ID,
    SYSCTRL_GPIO32_ID,
    SYSCTRL_GPIO33_ID,
    SYSCTRL_GPIO34_ID,
    SYSCTRL_GPIO35_ID,
    SYSCTRL_GPIO36_ID,
    SYSCTRL_GPIO37_ID,
    SYSCTRL_GPIO38_ID,
    SYSCTRL_GPIO39_ID
};


#define SCTRL_BLOCK_EN_CFG                    (SCTRL_BASE + 75 * 4)
#define BLOCK_EN_WORD_POSI                       (20)
#define BLOCK_EN_WORD_MASK                       (0xFFFUL)
#define BLOCK_EN_WORD_PWD                        (0xA5CUL)

#define BLOCK_EN_VALID_MASK                      (0xFFFFF)

#if (CFG_SOC_NAME == SOC_BK7231)
#define BLK_EN_BIT_LINEIN                        (1 << 19)
#define BLK_EN_MIC_R_CHANNEL                     (1 << 18)
#define BLK_EN_MIC_L_CHANNEL                     (1 << 17)
#define BLK_EN_AUDIO_R_CHANNEL                   (1 << 16)
#define BLK_EN_AUDIO_L_CHANNEL                   (1 << 15)
#elif (CFG_SOC_NAME == SOC_BK7231U) || (CFG_SOC_NAME == SOC_BK7231N) || (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define BLK_EN_NC                                (1 << 19)
#define BLK_EN_MIC_QSPI_RAM_OR_FLASH             (1 << 18)
#define BLK_EN_MIC_PGA                           (1 << 17)
#define BLK_EN_AUDIO_PLL                         (1 << 16)
#define BLK_EN_AUDIO_RANDOM_GENERATOR            (1 << 15)
#elif (CFG_SOC_NAME == SOC_BK7221U)
#define BLK_EN_NC                                (1 << 19)
#define BLK_EN_MIC_QSPI_RAM_OR_FLASH             (1 << 18)
#define BLK_EN_AUDIO                             (1 << 17)
#define BLK_EN_AUDIO_PLL                         (1 << 16)
#define BLK_EN_AUDIO_RANDOM_GENERATOR            (1 << 15)
#endif // (CFG_SOC_NAME == SOC_BK7231)
#define BLK_EN_SARADC                            (1 << 13)
#define BLK_EN_TEMPRATURE_SENSOR                 (1 << 12)
#define BLK_EN_26M_XTAL_LOW_POWER                (1 << 11)
#define BLK_EN_XTAL2RF                           (1 << 10)
#define BLK_EN_IO_LDO_LOW_POWER                  (1 << 09)
#define BLK_EN_ANALOG_SYS_LDO                    (1 << 8)
#define BLK_EN_DIGITAL_CORE_LDO_LOW_POWER        (1 << 07)
#define BLK_EN_DIGITAL_CORE                      (1 << 06)
#define BLK_EN_DPLL_480M                         (1 << 05)
#define BLK_EN_32K_XTAL                          (1 << 04)
#define BLK_EN_26M_XTAL                          (1 << 03)
#define BLK_EN_ROSC32K                           (1 << 02)
#define BLK_EN_DCO                               (1 << 01)
#define BLK_EN_FLASH                             (1 << 00)


#define SCTRL_BIAS                            (SCTRL_BASE + 76 * 4)
#define BIAS_CAL_OUT_POSI                        (16)
#define BIAS_CAL_OUT_MASK                        (0x1F)
#define LDO_VAL_MANUAL_POSI                      (8)
#define LDO_VAL_MANUAL_MASK                      (0x1F)
#define BIAS_CAL_MANUAL_BIT                      (1 << 4)
#define BIAS_CAL_TRIGGER_BIT                     (1 << 0)


#define SCTRL_ROSC_CAL                        (SCTRL_BASE + 77 * 4)
#if (CFG_SOC_NAME != SOC_BK7238) && (CFG_SOC_NAME != SOC_BK7252N)
#define ROSC_CAL_MANU_FIN_POSI                   (22)
#define ROSC_CAL_MANU_FIN_MASK                   (0x1FF)
#define ROSC_CAL_MANU_CIN_POSI                   (17)
#define ROSC_CAL_MANU_CIN_MASK                   (0x1F)
#define ROSC_CAL_MANU_EN_BIT                     (1 << 16)
#define ROSC_CAL_INTVAL_POSI                     (4)
#define ROSC_CAL_INTVAL_MASK                     (0x7)
#define ROSC_CAL_MODE_BIT                        (1 << 2)
#define ROSC_CAL_TRIG_BIT                        (1 << 1)
#define ROSC_CAL_EN_BIT                          (1 << 0)
#endif

#if !(CFG_SOC_NAME == SOC_BK7252N)
#if (CFG_SOC_NAME == SOC_BK7231N) || (CFG_SOC_NAME == SOC_BK7238)
#define SCTRL_GPIO_WAKEUP_TYPE_SELECT         (SCTRL_BASE + 78 * 4)
#else
#define SCTRL_USB_PLUG_WAKEUP                 (SCTRL_BASE + 78 * 4)
#define USB_PLUG_IN_EN_BIT                       (1 << 0)
#define USB_PLUG_OUT_EN_BIT                      (1 << 1)
#define USB_PLUG_IN_INT_BIT                      (1 << 2)
#define USB_PLUG_OUT_INT_BIT                     (1 << 3)
#endif
#endif

#define SCTRL_BLOCK_EN_MUX                    (SCTRL_BASE + 79 * 4)

#if !(CFG_SOC_NAME == SOC_BK7252N)
#define SCTRL_ROSC_TIMER_H                    (SCTRL_BASE + 80 * 4)
#define ROSC_TIMER_H_PERIOD_POSI                 (0)
#define ROSC_TIMER_H_PERIOD_MASK                 (0xFFFF)
#endif

#if (CFG_SOC_NAME != SOC_BK7231N) && (CFG_SOC_NAME != SOC_BK7238) && (CFG_SOC_NAME != SOC_BK7252N)
#define SCTRL_GPIO_WAKEUP_EN1                 (SCTRL_BASE + 81 * 4)
#define SCTRL_GPIO_WAKEUP_TYPE1               (SCTRL_BASE + 82 * 4)
#define SCTRL_GPIO_WAKEUP_INT_STATUS1         (SCTRL_BASE + 83 * 4)
#endif

#define SCTRL_SW_RETENTION                    (SCTRL_BASE + 84 * 4)
#if (CFG_SOC_NAME == SOC_BK7252N)
#define SW_RETENTION_POSI                        (0)
#define SW_RETENTION_MASK                        (0xFFFF)
#define WDT_RST_FLAG                             (1 << 16)
#endif

#if (CFG_SOC_NAME == SOC_BK7252N)
#define SCTRL_GPIO_WAKEUP_EN1                 (SCTRL_BASE + 85 * 4)
#define SCTRL_GPIO_WAKEUP_TYPE_H              (SCTRL_BASE + 86 * 4)
#define SCTRL_GPIO_WAKEUP_INT_STATUS1         (SCTRL_BASE + 87 * 4)

#define SCTRL_SW_RETENTION1                   (SCTRL_BASE + 88 * 4)
#endif

/*GPIO interrupt config*/
#if (CFG_SOC_NAME == SOC_BK7252N)
#define REG_SYS_CTRL_GPIO_INTEN               (SCTRL_GPIO_WAKEUP_EN)
#define REG_SYS_CTRL_GPIO_INTLV0              (SCTRL_GPIO_WAKEUP_TYPE_L)
#define REG_SYS_CTRL_GPIO_INTLV1              (SCTRL_GPIO_WAKEUP_TYPE_M)
#define REG_SYS_CTRL_GPIO_INTSTA              (SCTRL_GPIO_WAKEUP_INT_STATUS)
#define REG_SYS_CTRL_GPIO_INTEN2              (SCTRL_GPIO_WAKEUP_EN1)
#define REG_SYS_CTRL_GPIO_INTLV3              (SCTRL_GPIO_WAKEUP_TYPE_H)
#define REG_SYS_CTRL_GPIO_INTSTA2             (SCTRL_GPIO_WAKEUP_INT_STATUS1)
#endif

#define DCO_CNTI_120M           (0x127U)  // set DCO out clk with 120M

#include "arm_arch.h"
__inline static UINT32 sctrl_analog_get(UINT32 address)
{
	while(REG_READ(SCTRL_ANALOG_SPI) & (ANA_SPI_STAET_MASK<<ANA_SPI_STATE_POSI));
	return REG_READ(address);
}

__inline static void sctrl_analog_set(UINT32 address, UINT32 value)
{
	REG_WRITE(address, value);
	while(REG_READ(SCTRL_ANALOG_SPI) & (ANA_SPI_STAET_MASK<<ANA_SPI_STATE_POSI));
}

typedef struct  sctrl_ps_save_values{
UINT32    peri_clk_cfg;
UINT32    int_enable_cfg;
}SCTRL_PS_SAVE_VALUES;

#define PS_CLOSE_PERI_CLK            0

/*******************************************************************************
* Function Declarations
*******************************************************************************/
extern UINT32 sctrl_ctrl(UINT32 cmd, void *parm);
extern void sctrl_sub_reset(void);
extern void sctrl_mclk_select(UINT32 mode,UINT32 div);
extern void WFI( void );
#if ((1 == CFG_LOW_VOLTAGE_PS) && (1 == CFG_LOW_VOLTAGE_PS_TEST))
extern void lv_ps_info_mcu_sleep(uint64_t current_time);
extern void lv_ps_info_rf_sleep(bool pre_flag);
extern void lv_ps_info_rf_wakeup(bool restart_flag);
#endif
extern void sctrl_disable_rosc_timer(void);
extern void sctrl_enable_rosc_timer(UINT32 rosc_period);

#if CFG_USE_BLE_PS
void sctrl_ble_ps_init(void);
#endif

#if (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
void rosc_calib_manual_trigger(void);
void rosc_calib_auto_trigger(uint32_t interval);
void rosc_calib_auto_trigger_disable(void);
#endif
#endif // _SCTRL_H_
// eof

