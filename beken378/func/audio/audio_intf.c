#include "include.h"
#include "arm_arch.h"

#if ((CFG_USE_AUDIO) && (CFG_SOC_NAME == SOC_BK7221U || CFG_SOC_NAME == SOC_BK7252N))
#include "audio_pub.h"

#include "mem_pub.h"
#include "drv_model_pub.h"
#include "sys_rtos.h"
#include "rtos_pub.h"
#include "error.h"

//#include "song.h"

#define AUD_INTF_DEBUG
#include "uart_pub.h"
#ifdef AUD_INTF_DEBUG
#define AUD_INTF_PRT                os_printf
#define AUD_INTF_WPRT               warning_prf
#define AUD_INTF_FATAL              fatal_prf
#else
#define AUD_INTF_PRT                null_prf
#define AUD_INTF_WPRT               null_prf
#define AUD_INTF_FATAL              null_prf
#endif

enum
{
    AUD_INTF_DAC_FILL_TIMER   = 0,
    AUD_INTF_DAC_PLAY,
    AUD_INTF_DAC_PAUSE,
    AUD_INTF_DAC_SET_VOLUME,

    AUD_INTF_ADC_GET_TIMER,
    AUD_INTF_ADC_PLAY,
    AUD_INTF_ADC_PAUSE,
    AUD_INTF_ADC_LINEIN_TIMER,

    AUD_INTF_SET_SAMPLE_RATE,

    AUD_INTF_EXIT,
};

typedef struct audio_message 
{
	UINT32 audio_msg;
}AUDIO_MSG_T;
#define AUDIO_QITEM_COUNT          (5)

beken_queue_t audio_msg_que = NULL;
beken_thread_t  audio_handle = NULL;

void audio_intf_send_msg(u32 new_msg);
static UINT32 audio_sample_rate = 44100;

#if CFG_USE_AUD_DAC
static const unsigned long PCM_8000[] = {
	0x00010000, 0x5A825A81, 0x7FFF7FFF, 0x5A825A83, 0x00000000, 0xA57FA57E, 0x80018002, 0xA57EA57E,
};

static const unsigned long PCM_16000[] = {
	0x00000001, 0x30FC30FA, 0x5A815A82, 0x76417641, 0x7FFF7FFF, 0x76417641, 0x5A815A82, 0x30FB30FB,
	0x00000000, 0xCF05CF04, 0xA57EA57E, 0x89BF89BF, 0x80018000, 0x89C089BF, 0xA57EA57F, 0xCF05CF04,
};

