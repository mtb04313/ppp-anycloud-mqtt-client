/******************************************************************************
* File Name:   subscriber_task.h
*
* Description: This file is the public interface of subscriber_task.c
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

#ifndef SOURCE_SUBSCRIBER_TASK_H_
#define SOURCE_SUBSCRIBER_TASK_H_

#include "feature_config.h"
#include "cy_debug.h"

#include "cyabs_rtos.h"
#include "cy_mqtt_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
* Macros
********************************************************************************/
/* Task parameters for Subscriber Task. */
#define SUBSCRIBER_TASK_NAME               "Subscriber task"
#define SUBSCRIBER_TASK_PRIORITY           CY_RTOS_PRIORITY_BELOWNORMAL
#define SUBSCRIBER_TASK_STACK_SIZE         (1024 * 2)

/* 8-bit value denoting the device (LED) state. */
#define DEVICE_ON_STATE                    (0x00u)
#define DEVICE_OFF_STATE                   (0x01u)

/*******************************************************************************
* Global Variables
********************************************************************************/
/* Commands for the Subscriber Task. */
typedef enum {
    SUBSCRIBE_TO_TOPIC,
    UNSUBSCRIBE_FROM_TOPIC,
    UPDATE_DEVICE_STATE
} subscriber_cmd_t;

/* Struct to be passed via the subscriber task queue */
typedef struct {
    subscriber_cmd_t cmd;
    uint8_t data;
} subscriber_data_t;

/*******************************************************************************
* Extern Variables
********************************************************************************/
extern cy_thread_t g_subscriber_task_handle;
extern cy_queue_t g_subscriber_task_q;
extern uint32_t g_current_device_state;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void subscriber_task(cy_thread_arg_t pvParameters);
void mqtt_subscription_callback(cy_mqtt_publish_info_t *received_msg_info);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_SUBSCRIBER_TASK_H_ */

/* [] END OF FILE */
