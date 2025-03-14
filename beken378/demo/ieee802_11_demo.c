#include "include.h"
#include "common.h"
#include "rwnx_config.h"
#include "app.h"

#if (NX_POWERSAVE)
#include "ps.h"
#endif //(NX_POWERSAVE)

#include "sa_ap.h"
#include "sa_station.h"
#include "main_none.h"
#include "sm.h"

#include "sys_rtos.h"
#include "rtos_pub.h"
#include "error.h"

#include "wlan_ui_pub.h"
#include "uart_pub.h"
#include "mem_pub.h"
#include "str_pub.h"
#include "ieee802_11_demo.h"
#include "rw_pub.h"
#include "ethernet_intf.h"
#include "ieee802_11_defs.h"
#include "power_save_pub.h"

#include "net.h"

#if CFG_ROLE_LAUNCH
#include "role_launch.h"
#endif

extern int hostapd_scan_started;

static const char *crypto_str[] = {
	"None",
	"WEP",
	"WPA_TKIP",
	"WPA_AES",
	"WPA_MIXED",
	"WPA2_TKIP",
	"WPA2_AES",
	"WPA2_MIXED",		////BK_SECURITY_TYPE_WPA3_SAE
	"WPA3_SAE", 		/**< WPA3 SAE */
	"WPA3_WPA2_MIXED",	/** WPA3 SAE or WPA2 AES */
	"EAP",
	"OWE",
	"AUTO",
};


static void scan_cb(void *ctxt, uint8_t param)
{
#if !CFG_WPA_CTRL_IFACE
	struct scanu_rst_upload *scan_rst;
	ScanResult apList;
	int i;
	GLOBAL_INT_DECLARATION();

	apList.ApList = NULL;

	GLOBAL_INT_DISABLE();
	scan_rst = sr_get_scan_results();
	if (scan_rst == NULL) {
		GLOBAL_INT_RESTORE();
		apList.ApNum = 0;
		return;
	} else {
		apList.ApNum = scan_rst->scanu_num;
	}
	if (apList.ApNum > 0) {
		apList.ApList = (void *)os_zalloc(sizeof(*apList.ApList) * apList.ApNum);
		if(apList.ApList == NULL){
			GLOBAL_INT_RESTORE();
			bk_printf("Got ap count: %d,but malloc failed\r\n", apList.ApNum);
			return;
		}
		for (i = 0; i < scan_rst->scanu_num; i++) {
			os_memcpy(apList.ApList[i].ssid, scan_rst->res[i]->ssid, 32);
			apList.ApList[i].ApPower = scan_rst->res[i]->level;
		}
	}
	GLOBAL_INT_RESTORE();

	if (apList.ApList == NULL)
		apList.ApNum = 0;

	bk_printf("Got ap count: %d\r\n", apList.ApNum);
	for (i = 0; i < apList.ApNum; i++) {
		if (os_strlen(apList.ApList[i].ssid) >= SSID_MAX_LEN) {
			char temp_ssid[33];
			os_memset(temp_ssid, 0, 33);
			os_memcpy(temp_ssid, apList.ApList[i].ssid, 32);
			bk_printf("    %s, RSSI=%d\r\n", temp_ssid, apList.ApList[i].ApPower);
		} else {
			bk_printf("    %s, RSSI=%d\r\n", apList.ApList[i].ssid, apList.ApList[i].ApPower);
		}
	}
	bk_printf("Get ap end.......\r\n\r\n");

	if (apList.ApList != NULL) {
		os_free(apList.ApList);
		apList.ApList = NULL;
	}

#if CFG_ROLE_LAUNCH
    rl_pre_sta_set_status(RL_STATUS_STA_LAUNCHED);
#endif

	sr_release_scan_results(scan_rst);

#else	/* CFG_WPA_CTRL_IFACE */
	int ret;

	ScanResult_adv apList;
	if (bk_wlan_ap_is_up() > 0 || hostapd_scan_started)
		ret = wlan_ap_scan_result(&apList);
	else
		ret = wlan_sta_scan_result(&apList);

	if (!ret) {
		int ap_num = apList.ApNum;
		int i;

		bk_printf("Got ap count: %d\r\n", apList.ApNum);
		for (i = 0; i < ap_num; i++)
			bk_printf("    \"%s\", %02x:%02x:%02x:%02x:%02x:%02x, %d, %s, %d\n",
					apList.ApList[i].ssid, MAC2STR(apList.ApList[i].bssid),
					apList.ApList[i].ApPower, crypto_str[apList.ApList[i].security],
					apList.ApList[i].channel);
		os_free(apList.ApList);
	}
#endif /* CFG_WPA_CTRL_IFACE */
}