static const unsigned long PCM_44100[] = {
	0x00000000, 0x122D122E, 0x23FC23FB, 0x350F350F, 0x450F4510, 0x53AA53AA, 0x60926092, 0x6B866B85,
	0x744A744A, 0x7AB57AB5, 0x7EA37EA2, 0x7FFF7FFF, 0x7EC37EC4, 0x7AF77AF6, 0x74AB74AC, 0x6C036C04,
	0x612B612A, 0x5459545A, 0x45D445D3, 0x35E335E3, 0x24DB24DA, 0x13141313, 0x00EA00EA, 0xEEBAEEBB,
	0xDCE5DCE4, 0xCBC5CBC6, 0xBBB6BBB5, 0xAD08AD08, 0xA008A008, 0x94FA94FA, 0x8C188C17, 0x858E858E,
	0x81818181, 0x80038003, 0x811D811D, 0x84C984CA, 0x8AF58AF5, 0x93809381, 0x9E3E9E3E, 0xAAF7AAF7,
	0xB969B96A, 0xC94AC94A, 0xDA46DA46, 0xEC05EC06, 0xFE2DFE2E, 0x105E105E, 0x223B223B, 0x33653365,
	0x43854384, 0x52465246, 0x5F5D5F5D, 0x6A856A85, 0x73847384, 0x7A2C7A2C, 0x7E5B7E5B, 0x7FFA7FFA,
	0x7F007F01, 0x7B747B75, 0x75697569, 0x6CFB6CFB, 0x62586258, 0x55B755B7, 0x47594759, 0x37893789,
	0x26992699, 0x14E114E1, 0x02BC02BC, 0xF08AF08A, 0xDEA6DEA6, 0xCD72CD72, 0xBD42BD43, 0xAE6DAE6D,
	0xA13FA13F, 0x95FD95FD, 0x8CE18CE2, 0x86198619, 0x81CA81CB, 0x800B800C, 0x80E380E3, 0x844E844E,
	0x8A3C8A3C, 0x928B928B, 0x9D149D13, 0xA99CA99C, 0xB7E6B7E6, 0xC7A4C7A5, 0xD889D888, 0xEA39EA39,
	0xFC5AFC5A, 0x0E8F0E8F, 0x20782077, 0x31B831B8, 0x41F641F6, 0x50DF50DF, 0x5E235E23, 0x697F697F,
	0x72B972B8, 0x799F799E, 0x7E0D7E0D, 0x7FEE7FEE, 0x7F387F38, 0x7BED7BED, 0x761F761F, 0x6DEC6DED,
	0x63806380, 0x570F5710, 0x48DB48DA, 0x392C392C, 0x28552855, 0x16AC16AD, 0x048F048F, 0xF25AF259,
	0xE06BE06B, 0xCF1FCF1F, 0xBED2BED3, 0xAFD8AFD7, 0xA27CA27B, 0x97059706, 0x8DB08DB1, 0x86AB86AB,
	0x821C821C, 0x801A801A, 0x80B080AF, 0x83DA83DA, 0x89888988, 0x919C919C, 0x9BED9BEE, 0xA847A846,
	0xB666B666, 0xC603C603, 0xD6CED6CD, 0xE86DE86D, 0xFA89FA87, 0x0CBE0CBF, 0x1EB31EB3, 0x30083007,
	0x40644064, 0x4F734F73, 0x5CE55CE4, 0x68756874, 0x71E771E7, 0x790A790A, 0x7DB87DB9, 0x7FDC7FDD,
	0x7F677F68, 0x7C5F7C5E, 0x76CF76D0, 0x6ED96EDA, 0x64A364A3, 0x58635863, 0x4A594A58, 0x3ACD3ACC,
	0x2A102A0F, 0x18781878, 0x06610661, 0xF429F42A, 0xE230E22F, 0xD0D0D0D0, 0xC066C066, 0xB145B145,
	0xA3BCA3BD, 0x98149813, 0x8E848E85, 0x87438743, 0x82738273, 0x802F8030, 0x80848083, 0x836B836B,
	0x88DA88DB, 0x90B390B3, 0x9ACE9ACD, 0xA6F5A6F5, 0xB4EAB4EA, 0xC464C465, 0xD515D515, 0xE6A3E6A3,
	0xF8B6F8B6, 0x0AED0AEE, 0x1CEC1CED, 0x2E562E57, 0x3ED03ED0, 0x4E024E03, 0x5BA15BA0, 0x67646764,
	0x710E710F, 0x786F786E, 0x7D5E7D5E, 0x7FC37FC3, 0x7F917F91, 0x7CCA7CC9, 0x777A777A, 0x6FBF6FC0,
	0x65C165C2, 0x59B259B3, 0x4BD34BD3, 0x3C693C69, 0x2BC62BC7, 0x1A411A40, 0x08330834, 0xF5FBF5FB,
	0xE3F6E3F7, 0xD283D284, 0xC1FCC1FD, 0xB2B7B2B8, 0xA503A503, 0x99279926, 0x8F608F60, 0x87E187E2,
	0x82D282D2, 0x804D804C, 0x805D805C, 0x83038304, 0x88338833, 0x8FCF8FCF, 0x99B299B1, 0xA5A8A5A8,
	0xB371B372, 0xC2C8C2CA, 0xD35ED35E, 0xE4DAE4DB, 0xF6E4F6E4, 0x091C091C, 0x1B261B25, 0x2CA22CA2,
	0x3D373D37, 0x4C8E4C8E, 0x5A595A59, 0x664E664E, 0x70307030, 0x77CD77CE, 0x7CFD7CFD, 0x7FA37FA3,
	0x7FB37FB4, 0x7D2E7D2E, 0x781F781F, 0x70A170A0, 0x66DA66DA, 0x5AFE5AFD, 0x4D494D49, 0x3E033E04,
	0x2D7D2D7D, 0x1C091C0A, 0x0A050A05, 0xF7CCF7CD, 0xE5BEE5BE, 0xD439D439, 0xC396C396, 0xB42DB42D,
	0xA64DA64E, 0x9A3E9A3E, 0x90409040, 0x88868886, 0x83378337, 0x806E806E, 0x803D803E, 0x82A382A1,
	0x87928791, 0x8EF18EF2, 0x989C989C, 0xA45FA45F, 0xB1FEB1FD, 0xC131C131, 0xD1AAD1AA, 0xE313E313,
	0xF512F512, 0x074A074B, 0x195D195C, 0x2AEC2AEC, 0x3B9C3B9C, 0x4B164B16, 0x590C590B, 0x65336532,
	0x6F4D6F4C, 0x77267726, 0x7C957C95, 0x7F7D7F7D, 0x7FD17FD1, 0x7D8C7D8C, 0x78BD78BC, 0x717B717A,
	0x67ED67ED, 0x5C445C43, 0x4EBB4EBB, 0x3F9B3F9A, 0x2F302F30, 0x1DD01DD0, 0x0BD60BD7, 0xF99FF99E,
	0xE788E789, 0xD5F1D5F1, 0xC533C533, 0xB5A8B5A7, 0xA79DA79D, 0x9B5D9B5D, 0x91289127, 0x89308930,
	0x83A283A2, 0x80988098, 0x80248024, 0x82468247, 0x86F786F6, 0x8E1A8E19, 0x978C978C, 0xA31CA31C,
	0xB08EB08D, 0xBF9CBF9B, 0xCFF7CFF8, 0xE14DE14C, 0xF342F342, 0x05780578, 0x17931792, 0x29332933,
	0x39FE39FD, 0x499B499A, 0x57B957BA, 0x64126413, 0x6E636E64, 0x76787678, 0x7C277C27, 0x7F517F50,
	0x7FE67FE5, 0x7DE47DE3, 0x79557955, 0x72517250, 0x68FA68FB, 0x5D855D84, 0x50295029, 0x412E412E,
	0x30E030E1, 0x1F961F95, 0x0DA70DA6, 0xFB71FB70, 0xE954E953, 0xD7ABD7AB, 0xC6D4C6D3, 0xB726B725,
	0xA8F1A8F1, 0x9C809C80, 0x92139213, 0x89E189E0, 0x84138413, 0x80C880C9, 0x80128011, 0x81F381F3,
	0x86618663, 0x8D478D48, 0x96819680, 0xA1DDA1DD, 0xAF22AF22, 0xBE0ABE0A, 0xCE48CE48, 0xDF88DF89,
	0xF172F171, 0x03A603A5, 0x15C715C8, 0x27782777, 0x385B385B, 0x481B481A, 0x56635664, 0x62ED62EC,
	0x6D746D75, 0x75C475C5, 0x7BB17BB2, 0x7F1C7F1C, 0x7FF57FF4, 0x7E347E35, 0x79E679E6, 0x731F731F,
	0x6A046A03, 0x5EC15EC1, 0x51935194, 0x42BE42BE, 0x328E328F, 0x21592159, 0x0F770F77, 0xFD43FD44,
	0xEB20EB1F, 0xD967D966, 0xC877C877, 0xB8A8B8A7, 0xAA49AA49, 0x9DA89DA9, 0x93059306, 0x8A988A98,
	0x848B848B, 0x80FF80FF, 0x80068006, 0x81A581A4, 0x85D485D3, 0x8C7B8C7C, 0x957B957A, 0xA0A4A0A2,
	0xADBAADBA, 0xBC7BBC7B, 0xCC9CCC9C, 0xDDC6DDC6, 0xEFA2EFA2, 0x01D201D3, 0x13FB13FB, 0x25BA25BA,
	0x36B636B7, 0x46964697, 0x550A5509, 0x61C261C2, 0x6C806C7F, 0x750B750A, 0x7B377B36, 0x7EE37EE2,
	0x7FFD7FFE, 0x7E7F7E80, 0x7A717A71, 0x73E973E8, 0x6B066B06, 0x5FF85FF8, 0x52F952F8, 0x444A444B,
	0x343B343A, 0x231B231A, 0x11461146, 0xFF17FF17, 0xECEDECEC, 0xDB26DB25, 0xCA1DCA1E, 0xBA2CBA2D,
	0xABA6ABA6, 0x9ED59ED5, 0x93FD93FD, 0x8B558B55, 0x85098509, 0x813C813D, 0x80018001, 0x815E815E,
	0x854B854C, 0x8BB58BB5, 0x947B947B, 0x9F6D9F6F, 0xAC56AC57, 0xBAF1BAF0, 0xCAF1CAF1, 0xDC05DC04,
	0xEDD2EDD4,
};

