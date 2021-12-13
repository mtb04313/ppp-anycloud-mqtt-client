/*******************************************************************************
* File Name: app_bt_gatt_handler.c
*
* Description: This file implements the functions and GATT Server callbacks
*              needed by a BLE application.
*
* Related Document: See README.md
*
*
 *********************************************************************************
 Copyright 2020-2021, Cypress Semiconductor Corporation (an Infineon company) or
 an affiliate of Cypress Semiconductor Corporation.  All rights reserved.

 This software, including source code, documentation and related
 materials ("Software") is owned by Cypress Semiconductor Corporation
 or one of its affiliates ("Cypress") and is protected by and subject to
 worldwide patent protection (United States and foreign),
 United States copyright laws and international treaty provisions.
 Therefore, you may use this Software only as provided in the license
 agreement accompanying the software package from which you
 obtained this Software ("EULA").
 If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 non-transferable license to copy, modify, and compile the Software
 source code solely for use in connection with Cypress's
 integrated circuit products.  Any reproduction, modification, translation,
 compilation, or representation of this Software except as specified
 above is prohibited without the express written permission of Cypress.

 Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 reserves the right to make changes to the Software without notice. Cypress
 does not assume any liability arising out of the application or use of the
 Software or any product or circuit described in the Software. Cypress does
 not authorize its products for use in any products where a malfunction or
 failure of the Cypress product may reasonably be expected to result in
 significant property damage, injury or death ("High Risk Product"). By
 including Cypress's product in a High Risk Product, the manufacturer
 of such system or application assumes all risk of such use and in doing
 so agrees to indemnify Cypress against all liability.
 *******************************************************************************/

#include "feature_config.h"
#include "app_bt_gatt_handler.h"
#include "app_bt_utils.h"
#include "GeneratedSource/cycfg_gatt_db.h"
#include "cyhal_gpio.h"
#include "cybt_platform_trace.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_gatt.h"

#include "cy_memtrack.h"
#include "cy_debug.h"
#include "ble_modem_task.h"
#include "cybt_platform_config.h"
#include "cybsp_bt_config.h"

#include "wiced_bt_ble.h"
#include "wiced_bt_uuid.h"
#include "wiced_memory.h"
#include "wiced_bt_stack.h"
#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"


/*-- Local Definitions -------------------------------------------------*/

/* The error code for invalid  attribute index in attribute table */
#define INVALID_ATT_TBL_INDEX            (0xFFFFFFFF)

/* Number of advertisment packet */
#define NUM_ADV_PACKETS                 (3u)

/* LE Key Size */
#define MAX_KEY_SIZE (0x10)

typedef void (*pfn_free_buffer_t)(uint8_t *);


/*-- Public Data -------------------------------------------------*/

/* Maintains the connection id of the current connection */
uint16_t g_conn_id = 0;

/* Maintains the MTU size of the current connection */
uint16_t g_mtu = DEFAULT_GATT_MTU_SIZE;


/*-- Local Data -------------------------------------------------*/

static const char *TAG = "app_bt_gatt";


/*-- Local Functions -------------------------------------------------*/

/*******************************************************************************
 * Function Name: app_free_buffer
 *******************************************************************************
 * Summary:
 *  This function frees up the memory buffer
 *
 *
 * Parameters:
 *  uint8_t *p_data: Pointer to the buffer to be free
 *
 ******************************************************************************/
static void app_free_buffer(uint8_t *p_buf)
{
    CY_MEMTRACK_FREE(p_buf);
}


/*******************************************************************************
 * Function Name: app_alloc_buffer
 *******************************************************************************
 * Summary:
 *  This function allocates a memory buffer.
 *
 *
 * Parameters:
 *  int len: Length to allocate
 *
 ******************************************************************************/
static void* app_alloc_buffer(uint16_t len)
{
    return CY_MEMTRACK_MALLOC(len);
}

/**
 * @brief This function returns the corresponding index for the respective
 *        attribute handle from the attribute table. Please ensure that GATT DB
 *        is sorted by attribute handle.
 *
 * @param attr_handle 16-bit attribute handle for the characteristics and descriptors
 * @return int32_t The index of the valid attribute handle otherwise
 *         INVALID_ATT_TBL_INDEX
 */
