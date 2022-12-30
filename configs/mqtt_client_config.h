/******************************************************************************
* File Name:   mqtt_client_config.h
*
* Description: This file contains all the configuration macros used by the
*              MQTT client in this example.
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

#ifndef MQTT_CLIENT_CONFIG_H_
#define MQTT_CLIENT_CONFIG_H_

#include "cy_mqtt_api.h"

/*******************************************************************************
* Macros
********************************************************************************/

/***************** MQTT CLIENT CONNECTION CONFIGURATION MACROS *****************/
/* MQTT Broker/Server address and port used for the MQTT connection. */
#define MQTT_BROKER_ADDRESS               "a2qabkxxyoigng-ats.iot.ap-southeast-1.amazonaws.com"     //was "MY_MQTT_BROKER_ADDRESS"
#define MQTT_PORT                         8883

/* Set this macro to 1 if a secure (TLS) connection to the MQTT Broker is  
 * required to be established, else 0.
 */
#define MQTT_SECURE_CONNECTION            ( 1 )

/* Configure the user credentials to be sent as part of MQTT CONNECT packet */
#define MQTT_USERNAME                     "User"
#define MQTT_PASSWORD                     ""


/********************* MQTT MESSAGE CONFIGURATION MACROS **********************/
/* The MQTT topics to be used by the publisher and subscriber. */
#define MQTT_PUB_TOPIC                    "ledstatus"
#define MQTT_SUB_TOPIC                    "ledstatus"

/* Set the QoS that is associated with the MQTT publish, and subscribe messages.
 * Valid choices are 0, 1, and 2. Other values should not be used in this macro.
 */
#define MQTT_MESSAGES_QOS                 ( 1 )

/* Configuration for the 'Last Will and Testament (LWT)'. It is an MQTT message 
 * that will be published by the MQTT broker if the MQTT connection is 
 * unexpectedly closed. This configuration is sent to the MQTT broker during 
 * MQTT connect operation and the MQTT broker will publish the Will message on 
 * the Will topic when it recognizes an unexpected disconnection from the client.
 * 
 * If you want to use the last will message, set this macro to 1 and configure
 * the topic and will message, else 0.
 */
#define ENABLE_LWT_MESSAGE                ( 0 )
#if ENABLE_LWT_MESSAGE
    #define MQTT_WILL_TOPIC_NAME          MQTT_PUB_TOPIC "/will"
    #define MQTT_WILL_MESSAGE             ("MQTT client unexpectedly disconnected!")
#endif

/* MQTT messages which are published on the MQTT_PUB_TOPIC that controls the
 * device (user LED in this example) state in this code example.
 */
#define MQTT_DEVICE_ON_MESSAGE            "TURN ON"
#define MQTT_DEVICE_OFF_MESSAGE           "TURN OFF"


/******************* OTHER MQTT CLIENT CONFIGURATION MACROS *******************/
/* A unique client identifier to be used for every MQTT connection. */
#define MQTT_CLIENT_IDENTIFIER            "psoc6-mqtt-client"

/* The timeout in milliseconds for MQTT operations in this example. */
#define MQTT_TIMEOUT_MS                   ( 5000 )

/* The keep-alive interval in seconds used for MQTT ping request. */
#define MQTT_KEEP_ALIVE_SECONDS           ( 60 )

/* Every active MQTT connection must have a unique client identifier. If you 
 * are using the above 'MQTT_CLIENT_IDENTIFIER' as client ID for multiple MQTT 
 * connections simultaneously, set this macro to 1. The device will then
 * generate a unique client identifier by appending a timestamp to the 
 * 'MQTT_CLIENT_IDENTIFIER' string. Example: 'psoc6-mqtt-client5927'
 */
#define GENERATE_UNIQUE_CLIENT_ID         ( 1 )

/* The longest client identifier that an MQTT server must accept (as defined
 * by the MQTT 3.1.1 spec) is 23 characters. However some MQTT brokers support 
 * longer client IDs. Configure this macro as per the MQTT broker specification. 
 */
#define MQTT_CLIENT_IDENTIFIER_MAX_LEN    ( 23 )

/* As per Internet Assigned Numbers Authority (IANA) the port numbers assigned 
 * for MQTT protocol are 1883 for non-secure connections and 8883 for secure
 * connections. In some cases there is a need to use other ports for MQTT like
 * port 443 (which is reserved for HTTPS). Application Layer Protocol 
 * Negotiation (ALPN) is an extension to TLS that allows many protocols to be 
 * used over a secure connection. The ALPN ProtocolNameList specifies the 
 * protocols that the client would like to use to communicate over TLS.
 * 
 * This macro specifies the ALPN Protocol Name to be used that is supported
 * by the MQTT broker in use.
 * Note: For AWS IoT, currently "x-amzn-mqtt-ca" is the only supported ALPN 
 *       ProtocolName and it is only supported on port 443.
 * 
 * Uncomment the below line and specify the ALPN Protocol Name to use this 
 * feature.
 */