static const unsigned long PCM_48000[] = {
	0x0000FFFF, 0x10B510B5, 0x21202121, 0x30FB30FC, 0x40003FFF, 0x4DEC4DEC, 0x5A825A82, 0x658C658C,
	0x6EDA6ED9, 0x76417641, 0x7BA37BA2, 0x7EE77EE7, 0x7FFF7FFF, 0x7EE67EE6, 0x7BA27BA3, 0x76417641,
	0x6ED86EDA, 0x658C658B, 0x5A825A82, 0x4DEB4DEB, 0x3FFF3FFF, 0x30FC30FB, 0x21202120, 0x10B510B5,
	0x00010000, 0xEF4AEF4B, 0xDEDFDEDF, 0xCF05CF05, 0xC000C000, 0xB214B215, 0xA57EA57F, 0x9A759A74,
	0x91269127, 0x89C089BF, 0x845D845D, 0x811A8119, 0x80018001, 0x811A811A, 0x845E845D, 0x89BF89C0,
	0x91279127, 0x9A749A74, 0xA57EA57E, 0xB214B216, 0xC001C001, 0xCF05CF04, 0xDEDFDEDF, 0xEF4BEF4B,
};

AUD_DAC_CFG_ST aud_dac_cfg;
DD_HANDLE aud_dac_hdl = DD_HANDLE_UNVALID;