void demo_scan_app_init(void)
{
	mhdr_scanu_reg_cb(scan_cb, 0);
	bk_wlan_start_scan();
}

void demo_scan_adv_app_init(uint8_t *oob_ssid)
{
    uint8_t **ssid_array;

    ssid_array = &oob_ssid;
	mhdr_scanu_reg_cb(scan_cb, 0);

	bk_printf("scan for ssid:%s\r\n", oob_ssid);
	bk_wlan_start_assign_scan(ssid_array, 1);
}

void demo_softap_app_init(char *ap_ssid, char *ap_key)
{
    network_InitTypeDef_st wNetConfig;
    int ssid_len, key_len;

    os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

    ssid_len = os_strlen(ap_ssid);
    key_len = os_strlen(ap_key);

    if (SSID_MAX_LEN < ssid_len)
    {
        bk_printf("ssid name more than 32 Bytes\r\n");
        return;
    }
	
    if (AP_KEY_MAX_LEN < key_len)
    {
        bk_printf("key more than 64 Bytes\r\n");
        return;
    }

    os_strcpy((char *)wNetConfig.wifi_ssid, ap_ssid);
    os_strcpy((char *)wNetConfig.wifi_key, ap_key);

    wNetConfig.wifi_mode = BK_SOFT_AP;
    wNetConfig.dhcp_mode = DHCP_SERVER;
    wNetConfig.wifi_retry_interval = 100;
    os_strcpy((char *)wNetConfig.local_ip_addr, WLAN_DEFAULT_IP);
    os_strcpy((char *)wNetConfig.net_mask, WLAN_DEFAULT_MASK);
    os_strcpy((char *)wNetConfig.gateway_ip_addr, WLAN_DEFAULT_GW);
    os_strcpy((char *)wNetConfig.dns_server_ip_addr, WLAN_DEFAULT_GW);

    bk_printf("ssid:%s  key:%s\r\n", wNetConfig.wifi_ssid, wNetConfig.wifi_key);
	bk_wlan_start(&wNetConfig);
}

#if CFG_SUPPORT_BSSID_CONNECT
void demo_sta_bssid_app_init(uint8_t *bssid, char *connect_key)
{
	network_InitTypeDef_st wNetConfig;
	int key_len;
	os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

	key_len = os_strlen(connect_key);

	if (STA_KEY_MAX_LEN < key_len)
	{
		bk_printf("key more than buffer Bytes(107)\r\n");
		return;
	}

	os_memcpy((char *)wNetConfig.wifi_bssid, bssid, sizeof(wNetConfig.wifi_bssid));
	os_strlcpy((char *)wNetConfig.wifi_key, connect_key, sizeof(wNetConfig.wifi_key));

	wNetConfig.wifi_mode = BK_STATION;
	wNetConfig.dhcp_mode = DHCP_CLIENT;
	wNetConfig.wifi_retry_interval = 100;
#if CFG_STA_AUTO_RECONNECT
	wNetConfig.auto_reconnect_count = 0;
	wNetConfig.auto_reconnect_timeout = 0;
	wNetConfig.disable_auto_reconnect_after_disconnect = 0;
#endif

#if CFG_QUICK_TRACK
	wNetConfig.key_mgmt = BIT(1) | BIT(10);  // PSK & SAE         WPA_KEY_MGMT_PSK WPA_KEY_MGMT_SAE
	wNetConfig.proto = BIT(0) | BIT(1);	// WPA & RSN              WPA_PROTO_WPA & WPA_PROTO_RSN
	wNetConfig.pairwise_cipher = BIT(3) | BIT(4); // TKIP & CCMP  WPA_CIPHER_CCMP WPA_CIPHER_TKIP
	wNetConfig.group_cipher = BIT(3) | BIT(4); // TKIP & CCMP     WPA_CIPHER_CCMP WPA_CIPHER_TKIP
	wNetConfig.ieee80211w = 1; // MFP Optional
#endif

	bk_wlan_start(&wNetConfig);
}
#endif

