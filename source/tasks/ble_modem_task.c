/*******************************************************************************
* File Name: ble_modem_task.c
*
* Description: This file defines whether HW or Fake variant is in-use.
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

#include "variant_config.h"  /* for VARIANT_BLE */

#if (VARIANT_BLE == HW_VARIANT)

#include "feature_config.h"  /* for FEATURE_BLE_MODEM */

#if (FEATURE_BLE_MODEM == ENABLE_FEATURE)
#include "ble_modem_task.h"
#include "app_bt_gatt_handler.h"

#include <string.h>

#include "cy_memtrack.h"
#include "cy_debug.h"
#include "cy_string.h"

#include "cycfg_gatt_db.h"
#include "wiced_bt_stack.h"
#include "app_bt_utils.h"

#include "cy_uicc_modem.h"


/*-- Local Definitions -------------------------------------------------*/

#define USE_SHARED_BUFFER_FOR_SEND_AND_RESPONSE   1 /* 1=enable, 0=disable */
#define SEND_BUF_MAX_SIZE             2048

#if (USE_SHARED_BUFFER_FOR_SEND_AND_RESPONSE == 0)
#define RESPONSE_BUF_MAX_SIZE         256
#endif

#define RESPONSE_CHUNK_HEADER_SIZE    2

#define LAST_CHUNK_INDICATOR    0x00
#define FIRST_CHUNK_INDICATOR   0x01
#define MID_CHUNK_INDICATOR     0x02

#define ACK_TRANSRECEIVE_CHUNK  0x01


/*-- Public Data -------------------------------------------------*/

cy_thread_t g_ble_modem_task_handle = NULL;


/*-- Local Data -------------------------------------------------*/

#define BLE_MODEM_TASK_QUEUE_SIZE    10
static cy_queue_t s_queue = NULL;

static const char *TAG = "ble_modem_task";
static uint8_t s_dataArray[SEND_BUF_MAX_SIZE];
static size_t s_dataArrayLen = 0;


/*-- Local Functions -------------------------------------------------*/

#if (FEATURE_PPP == ENABLE_FEATURE)
#include "cy_pcm.h"
#include "cy_modem.h"


static cy_modem_mode_t s_previous_modem_mode = CY_MODEM_COMMAND_MODE;
static bool s_is_modem_locked_by_ble = false;

static bool ble_lock_modem(void)
{
    cy_rslt_t result;

    CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    // remember the modem mode so that we can restore it later
    result = cy_pcm_get_modem_mode(&s_previous_modem_mode);

    if (result == CY_RSLT_PCM_MODEM_IS_NULL) {
        // PPP has not been connected, hence modem is NULL
        // Proceed to connect the modem for BLE, getting it into
        // command mode if everything goes well

        cy_pcm_connect_params_t lpa_conn_param;
        s_previous_modem_mode = CY_MODEM_COMMAND_MODE;

        memset(&lpa_conn_param, 0, sizeof(lpa_conn_param));
        lpa_conn_param.connect_ppp = false;

        result = cy_pcm_connect_modem(  &lpa_conn_param,
                                        NULL,
                                        PCM_CONNECT_MODEM_TIMEOUT_MSEC);

        if (result == CY_RSLT_PCM_TIMEOUT) {
            // someone is using the modem
            CY_LOGE(TAG, "cy_pcm_connect_modem timeout, try again later");
            return false;
        } else if (result == CY_RSLT_SUCCESS) {
            s_is_modem_locked_by_ble = true;
        }
    }

    if (result != CY_RSLT_SUCCESS) {
        CY_LOGE(TAG, "try again later");
        return false;
    }

    if (s_previous_modem_mode == CY_MODEM_PPP_MODE) {
        result = cy_pcm_change_modem_mode(CY_MODEM_COMMAND_MODE);

        if (result != CY_RSLT_SUCCESS) {
            CY_LOGE(TAG, "cy_pcm_change_modem_mode %d failed", CY_MODEM_COMMAND_MODE);
            return false;
        }
    }

    CY_LOGD(TAG, "%s [%d] success", __FUNCTION__, __LINE__);

    return true;
}