static int32_t app_get_attr_index_by_handle(uint16_t attr_handle)
{
    uint16_t left = 0;
    uint16_t right = app_gatt_db_ext_attr_tbl_size;

    while(left <= right) {
        uint16_t mid = left + (right - left)/2;

        if (app_gatt_db_ext_attr_tbl[mid].handle == attr_handle) {
            return mid;
        }

        if (app_gatt_db_ext_attr_tbl[mid].handle < attr_handle) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    return INVALID_ATT_TBL_INDEX;
}

/**
 * @brief This function starts the Blueooth LE advertisements and describes
 *        the pairing support
 */
static void app_start_advertisement(void)
{
    wiced_result_t result;

    /* Set Advertisement Data */
    result = wiced_bt_ble_set_raw_advertisement_data(NUM_ADV_PACKETS,
                                                     cy_bt_adv_packet_data);

    if (WICED_SUCCESS != result) {
        CY_LOGE(TAG, "wiced_bt_ble_set_raw_advertisement_data failed: 0x%x", result);
    }

    /* Start Undirected LE Advertisements on device startup. */
    result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH,
                                           BLE_ADDR_PUBLIC,
                                           NULL);

    if (WICED_SUCCESS != result) {
        CY_LOGE(TAG, "wiced_bt_start_advertisements failed: 0x%x", result);
    }
}


/*
 Function Name:
 app_gatt_connect_handler

 Function Description:
 @brief  The callback function is invoked when GATT_CONNECTION_STATUS_EVT occurs
         in GATT Event handler function

 @param p_conn_status     Pointer to Bluetooth LE GATT connection status

 @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t
app_gatt_connect_handler(wiced_bt_gatt_connection_status_t *p_conn_status)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_ERROR;

    CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    if (NULL != p_conn_status) {
        //if ((p_conn_status->connected) && (0 == g_conn_id))
        if (p_conn_status->connected)
        {
            /* Device has connected */
            print_bd_address("\nConnected: Peer BD Address: ", p_conn_status->bd_addr);
            CY_LOGD(TAG, "Connection ID: '%d'", p_conn_status->conn_id);

            g_conn_id  = p_conn_status->conn_id;
            g_mtu = DEFAULT_GATT_MTU_SIZE;
        }
        else {
            /* Device has disconnected */
            print_bd_address("\nDisconnected: Peer BD Address: ", p_conn_status->bd_addr);
            CY_LOGD(TAG, "Connection ID: '%d'", p_conn_status->conn_id);
            CY_LOGD(TAG, "Reason for disconnection: \t%s",
                    get_gatt_disconn_reason_name(p_conn_status->reason));

            /* Handle the disconnection */
            g_conn_id  = 0;
            g_mtu = DEFAULT_GATT_MTU_SIZE;

            app_start_advertisement();
        }
        gatt_status = WICED_BT_GATT_SUCCESS;
    }

    return gatt_status;
}

