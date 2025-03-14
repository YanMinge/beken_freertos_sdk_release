/**
 ****************************************************************************************
 *
 * @file scanu.h
 *
 * @brief Declaration of the Template module environment.
 *
 * Copyright (C) RivieraWaves 2011-2016
 *
 ****************************************************************************************
 */
#ifndef _SCANU_H_
#define _SCANU_H_

/** @defgroup SCANU SCANU
 * @ingroup UMAC
 * @brief Declaration of all structures and functions used by the SCAN module.
 * @{
 */
#include "co_int.h"
#include "co_bool.h"
#include "hal_desc.h"

// for ke_task_id_t
#include "ke_msg.h"

// for MAX_BSS_LIST
#include "mac_common.h"

// for mac_scan_result and other structures
#include "mac.h"

#include "hal_dma.h"

/// Maximum length of the additional ProbeReq IEs
#define SCANU_MAX_IE_LEN                     200
#define DIS_HT_SCAN_RST_RSSI_THRED          (-50)

#define TYPE_ACTIVE_SCAN                     1
#define TYPE_PASSIVE_SCAN                    2

struct rxu_mgt_ind;

/** The SCAN environment.
 * This environment can be used by any element that requires the SCAN.
 */
struct scanu_setting
{
	/* active-scan or passive-scan*/
	uint16_t scan_method;

	/* 1 or 2*/
	uint16_t probe_cnt;

	/* delay before switching on a channel*/
	uint32_t chan_delay;
	uint32_t joining_chan_delay;

	/* scan duration on each channel*/
	uint32_t scan_time;
};

struct scanu_env_tag
{
    /// Parameters of the Scan request
    struct scanu_start_req const *param;
    /// GP DMA descriptor
    struct hal_dma_desc_tag dma_desc;
    /// The number of scan results obtained from LMAC
    uint16_t result_cnt;
    /// Array containing the results of the scan
    struct mac_scan_result scan_result[MAX_BSS_LIST];
    /// Task ID of the sender of the scan request.
    ke_task_id_t src_id;
    /// Fast scan mode indication.
    bool fast;
    /// Fast scan mode indication.
    bool joining;
    /// Band currently scanned
    uint8_t band;
    /// is send_cancel
    uint8_t send_cancel;
    /// BSSID looked for.
    struct mac_addr bssid;
    /// SSID looked for.
    struct mac_ssid ssid;
    #if (NX_P2P)
    /// P2P Scan (ssid we are looking for is "DIRECT-")
    bool p2p_scan;
    #endif //(NX_P2P)
    struct scanu_setting setting;

    uint8_t upload_scanu_result;
    // The number of receiving frame.
    uint8_t recv_frame_cnt;
    uint32_t start_time;
};

/// Definition of an additional IE buffer
struct scanu_add_ie_tag
{
    /// DMA descriptor to download the IEs from host memory
    struct dma_desc dma_desc;
    /// Buffer space for IEs
    uint32_t buf[SCANU_MAX_IE_LEN/4];
};

/// SCAN module environment declaration.
extern struct scanu_env_tag scanu_env;
extern struct scanu_add_ie_tag scanu_add_ie;
extern struct scanu_env_tag scanu_env;

/**
 ****************************************************************************************
 * @brief Initialize the SCAN environment and task.
 *
 ****************************************************************************************
 */
void scanu_init(void);

/**
 ****************************************************************************************
 * @brief Handle the reception of a beacon or probe response frame.
 *
 * It extracts all required information from them and save them in scan result array.
 *
 * @param[in] frame Pointer to the received beacon/probe-response message.
 *
 ****************************************************************************************
 */
int scanu_frame_handler(struct rxu_mgt_ind const *frame);

/**
 ****************************************************************************************
 * @brief Look for a given BSS in the scan results and returns its parameters.
 *
 * The flag scan_related should be set true only when this function is called from the scan
 * module. In this case, the function returns a pointer for an empty entry
 *
 * @param[in] bssid_ptr Pointer to the BSSID looked for.
 * @param[in] allocate Set to true only a result location is required for allocation.
 *
 * @return If the allocation parameter was set, return the pointer to this BSS parameters
 * if the BSS exists or the pointer to the first free BSS entry.  In this case, if return
 * NULL, then no space was found.  If the allocation parameter was not set, return NULL
 * if the BSS was not found.
 ****************************************************************************************
 */
struct mac_scan_result* scanu_find_result(struct mac_addr const *bssid_ptr, bool allocate);
struct mac_scan_result *scanu_search_by_bssid(struct mac_addr const *bssid);
uint32_t scanu_is_active_scan(void);
uint32_t scanu_is_passive_scan(void);
struct mac_scan_result *scanu_search_by_ssid(struct mac_ssid const *ssid);
uint32_t scanu_is_joining(void);
uint32_t scanu_is_more_probe_request(void);
uint32_t chan_fast_end(void);

/**
 ****************************************************************************************
 * @brief Start scanning process.
 *
 ****************************************************************************************
 */
void scanu_start(void);
struct mac_scan_result *scanu_get_result_from_idx(uint8_t result_idx);

/**
 ****************************************************************************************
 * @brief Send a scan request to LMAC for the next band to scan
 *
 ****************************************************************************************
 */
void scanu_scan_next(uint8_t status);

/**
 ****************************************************************************************
 * @brief Send the appropriate confirmation to the source task.
 *
 * @param[in] status  Status to be sent to the source task
 *
 ****************************************************************************************
 */
void scanu_confirm(uint8_t status);

void scanu_change_ht_supported(int8_t scan_rst_rssi);

/// @} end of group

#endif // _SCANU_H_