#define AUD_DAC_BUF_LEN            (4 * 1024)
UINT8 audio_dac_buf[AUD_DAC_BUF_LEN];
UINT8 *audio_read;

#define DAC_TIMER_INTVAL           (15)
beken_timer_t audio_dac_fill_timer;

__maybe_unused static void audio_intf_dac_timer_handler(void *data);

static void audio_intf_dac_timer_poll(void)
{
#if 0
    UINT32 free_len;
    UINT32 left = (QQQG + sizeof(QQQG)) - audio_read;

    free_len = ddev_control(aud_dac_hdl, AUD_DAC_CMD_GET_FREE_BUF_SIZE, NULL);

    if(free_len <  left) {
        ddev_write(aud_dac_hdl, (char *)audio_read, free_len, 0);
        audio_read += free_len;
    }
    else if(left) {
        UINT32 copy_from_start = free_len - left;
        ddev_write(aud_dac_hdl, (char *)audio_read, left, 0);
        ddev_write(aud_dac_hdl, (char *)QQQG, copy_from_start, 0);
        audio_read = (UINT8*)&QQQG[copy_from_start];
    }
    
    //AUD_INTF_PRT("%d,%d\r\n", free_len, left);
    
    rtos_reload_timer(&audio_dac_fill_timer);
    //AUD_INTF_PRT("audio_intf_dac_timer_poll, restart timer\r\n");
#endif
}

static void audio_intf_dac_timer_handler(void *data)
{
    audio_intf_send_msg(AUD_INTF_DAC_FILL_TIMER);
}

static void audio_intf_dac_pause_poll(void)
{
    ddev_control(aud_dac_hdl, AUD_DAC_CMD_PAUSE, NULL);
}

void audio_intf_dac_pause(void)
{
    audio_intf_send_msg(AUD_INTF_DAC_PAUSE);
}

static void audio_intf_dac_play_poll(void)
{
    ddev_control(aud_dac_hdl, AUD_DAC_CMD_PLAY, NULL);
}

void audio_intf_dac_play(void)
{
    audio_intf_send_msg(AUD_INTF_DAC_PLAY);
}

static void audio_intf_dac_set_volume_poll(void)
{
    static UINT32 dac_vol = 0;

    dac_vol += 10;
    if(dac_vol >= 100)
        dac_vol = 0;

    AUD_INTF_PRT("set dac vol :%d\r\n", dac_vol);

    ddev_control(aud_dac_hdl, AUD_DAC_CMD_SET_VOLUME, &dac_vol);
}

void audio_intf_dac_set_volume(void)
{
    audio_intf_send_msg(AUD_INTF_DAC_SET_VOLUME);
}

#endif // CFG_USE_AUD_DAC

