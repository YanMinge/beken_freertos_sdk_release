/**
 ****************************************************************************************
 *
 * @file app_mm_msg.c
 *
 * @brief mesh Application Module
 *
 * @auth  gang.cheng
 *
 * @date  2019.03.31
 *
 * Copyright (C) Beken 2009-2020
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_mm_msg.h"                // Bracese Application Module Definitions
#include "m_api.h"
#include "application.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "common_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "lld_evt.h"
#include "ble_uart.h"
#include "m_api.h"
#include "mesh_api_msg.h"
#include "mal.h"
#include "m_bcn.h"
#include "m_prov_int.h"     // Mesh Provisioning Internal Defines
#include "m_tb_state.h"
#include "mm_vendors.h"

extern uint16_t light_lightness;
//CTL model states
extern uint16_t ctl_lightness;
extern uint16_t ctl_delta_uv;

//HSL model states
extern uint16_t hsl_lightness;
extern uint16_t hsl_hue;
extern uint16_t hsl_saturation;

extern m_lid_t  g_oo_mdl_lid;

void app_light_param_set_handle(uint32_t state, uint32_t trans_time_ms, uint16_t state_id, uint8_t elmt_idx);

int app_models_msg_pro_handler(kernel_msg_id_t const msgid,
                               void const *param,
                               kernel_task_id_t const dest_id,
                               kernel_task_id_t const src_id)
{
    // Drop the message
    MESH_APP_PRINT_DEBUG("%s\n",__func__);
    MESH_APP_PRINT_DEBUG("msgid = 0x%04x,destid = 0x%x,srcid = 0x%x\n",msgid,dest_id,src_id);
    switch (msgid)
    {
        case MESH_MDL_API_IND:
        {
            mm_api_srv_state_upd_ind_t *ind = (mm_api_srv_state_upd_ind_t *)param;
            if (ind->ind_code == MM_API_SRV_STATE_UPD_IND)
            {
                MESH_APP_PRINT_INFO("ind_code:0x%04x = MM_API_SRV_STATE_UPD_IND,state = %d,trans_time_ms = %d,state_id = %d,elmt_idx = 0x%x\r\n",
                                    ind->ind_code,ind->state,ind->trans_time_ms,ind->state_id,ind->elmt_idx);

                app_light_param_set_handle(ind->state, ind->trans_time_ms, ind->state_id, ind->elmt_idx);
            }
            if (ind->ind_code == MM_API_SRV_ARRAY_STATE_UPD_IND)
            {
                mm_api_srv_array_state_upd_ind_t *ind = (mm_api_srv_array_state_upd_ind_t *)param;
                if (ind->state_id == MM_STATE_VENDORS_HSL)
                {
                    MESH_APP_PRINT_INFO("ind len:%d \nDATA:", ind->len);
                    MESH_APP_PRINT_INFO("%s \n", mesh_buffer_to_hex(ind->val, ind->len));
                }
                uint16_t ln, hue, sat;
                ln = common_read16p(&ind->val[MM_LIGHT_HSL_SET_LIGHTNESS_POS]);
                hue = common_read16p(&ind->val[MM_LIGHT_HSL_SET_HUE_POS]);
                sat = common_read16p(&ind->val[MM_LIGHT_HSL_SET_SAT_POS]);
            }

            if (ind->ind_code == MM_API_REGISTER_IND)
            {
                mm_api_register_ind_t *p_ind = (mm_api_register_ind_t *)param;
                MESH_APP_PRINT_INFO("ind_code:0x%04x = MM_API_REGISTER_IND,model_id = 0x%x,elmt_idx = 0x%x,mdl_lid = 0x%x\n",
                                    p_ind->ind_code, p_ind->model_id, p_ind->elmt_idx, p_ind->mdl_lid);
                if ((p_ind->model_id == 0x1000) && (p_ind->elmt_idx == 0))
                {
                    g_oo_mdl_lid = p_ind->mdl_lid;
                }
            }
        } break;
    }

    return (KERNEL_MSG_CONSUMED);
}

void app_light_param_set_handle(uint32_t state, uint32_t trans_time_ms, uint16_t state_id, uint8_t elmt_idx)
{
    if (trans_time_ms)
    {

        switch (state_id)
        {
            case MM_STATE_GEN_ONOFF:
            {
                uint8_t on_off = (uint8_t)state;
                MESH_APP_PRINT_INFO("MM_STATE_GEN_ONOFF\n");
                MESH_APP_PRINT_INFO("generic_on_off  = %d,elmt_idx:%x\n", on_off, elmt_idx);
                MESH_APP_PRINT_INFO("gen on off--server-OnOff = %d,Transition_Time = %d\n", on_off, trans_time_ms);

            } break;
            case MM_STATE_LIGHT_LN:
            {
                uint16_t ln = (uint16_t)state;
                MESH_APP_PRINT_INFO("MM_STATE_LIGHT_LN\r\n");
                MESH_APP_PRINT_INFO("lightness  = %d,elmt_idx:%x\r\n", ln, elmt_idx);
                MESH_APP_PRINT_INFO("lightness---server-lightness = %d,Transition_Time = %d\n", ln, trans_time_ms);

            } break;
            case MM_STATE_LIGHT_CTL_LN:
            {
                uint16_t ln = (uint16_t)state;
                MESH_APP_PRINT_INFO("MM_STATE_LIGHT_CTL_LN\r\n");
                MESH_APP_PRINT_INFO("lightness  = %d,elmt_idx:%x\r\n", ln, elmt_idx);
                MESH_APP_PRINT_INFO("ctl-server-lightness = %d,Transition_Time = %d\n", ln, trans_time_ms);

            } break;
            case MM_STATE_LIGHT_CTL_TEMP:
            {
                uint16_t temp = (uint16_t)state;
                MESH_APP_PRINT_INFO("MM_STATE_LIGHT_CTL_TEMP\r\n");
                MESH_APP_PRINT_INFO("temp  = %d,elmt_idx:%x\r\n", temp, elmt_idx);
                MESH_APP_PRINT_INFO("ctl-server-temp = %d,Transition_Time = %d\n", state, trans_time_ms);

            } break;
            case MM_STATE_LIGHT_HSL_LN:
            {
                uint16_t ln = (uint16_t)state;
                MESH_APP_PRINT_INFO("MM_STATE_LIGHT_HSL_LN\r\n");
                MESH_APP_PRINT_INFO("lightness  = %d,elmt_idx:%x\r\n", ln, elmt_idx);
                MESH_APP_PRINT_INFO("hsl-server--lightness = %d,Transition_Time = %d\n", ln, trans_time_ms);

            } break;
            case MM_STATE_LIGHT_HSL_HUE:
            {
                uint16_t hue = (uint16_t)state;
                MESH_APP_PRINT_INFO("MM_STATE_LIGHT_HSL_HUE\n");
                MESH_APP_PRINT_INFO("hue  = %d,elmt_idx:%x\n", hue, elmt_idx);
                MESH_APP_PRINT_INFO("hsl-server-hue = %d,Transition_Time = %d\n", hue, trans_time_ms);

            } break;
            case MM_STATE_LIGHT_HSL_SAT:
            {
                uint16_t sat = (uint16_t)state;
                MESH_APP_PRINT_INFO("MM_STATE_LIGHT_HSL_SAT\n");
                MESH_APP_PRINT_INFO("sat  = %d,elmt_idx:%x\n", sat, elmt_idx);
                MESH_APP_PRINT_INFO("hsl-server-sat = %d,Transition_Time = %d\n", sat, trans_time_ms);
            } break;
            default:
            {
                MESH_APP_PRINT_INFO("default-0 -state_id = %d\n", state_id);
            } break;

        }
    }
    else
    {
        switch (state_id)
        {
            case MM_STATE_LIGHT_LN:
            {
                uint16_t ln = (uint16_t)state;
                MESH_APP_PRINT_INFO("MM_STATE_LIGHT_LN\n");
                MESH_APP_PRINT_INFO("lightness  = %d,elmt_idx:%x\n", ln, elmt_idx);
                MESH_APP_PRINT_INFO("ln-server-lightness = %d,Transition_Time = %d\n", ln, trans_time_ms);

            } break;
            default:
            {
                MESH_APP_PRINT_INFO("default 1-state_id = %d\n", state_id);
            } break;
        }
    }
}