/*
 Function Name:
 app_gatt_attr_read_handler

 Function Description:
 @brief  The function is invoked when GATT_REQ_READ is received from the
         client device and is invoked by GATT Server Event Callback function.
         This handles "Read Requests" received from Client device

@param conn_id       Connection ID
@param opcode        Bluetooth LE GATT request type opcode
@param p_read_req    Pointer to read request containing the handle to read
@param len_requested length of data requested

 @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t
app_gatt_attr_read_handler( uint16_t conn_id,
                            wiced_bt_gatt_opcode_t opcode,
                            wiced_bt_gatt_read_t *p_read_req,
                            uint16_t len_requested)
{
    wiced_bt_gatt_status_t gatt_status;
    int32_t index;
    uint16_t len_to_send;

    CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    /* Validate the length of the attribute and read from the attribute */
    index = app_get_attr_index_by_handle((p_read_req->handle));

    if (INVALID_ATT_TBL_INDEX == index) {
        CY_LOGE(TAG, "Read handle attribute not found. Handle:0x%X",
                p_read_req->handle);
        wiced_bt_gatt_server_send_error_rsp(conn_id,
                                            opcode,
                                            p_read_req->handle,
                                            WICED_BT_GATT_INVALID_HANDLE);
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* If the incoming offset is greater than the current length in the GATT DB
    then the data cannot be read back */
    if (p_read_req->offset >= app_gatt_db_ext_attr_tbl[index].cur_len)
    {
        CY_LOGE(TAG, "Bad offset value:%u, cur_len:%u",
                p_read_req->offset, app_gatt_db_ext_attr_tbl[index].cur_len);
        wiced_bt_gatt_server_send_error_rsp(conn_id,
                                            opcode,
                                            p_read_req->handle,
                                            WICED_BT_GATT_INVALID_OFFSET);
        return (WICED_BT_GATT_INVALID_OFFSET);
    }

    len_to_send = MIN(len_requested,
                      app_gatt_db_ext_attr_tbl[index].cur_len - p_read_req->offset);

    /*
     * Set the pv_app_context parameter to NULL, since we don't want to free
     * app_gatt_db_ext_attr_tbl[index].p_data on transmit complete
     */
    uint8_t *from = ((uint8_t *)app_gatt_db_ext_attr_tbl[index].p_data) + p_read_req->offset;
    gatt_status = wiced_bt_gatt_server_send_read_handle_rsp(conn_id,
                                                            opcode,
                                                            len_to_send,
                                                            from,
                                                            NULL);

    return (gatt_status);
}

/**
 * Function Name:
 * app_bt_gatt_req_read_by_type_handler
 *
 * Function Description:
 * @brief  Process read-by-type request from peer device
 *
 * @param conn_id       Connection ID
 * @param opcode        BLE GATT request type opcode
 * @param p_read_req    Pointer to read request containing the handle to read
 * @param len_requested length of data requested
 *
 * @return wiced_bt_gatt_status_t  BLE GATT status
 */