static void ble_unlock_modem(void)
{
    cy_rslt_t result;

    CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    if (s_previous_modem_mode == CY_MODEM_PPP_MODE) {
        // restore the previous mode
        result = cy_pcm_change_modem_mode(s_previous_modem_mode);

        if (result != CY_RSLT_SUCCESS) {
            CY_LOGE(TAG, "cy_pcm_change_modem_mode %d failed", s_previous_modem_mode);
        }
    }

    if (s_is_modem_locked_by_ble) {
        result = cy_pcm_disconnect_modem(CY_RTOS_NEVER_TIMEOUT, false);

        if (result != CY_RSLT_SUCCESS) {
            CY_LOGE(TAG, "cy_pcm_disconnect_modem failed!");
        } else {
            CY_LOGD(TAG, "cy_pcm_disconnect_modem ok");
        }

        s_is_modem_locked_by_ble = false;
    }

    CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);
}

#else
static bool ble_lock_modem(void)
{
    return true;
}
static void ble_unlock_modem(void)  {}

#endif // (FEATURE_PPP == ENABLE_FEATURE)


static void update_gatt_db_modem_ack(uint8_t ackValue)
{
    VoidAssert(app_uicc_service_modem_ack_len == 1);

    memset( app_uicc_service_modem_ack,
            0,
            app_uicc_service_modem_ack_len);

    app_uicc_service_modem_ack[0] = ackValue;

    /* Send notification */
    /* Check if the connection is active, notifications are enabled
     */
    //CY_LOGD(TAG, "g_conn_id = %d", g_conn_id);
    print_bytes("app_uicc_service_modem_ack: ",
                app_uicc_service_modem_ack,
                app_uicc_service_modem_ack_len);

    if ((g_conn_id != 0) &&
            (app_uicc_service_modem_ack_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION)) {
        CY_LOGD(TAG, "*** Notification SENT ***");
        //wiced_bt_gatt_send_notification(g_conn_id,
        wiced_bt_gatt_server_send_notification(g_conn_id,
                                               HDLC_UICC_SERVICE_MODEM_ACK_VALUE,
                                               app_uicc_service_modem_ack_len,
                                               app_uicc_service_modem_ack,
                                               NULL);
    } else { /* Notification not sent */
        CY_LOGE(TAG, "Notification not sent");
    }
}

static void update_gatt_db_modem_handle(Modem_Handle_t handle)
{
    uint32_t temp = (uint32_t)handle;
    VoidAssert(sizeof(handle) == 4);
    VoidAssert(app_uicc_service_modem_handle_len == sizeof(handle));

    memset( app_uicc_service_modem_handle,
            0,
            app_uicc_service_modem_handle_len);

    app_uicc_service_modem_handle[0] = LOBYTE(LOWORD(temp));
    app_uicc_service_modem_handle[1] = HIBYTE(LOWORD(temp));
    app_uicc_service_modem_handle[2] = LOBYTE(HIWORD(temp));
    app_uicc_service_modem_handle[3] = HIBYTE(HIWORD(temp));

    /* Send notification */
    /* Check if the connection is active, notifications are enabled
     */
    //CY_LOGD(TAG, "g_conn_id = %d", g_conn_id);
    print_bytes("app_uicc_service_modem_handle: ",
                app_uicc_service_modem_handle,
                app_uicc_service_modem_handle_len);

    if ((g_conn_id != 0) &&
            (app_uicc_service_modem_handle_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION)) {
        CY_LOGD(TAG, "*** Notification SENT ***");
        //wiced_bt_gatt_send_notification(g_conn_id,
        wiced_bt_gatt_server_send_notification(g_conn_id,
                                               HDLC_UICC_SERVICE_MODEM_HANDLE_VALUE,
                                               app_uicc_service_modem_handle_len,
                                               app_uicc_service_modem_handle,
                                               NULL);
    } else { /* Notification not sent */
        CY_LOGE(TAG, "Notification not sent");
    }
}

