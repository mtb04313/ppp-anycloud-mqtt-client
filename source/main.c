/******************************************************************************
* File Name:   main.c
*
* Description: This file implements the main function.
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

/* Header file includes */
#include "feature_config.h"
#include "cy_debug.h"

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "wifi_task.h"
#include "ppp_task.h"
#include "mqtt_task.h"
#include "console_task.h"

#include "cyabs_rtos.h"
#include "cy_log.h"
#include "cy_memtrack.h"

/******************************************************************************
* Global Variables
******************************************************************************/
/* This enables RTOS aware debugging. */
volatile int uxTopUsedPriority;

/******************************************************************************
 * Function Name: main
 ******************************************************************************
 * Summary:
 *  System entrance point. This function initializes retarget IO, sets up
 *  the MQTT client task, and then starts the RTOS scheduler.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 ******************************************************************************/
int main_thread(void)
{
    cy_rslt_t result;

    /* Enable global interrupts. */
    __enable_irq();

    CY_MEMTRACK_INITIALIZE();

    /* Initialize retarget-io to use the debug UART port. */
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                        CY_RETARGET_IO_BAUDRATE);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen. */
    printf("\x1b[2J\x1b[;H");
    printf("===============================================================\n");
    printf("CE229889 - AnyCloud Example: MQTT Client (%s)\n",
#ifdef COMPONENT_FREERTOS
            "FreeRTOS"
#else
            "RT-Thread"
#endif
            );
    printf("===============================================================\n\n");

    // enable MQTT verbose logs
    //result = cy_log_init(CY_LOG_PRINTF, NULL, NULL);
    //DEBUG_ASSERT(result == CY_RSLT_SUCCESS);

#if (FEATURE_WIFI == ENABLE_FEATURE)
    result = cy_rtos_create_thread( &g_wifi_task_handle,
                                    wifi_task,
                                    WIFI_TASK_NAME,
                                    NULL,
                                    WIFI_TASK_STACK_SIZE,
                                    WIFI_TASK_PRIORITY,
                                    (cy_thread_arg_t) NULL
                                  );
    DEBUG_ASSERT(result == CY_RSLT_SUCCESS);

#endif

#if (FEATURE_PPP == ENABLE_FEATURE)
    ppp_modem_init();
    result = cy_rtos_create_thread( &g_ppp_task_handle,
                                    ppp_task,
                                    PPP_TASK_NAME,
                                    NULL,
                                    PPP_TASK_STACK_SIZE,
                                    PPP_TASK_PRIORITY,
                                    (cy_thread_arg_t) NULL
                                  );
    DEBUG_ASSERT(result == CY_RSLT_SUCCESS);

#endif


#if (FEATURE_MQTT == ENABLE_FEATURE)
    result = cy_rtos_create_thread( &g_mqtt_task_handle,
                                    mqtt_client_task,
                                    MQTT_CLIENT_TASK_NAME,
                                    NULL,
                                    MQTT_CLIENT_TASK_STACK_SIZE,
                                    MQTT_CLIENT_TASK_PRIORITY,
                                    (cy_thread_arg_t) NULL
                                  );
    DEBUG_ASSERT(result == CY_RSLT_SUCCESS);
#endif

#if (FEATURE_CONSOLE == ENABLE_FEATURE)
    result = cy_rtos_create_thread( &g_console_task_handle,
                                    console_task,
                                    CONSOLE_TASK_NAME,
                                    NULL,
                                    CONSOLE_TASK_STACK_SIZE,
                                    CONSOLE_TASK_PRIORITY,
                                    (cy_thread_arg_t) NULL
                                  );
    DEBUG_ASSERT(result == CY_RSLT_SUCCESS);
#endif

    return 0;
}

int main(void)
{
#ifdef COMPONENT_FREERTOS
    int result;

    uxTopUsedPriority = configMAX_PRIORITIES - 1;

    if (CY_RSLT_SUCCESS != cybsp_init()) {
        CY_ASSERT(0);
    }

    result = main_thread();

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /* Should never get here */
    CY_ASSERT(0);

    return result;

#elif defined COMPONENT_RTTHREAD
    extern int entry(void);

    static bool initialized = false;

    uxTopUsedPriority = RT_THREAD_PRIORITY_MAX - 1 ;

    if (!initialized) {
        initialized = true;
        return entry();
    }
    else {
        return main_thread();
    }
#endif
}

/* [] END OF FILE */
