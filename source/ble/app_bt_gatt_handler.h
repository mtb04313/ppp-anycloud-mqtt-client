/*******************************************************************************
* File Name: app_bt_gatt_handler.h
*
* Description: This file is the public interface of app_bt_gatt_handler.c
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

#ifndef SOURCE_APP_BT_GATT_HANDLER_H_
#define SOURCE_APP_BT_GATT_HANDLER_H_

#include "wiced_bt_gatt.h"
#include "cybsp_types.h"
#include "GeneratedSource/cycfg_pins.h"
#include "GeneratedSource/cycfg_gap.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*-- Public Definitions -------------------------------------------------*/

/* Default MTU size */
#define DEFAULT_GATT_MTU_SIZE               23

/**
 * wiced_bt_gatt_server_send_notification() will send a long (1 up to (MTU -3) bytes)
 * notification to the client
 */
#define GATT_NOTIFICATION_RESERVED_SIZE     3

enum sub_task_notifications
{
  NOTIF_RESTART_BT_ADVERT,
  NOTIF_GATT_DB_MODEM_OPEN,
  NOTIF_GATT_DB_MODEM_CLOSE,
  NOTIF_GATT_DB_MODEM_TRANSRECEIVE,
};


/*-- Public Data -------------------------------------------------*/

/* A Global variable to check the status of this device if it is
 * connected to any peer devices*/
extern uint16_t g_conn_id;

/* Maintains the MTU size of the current connection */
extern uint16_t g_mtu;


/*-- Public Functions -------------------------------------------------*/

bool ble_init(void);

gatt_db_lookup_table_t * app_get_attribute(uint16_t handle);


#ifdef __cplusplus
}
#endif

#endif /* SOURCE_APP_BT_GATT_HANDLER_H_ */

/* [] END OF FILE */