// send notification in chunks
static void update_gatt_db_modem_transreceive(uint8_t *response_p,
        uint16_t responseLen)
{
    VoidAssert(response_p != NULL);

    /* Check if the connection is active, notifications are enabled */
    if ((g_conn_id != 0) &&
            (app_uicc_service_modem_transreceive_client_char_config[0] & GATT_CLIENT_CONFIG_NOTIFICATION)) {
        // On Pixel3XL, if MTU is set to a non-default value (e.g. 256), the Pixel3XL
        // will not receive the last 3 bytes of the notification value.
        // PSoC wiced_bt_gatt_server_send_notification() will not send the last 3 bytes
        // to the client.  Therefore, reserve these using GATT_NOTIFICATION_RESERVED_SIZE

        uint16_t RESPONSE_CHUNK_MAX_SIZE = (g_mtu < app_uicc_service_modem_transreceive_len)?
                                           g_mtu : app_uicc_service_modem_transreceive_len;
        uint16_t RESPONSE_CHUNK_PAYLOAD_SIZE =
            (RESPONSE_CHUNK_MAX_SIZE - RESPONSE_CHUNK_HEADER_SIZE - GATT_NOTIFICATION_RESERVED_SIZE);
        uint16_t remainder = responseLen;
        bool firstChunk = true;

        CY_LOGD(TAG, "RESPONSE_CHUNK_PAYLOAD_SIZE = %u", RESPONSE_CHUNK_PAYLOAD_SIZE);

        do {
            uint16_t tempLen;
            uint16_t payloadSize = remainder;
            uint8_t indicator = LAST_CHUNK_INDICATOR;

            if (payloadSize > RESPONSE_CHUNK_PAYLOAD_SIZE) {
                payloadSize = RESPONSE_CHUNK_PAYLOAD_SIZE;
                indicator = firstChunk? FIRST_CHUNK_INDICATOR : MID_CHUNK_INDICATOR;
            }

            tempLen = payloadSize + RESPONSE_CHUNK_HEADER_SIZE;
            //CY_LOGD(TAG, "tempLen = %u, app_uicc_service_modem_transreceive_len = %d", tempLen, app_uicc_service_modem_transreceive_len);
            VoidAssert(tempLen <= app_uicc_service_modem_transreceive_len);

            memset( app_uicc_service_modem_transreceive,
                    0,
                    app_uicc_service_modem_transreceive_len);

            app_uicc_service_modem_transreceive[0] = indicator;
            app_uicc_service_modem_transreceive[1] = payloadSize;

            memcpy( &app_uicc_service_modem_transreceive[2],
                    &response_p[responseLen - remainder],
                    payloadSize);

            /* Send notification */
            //CY_LOGD(TAG, "g_conn_id = %d", g_conn_id);
            print_bytes("app_uicc_service_modem_transreceive: ",
                        app_uicc_service_modem_transreceive,
                        (int)tempLen);

            CY_LOGD(TAG, "*** Notification SENT ***");
            wiced_bt_gatt_server_send_notification(g_conn_id,
                                                   HDLC_UICC_SERVICE_MODEM_TRANSRECEIVE_VALUE,
                                                   tempLen,
                                                   app_uicc_service_modem_transreceive,
                                                   NULL);

            remainder -= payloadSize;
            firstChunk = false;

            if (remainder > 0) {
                // if host's BLE handling is slow, we may need to add some delay
                // before sending the next notification
                // e.g. msleep(100);
            }

        } while(remainder > 0);
    } else { /* Notification not sent */
        CY_LOGE(TAG, "Notification not sent");
    }
}


/*-- Public Functions -------------------------------------------------*/

void ble_modem_task_notify( uint32_t msg_id,
                            bool in_isr)
{
    cy_rslt_t result;

    VoidAssert(s_queue != NULL);
    result = cy_rtos_put_queue( &s_queue,
                                &msg_id,
                                0,
                                in_isr);

    if (result != CY_RSLT_SUCCESS) {
        size_t num_items = 0;
        cy_rtos_count_queue(&s_queue,
                            &num_items);
        DEBUG_ASSERT(0);
    }
}