// #define MQTT_ALPN_PROTOCOL_NAME           "x-amzn-mqtt-ca"

/* Server Name Indication (SNI) is extension to the Transport Layer Security 
 * (TLS) protocol. As required by some MQTT Brokers, SNI typically includes the 
 * hostname in the Client Hello message sent during TLS handshake.
 * 
 * Uncomment the below line and specify the SNI Host Name to use this extension
 * as specified by the MQTT Broker.
 */
// #define MQTT_SNI_HOSTNAME                 "SNI_HOST_NAME"

/* A Network buffer is allocated for sending and receiving MQTT packets over 
 * the network. Specify the size of this buffer using this macro.
 * 
 * Note: The minimum buffer size is defined by 'CY_MQTT_MIN_NETWORK_BUFFER_SIZE' 
 * macro in the MQTT library. Please ensure this macro value is larger than 
 * 'CY_MQTT_MIN_NETWORK_BUFFER_SIZE'.
 */
#define MQTT_NETWORK_BUFFER_SIZE          ( 2 * CY_MQTT_MIN_NETWORK_BUFFER_SIZE )

/* Maximum MQTT connection re-connection limit. */
#define MAX_MQTT_CONN_RETRIES            (150u)

/* MQTT re-connection time interval in milliseconds. */
#define MQTT_CONN_RETRY_INTERVAL_MS      (10000)


/**************** MQTT CLIENT CERTIFICATE CONFIGURATION MACROS ****************/

/* Configure the below credentials in case of a secure MQTT connection. */
/* PEM-encoded client certificate */
#if 1
#define CLIENT_CERTIFICATE      \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWjCCAkKgAwIBAgIVAOai+9jkiQPezLfziYwnkG9h21r2MA0GCSqGSIb3DQEB\n" \
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMjAyMjgwNjMy\n" \
"NDBaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDnagEITFtG2FFBw4O4\n" \
"0q1JIOIcpvwQS4YCIYQ2I6bV52rctwXHRnuXg4rt8PcvrMWkp9yAEZzyiQio/6mC\n" \
"rS49257d5fR3NI9ump5+GwAE8apdvOdaK6l5N4FUqf38Ywcq+kzY1x4npLOf1DXx\n" \
"F8qOfu6OR1+9+9rGajtbZwE2WsGZwv0NzRJxNCqouMhNjCfw2Z42jTjrv89rd/xV\n" \
"ycsQH5uMLkKLAoDtVvkLp2v3z4LqMwcl9EDP22r/ZZmk0bktIUYwrqXlfSXy+dhU\n" \
"x4UjPLJMxhvK0hlsOaJZ29rWsSem9bWm0x7L0ctwd2ndc9MSWhuz8cUhDZKcxwiz\n" \
"GC5RAgMBAAGjYDBeMB8GA1UdIwQYMBaAFEraKr1yIagbLjX9+pgsHAm57JsHMB0G\n" \
"A1UdDgQWBBQAl4tRw3BSo2BPVnHR5B1JQgZRYTAMBgNVHRMBAf8EAjAAMA4GA1Ud\n" \
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAWFCscvBpROQ/2qs69e9FOhzL\n" \
"bZd4IKDAYrIVpxqbmsY/sGbtlH61TO3x+Ccfu/6tP3Xszk1WTQ5wvCspUjNZmNAQ\n" \
"qNG12uhm6gic8Y0KXycL7LUOF6/qk0MyDZXIadsj2zRpUUdu9gVlo+RZl7JCkHsW\n" \
"vozVtR56E0H/GMUuixnS/oRjytMX5Q0zPv3/Jw74ZtPCMrRKEHwF0ir5AzoJy4KH\n" \
"Q7Ohy1s5LjspZuwkQ3S89dPWdkUhYCwAgOJp2f/fHjvrbwxrQvbr/wM//ZFzibmg\n" \
"5v0JeHFnDQB+czu/I83tn5oQdOtoGAqXdi4PUpzfDahLtsS6izmjBBRv1C49jg==\n" \
"-----END CERTIFICATE-----"