static wiced_bt_gatt_status_t
app_gatt_read_by_type_handler(  uint16_t conn_id,
                                wiced_bt_gatt_opcode_t opcode,
                                wiced_bt_gatt_read_by_type_t *p_read_req,
                                uint16_t len_requested)
{
    uint16_t    attr_handle = p_read_req->s_handle;
    uint8_t     *p_rsp = app_alloc_buffer(len_requested);
    uint8_t     pair_len = 0;
    int         index = 0;
    int         used = 0;
    int         filled = 0;

    CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    if (p_rsp == NULL)
    {
        CY_LOGE(TAG, "Out of Memory! len_requested: %u",len_requested);
        wiced_bt_gatt_server_send_error_rsp(conn_id,
                                            opcode,
                                            attr_handle,
                                            WICED_BT_GATT_INSUF_RESOURCE);
        return WICED_BT_GATT_INSUF_RESOURCE;
    }

    /* Read by type returns all attributes of the specified type,
     * between the start and end handles */
    while (TRUE)
    {
        attr_handle = wiced_bt_gatt_find_handle_by_type(attr_handle,
                                                        p_read_req->e_handle,
                                                        &p_read_req->uuid);

        if (attr_handle == 0)
            break;

        index = app_get_attr_index_by_handle(attr_handle);
        if (INVALID_ATT_TBL_INDEX != index)
        {
            CY_LOGD(TAG, "attr_handle %x", attr_handle);
            filled = wiced_bt_gatt_put_read_by_type_rsp_in_stream( p_rsp + used,
                                                        len_requested - used,
                                                        &pair_len,
                                                        attr_handle,
                                        app_gatt_db_ext_attr_tbl[index].cur_len,
                                        app_gatt_db_ext_attr_tbl[index].p_data);
            if (filled == 0)
            {
                CY_LOGD(TAG, "No data is filled");
                break;
            }
            used += filled;
        }
        else
        {
            wiced_bt_gatt_server_send_error_rsp(conn_id,
                                                opcode,
                                                p_read_req->s_handle,
                                                WICED_BT_GATT_ERR_UNLIKELY);
            app_free_buffer(p_rsp);
            return WICED_BT_GATT_ERR_UNLIKELY;
        }

        /* Increment starting handle for next search to one past current */
        attr_handle++;
    } // End of adding the data to the stream

    if (used == 0)
    {
        CY_LOGE(TAG, "attr not found  start_handle: 0x%04x  end_handle: 0x%04x  Type: 0x%04x",
               p_read_req->s_handle, p_read_req->e_handle, p_read_req->uuid.uu.uuid16);
        wiced_bt_gatt_server_send_error_rsp(conn_id,
                                            opcode,
                                            p_read_req->s_handle,
                                            WICED_BT_GATT_INVALID_HANDLE);
        app_free_buffer(p_rsp);
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    /* Send the response */
    wiced_bt_gatt_server_send_read_by_type_rsp( conn_id,
                                                opcode,
                                                pair_len,
                                                used,
                                                p_rsp,
                            (wiced_bt_gatt_app_context_t)app_free_buffer);

    return WICED_BT_GATT_SUCCESS;
}


/*
 Function Name:
 app_gatt_attr_write_handler

 Function Description:
 @brief  The function is invoked when GATT_REQ_WRITE is received from the
         client device and is invoked GATT Server Event Callback function. This
         handles "Write Requests" received from Client device.
 @param conn_id       Connection ID
 @param opcode        Bluetooth LE GATT request type opcode
 @param p_write_req   Pointer to Bluetooth LE GATT write request
 @param len_requested       length of data requested

 @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t
app_gatt_attr_write_handler(uint16_t conn_id,
                            wiced_bt_gatt_opcode_t opcode,
                            wiced_bt_gatt_write_req_t *p_write_req)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_SUCCESS;
    gatt_db_lookup_table_t *puAttribute;

    uint16_t attr_handle = p_write_req->handle;
    uint8_t *p_val = p_write_req->p_val;
    uint16_t len = p_write_req->val_len;

    CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    /* Get the right address for the handle in Gatt DB */
    if (NULL == (puAttribute = app_get_attribute(attr_handle)))
    {
        CY_LOGE(TAG, "Write Handle attr not found. Handle:0x%X", attr_handle);
        wiced_bt_gatt_server_send_error_rsp(conn_id,
                                            opcode,
                                            attr_handle,
                                            WICED_BT_GATT_INVALID_HANDLE);
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    switch (attr_handle)
    {
        /* Write request for the Modem Open characteristic. */
        case HDLC_UICC_SERVICE_MODEM_OPEN_VALUE:
            memset(app_uicc_service_modem_open, 0, app_uicc_service_modem_open_len);
            DEBUG_ASSERT(len <= app_uicc_service_modem_open_len);

            memcpy(app_uicc_service_modem_open, p_val, len);
            puAttribute->cur_len = len;
            CY_LOGD(TAG, "Modem Open: 0x%02x", app_uicc_service_modem_open[0]);

            if (g_ble_modem_task_handle == NULL) {
                CY_LOGE(TAG, "g_ble_modem_task_handle is NULL");
            }
            else {
                ble_modem_task_notify(NOTIF_GATT_DB_MODEM_OPEN, false);
            }
            break;

        /* Write request for the Modem Close characteristic. */
        case HDLC_UICC_SERVICE_MODEM_CLOSE_VALUE:
            memset(app_uicc_service_modem_close, 0, app_uicc_service_modem_close_len);
            DEBUG_ASSERT(len <= app_uicc_service_modem_close_len);

            memcpy(app_uicc_service_modem_close, p_val, len);
            puAttribute->cur_len = len;
            CY_LOGD(TAG, "Modem Close: 0x%02x", app_uicc_service_modem_close[0]);

            if (g_ble_modem_task_handle == NULL) {
                CY_LOGE(TAG, "g_ble_modem_task_handle is NULL");
            }
            else {
                ble_modem_task_notify(NOTIF_GATT_DB_MODEM_CLOSE, false);
            }
            break;

        /* Write request for the Modem TransReceive characteristic. */
        case HDLC_UICC_SERVICE_MODEM_TRANSRECEIVE_VALUE:
            memset(app_uicc_service_modem_transreceive, 0, app_uicc_service_modem_transreceive_len);
            DEBUG_ASSERT(len <= app_uicc_service_modem_transreceive_len);

            memcpy(app_uicc_service_modem_transreceive, p_val, len);
            puAttribute->cur_len = len;
            CY_LOGD(TAG, "Modem TransReceive: 0x%02x", app_uicc_service_modem_transreceive[0]);

            if (g_ble_modem_task_handle == NULL) {
                CY_LOGE(TAG, "g_ble_modem_task_handle is NULL");
            }
            else {
                ble_modem_task_notify(NOTIF_GATT_DB_MODEM_TRANSRECEIVE, false);
            }
            break;

        /* Notification for connection characteristic. If enabled, notification can
         * be sent to the client if the connection was successful or not
         */

        case HDLD_UICC_SERVICE_MODEM_OPEN_CLIENT_CHAR_CONFIG:
            DEBUG_ASSERT(len == app_uicc_service_modem_open_client_char_config_len);
            app_uicc_service_modem_open_client_char_config[0] = p_val[0];
            app_uicc_service_modem_open_client_char_config[1] = p_val[1];
            CY_LOGD(TAG, "Modem Open (Notify): 0x%02x", app_uicc_service_modem_open_client_char_config[0]);
            break;

        case HDLD_UICC_SERVICE_MODEM_CLOSE_CLIENT_CHAR_CONFIG:
            DEBUG_ASSERT(len == app_uicc_service_modem_close_client_char_config_len);
            app_uicc_service_modem_close_client_char_config[0] = p_val[0];
            app_uicc_service_modem_close_client_char_config[1] = p_val[1];
            CY_LOGD(TAG, "Modem Close (Notify): 0x%02x", app_uicc_service_modem_close_client_char_config[0]);
            break;

        case HDLD_UICC_SERVICE_MODEM_TRANSRECEIVE_CLIENT_CHAR_CONFIG:
            DEBUG_ASSERT(len == app_uicc_service_modem_transreceive_client_char_config_len);
            app_uicc_service_modem_transreceive_client_char_config[0] = p_val[0];
            app_uicc_service_modem_transreceive_client_char_config[1] = p_val[1];
            CY_LOGD(TAG, "Modem Transceive (Notify): 0x%02x", app_uicc_service_modem_transreceive_client_char_config[0]);
            break;

        case HDLD_UICC_SERVICE_MODEM_HANDLE_CLIENT_CHAR_CONFIG:
            DEBUG_ASSERT(len == app_uicc_service_modem_handle_client_char_config_len);
            app_uicc_service_modem_handle_client_char_config[0] = p_val[0];
            app_uicc_service_modem_handle_client_char_config[1] = p_val[1];
            CY_LOGD(TAG, "Modem Handle (Notify): 0x%02x", app_uicc_service_modem_handle_client_char_config[0]);
            break;

        case HDLD_UICC_SERVICE_MODEM_ACK_CLIENT_CHAR_CONFIG:
            DEBUG_ASSERT(len == app_uicc_service_modem_ack_client_char_config_len);
            app_uicc_service_modem_ack_client_char_config[0] = p_val[0];
            app_uicc_service_modem_ack_client_char_config[1] = p_val[1];
            CY_LOGD(TAG, "Modem Ack (Notify): 0x%02x", app_uicc_service_modem_ack_client_char_config[0]);
            break;

        default:
            CY_LOGE(TAG, "Write GATT Handle not found");
            gatt_status = WICED_BT_GATT_INVALID_HANDLE;
            break;
    }

    if (gatt_status == WICED_BT_GATT_SUCCESS) {
        wiced_bt_gatt_server_send_write_rsp(conn_id,
                                            opcode,
                                            attr_handle);
    }
    else {
        CY_LOGE(TAG, "GATT set attr status 0x%x", gatt_status);
        wiced_bt_gatt_server_send_error_rsp(conn_id,
                                            opcode,
                                            attr_handle,
                                            gatt_status);
    }

    return (gatt_status);
}

/*
 Function Name:
 app_gatts_attr_req_handler

 Function Description:
 @brief  The callback function is invoked when GATT_ATTRIBUTE_REQUEST_EVT occurs
         in GATT Event handler function. GATT Server Event Callback function.

 @param type  Pointer to GATT Attribute request

 @return wiced_bt_gatt_status_t  Bluetooth LE GATT gatt_status
 */
static wiced_bt_gatt_status_t
app_gatts_attr_req_handler(wiced_bt_gatt_attribute_request_t *p_attr_req)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_INVALID_PDU;

    //CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    switch (p_attr_req->opcode)
    {
        case GATT_REQ_READ:
        case GATT_REQ_READ_BLOB:
             gatt_status = app_gatt_attr_read_handler(p_attr_req->conn_id,
                                                      p_attr_req->opcode,
                                                      &p_attr_req->data.read_req,
                                                      p_attr_req->len_requested);
             break;

        case GATT_REQ_READ_BY_TYPE:
            gatt_status = app_gatt_read_by_type_handler(p_attr_req->conn_id,
                                                        p_attr_req->opcode,
                                                        &p_attr_req->data.read_by_type,
                                                        p_attr_req->len_requested);
            break;

        case GATT_REQ_WRITE:
        case GATT_CMD_WRITE:
        case GATT_CMD_SIGNED_WRITE:
             gatt_status = app_gatt_attr_write_handler(p_attr_req->conn_id,
                                                       p_attr_req->opcode,
                                                       &p_attr_req->data.write_req);
             break;

        case GATT_REQ_MTU:
            /* This is the response for GATT MTU exchange and MTU size is set
             * in the BT-Configurator.
             */
            CY_LOGD(TAG, "Exchanged MTU from client: %d", p_attr_req->data.remote_mtu);
            g_mtu = p_attr_req->data.remote_mtu;

            gatt_status = wiced_bt_gatt_server_send_mtu_rsp(p_attr_req->conn_id,
                                                            p_attr_req->data.remote_mtu,
                                                            CY_BT_MTU_SIZE);
            break;

        case GATT_HANDLE_VALUE_NOTIF:
            CY_LOGD(TAG, "Notification send complete");
            gatt_status = WICED_BT_GATT_SUCCESS;
            break;

        default:
            CY_LOGE(TAG, "ERROR: Unhandled GATT Attribute Request case: %d", p_attr_req->opcode);
            break;
    }

    return gatt_status;
}