void demo_sta_app_init(char *oob_ssid,char *connect_key)
{
	network_InitTypeDef_st wNetConfig;
	int ssid_len, key_len;
	os_memset(&wNetConfig, 0x0, sizeof(network_InitTypeDef_st));

	ssid_len = os_strlen(oob_ssid);
	key_len = os_strlen(connect_key);
	if (SSID_MAX_LEN < ssid_len)
	{
		bk_printf("ssid name more than 32 Bytes\r\n");
		return;
	}

	if (STA_KEY_MAX_LEN < key_len)
	{
		bk_printf("key more than buffer Bytes(107)\r\n");
		return;
	}

	os_strlcpy((char *)wNetConfig.wifi_ssid, oob_ssid, sizeof(wNetConfig.wifi_ssid));
	os_strlcpy((char *)wNetConfig.wifi_key, connect_key, sizeof(wNetConfig.wifi_key));

	wNetConfig.wifi_mode = BK_STATION;
	wNetConfig.dhcp_mode = DHCP_CLIENT;
	wNetConfig.wifi_retry_interval = 100;
#if CFG_STA_AUTO_RECONNECT
	wNetConfig.auto_reconnect_count = 0;
	wNetConfig.auto_reconnect_timeout = 0;
	wNetConfig.disable_auto_reconnect_after_disconnect = 0;
#endif

	bk_printf("ssid:%s key:%s\r\n", wNetConfig.wifi_ssid, wNetConfig.wifi_key);
#if CFG_QUICK_TRACK
	wNetConfig.key_mgmt = BIT(1) | BIT(10);  // PSK & SAE         WPA_KEY_MGMT_PSK WPA_KEY_MGMT_SAE
	wNetConfig.proto = BIT(0) | BIT(1);	// WPA & RSN              WPA_PROTO_WPA & WPA_PROTO_RSN
	wNetConfig.pairwise_cipher = BIT(3) | BIT(4); // TKIP & CCMP  WPA_CIPHER_CCMP WPA_CIPHER_TKIP
	wNetConfig.group_cipher = BIT(3) | BIT(4); // TKIP & CCMP     WPA_CIPHER_CCMP WPA_CIPHER_TKIP
	wNetConfig.ieee80211w = 1; // MFP Optional
#endif

	bk_wlan_start(&wNetConfig);
}

void demo_sta_adv_app_init(char *oob_ssid,char *connect_key)
{
	network_InitTypeDef_adv_st	wNetConfigAdv;

	os_memset( &wNetConfigAdv, 0x0, sizeof(network_InitTypeDef_adv_st) );

	os_strcpy((char*)wNetConfigAdv.ap_info.ssid, oob_ssid);
	hwaddr_aton("48:ee:0c:48:93:12", (u8 *)(wNetConfigAdv.ap_info.bssid));
	wNetConfigAdv.ap_info.security = BK_SECURITY_TYPE_WPA2_MIXED;
	wNetConfigAdv.ap_info.channel = 11;

	os_strcpy((char*)wNetConfigAdv.key, connect_key);
	wNetConfigAdv.key_len = os_strlen(connect_key);
	wNetConfigAdv.dhcp_mode = DHCP_CLIENT;
	wNetConfigAdv.wifi_retry_interval = 100;

	bk_wlan_start_sta_adv(&wNetConfigAdv);
}

