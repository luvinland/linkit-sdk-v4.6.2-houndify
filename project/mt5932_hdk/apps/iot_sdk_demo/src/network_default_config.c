/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "os.h"

#include "network_default_config.h"

int32_t dhcp_config_init(void)
{
    return (USE_DHCP == 0) ? STA_IP_MODE_STATIC : STA_IP_MODE_DHCP;
}

int32_t tcpip_config_init(lwip_tcpip_config_t *tcpip_config)
{
    tcpip_config->ip_mode = dhcp_config_init();

    ip4addr_aton(STA_IPADDR, &(tcpip_config->sta_addr));
    ip4addr_aton(STA_NETMASK, &tcpip_config->sta_mask);
    ip4addr_aton(STA_GATEWAY, &tcpip_config->sta_gateway);

    ip4addr_aton(AP_IPADDR, &(tcpip_config->ap_addr));
    ip4addr_aton(AP_NETMASK, &tcpip_config->ap_mask);
    ip4addr_aton(AP_GATEWAY, &tcpip_config->ap_gateway);

    return 0;
}

void dhcpd_settings_init(const lwip_tcpip_config_t *tcpip_config,
                         dhcpd_settings_t *dhcpd_settings)
{
    strcpy((char *)dhcpd_settings->dhcpd_server_address, AP_IPADDR);
    strcpy((char *)dhcpd_settings->dhcpd_netmask,        AP_NETMASK);
    strcpy((char *)dhcpd_settings->dhcpd_gateway,        AP_GATEWAY);
    strcpy((char *)dhcpd_settings->dhcpd_primary_dns,    PRIMARY_DNS);
    strcpy((char *)dhcpd_settings->dhcpd_secondary_dns,  SECONDARY_DNS);
    strcpy((char *)dhcpd_settings->dhcpd_ip_pool_start,  IP_POOL_START);
    strcpy((char *)dhcpd_settings->dhcpd_ip_pool_end,    IP_POOL_END);
}