/*
 Function Name:
 app_bt_gatt_event_callback

 Function Description:
 @brief  This Function handles the all the GATT events - GATT Event Handler

 @param event            Bluetooth LE GATT event type
 @param p_event_data     Pointer to Bluetooth LE GATT event data

 @return wiced_bt_gatt_status_t  Bluetooth LE GATT status
 */
static wiced_bt_gatt_status_t
app_bt_gatt_event_callback(wiced_bt_gatt_evt_t event,
                           wiced_bt_gatt_event_data_t *p_event_data)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_INVALID_PDU;

    //CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    switch (event)
    {
    case GATT_CONNECTION_STATUS_EVT:
        gatt_status = app_gatt_connect_handler(&p_event_data->connection_status);
        break;

    case GATT_ATTRIBUTE_REQUEST_EVT:
        gatt_status = app_gatts_attr_req_handler(&p_event_data->attribute_request);
        break;

    case GATT_GET_RESPONSE_BUFFER_EVT:
    {
        wiced_bt_gatt_buffer_request_t *p_buf_req = &p_event_data->buffer_request;

        //CY_LOGD(TAG, "%s [%d]: GATT_GET_RESPONSE_BUFFER_EVT len_requested %d",
        //        __FUNCTION__, __LINE__, p_buf_req->len_requested);

        p_buf_req->buffer.p_app_rsp_buffer = app_alloc_buffer(p_buf_req->len_requested);
        p_buf_req->buffer.p_app_ctxt = (void *)app_free_buffer;

        gatt_status = WICED_BT_GATT_SUCCESS;
        break;
    }

    case GATT_APP_BUFFER_TRANSMITTED_EVT:
    {
        pfn_free_buffer_t pfn_free;
        pfn_free = (pfn_free_buffer_t)p_event_data->buffer_xmitted.p_app_ctxt;

        /* If the buffer is dynamic, the context will point to a function to
         * free it.
         */
        if (pfn_free) {
            //CY_LOGD(TAG, "%s [%d]: GATT_APP_BUFFER_TRANSMITTED_EVT",
            //        __FUNCTION__, __LINE__);

            pfn_free(p_event_data->buffer_xmitted.p_app_data);
        }

        gatt_status = WICED_BT_GATT_SUCCESS;
        break;
    }

    default:
        //CY_LOGD(TAG, "Unhandled GATT Event %d", event);
        break;
    }

    return gatt_status;
}