void ble_modem_task(cy_thread_arg_t arg)
{
    cy_rslt_t result;
    Modem_Handle_t hModem = INVALID_HANDLE;

    CY_LOGD(TAG, "%s [%d]", __FUNCTION__, __LINE__);

    VoidAssert(s_queue == NULL);

    result = cy_rtos_init_queue(&s_queue,
                                BLE_MODEM_TASK_QUEUE_SIZE,
                                sizeof(uint32_t));
    VoidAssert(result == CY_RSLT_SUCCESS);
    VoidAssert(s_queue != NULL);


    while (true) {
        /* Notification values received from other tasks */
        uint32_t ulNotifiedValue;

        while (cy_rtos_get_queue( &s_queue,
                                  &ulNotifiedValue,
                                  CY_RTOS_NEVER_TIMEOUT, //pdMS_TO_TICKS(portMAX_DELAY),
                                  false) != CY_RSLT_SUCCESS) {
            CY_LOGD(TAG, "%s [%d]: s_queue - timeout! repeat", __FUNCTION__, __LINE__);
        }

        if (NOTIF_RESTART_BT_ADVERT == ulNotifiedValue) {
            CY_LOGD(TAG, "NOTIF_RESTART_BT_ADVERT\n");
            if (wiced_bt_ble_get_current_advert_mode() == BTM_BLE_ADVERT_OFF) {
                result = wiced_bt_start_advertisements(BTM_BLE_ADVERT_UNDIRECTED_HIGH, 0, NULL);

                if(WICED_SUCCESS != result) {
                    CY_LOGD(TAG, "Failed to start ADV");
                }
            }
        } else if (NOTIF_GATT_DB_MODEM_OPEN == ulNotifiedValue) {
            char portName[MAX_SERIAL_PORT_NAME_LEN];

            /* Copy the Modem Open value from GATT DB */
            gatt_db_lookup_table_t *puAttribute;
            CY_LOGD(TAG, "NOTIF_GATT_DB_MODEM_OPEN");

            puAttribute = app_get_attribute(HDLC_UICC_SERVICE_MODEM_OPEN_VALUE);
            DEBUG_ASSERT((puAttribute->cur_len >= 0) && (puAttribute->cur_len < sizeof(portName)));

            memset(portName, 0, sizeof(portName));
            memcpy(portName, puAttribute->p_data, puAttribute->cur_len);
            CY_LOGD(TAG, "portName = %s", portName);

            if ((hModem == INVALID_HANDLE) && (portName[0] != '\0')) {
                if (ble_lock_modem()) {

#if 0
                    int res;
                    char modelName[MAX_MODEM_MODEL_LEN] = "";
                    char imei[MAX_MODEM_IMEI_LEN] = "";

                    res = Modem_Probe(portName,
                                      modelName,
                                      sizeof(modelName),
                                      imei,
                                      sizeof(imei));
#else
                    int res = RESULT_MODEM_OK;
#endif
                    if (res == RESULT_MODEM_OK) {
                        hModem = Modem_Open(portName);
                        CY_LOGD(TAG, "hModem = 0x%08x (Opened)", hModem);

                        update_gatt_db_modem_handle(hModem);
                    }
                }
            }
        } else if (NOTIF_GATT_DB_MODEM_CLOSE == ulNotifiedValue) {
            uint32_t hValue;

            /* Copy the Modem Close value from GATT DB */
            gatt_db_lookup_table_t *puAttribute;
            CY_LOGD(TAG, "NOTIF_GATT_DB_MODEM_CLOSE");

            puAttribute = app_get_attribute(HDLC_UICC_SERVICE_MODEM_CLOSE_VALUE);
            DEBUG_ASSERT(puAttribute->cur_len == 4);

            hValue = MAKE_ULONG(MAKE_UWORD(puAttribute->p_data[3], puAttribute->p_data[2]),  //uwHigh
                                MAKE_UWORD(puAttribute->p_data[1], puAttribute->p_data[0])); //uwLow
            CY_LOGD(TAG, "hValue = 0x%08x", hValue);

            if ((hValue == (uint32_t)hModem) && (hModem != INVALID_HANDLE)) {
                Modem_Close(hModem);
                hModem = INVALID_HANDLE;
                CY_LOGD(TAG, "hModem = 0x%08x (Closed)", hModem);
                update_gatt_db_modem_handle(hModem);
            }

            ble_unlock_modem();

        } else if (NOTIF_GATT_DB_MODEM_TRANSRECEIVE == ulNotifiedValue) {
            uint32_t hValue;
            uint8_t indicator;
            uint8_t chunkLen;
            uint8_t* chunkData_p;

            /* Copy the Modem TransReceive value from GATT DB */
            gatt_db_lookup_table_t *puAttribute;
            CY_LOGD(TAG, "NOTIF_GATT_DB_MODEM_TRANSRECEIVE");

            puAttribute = app_get_attribute(HDLC_UICC_SERVICE_MODEM_TRANSRECEIVE_VALUE);
            DEBUG_ASSERT(puAttribute->cur_len > 6);

            //print_bytes("puAttribute->p_data: ", puAttribute->p_data, puAttribute->cur_len);

            hValue = MAKE_ULONG(MAKE_UWORD(puAttribute->p_data[3], puAttribute->p_data[2]),  //uwHigh
                                MAKE_UWORD(puAttribute->p_data[1], puAttribute->p_data[0])); //uwLow
            indicator = puAttribute->p_data[4];
            chunkLen = puAttribute->p_data[5];
            chunkData_p = &puAttribute->p_data[6];

            CY_LOGD(TAG, "hValue = 0x%08x", hValue);
            CY_LOGD(TAG, "indicator = 0x%02x", indicator);
            CY_LOGD(TAG, "chunkLen = 0x%02x", chunkLen);

            DEBUG_ASSERT(chunkLen == (puAttribute->cur_len - 6));

            //print_bytes("chunkData_p: ", chunkData_p, chunkLen);

            if ((hValue == (uint32_t)hModem) && (hModem != INVALID_HANDLE)) {
                if (indicator == FIRST_CHUNK_INDICATOR) {
                    memset(s_dataArray, 0, sizeof(s_dataArray));
                    s_dataArrayLen = 0;
                }

                if (chunkLen != 0) {
                    memcpy(&s_dataArray[s_dataArrayLen], chunkData_p, chunkLen);
                    s_dataArrayLen += chunkLen;

                    // acknowledge receipt of the chunk, so the sender can send the next one
                    update_gatt_db_modem_ack(ACK_TRANSRECEIVE_CHUNK);
                }

                if ((indicator == LAST_CHUNK_INDICATOR) && (s_dataArrayLen > 0)) {
                    UICC_Result_t tempResult;
                    UICC_Buffer_t sCommand = {s_dataArray, s_dataArrayLen, s_dataArrayLen};

#if (USE_SHARED_BUFFER_FOR_SEND_AND_RESPONSE == 1)
                    // shared buffer for send/receive
                    UICC_Buffer_t sResponse = {s_dataArray, 0, sizeof(s_dataArray)};
#else
                    uint8_t responseArray[RESPONSE_BUF_MAX_SIZE];
                    UICC_Buffer_t sResponse = {responseArray, 0, sizeof(responseArray)};
#endif

                    tempResult = Modem_SimTransReceive( hModem,
                                                        &sCommand,
                                                        &sResponse);

                    if (tempResult == UICC_NO_ERROR) {
                        update_gatt_db_modem_transreceive(sResponse.p,
                                                          sResponse.len);
                    } else {
                        CY_LOGD(TAG, "%s [%d] Modem_SimTransReceive failed, error_code = 0x%08x",
                                __FUNCTION__, __LINE__, tempResult);
                    }

                    memset(s_dataArray, 0, sizeof(s_dataArray));
                    s_dataArrayLen = 0;
                }
            }
        }
    }

    if (s_queue != NULL) {
        cy_rtos_deinit_queue(&s_queue);
        s_queue = NULL;
    }

    while (true);
}


#endif /* FEATURE_BLE_MODEM */

#endif /* VARIANT_BLE */