/* PEM-encoded client private key */
#define CLIENT_PRIVATE_KEY          \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpAIBAAKCAQEA52oBCExbRthRQcODuNKtSSDiHKb8EEuGAiGENiOm1edq3LcF\n" \
"x0Z7l4OK7fD3L6zFpKfcgBGc8okIqP+pgq0uPdue3eX0dzSPbpqefhsABPGqXbzn\n" \
"WiupeTeBVKn9/GMHKvpM2NceJ6Szn9Q18RfKjn7ujkdfvfvaxmo7W2cBNlrBmcL9\n" \
"Dc0ScTQqqLjITYwn8NmeNo0467/Pa3f8VcnLEB+bjC5CiwKA7Vb5C6dr98+C6jMH\n" \
"JfRAz9tq/2WZpNG5LSFGMK6l5X0l8vnYVMeFIzyyTMYbytIZbDmiWdva1rEnpvW1\n" \
"ptMey9HLcHdp3XPTElobs/HFIQ2SnMcIsxguUQIDAQABAoIBABv7yJYtTZhajjDq\n" \
"qKIP7954+H7IfpCl4VWeofM+Cm2xZ027YBqB3m4q/QDa87kzJk9L8TEXcCgwA/kT\n" \
"uXbQ5FecmDBzH0XO+E1Cq0nKkA0JADYHot5Xi03aNWx8hfcgLny2+dX65W2b0BDS\n" \
"w5cc7mSe2tAft2cs7n6x1+2sngvpYLUnELjGWU3uh17qzUNeQqvqT1iJP/k/z3El\n" \
"gXHEseIXREud+fHGAN9bhf7Tr24d4GyzUSMcAa1iWNv6pTNcNOp3ku3fO2N0stt0\n" \
"pFoeYHezpvUfdh8jH+KoZvxBfWidafq28pZrXR/zwAJsSYLE9gx0b5OZwLk9DztK\n" \
"mwLsAkUCgYEA/oiwCNu+VjXzlcFmTlYT2FbVIBFdKSrncvdNl/4XBu59a6tXrVID\n" \
"HyQqkKRMF8pF7dt3+cXTvLMI/6gsPbUVuHNicB5cE83iGN7vkpIiXuXLxYtJ6woU\n" \
"fS9kTnL1pbKwtnfyTaxdK+vPYCZkjDtf+LqSVh4Q67Bk01+Aoe78LesCgYEA6L85\n" \
"2b6jIchS/Ag7snV3Z7nmqzZzVPZQNvOaG7/OTKZs+MX8W+ztE+RcE06GtAbca+rZ\n" \
"vsTXtlz5f3RxG6dUn/Tw1AWnlVUjx2VlFP2jjC9q9bYBC0topzIAqEm7B47VO+l/\n" \
"/QtKxklzMIZfnpN+ZL9DVEQTrv6SknTPEKEwebMCgYBW15v36cFO+Kla9tlI8OVk\n" \
"cnvUrRfz68d72hSHPxHsM4JnMdaAM/MMtPIw111+4Gxrcu3EMlLDlvIDCAXQJ/B5\n" \
"NiTny/PubdguVCG5CFLhvdWAWL2ni5DiBUFb4q0dE5JnLxVkmuJPEe13CKJVlgxw\n" \
"eHdlGmz7fPtpgrQIi9lOcQKBgQDMfC2sdARJSwI1sloYmYG13SufYzCDBgFFtlLA\n" \
"bI0o5NM64l+suAU3A9wtjkFk694+5lA2fiTzcM43v6scW7BK9N2dufYZinrr1daw\n" \
"UYOeR47Wn/hc3vzsYE8Zi+XJZyFLCQRM4t3oRmHw0S4zWWyjwTK7VzBgAAPwrrW5\n" \
"65R2ZwKBgQDqkXOZuLLrAMJvfimys7KrlnoCuVBOUOzhk85RHgNsxAz59DoEIjID\n" \
"M8i75ERSxe4Ik/+i5zHEgtMKaQ6oLnFYBYKEKZ75yQDXLjU4LuaHCbCB/YusPNFm\n" \
"JLJsBR2YdvagxNtkD7c30M7Z73mTn417Gh8k5ICOFPiNGmNstyu3dQ==\n" \
"-----END RSA PRIVATE KEY-----"


/* PEM-encoded Root CA certificate */
// AmazonRootCA1.pem.txt
#define ROOT_CA_CERTIFICATE     \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----"

#else
#define CLIENT_CERTIFICATE      \
"-----BEGIN CERTIFICATE-----\n" \
"........base64 data........\n" \
"-----END CERTIFICATE-----"

/* PEM-encoded client private key */
#define CLIENT_PRIVATE_KEY          \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"..........base64 data..........\n" \
"-----END RSA PRIVATE KEY-----"

/* PEM-encoded Root CA certificate */
#define ROOT_CA_CERTIFICATE     \
"-----BEGIN CERTIFICATE-----\n" \
"........base64 data........\n" \
"-----END CERTIFICATE-----"
#endif

/******************************************************************************
* Global Variables
*******************************************************************************/
extern cy_mqtt_broker_info_t broker_info;
extern cy_awsport_ssl_credentials_t  *security_info;
extern cy_mqtt_connect_info_t connection_info;


#endif /* MQTT_CLIENT_CONFIG_H_ */