void demo_wlan_app_init(VIF_ADDCFG_PTR cfg)
{
    network_InitTypeDef_st network_cfg;
	network_InitTypeDef_adv_st	networkadv_cfg;

    if(cfg->wlan_role == BK_STATION) {
        if(cfg->adv == 1) {
        	os_memset( &networkadv_cfg, 0x0, sizeof(network_InitTypeDef_adv_st) );

        	os_strcpy((char*)networkadv_cfg.ap_info.ssid, cfg->ssid);
        	hwaddr_aton("48:ee:0c:48:93:12", (u8 *)(networkadv_cfg.ap_info.bssid));
        	networkadv_cfg.ap_info.security = BK_SECURITY_TYPE_WPA2_MIXED;
        	networkadv_cfg.ap_info.channel = 11;

        	os_strcpy((char*)networkadv_cfg.key, cfg->key);
        	networkadv_cfg.key_len = os_strlen(cfg->key);
        	networkadv_cfg.dhcp_mode = DHCP_CLIENT;
        	networkadv_cfg.wifi_retry_interval = 100;

        	bk_wlan_start_sta_adv(&networkadv_cfg);
            return;
        } else {
        	os_memset(&network_cfg, 0x0, sizeof(network_InitTypeDef_st));

        	os_strcpy((char *)network_cfg.wifi_ssid, cfg->ssid);
        	os_strcpy((char *)network_cfg.wifi_key, cfg->key);

        	network_cfg.wifi_mode = BK_STATION;
        	network_cfg.dhcp_mode = DHCP_CLIENT;
        	network_cfg.wifi_retry_interval = 100;

        	bk_printf("ssid:%s key:%s\r\n", network_cfg.wifi_ssid, network_cfg.wifi_key);
        }
    } else if(cfg->wlan_role == BK_SOFT_AP) {
        os_memset(&network_cfg, 0x0, sizeof(network_InitTypeDef_st));
        os_strcpy((char *)network_cfg.wifi_ssid, cfg->ssid);
        os_strcpy((char *)network_cfg.wifi_key, cfg->key);

        network_cfg.wifi_mode = BK_SOFT_AP;
        network_cfg.dhcp_mode = DHCP_SERVER;
        network_cfg.wifi_retry_interval = 100;
        os_strcpy((char *)network_cfg.local_ip_addr, WLAN_DEFAULT_IP);
        os_strcpy((char *)network_cfg.net_mask, WLAN_DEFAULT_MASK);
        os_strcpy((char *)network_cfg.gateway_ip_addr, WLAN_DEFAULT_GW);
        os_strcpy((char *)network_cfg.dns_server_ip_addr, WLAN_DEFAULT_GW);

        bk_printf("ssid:%s  key:%s\r\n", network_cfg.wifi_ssid, network_cfg.wifi_key);
    }

    bk_wlan_start(&network_cfg);

}

void demo_state_app_init(void)
{
	LinkStatusTypeDef linkStatus;
	network_InitTypeDef_ap_st ap_info;
	char ssid[33] = {0};
#if CFG_IEEE80211N
	bk_printf("sta: %d, softap: %d, b/g/n\r\n", sta_ip_is_start(), uap_ip_is_start());
#else
	bk_printf("sta: %d, softap: %d, b/g\r\n", sta_ip_is_start(), uap_ip_is_start());
#endif

	if (sta_ip_is_start()) {
		os_memset(&linkStatus, 0x0, sizeof(LinkStatusTypeDef));
		bk_wlan_get_link_status(&linkStatus);
		os_memcpy(ssid, linkStatus.ssid, 33);
		bk_printf("sta:rssi=%d,ssid=%s,bssid=" MACSTR ",aid=%d,channel=%d,cipher_type:%s\n",
				  linkStatus.wifi_strength, ssid, MAC2STR(linkStatus.bssid), linkStatus.aid, linkStatus.channel,
				  crypto_str[linkStatus.security]);
	}

	if (uap_ip_is_start()) {
		os_memset(&ap_info, 0x0, sizeof(network_InitTypeDef_ap_st));
		bk_wlan_ap_para_info_get(&ap_info);
		os_memcpy(ssid, ap_info.wifi_ssid, 33);
		bk_printf("softap:ssid=%s,channel=%d,dhcp=%d,cipher_type:%s\n",
				  ssid, ap_info.channel, ap_info.dhcp_mode, crypto_str[ap_info.security]);
		bk_printf("ip=%s,gate=%s,mask=%s,dns=%s\r\n",
				  ap_info.local_ip_addr, ap_info.gateway_ip_addr, ap_info.net_mask, ap_info.dns_server_ip_addr);
	}
}

