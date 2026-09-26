/**
 * @file app_secrets.h
 * @brief Application Secrets, ThingsBoard Auto-Provisioning & Shared Configuration
 * 
 * @attribution
 * - Humidyne Labs / Humiditron
 * 
 * SPDX-License-Identifier: MIT
 */

#ifndef APP_SECRETS_H
#define APP_SECRETS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Fallback Wi-Fi Configuration (Empty strings trigger BLE Provisioning mode) */
#define CONFIG_FALLBACK_WIFI_SSID           ""
#define CONFIG_FALLBACK_WIFI_PASS           ""

/* ThingsBoard Broker Configuration */
#define CONFIG_THINGSBOARD_HOST             "humid1.com"
#define CONFIG_THINGSBOARD_PORT             8883
#define CONFIG_THINGSBOARD_URI              "mqtts://humid1.com:8883"

/* ThingsBoard Device Auto-Provisioning Keys */
#define CONFIG_THINGSBOARD_PROVISION_KEY    "joz5hqceqbkzzft3qzaf"
#define CONFIG_THINGSBOARD_PROVISION_SECRET "d9xpuylns0pdlk2w70hc"

/* Fallback static Access Token (leave empty to use dynamic auto-provisioning token) */
#define CONFIG_THINGSBOARD_ACCESS_TOKEN     ""

/* Optional CA Certificate in PEM format (NULL uses system CRT bundle) */
#define CONFIG_THINGSBOARD_CA_CERT          NULL

/* Device Claiming Configuration */
#define CONFIG_CLAIM_KEY_LENGTH             6      /* 6 to 8 characters (A-Z, 0-9) */
#define CONFIG_CLAIM_DURATION_MS            180000 /* 3 minutes (180,000 ms) */

/* BLE Provisioning Service Prefix */
#define CONFIG_BLE_PROV_PREFIX              "PROV_"

/* POSIX Timezone Configuration (US Eastern: EST5EDT with Daylight Saving Time) */
#define CONFIG_APP_TIMEZONE                 "EST5EDT,M3.2.0,M11.1.0"

#ifdef __cplusplus
}
#endif

#endif /* APP_SECRETS_H */
