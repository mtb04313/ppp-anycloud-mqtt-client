/******************************************************************************
* File Name:   ppp_config.h
*
* Description: This file contains the configuration macros required for the
*              PPP connection.
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

/*******************************************************************************
 *  Include guard
 ******************************************************************************/
#ifndef SOURCE_PPP_CONFIG_H_
#define SOURCE_PPP_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
* Macros
********************************************************************************/
/* Cellular Service Provider's Access Point Name to which the modem connects */
#define PPP_APN                          "move.dataxs.mobi"

/* Username for PPP authentication */
#define PPP_AUTH_USERNAME                ""

/* Password for PPP authentication */
#define PPP_AUTH_PASSWORD                ""

/* LwIP PPP protocol: Password Authentication Protocol (PAP) */
#define PPP_SECURITY_TYPE                PPPAUTHTYPE_PAP

/* Maximum PPP re-connection limit */
#define MAX_PPP_CONN_RETRIES             (10u)

/* PPP re-connection time interval in milliseconds */
#define PPP_CONN_RETRY_INTERVAL_MSEC     (10000)

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_PPP_CONFIG_H_ */

/* [] END OF FILE */