#if CFG_USE_AUD_ADC
AUD_ADC_CFG_ST aud_adc_cfg;
DD_HANDLE aud_adc_hdl = DD_HANDLE_UNVALID;

#define AUD_ADC_BUF_LEN            (3 * 1024)
UINT8 audio_adc_buf[AUD_ADC_BUF_LEN];

#define AUD_ADC_DATA_LEN        (2 * 1024)
UINT8 adc_data[AUD_ADC_DATA_LEN];

#define ADC_TIMER_INTVAL           (4)
beken_timer_t audio_adc_get_timer;

#define ADC_LINEIN_DETECT_TIMER_INTVAL           (10)
beken_timer_t audio_adc_linein_timer;

static void audio_intf_adc_timer_poll(void)
{
    UINT32 mic_filled_len, audio_free_len, copy_len;
    UINT8 *copy_ptr = NULL;

    audio_free_len = ddev_control(aud_dac_hdl, AUD_DAC_CMD_GET_FREE_BUF_SIZE, NULL);
    mic_filled_len = ddev_control(aud_adc_hdl, AUD_ADC_CMD_GET_FILL_BUF_SIZE, NULL);

    if (audio_free_len && mic_filled_len == 0) {
        if (audio_sample_rate == 8000) {
            copy_ptr = (UINT8 *)PCM_8000;
            copy_len = sizeof(PCM_8000);
        } else if (audio_sample_rate == 16000) {
            copy_ptr = (UINT8 *)PCM_16000;
            copy_len = sizeof(PCM_16000);
        } else if (audio_sample_rate == 44100) {
            copy_ptr = (UINT8 *)PCM_44100;
            copy_len = sizeof(PCM_44100);
        } else if (audio_sample_rate == 48000) {
            copy_ptr = (UINT8 *)PCM_48000;
            copy_len = sizeof(PCM_48000);
        } else {
            copy_ptr = NULL;
            copy_len = 0;
        }

        if (copy_ptr) {
            while (audio_free_len >= copy_len) {
                ddev_write(aud_dac_hdl, (char *)copy_ptr, copy_len, 0);
                audio_free_len = ddev_control(aud_dac_hdl, AUD_DAC_CMD_GET_FREE_BUF_SIZE, NULL);
            }
        }
    } else {
        copy_len = (audio_free_len > mic_filled_len)? mic_filled_len : audio_free_len;

        if (copy_len) {
            if (copy_len >  AUD_ADC_DATA_LEN) {
                copy_len = AUD_ADC_DATA_LEN;
            }
            ddev_read(aud_adc_hdl, (char *)adc_data, copy_len, 0);
            ddev_write(aud_dac_hdl, (char *)adc_data, copy_len, 0);
        }
    }

    //AUD_INTF_PRT("%d\r\n", copy_len);

    rtos_reload_timer(&audio_adc_get_timer);
}

static void audio_intf_adc_timer_handler(void *data)
{
    audio_intf_send_msg(AUD_INTF_ADC_GET_TIMER);
}

static void audio_intf_adc_pause_poll(void)
{
    ddev_control(aud_adc_hdl, AUD_ADC_CMD_PAUSE, NULL);
}

void audio_intf_adc_pause(void)
{
    audio_intf_send_msg(AUD_INTF_ADC_PAUSE);
}

static void audio_intf_adc_play_poll(void)
{
    ddev_control(aud_adc_hdl, AUD_ADC_CMD_PLAY, NULL);
}

void audio_intf_adc_play(void)
{
    audio_intf_send_msg(AUD_INTF_ADC_PLAY);
}

static void audio_intf_adc_linein_timer_poll(void)
{
    ddev_control(aud_adc_hdl, AUD_ADC_CMD_DO_LINEIN_DETECT, NULL);

    rtos_reload_timer(&audio_adc_linein_timer);
    //AUD_INTF_PRT("restart timer\r\n");
}

static void audio_intf_adc_linein_timer_handler(void *data)
{
    audio_intf_send_msg(AUD_INTF_ADC_LINEIN_TIMER);
}
#endif