void demo_ip_app_init(void)
{
    IPStatusTypedef ipStatus;

	os_memset(&ipStatus, 0x0, sizeof(IPStatusTypedef));
	bk_wlan_get_ip_status(&ipStatus, BK_STATION);

	bk_printf("dhcp=%d ip=%s gate=%s mask=%s mac=" MACSTR "\r\n",
				ipStatus.dhcp, ipStatus.ip, ipStatus.gate,
				ipStatus.mask, MAC2STR((unsigned char*)ipStatus.mac));
}

void bk_demo_monitor_cb(uint8_t *data, int len, wifi_link_info_t *info)
{
	os_printf("len:%d\r\n", len);

	//Only for reference
	/*
	User can get ssid and key by prase monitor data,
	refer to the following code, which is the way airkiss
	use monitor get wifi info from data
	*/
#if 0
	int airkiss_recv_ret;
	airkiss_recv_ret = airkiss_recv(ak_contex, data, len);
#endif

}

int wifi_demo(int argc, char **argv)
{
	char *oob_ssid = NULL;
	char *connect_key;

    if (strcmp(argv[1], "sta") == 0)
    {
		os_printf("sta_Command\r\n");
		if (argc == 3)
		{
			oob_ssid = argv[2];
			connect_key = "1";
		}
		else if (argc == 4)
		{
			oob_ssid = argv[2];
			connect_key = argv[3];
		}
		else
		{
			os_printf("parameter invalid\r\n");
			return -1;
		}

		if(oob_ssid)
		{
			demo_sta_app_init(oob_ssid, connect_key);
		}

		return 0;
    }

	if(strcmp(argv[1], "adv") == 0)
	{
	    os_printf("sta_adv_Command\r\n");
	    if (argc == 3)
	    {
	        oob_ssid = argv[2];
	        connect_key = "1";
	    }
	    else if (argc == 4)
	    {
	        oob_ssid = argv[2];
	        connect_key = argv[3];
	    }
	    else
	    {
	        os_printf("parameter invalid\r\n");
	        return -1;
	    }

	    if(oob_ssid)
	    {
	        demo_sta_adv_app_init(oob_ssid, connect_key);
	    }
		return 0;
	}

	if(strcmp(argv[1], "softap") == 0)
	{

		os_printf("SOFTAP_COMMAND\r\n\r\n");
		if (argc == 3)
		{
			oob_ssid = argv[2];
			connect_key = "1";
		}
		else if (argc == 4)
		{
			oob_ssid = argv[2];
			connect_key = argv[3];
		}
		else
		{
	        os_printf("parameter invalid\r\n");
	        return -1;
		}

		if(oob_ssid)
		{
			demo_softap_app_init(oob_ssid, connect_key);
		}
		return 0;
	}

	if(strcmp(argv[1], "status") == 0)
	{
		if(argc != 3)
		{
			os_printf("parameter invalid\r\n");
		}

		if(strcmp(argv[2], "net") == 0)
		{
			demo_ip_app_init();
		}
		else if(strcmp(argv[2], "link") == 0)
		{
			demo_state_app_init();
		}
		else
		{
			os_printf("parameter invalid\r\n");
		}
	}

	if(strcmp(argv[1], "scan") == 0)
	{
		if(argc == 2)
		{
			demo_scan_app_init();
		}else if(argc == 3)
		{
			demo_scan_adv_app_init((uint8_t *)argv[2]);
		}else
		{
			os_printf("parameter invalid\r\n");
		}
	}

	if(strcmp(argv[1], "monitor") == 0)
	{
		if(argc != 3)
		{
			os_printf("parameter invalid\r\n");
		}

		if(strcmp(argv[2], "start") == 0)
		{
			bk_wlan_register_monitor_cb(bk_demo_monitor_cb);
			bk_wlan_start_monitor();
		}
		else if(strcmp(argv[2], "stop") == 0)
		{
			bk_wlan_stop_monitor();
		}
		else
		{
			os_printf("parameter invalid\r\n");
		}
	}

	return 0;
}

#if CFG_SUPPORT_RTT
MSH_CMD_EXPORT(wifi_demo, wifi_demo command);
#endif

// eof