/*
 Function name:
 bt_app_init

 Function Description:
 @brief    This function is executed if BTM_ENABLED_EVT event occurs in
           Bluetooth management callback.

 @param    void

 @return    void
 */
static void bt_app_init(void)
{
    wiced_bt_gatt_status_t gatt_status = WICED_BT_GATT_ERROR;

    /* Register with stack to receive GATT callback */
    gatt_status = wiced_bt_gatt_register(app_bt_gatt_event_callback);
    CY_LOGD(TAG, "gatt_register status:\t%s", get_gatt_status_name(gatt_status));

    /* Initialize GATT Database */
    gatt_status = wiced_bt_gatt_db_init(gatt_database, gatt_database_len, NULL);

    if (WICED_BT_GATT_SUCCESS != gatt_status) {
        CY_LOGD(TAG, "GATT DB Initialization failed err 0x%x", gatt_status);
    }

    /* Allow peer to pair */
    wiced_bt_set_pairable_mode(WICED_TRUE, false);

    /* Start Bluetooth LE advertisements */
    app_start_advertisement();
}

/*
 * Function Name: app_bt_management_callback()
 *
 *@brief
 *  This is a Bluetooth stack event handler function to receive management events
 *  from the Bluetooth LE stack and process as per the application.
 *
 * @param wiced_bt_management_evt_t  Bluetooth LE event code of one byte length
 * @param wiced_bt_management_evt_data_t  Pointer to Bluetooth LE management event
 *                                        structures
 *
 * @return wiced_result_t Error code from WICED_RESULT_LIST or BT_RESULT_LIST
 *
 */