//UINT32 sample_rate_tab[] = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};
UINT32 sample_rate_tab[] = {8000, 16000, 44100, 48000};
static void audio_intf_set_sample_rate_poll(void)
{
    static UINT32 i = 0;

    i++;
    if(i >= sizeof(sample_rate_tab)/4)
        i = 0;

    audio_sample_rate = sample_rate_tab[i];

    #if CFG_USE_AUD_DAC
    ddev_control(aud_dac_hdl, AUD_DAC_CMD_SET_SAMPLE_RATE, &audio_sample_rate);
    #endif

    #if CFG_USE_AUD_ADC
    ddev_control(aud_adc_hdl, AUD_ADC_CMD_SET_SAMPLE_RATE, &audio_sample_rate);
    #endif

    AUD_INTF_PRT("set sample rate:%d\r\n", audio_sample_rate);
}

void audio_intf_set_sample_rate(void)
{
    audio_intf_send_msg(AUD_INTF_SET_SAMPLE_RATE);
}

static void audio_intf_main( beken_thread_arg_t data )
{
    OSStatus err;
    UINT32 status;
    GLOBAL_INT_DECLARATION();

#if CFG_USE_AUD_DAC
    aud_dac_cfg.buf = &audio_dac_buf[0];
    aud_dac_cfg.buf_len = AUD_DAC_BUF_LEN;
    aud_dac_cfg.freq = audio_sample_rate;
    aud_dac_cfg.channels = 2;
    aud_dac_cfg.dma_mode = 0;
    #if AUD_USE_EXT_PA
    aud_dac_cfg.mute_pin = AUD_DAC_MUTE_PIN;
    #else
    aud_dac_cfg.mute_pin = 0;
    #endif

    //audio_read = (UINT8*)&QQQG[0];

    GLOBAL_INT_DISABLE();
    aud_dac_hdl = ddev_open(AUD_DAC_DEV_NAME, &status, (UINT32)&aud_dac_cfg);
    status = status;
    if(DD_HANDLE_UNVALID == aud_dac_hdl)
    {
        GLOBAL_INT_RESTORE();
        goto audio_exit;
    }
    GLOBAL_INT_RESTORE();
#if 0
 	err = rtos_init_timer(&audio_dac_fill_timer, 
							DAC_TIMER_INTVAL, 
							audio_intf_dac_timer_handler, 
							(void *)0);
    ASSERT(kNoErr == err);
    
	err = rtos_start_timer(&audio_dac_fill_timer);
	ASSERT(kNoErr == err);
#endif
#endif

#if CFG_USE_AUD_ADC
    aud_adc_cfg.buf = &audio_adc_buf[0];
    aud_adc_cfg.buf_len = AUD_ADC_BUF_LEN;
    aud_adc_cfg.freq = audio_sample_rate;
    aud_adc_cfg.channels = 2;
    
    aud_adc_cfg.mode = 0;
    //aud_adc_cfg.mode |= AUD_ADC_MODE_DMA_BIT;
    aud_adc_cfg.linein_detect_pin = AUD_ADC_LINEIN_DETECT_PIN;

    GLOBAL_INT_DISABLE();
    aud_adc_hdl = ddev_open(AUD_ADC_DEV_NAME, &status, (UINT32)&aud_adc_cfg);
    status = status;
    if(DD_HANDLE_UNVALID == aud_adc_hdl)
    {
        GLOBAL_INT_RESTORE();
        goto audio_exit;
    }
    GLOBAL_INT_RESTORE();

    #if !AUD_ADC_DAC_HARDWARD_LOOPBACK
    err = rtos_init_timer(&audio_adc_get_timer, 
                            ADC_TIMER_INTVAL, 
                            audio_intf_adc_timer_handler, 
                            (void *)0);
    ASSERT(kNoErr == err);

    err = rtos_start_timer(&audio_adc_get_timer);
    ASSERT(kNoErr == err);
    #endif

    err = rtos_init_timer(&audio_adc_linein_timer, 
                            ADC_LINEIN_DETECT_TIMER_INTVAL, 
                            audio_intf_adc_linein_timer_handler, 
                            (void *)0);
    ASSERT(kNoErr == err);

    err = rtos_start_timer(&audio_adc_linein_timer);
    ASSERT(kNoErr == err);
#endif

    while(1)
    {
        AUDIO_MSG_T msg;
        err = rtos_pop_from_queue(&audio_msg_que, &msg, BEKEN_WAIT_FOREVER);
        if(kNoErr == err) {
            switch(msg.audio_msg) 
            {
                #if CFG_USE_AUD_DAC
                case AUD_INTF_DAC_FILL_TIMER:
                {
                    audio_intf_dac_timer_poll();
                }
                break;

                case AUD_INTF_DAC_PLAY:
                {
                    audio_intf_dac_play_poll();
                }
                break;

                case AUD_INTF_DAC_PAUSE:
                {
                    audio_intf_dac_pause_poll();
                }
                break;

                case AUD_INTF_DAC_SET_VOLUME:
                {
                    audio_intf_dac_set_volume_poll();
                }
                break;

                #endif // CFG_USE_AUD_DAC

                #if CFG_USE_AUD_ADC
                case AUD_INTF_ADC_GET_TIMER:
                {
                    audio_intf_adc_timer_poll();
                }
                break;

                case AUD_INTF_ADC_PLAY:
                {
                    audio_intf_adc_play_poll();
                }
                break;

                case AUD_INTF_ADC_PAUSE:
                {
                    audio_intf_adc_pause_poll();
                }
                break;

                case AUD_INTF_ADC_LINEIN_TIMER:
                {
                    audio_intf_adc_linein_timer_poll();
                }
                break;
                #endif // CFG_USE_AUD_DAC

                case AUD_INTF_SET_SAMPLE_RATE:
                {
                    audio_intf_set_sample_rate_poll();
                }
                break;
                
                case AUD_INTF_EXIT:
                {
                    AUD_INTF_FATAL("exit aud intf\r\n");
                    goto audio_exit;
                }
                break;
                    
                default:
                    break;
            }
        }
    }

audio_exit:

    #if CFG_USE_AUD_DAC
    GLOBAL_INT_DISABLE();
    status = ddev_close(aud_dac_hdl);
    GLOBAL_INT_RESTORE();    
    aud_dac_hdl = DD_HANDLE_UNVALID;

    err = rtos_deinit_timer(&audio_dac_fill_timer);
    ASSERT(kNoErr == err);
    #endif // CFG_USE_AUD_DAC

    #if CFG_USE_AUD_ADC
    GLOBAL_INT_DISABLE();
    status = ddev_close(aud_adc_hdl);
    GLOBAL_INT_RESTORE();    
    aud_adc_hdl = DD_HANDLE_UNVALID;

    err = rtos_deinit_timer(&audio_adc_get_timer);
    ASSERT(kNoErr == err);

    err = rtos_deinit_timer(&audio_adc_linein_timer);
    ASSERT(kNoErr == err);
    #endif // CFG_USE_AUD_ADC

    rtos_deinit_queue(&audio_msg_que);
    audio_msg_que = NULL;

    audio_handle = NULL;
    rtos_delete_thread(NULL);
}

