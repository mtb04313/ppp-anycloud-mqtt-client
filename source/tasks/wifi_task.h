/******************************************************************************
* File Name:   wifi_task.h
*
* Description: This file is the public interface of wifi_task.c
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2020-2021, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#ifndef SOURCE_WIFI_TASK_H_
#define SOURCE_WIFI_TASK_H_

#include "feature_config.h"
#include "cyabs_rtos.h"

#include <lwip/ip_addr.h>
#include "cy_wcm.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 *                                Constants
 ******************************************************************************/

/* Maximum number of connection retries to the Wi-Fi network. */
#define MAX_CONNECTION_RETRIES            (3)

/* Task parameters for WiFi tasks */
#define WIFI_TASK_NAME                  "WIFI task"
#define WIFI_TASK_STACK_SIZE            (4096u)
#define WIFI_TASK_PRIORITY              CY_RTOS_PRIORITY_HIGH

#if (FEATURE_WIFI == ENABLE_FEATURE)
extern cy_thread_t g_wifi_task_handle;
#endif


/******************************************************************************
 *                              Function Prototypes
 ******************************************************************************/

void wifi_task(cy_thread_arg_t arg);

bool is_wcm_initialized(void);

bool is_wifi_connected(void);

const ip_addr_t* get_wifi_dns_address(void);

const cy_wcm_ip_address_t* get_wifi_ip_address(void);

bool notify_wifi(uint32_t new_notification_value,
                 bool in_isr);

const char* get_wifi_status(void);

#ifdef __cplusplus
}
#endif

#endif      /* SOURCE_WIFI_TASK_H_ */

/* [] END OF FILE */
