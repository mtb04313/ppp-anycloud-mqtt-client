/*******************************************************************************
* File Name: modem_test.c
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

#include "feature_config.h"  /* for FEATURE_UNIT_TEST_MODEM */

#if (FEATURE_UNIT_TEST_MODEM == ENABLE_FEATURE)
#include "modem_test.h"
#include "cy_modem.h"
#include "cy_uicc_modem.h"

#include "cy_debug.h"
#include "cy_string.h"


/*-- Local Definitions -------------------------------------------------*/


/*-- Public Data -------------------------------------------------*/


/*-- Local Data -------------------------------------------------*/


/*-- Local Functions -------------------------------------------------*/


/*-- Public Functions -------------------------------------------------*/

void test_disable_wireless(void)
{
    cy_modem_disable_wireless();
}

void test_enable_wireless(void)
{
    cy_modem_enable_wireless();

}

bool test_is_wireless_enabled(void)
{
    bool is_enabled =  cy_modem_is_wireless_enabled();
    DEBUG_PRINT(("is_enabled = %d\n", is_enabled));
    return is_enabled;
}

void test_reset_modem(void)
{
    cy_modem_reset();
}


#endif /* FEATURE_UNIT_TEST_MODEM */