static wiced_result_t
app_bt_management_callback(wiced_bt_management_evt_t event,
                           wiced_bt_management_evt_data_t *p_event_data)
{
    wiced_result_t result = WICED_BT_SUCCESS;

    CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    switch (event) {

    case BTM_ENABLED_EVT:
        CY_LOGD(TAG, "Discover this device with the name: %s", app_gap_device_name);

        print_local_bd_address();
        CY_LOGD(TAG, "Bluetooth Management Event: \t%s", get_btm_event_name(event));

        /* Perform application-specific initialization */
        bt_app_init();
        break;

    case BTM_DISABLED_EVT:
        /* Bluetooth Controller and Host Stack Disabled */

        CY_LOGD(TAG, "Bluetooth Management Event: \t%s", get_btm_event_name(event));
        CY_LOGD(TAG, "Bluetooth Disabled");
        break;

    case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
        p_event_data->pairing_io_capabilities_ble_request.local_io_cap =
                                                  BTM_IO_CAPABILITIES_NONE;

        p_event_data->pairing_io_capabilities_ble_request.oob_data =
                                                              BTM_OOB_NONE;

        p_event_data->pairing_io_capabilities_ble_request.auth_req =
                                                          BTM_LE_AUTH_REQ_SC;

        p_event_data->pairing_io_capabilities_ble_request.max_key_size = MAX_KEY_SIZE;

        p_event_data->pairing_io_capabilities_ble_request.init_keys =
                                                      BTM_LE_KEY_PENC |
                                                      BTM_LE_KEY_PID |
                                                      BTM_LE_KEY_PCSRK |
                                                      BTM_LE_KEY_LENC;

        p_event_data->pairing_io_capabilities_ble_request.resp_keys =
                                                      BTM_LE_KEY_PENC|
                                                      BTM_LE_KEY_PID|
                                                      BTM_LE_KEY_PCSRK|
                                                      BTM_LE_KEY_LENC;
        break;

    case BTM_PAIRING_COMPLETE_EVT:
        if (WICED_SUCCESS == p_event_data->pairing_complete.pairing_complete_info.ble.status)
        {
            CY_LOGD(TAG, "Pairing Complete: SUCCESS");
        }
        else /* Pairing Failed */
        {
            CY_LOGE(TAG, "Pairing Complete: FAILED");
        }
        break;

    case BTM_PAIRED_DEVICE_LINK_KEYS_UPDATE_EVT:
        /* Paired Device Link Keys update */
        result = WICED_SUCCESS;
        break;

    case  BTM_PAIRED_DEVICE_LINK_KEYS_REQUEST_EVT:
        /* Paired Device Link Keys Request */
        result = WICED_BT_ERROR;
        break;

    case BTM_LOCAL_IDENTITY_KEYS_UPDATE_EVT:
        /* Local identity Keys Update */
        result = WICED_SUCCESS;
        break;

    case  BTM_LOCAL_IDENTITY_KEYS_REQUEST_EVT:
        /* Local identity Keys Request */
        result = WICED_BT_ERROR;
        break;

    case BTM_ENCRYPTION_STATUS_EVT:
        if (WICED_SUCCESS == p_event_data->encryption_status.result)
        {
            CY_LOGD(TAG, "Encryption Status Event: SUCCESS");
        }
        else /* Encryption Failed */
        {
            CY_LOGE(TAG, "Encryption Status Event: FAILED");
        }
        break;

    case BTM_SECURITY_REQUEST_EVT:
        wiced_bt_ble_security_grant(p_event_data->security_request.bd_addr,
                                                      WICED_BT_SUCCESS);
        break;

    case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
    {
        wiced_bt_ble_advert_mode_t *p_adv_mode = &p_event_data->ble_advert_state_changed;
        /* Advertisement State Changed */
        CY_LOGD(TAG, "Bluetooth Management Event: \t%s", get_btm_event_name(event));
        CY_LOGD(TAG, "Advertisement state changed to %s", get_btm_advert_mode_name(*p_adv_mode));
        break;
    }

    default:
        CY_LOGD(TAG, "Unhandled Bluetooth Management Event: %d %s",
                event,
                get_btm_event_name(event));
        break;
    }

    return result;
}