UINT32 audio_intf_init(void)
{
    int ret;

    if((!audio_handle) && (!audio_msg_que))
    {

        ret = rtos_init_queue(&audio_msg_que, 
                                "audio_queue",
                                sizeof(AUDIO_MSG_T),
                                AUDIO_QITEM_COUNT);
        if (kNoErr != ret) 
        {
            AUD_INTF_FATAL("audio_intf ceate queue failed\r\n");
            return kGeneralErr;
        }

        ret = rtos_create_thread(&audio_handle,
                                      BEKEN_DEFAULT_WORKER_PRIORITY,
                                      "audio",
                                      (beken_thread_function_t)audio_intf_main,
                                      1024,
                                      NULL);
        if (ret != kNoErr)
        {
            rtos_deinit_queue(&audio_msg_que);
            audio_msg_que = NULL;
            AUD_INTF_FATAL("Error: Failed to create audio_intf: %d\r\n", ret);
            return kGeneralErr;
        }
    }

    return kNoErr;
}

void audio_intf_uninit(void)
{
    if((audio_handle) && (audio_msg_que)) {
        audio_intf_send_msg(AUD_INTF_EXIT);
        // wait untill task exit
        while(audio_handle) {
            rtos_delay_milliseconds(100);
        }
    }
}

void audio_intf_send_msg(u32 new_msg)
{
    OSStatus ret;
    AUDIO_MSG_T msg;

    if(audio_msg_que) {
        msg.audio_msg = new_msg;

        ret = rtos_push_to_queue(&audio_msg_que, &msg, BEKEN_NO_WAIT);
        if(kNoErr != ret) {
            os_printf("audio_intf_send_msg failed\r\n");
        }
    }
}


#endif // CFG_USE_AUDIO
