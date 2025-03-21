#ifndef __TEMP_DETECT_PUB_H__
#define __TEMP_DETECT_PUB_H__

#define TMP_DETECT_DEBUG   0
#if TMP_DETECT_DEBUG
#define TMP_DETECT_PRT      os_printf
#define TMP_DETECT_WARN     warning_prf
#define TMP_DETECT_FATAL    fatal_prf
#else
#define TMP_DETECT_PRT      null_prf
#define TMP_DETECT_WARN     null_prf
#define TMP_DETECT_FATAL    null_prf
#endif

#if (CFG_SOC_NAME == SOC_BK7221U)
#define ADC_VOLT_SENSER_CHANNEL                     0
#define ADC_TEMP_SENSER_CHANNEL                     8
#elif (CFG_SOC_NAME == SOC_BK7231N)
#define ADC_VOLT_SENSER_CHANNEL                     0
#define ADC_TEMP_SENSER_CHANNEL                     7
#define ADC_TSSI_SENSER_CHANNEL                     8
#elif(CFG_SOC_NAME == SOC_BK7271)
#define ADC_TEMP_SENSER_CHANNEL                     0
#elif(CFG_SOC_NAME == SOC_BK7252N)
#define ADC_VOLT_SENSER_CHANNEL                     0
#define ADC_TEMP_SENSER_CHANNEL                     9
#define ADC_TSSI_SENSER_CHANNEL                     8
#else
#define ADC_VOLT_SENSER_CHANNEL                     0
#define ADC_TEMP_SENSER_CHANNEL                     7
#define ADC_TSSI_SENSER_CHANNEL                     8
#endif

#if (CFG_SOC_NAME == SOC_BK7231N) || (CFG_SOC_NAME == SOC_BK7236)
#define ADC_TEMP_BUFFER_SIZE                        (5+5)//(+5 for skip)
#else
#define ADC_TEMP_BUFFER_SIZE                        5
#endif

#define ADC_TMEP_DETECT_INTVAL                      (15)  // 15s  how many second
#if CFG_USE_TEMPERATURE_DETECT && CFG_USE_VOLTAGE_DETECT
#define ADC_TMEP_DETECT_INTVAL_INIT                 (2)   // 2s
#define ADC_TMEP_XTAL_INIT                          (60)  // 60s
#define ADC_TMEP_DETECT_INTVAL_CHANGE               (60) // 2 mins
#else
#define ADC_TMEP_DETECT_INTVAL_INIT                 (1)   // 1s
#define ADC_TMEP_XTAL_INIT                          (60)  // 60s
#define ADC_TMEP_DETECT_INTVAL_CHANGE               (120) // 2 mins
#endif

#define ADC_TMEP_DIST_INTIAL_VAL                    (0)
#if (CFG_SOC_NAME == SOC_BK7231)
#define ADC_TMEP_LSB_PER_10DEGREE                   (24)
#define ADC_TEMP_VAL_MIN                            (50)
#define ADC_TEMP_VAL_MAX                            (700)
#elif (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N)
#define ADC_TMEP_LSB_PER_10DEGREE                   (21)
#define ADC_TEMP_VAL_MIN                            (10)
#define ADC_TEMP_VAL_MAX                            (700)
#elif (CFG_SOC_NAME == SOC_BK7231N) || (CFG_SOC_NAME == SOC_BK7236)
#define ADC_TMEP_LSB_PER_10DEGREE                   (18)
#define ADC_TEMP_VAL_MIN                            (50)
#define ADC_TEMP_VAL_MAX                            (700)
#else
#define ADC_TMEP_LSB_PER_10DEGREE                   (12)// 7231:24,7231U:22,
#define ADC_TEMP_VAL_MIN                            (50)
#define ADC_TEMP_VAL_MAX                            (700)
#endif

#define ADC_TMEP_10DEGREE_PER_DBPWR                 (1) // 7231:1,7231U:1,
#define ADC_XTAL_DIST_INTIAL_VAL                    (70)

/*******************************************************************************
* Function Declarations
*******************************************************************************/
#if CFG_USE_TEMPERATURE_DETECT || CFG_USE_VOLTAGE_DETECT
void temp_detect_change_configuration(UINT32 intval, UINT32 thre, UINT32 dist);
UINT32 temp_detect_init(UINT32 init_val);
UINT32 temp_detect_uninit(void);
void temp_detect_pause_timer(void);
void temp_detect_restart_detect(void);
UINT32 temp_get_detect_time(void);
UINT32 temp_detect_is_opened_saradc(void) ;
UINT32 temp_detect_is_init(void);
#endif

UINT32 temp_single_get_current_temperature(UINT32 *temp_value);
UINT32 volt_single_get_current_voltage(UINT32 *volt_value);

#endif
// eof