/*-- Public Functions -------------------------------------------------*/

bool ble_init(void)
{
    wiced_result_t wiced_result;

    // for BT debugging
    //cybt_platform_set_trace_level(CYBT_TRACE_ID_ALL,      //cybt_trace_id_t id,
    //                              CYBT_TRACE_LEVEL_MAX);  //cybt_trace_level_t level

    /* Initialising the HCI UART for Host contol */
    cybt_platform_config_init(&cybsp_bt_platform_cfg);

    /* Register call back and configuration with stack */
    wiced_result = wiced_bt_stack_init(app_bt_management_callback,
                                 &wiced_bt_cfg_settings);

    /* Check if stack initialization was successful */
    if (WICED_BT_SUCCESS == wiced_result) {
        CY_LOGD(TAG, "Bluetooth Stack Initialization Successful");
    } else {
        CY_LOGD(TAG, "Bluetooth Stack Initialization failed!!");
        return false;
    }

    return true;
}

/*******************************************************************************
* Function Name: app_get_attribute
********************************************************************************
* Summary:
* This function searches through the GATT DB to point to the attribute
* corresponding to the given handle
*
* Parameters:
*  uint16_t handle: Handle to search for in the GATT DB
*
* Return:
*  gatt_db_lookup_table_t *: Pointer to the correct attribute in the GATT DB
*
*******************************************************************************/
gatt_db_lookup_table_t * app_get_attribute(uint16_t handle)
{
    /* Binary search of handles is done; Make sure the handles are sorted */
    int32_t index = app_get_attr_index_by_handle(handle);

    if (INVALID_ATT_TBL_INDEX == index) {
        return NULL;
    }

    return (&app_gatt_db_ext_attr_tbl[index]);
}

/* [] END OF FILE */
