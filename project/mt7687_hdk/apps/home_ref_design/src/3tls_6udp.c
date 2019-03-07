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


#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#endif

#include "syslog.h"
#include "3tls_6udp.h"

log_create_module(_3tls_6udp, PRINT_LEVEL_INFO);

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_ENTROPY_C) ||  \
    !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_CLI_C) || \
    !defined(MBEDTLS_NET_C) || !defined(MBEDTLS_RSA_C) ||         \
    !defined(MBEDTLS_PEM_PARSE_C) || \
    !defined(MBEDTLS_CTR_DRBG_C) || !defined(MBEDTLS_X509_CRT_PARSE_C)
/**
* @brief           This function is a demonstration of ssl client.
* @return         0, if OK.\n
*                     Error code, if errors occurred.\n
*/
uint8_t _3tls_6udp_main(uint8_t len, char *param[])
{
    LOG_I(_3tls_6udp, "MBEDTLS_BIGNUM_C and/or MBEDTLS_ENTROPY_C and/or "
    "MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_CLI_C and/or "
    "MBEDTLS_NET_C and/or MBEDTLS_RSA_C and/or "
    "MBEDTLS_CTR_DRBG_C and/or MBEDTLS_X509_CRT_PARSE_C "
    "not defined.\n");
    return( 0 );
}

#else
#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "ethernetif.h"
#include "sockets.h"
#include "netdb.h"
#include <string.h>
#include "task.h"

typedef struct
{
    mbedtls_ssl_context ssl_ctx;    /* mbedtls ssl context */
    mbedtls_net_context net_ctx;    /* Fill in socket id */
    mbedtls_ssl_config ssl_conf;    /* SSL configuration */
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_x509_crt_profile profile;
    mbedtls_x509_crt cacert;    /* AppleHomekit CA */
    mbedtls_x509_crt clicert;   /* Accessory Certification */
    mbedtls_pk_context pkey;    /* Accessory LTSK */
}_3tls_context_struct;


#define _3TLS_SERVER_PORT  "443"
#if defined(MBEDTLS_DEBUG_C)
#define _3TLS_DEBUG_LEVEL 0
#endif


static char _3tls_test_type[5];
static char _3tls_server_name[50];
static char _6udp_server_name[50];
static char _6udp_server_port[6];

/* RSA MBEDTLS */
const char _3tls_6udp_svr_ca_crt_rsa[] =                                \
"-----BEGIN CERTIFICATE-----\r\n"                                       \
"MIIDhzCCAm+gAwIBAgIBADANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDER\r\n"  \
"MA8GA1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\r\n"  \
"MTEwMjEyMTQ0NDAwWhcNMjEwMjEyMTQ0NDAwWjA7MQswCQYDVQQGEwJOTDERMA8G\r\n"  \
"A1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwggEiMA0G\r\n"  \
"CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDA3zf8F7vglp0/ht6WMn1EpRagzSHx\r\n"  \
"mdTs6st8GFgIlKXsm8WL3xoemTiZhx57wI053zhdcHgH057Zk+i5clHFzqMwUqny\r\n"  \
"50BwFMtEonILwuVA+T7lpg6z+exKY8C4KQB0nFc7qKUEkHHxvYPZP9al4jwqj+8n\r\n"  \
"YMPGn8u67GB9t+aEMr5P+1gmIgNb1LTV+/Xjli5wwOQuvfwu7uJBVcA0Ln0kcmnL\r\n"  \
"R7EUQIN9Z/SG9jGr8XmksrUuEvmEF/Bibyc+E1ixVA0hmnM3oTDPb5Lc9un8rNsu\r\n"  \
"KNF+AksjoBXyOGVkCeoMbo4bF6BxyLObyavpw/LPh5aPgAIynplYb6LVAgMBAAGj\r\n"  \
"gZUwgZIwDAYDVR0TBAUwAwEB/zAdBgNVHQ4EFgQUtFrkpbPe0lL2udWmlQ/rPrzH\r\n"  \
"/f8wYwYDVR0jBFwwWoAUtFrkpbPe0lL2udWmlQ/rPrzH/f+hP6Q9MDsxCzAJBgNV\r\n"  \
"BAYTAk5MMREwDwYDVQQKEwhQb2xhclNTTDEZMBcGA1UEAxMQUG9sYXJTU0wgVGVz\r\n"  \
"dCBDQYIBADANBgkqhkiG9w0BAQUFAAOCAQEAuP1U2ABUkIslsCfdlc2i94QHHYeJ\r\n"  \
"SsR4EdgHtdciUI5I62J6Mom+Y0dT/7a+8S6MVMCZP6C5NyNyXw1GWY/YR82XTJ8H\r\n"  \
"DBJiCTok5DbZ6SzaONBzdWHXwWwmi5vg1dxn7YxrM9d0IjxM27WNKs4sDQhZBQkF\r\n"  \
"pjmfs2cb4oPl4Y9T9meTx/lvdkRYEug61Jfn6cA+qHpyPYdTH+UshITnmp5/Ztkf\r\n"  \
"m/UTSLBNFNHesiTZeH31NcxYGdHSme9Nc/gfidRa0FLOCfWxRlFqAI47zG9jAQCZ\r\n"  \
"7Z2mCGDNMhjQc+BYcdnl0lPXjdDK6V0qCg1dVewhUBcW5gZKzV7e9+DpVA==\r\n"      \
"-----END CERTIFICATE-----\r\n";

const char _3tls_6udp_cli_crt_rsa[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDPzCCAiegAwIBAgIBBDANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDER\r\n"
"MA8GA1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\r\n"
"MTEwMjEyMTQ0NDA3WhcNMjEwMjEyMTQ0NDA3WjA8MQswCQYDVQQGEwJOTDERMA8G\r\n"
"A1UEChMIUG9sYXJTU0wxGjAYBgNVBAMTEVBvbGFyU1NMIENsaWVudCAyMIIBIjAN\r\n"
"BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyHTEzLn5tXnpRdkUYLB9u5Pyax6f\r\n"
"M60Nj4o8VmXl3ETZzGaFB9X4J7BKNdBjngpuG7fa8H6r7gwQk4ZJGDTzqCrSV/Uu\r\n"
"1C93KYRhTYJQj6eVSHD1bk2y1RPD0hrt5kPqQhTrdOrA7R/UV06p86jt0uDBMHEw\r\n"
"MjDV0/YI0FZPRo7yX/k9Z5GIMC5Cst99++UMd//sMcB4j7/Cf8qtbCHWjdmLao5v\r\n"
"4Jv4EFbMs44TFeY0BGbH7vk2DmqV9gmaBmf0ZXH4yqSxJeD+PIs1BGe64E92hfx/\r\n"
"/DZrtenNLQNiTrM9AM+vdqBpVoNq0qjU51Bx5rU2BXcFbXvI5MT9TNUhXwIDAQAB\r\n"
"o00wSzAJBgNVHRMEAjAAMB0GA1UdDgQWBBRxoQBzckAvVHZeM/xSj7zx3WtGITAf\r\n"
"BgNVHSMEGDAWgBS0WuSls97SUva51aaVD+s+vMf9/zANBgkqhkiG9w0BAQUFAAOC\r\n"
"AQEAAn86isAM8X+mVwJqeItt6E9slhEQbAofyk+diH1Lh8Y9iLlWQSKbw/UXYjx5\r\n"
"LLPZcniovxIcARC/BjyZR9g3UwTHNGNm+rwrqa15viuNOFBchykX/Orsk02EH7NR\r\n"
"Alw5WLPorYjED6cdVQgBl9ot93HdJogRiXCxErM7NC8/eP511mjq+uLDjLKH8ZPQ\r\n"
"8I4ekHJnroLsDkIwXKGIsvIBHQy2ac/NwHLCQOK6mfum1pRx52V4Utu5dLLjD5bM\r\n"
"xOBC7KU4xZKuMXXZM6/93Yb51K/J4ahf1TxJlTWXtnzDr9saEYdNy2SKY/6ZiDNH\r\n"
"D+stpAKiQLAWaAusIWKYEyw9MQ==\r\n"
"-----END CERTIFICATE-----\r\n";

const char _3tls_6udp_cli_key_rsa[] =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEpAIBAAKCAQEAyHTEzLn5tXnpRdkUYLB9u5Pyax6fM60Nj4o8VmXl3ETZzGaF\r\n"
"B9X4J7BKNdBjngpuG7fa8H6r7gwQk4ZJGDTzqCrSV/Uu1C93KYRhTYJQj6eVSHD1\r\n"
"bk2y1RPD0hrt5kPqQhTrdOrA7R/UV06p86jt0uDBMHEwMjDV0/YI0FZPRo7yX/k9\r\n"
"Z5GIMC5Cst99++UMd//sMcB4j7/Cf8qtbCHWjdmLao5v4Jv4EFbMs44TFeY0BGbH\r\n"
"7vk2DmqV9gmaBmf0ZXH4yqSxJeD+PIs1BGe64E92hfx//DZrtenNLQNiTrM9AM+v\r\n"
"dqBpVoNq0qjU51Bx5rU2BXcFbXvI5MT9TNUhXwIDAQABAoIBAGdNtfYDiap6bzst\r\n"
"yhCiI8m9TtrhZw4MisaEaN/ll3XSjaOG2dvV6xMZCMV+5TeXDHOAZnY18Yi18vzz\r\n"
"4Ut2TnNFzizCECYNaA2fST3WgInnxUkV3YXAyP6CNxJaCmv2aA0yFr2kFVSeaKGt\r\n"
"ymvljNp2NVkvm7Th8fBQBO7I7AXhz43k0mR7XmPgewe8ApZOG3hstkOaMvbWAvWA\r\n"
"zCZupdDjZYjOJqlA4eEA4H8/w7F83r5CugeBE8LgEREjLPiyejrU5H1fubEY+h0d\r\n"
"l5HZBJ68ybTXfQ5U9o/QKA3dd0toBEhhdRUDGzWtjvwkEQfqF1reGWj/tod/gCpf\r\n"
"DFi6X0ECgYEA4wOv/pjSC3ty6TuOvKX2rOUiBrLXXv2JSxZnMoMiWI5ipLQt+RYT\r\n"
"VPafL/m7Dn6MbwjayOkcZhBwk5CNz5A6Q4lJ64Mq/lqHznRCQQ2Mc1G8eyDF/fYL\r\n"
"Ze2pLvwP9VD5jTc2miDfw+MnvJhywRRLcemDFP8k4hQVtm8PMp3ZmNECgYEA4gz7\r\n"
"wzObR4gn8ibe617uQPZjWzUj9dUHYd+in1gwBCIrtNnaRn9I9U/Q6tegRYpii4ys\r\n"
"c176NmU+umy6XmuSKV5qD9bSpZWG2nLFnslrN15Lm3fhZxoeMNhBaEDTnLT26yoi\r\n"
"33gp0mSSWy94ZEqipms+ULF6sY1ZtFW6tpGFoy8CgYAQHhnnvJflIs2ky4q10B60\r\n"
"ZcxFp3rtDpkp0JxhFLhiizFrujMtZSjYNm5U7KkgPVHhLELEUvCmOnKTt4ap/vZ0\r\n"
"BxJNe1GZH3pW6SAvGDQpl9sG7uu/vTFP+lCxukmzxB0DrrDcvorEkKMom7ZCCRvW\r\n"
"KZsZ6YeH2Z81BauRj218kQKBgQCUV/DgKP2985xDTT79N08jUo3hTP5MVYCCuj/+\r\n"
"UeEw1TvZcx3LJby7P6Xad6a1/BqveaGyFKIfEFIaBUBItk801sDDpDaYc4gL00Xc\r\n"
"7lFuBHOZkxJYlss5QrGpuOEl9ZwUt5IrFLBdYaKqNHzNVC1pCPfb/JyH6Dr2HUxq\r\n"
"gxUwAQKBgQCcU6G2L8AG9d9c0UpOyL1tMvFe5Ttw0KjlQVdsh1MP6yigYo9DYuwu\r\n"
"bHFVW2r0dBTqegP2/KTOxKzaHfC1qf0RGDsUoJCNJrd1cwoCLG8P2EF4w3OBrKqv\r\n"
"8u4ytY0F+Vlanj5lm3TaoHSVF1+NWPyOTiwevIECGKwSxvlki4fDAA==\r\n"
"-----END RSA PRIVATE KEY-----\r\n";

/* EC MBEDTLS */
const char _3tls_6udp_svr_ca_crt_ec[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIICUjCCAdegAwIBAgIJAMFD4n5iQ8zoMAoGCCqGSM49BAMCMD4xCzAJBgNVBAYT\r\n"
"Ak5MMREwDwYDVQQKEwhQb2xhclNTTDEcMBoGA1UEAxMTUG9sYXJzc2wgVGVzdCBF\r\n"
"QyBDQTAeFw0xMzA5MjQxNTQ5NDhaFw0yMzA5MjIxNTQ5NDhaMD4xCzAJBgNVBAYT\r\n"
"Ak5MMREwDwYDVQQKEwhQb2xhclNTTDEcMBoGA1UEAxMTUG9sYXJzc2wgVGVzdCBF\r\n"
"QyBDQTB2MBAGByqGSM49AgEGBSuBBAAiA2IABMPaKzRBN1gvh1b+/Im6KUNLTuBu\r\n"
"ww5XUzM5WNRStJGVOQsj318XJGJI/BqVKc4sLYfCiFKAr9ZqqyHduNMcbli4yuiy\r\n"
"aY7zQa0pw7RfdadHb9UZKVVpmlM7ILRmFmAzHqOBoDCBnTAdBgNVHQ4EFgQUnW0g\r\n"
"JEkBPyvLeLUZvH4kydv7NnwwbgYDVR0jBGcwZYAUnW0gJEkBPyvLeLUZvH4kydv7\r\n"
"NnyhQqRAMD4xCzAJBgNVBAYTAk5MMREwDwYDVQQKEwhQb2xhclNTTDEcMBoGA1UE\r\n"
"AxMTUG9sYXJzc2wgVGVzdCBFQyBDQYIJAMFD4n5iQ8zoMAwGA1UdEwQFMAMBAf8w\r\n"
"CgYIKoZIzj0EAwIDaQAwZgIxAMO0YnNWKJUAfXgSJtJxexn4ipg+kv4znuR50v56\r\n"
"t4d0PCu412mUC6Nnd7izvtE2MgIxAP1nnJQjZ8BWukszFQDG48wxCCyci9qpdSMv\r\n"
"uCjn8pwUOkABXK8Mss90fzCfCEOtIA==\r\n"
"-----END CERTIFICATE-----\r\n";

const char _3tls_6udp_cli_crt_ec[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIICLDCCAbKgAwIBAgIBDTAKBggqhkjOPQQDAjA+MQswCQYDVQQGEwJOTDERMA8G\r\n"
"A1UEChMIUG9sYXJTU0wxHDAaBgNVBAMTE1BvbGFyc3NsIFRlc3QgRUMgQ0EwHhcN\r\n"
"MTMwOTI0MTU1MjA0WhcNMjMwOTIyMTU1MjA0WjBBMQswCQYDVQQGEwJOTDERMA8G\r\n"
"A1UEChMIUG9sYXJTU0wxHzAdBgNVBAMTFlBvbGFyU1NMIFRlc3QgQ2xpZW50IDIw\r\n"
"WTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARX5a6xc9/TrLuTuIH/Eq7u5lOszlVT\r\n"
"9jQOzC7jYyUL35ji81xgNpbA1RgUcOV/n9VLRRjlsGzVXPiWj4dwo+THo4GdMIGa\r\n"
"MAkGA1UdEwQCMAAwHQYDVR0OBBYEFHoAX4Zk/OBd5REQO7LmO8QmP8/iMG4GA1Ud\r\n"
"IwRnMGWAFJ1tICRJAT8ry3i1Gbx+JMnb+zZ8oUKkQDA+MQswCQYDVQQGEwJOTDER\r\n"
"MA8GA1UEChMIUG9sYXJTU0wxHDAaBgNVBAMTE1BvbGFyc3NsIFRlc3QgRUMgQ0GC\r\n"
"CQDBQ+J+YkPM6DAKBggqhkjOPQQDAgNoADBlAjBKZQ17IIOimbmoD/yN7o89u3BM\r\n"
"lgOsjnhw3fIOoLIWy2WOGsk/LGF++DzvrRzuNiACMQCd8iem1XS4JK7haj8xocpU\r\n"
"LwjQje5PDGHfd3h9tP38Qknu5bJqws0md2KOKHyeV0U=\r\n"
"-----END CERTIFICATE-----\r\n";

const char _3tls_6udp_cli_key_ec[] =
"-----BEGIN EC PRIVATE KEY-----\r\n"
"MHcCAQEEIPb3hmTxZ3/mZI3vyk7p3U3wBf+WIop6hDhkFzJhmLcqoAoGCCqGSM49\r\n"
"AwEHoUQDQgAEV+WusXPf06y7k7iB/xKu7uZTrM5VU/Y0Dswu42MlC9+Y4vNcYDaW\r\n"
"wNUYFHDlf5/VS0UY5bBs1Vz4lo+HcKPkxw==\r\n"
"-----END EC PRIVATE KEY-----\r\n";

/* EC 384 */
const char _3tls_6udp_svr_ca_crt_384[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIICaTCCAe6gAwIBAgIJAMkg9wUDHkTZMAoGCCqGSM49BAMCMHIxCzAJBgNVBAYT\r\n"
"AkNOMQswCQYDVQQIDAJTQzELMAkGA1UEBwwCQ0QxDDAKBgNVBAoMA01USzEMMAoG\r\n"
"A1UECwwDTVRLMQwwCgYDVQQDDANNVEsxHzAdBgkqhkiG9w0BCQEWEE1US0BtZWRp\r\n"
"YXRlay5jb20wHhcNMTYwNzE0MDgyMTAxWhcNMjYwNzEyMDgyMTAxWjByMQswCQYD\r\n"
"VQQGEwJDTjELMAkGA1UECAwCU0MxCzAJBgNVBAcMAkNEMQwwCgYDVQQKDANNVEsx\r\n"
"DDAKBgNVBAsMA01USzEMMAoGA1UEAwwDTVRLMR8wHQYJKoZIhvcNAQkBFhBNVEtA\r\n"
"bWVkaWF0ZWsuY29tMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEM2frz/CiKAQjvGnO\r\n"
"kWYHnbMYKun0KlGPcT/sTxXUoq9N0L4j4uVmfFv8ohulkNnaWuvRUuO/arR4Jr6M\r\n"
"QyZC9EhVjCuEA+7x1RoL1cjMRbvrwHLyfOJ1ZxjSpYe8D95No1AwTjAdBgNVHQ4E\r\n"
"FgQUsJSG8dTVU4sr9+p1Gkb9g8Oh+TUwHwYDVR0jBBgwFoAUsJSG8dTVU4sr9+p1\r\n"
"Gkb9g8Oh+TUwDAYDVR0TBAUwAwEB/zAKBggqhkjOPQQDAgNpADBmAjEAzmACI/CH\r\n"
"7uZALiNIPPKEh+yZMAIZFMn7Rlmi0MmnjlLSamjHsekNaGKyjrM4l01wAjEAzmTz\r\n"
"wpiBmP91ryYvqx4OS2S2jnw0qg6G6J11wmlLeP3r/Cj+Cw8VPYJ+0G5+jTYG\r\n"
"-----END CERTIFICATE-----\r\n";

const char _3tls_6udp_cli_crt_384[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIICaTCCAe6gAwIBAgIJAMkg9wUDHkTZMAoGCCqGSM49BAMCMHIxCzAJBgNVBAYT\r\n"
"AkNOMQswCQYDVQQIDAJTQzELMAkGA1UEBwwCQ0QxDDAKBgNVBAoMA01USzEMMAoG\r\n"
"A1UECwwDTVRLMQwwCgYDVQQDDANNVEsxHzAdBgkqhkiG9w0BCQEWEE1US0BtZWRp\r\n"
"YXRlay5jb20wHhcNMTYwNzE0MDgyMTAxWhcNMjYwNzEyMDgyMTAxWjByMQswCQYD\r\n"
"VQQGEwJDTjELMAkGA1UECAwCU0MxCzAJBgNVBAcMAkNEMQwwCgYDVQQKDANNVEsx\r\n"
"DDAKBgNVBAsMA01USzEMMAoGA1UEAwwDTVRLMR8wHQYJKoZIhvcNAQkBFhBNVEtA\r\n"
"bWVkaWF0ZWsuY29tMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEM2frz/CiKAQjvGnO\r\n"
"kWYHnbMYKun0KlGPcT/sTxXUoq9N0L4j4uVmfFv8ohulkNnaWuvRUuO/arR4Jr6M\r\n"
"QyZC9EhVjCuEA+7x1RoL1cjMRbvrwHLyfOJ1ZxjSpYe8D95No1AwTjAdBgNVHQ4E\r\n"
"FgQUsJSG8dTVU4sr9+p1Gkb9g8Oh+TUwHwYDVR0jBBgwFoAUsJSG8dTVU4sr9+p1\r\n"
"Gkb9g8Oh+TUwDAYDVR0TBAUwAwEB/zAKBggqhkjOPQQDAgNpADBmAjEAzmACI/CH\r\n"
"7uZALiNIPPKEh+yZMAIZFMn7Rlmi0MmnjlLSamjHsekNaGKyjrM4l01wAjEAzmTz\r\n"
"wpiBmP91ryYvqx4OS2S2jnw0qg6G6J11wmlLeP3r/Cj+Cw8VPYJ+0G5+jTYG\r\n"
"-----END CERTIFICATE-----\r\n";

const char _3tls_6udp_cli_key_384[] =
"-----BEGIN PRIVATE KEY-----\r\n"
"MIG2AgEAMBAGByqGSM49AgEGBSuBBAAiBIGeMIGbAgEBBDDnsEtpsjUnkVe4u2GP\r\n"
"2lIucJoARr/xK7R6Pa/9MmetFreYvYjErU67UsIFVmy49nqhZANiAAQzZ+vP8KIo\r\n"
"BCO8ac6RZgedsxgq6fQqUY9xP+xPFdSir03QviPi5WZ8W/yiG6WQ2dpa69FS479q\r\n"
"tHgmvoxDJkL0SFWMK4QD7vHVGgvVyMxFu+vAcvJ84nVnGNKlh7wP3k0=\r\n"
"-----END PRIVATE KEY-----\r\n";


static void _6udp_client_start(unsigned int index)
{
    int ret = -1, s = -1;
    struct addrinfo hints, *addr_list, *cur;

    LOG_I(_3tls_6udp, "************udp start: %d***************\r\n", index);

    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    if (getaddrinfo(_6udp_server_name, _6udp_server_port, &hints, &addr_list ) != 0)
        return;

    /* Try the sockaddrs until a connection succeeds */
    for (cur = addr_list; cur != NULL; cur = cur->ai_next)
    {
        s = (int)lwip_socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if(s < 0)
        {
            ret = -2;
            continue;
        }

        if(lwip_connect(s, cur->ai_addr, cur->ai_addrlen ) == 0)
        {
            ret = 0;
            break;
        }

        lwip_close(s);
        ret = -3;
    }

    freeaddrinfo(addr_list);

    LOG_I(_3tls_6udp, "************udp end, ret:%d***************\r\n", ret);

    return;
}


static void _6udp_client_test(void)
{
    LOG_I(_3tls_6udp, "UDP server:%s, port:%s\r\n", _6udp_server_name, _6udp_server_port);

    _6udp_client_start(0);
    _6udp_client_start(1);
    _6udp_client_start(2);
    _6udp_client_start(3);
    _6udp_client_start(4);
    _6udp_client_start(5);
}


static void my_debug(void *ctx, int level,
                         const char *file, int line,
                         const char *str )
{
    LOG_I(_3tls_6udp, "mbedtls[%d]%s:%04d: %s\r\n", level, file, line, str);
}


static void *_3tls_tls_connect(char *server_name, char *port, unsigned int *err,
                                  unsigned int svr_cert_len, const char *svr_cert,
                                  unsigned int cli_cert_len, const char *cli_cert,
                                  unsigned int cli_pk_len, const char *cli_pk)
{
    _3tls_context_struct *ssl = NULL;
    unsigned int authmode = MBEDTLS_SSL_VERIFY_NONE;
    const char *pers = "3tls_6udp";
    unsigned int flags = 0;
    int value, ret = 0;

    LOG_I(_3tls_6udp, "%s\r\n", __FUNCTION__);

    if (!server_name || !port ||
    (!svr_cert_len && svr_cert) || (svr_cert_len && !svr_cert) ||
    (!cli_cert_len && cli_cert) || (cli_cert_len && !cli_cert) ||
    (!cli_pk_len && cli_pk) || (cli_pk_len && !cli_pk) ||
    (cli_cert && !cli_pk) || (!cli_cert && cli_pk))
    {
        LOG_I(_3tls_6udp, "Parameter error\r\n");
        ret = -1;
        goto exit;
    }

    ssl = (_3tls_context_struct *)pvPortCalloc(1, sizeof(_3tls_context_struct));
    if (!ssl)
    {
        LOG_I(_3tls_6udp, "No memory\r\n");
        ret = -3;
        goto exit;
    }

    #if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(_3TLS_DEBUG_LEVEL);
    #endif

    if (svr_cert_len && svr_cert)
    {
        authmode = MBEDTLS_SSL_VERIFY_REQUIRED;
    }

    mbedtls_net_init(&ssl->net_ctx);
    mbedtls_ssl_init(&ssl->ssl_ctx);
    mbedtls_ssl_config_init(&ssl->ssl_conf);
    mbedtls_x509_crt_init(&ssl->cacert);
    mbedtls_x509_crt_init(&ssl->clicert);
    mbedtls_pk_init(&ssl->pkey);
    mbedtls_ctr_drbg_init(&ssl->ctr_drbg);

    /*
        * 0. Initialize the RNG and the session data
        */
    mbedtls_entropy_init(&ssl->entropy);
    if ((value = mbedtls_ctr_drbg_seed(&ssl->ctr_drbg,
        mbedtls_entropy_func,
        &ssl->entropy,
        (const uint8_t*)pers,
        strlen(pers))) != 0)
    {
        LOG_I(_3tls_6udp, "mbedtls_ctr_drbg_seed() failed, value:-0x%x\r\n", -value);

        ret = -1;
        goto exit;
    }

    /*
        * 0. Load the Client certificate
        */
    ret = mbedtls_x509_crt_parse(&ssl->clicert, (const uint8_t *)cli_cert, cli_cert_len);

    if(ret < 0)
    {
        LOG_I(_3tls_6udp, "Loading cli_cert failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        goto exit;
    }

    ret = mbedtls_pk_parse_key(&ssl->pkey, (const uint8_t *)cli_pk, cli_pk_len, NULL, 0);

    if(ret != 0)
    {
        LOG_I(_3tls_6udp, " failed\n  !  mbedtls_pk_parse_key returned -0x%x\n\n", -ret);
        goto exit;
    }

    /*
        * 1. Load the trusted CA
        */

    /* cert_len passed in is gotten from sizeof not strlen */
    if (svr_cert && ((value = mbedtls_x509_crt_parse(&ssl->cacert,
        (const uint8_t *)svr_cert,
        svr_cert_len)) < 0))
    {
        LOG_I(_3tls_6udp, "mbedtls_x509_crt_parse() failed, value:-0x%x\r\n", -value);
        ret = -1;
        goto exit;
    }

    if((ret = mbedtls_net_connect(&ssl->net_ctx, server_name, port, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        LOG_I(_3tls_6udp, " failed\n  ! mbedtls_net_connect returned %d, port:%s\n\n", ret, port);
        goto exit;
    }

    /*
        * 2. Setup stuff
        */
    if((value = mbedtls_ssl_config_defaults(&ssl->ssl_conf,
        MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_STREAM,
        MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        LOG_I(_3tls_6udp, "mbedtls_ssl_config_defaults() failed, value:-0x%x\r\n", -value);
        ret = -1;
        goto exit;
    }

    // memcpy(&ssl->profile, ssl->ssl_conf.cert_profile, sizeof(mbedtls_x509_crt_profile));
    // ssl->profile.allowed_mds = ssl->profile.allowed_mds | MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_MD5 );
    // mbedtls_ssl_conf_cert_profile(&ssl->ssl_conf, &ssl->profile);

    mbedtls_ssl_conf_authmode(&ssl->ssl_conf, authmode);
    mbedtls_ssl_conf_ca_chain(&ssl->ssl_conf, &ssl->cacert, NULL);

    if((ret = mbedtls_ssl_conf_own_cert(&ssl->ssl_conf, &ssl->clicert, &ssl->pkey)) != 0)
    {
        LOG_I(_3tls_6udp, " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_ssl_conf_rng(&ssl->ssl_conf,
    mbedtls_ctr_drbg_random,
    &ssl->ctr_drbg);

    mbedtls_ssl_conf_dbg(&ssl->ssl_conf, my_debug, NULL);

    if ((value = mbedtls_ssl_setup(&ssl->ssl_ctx,
        &ssl->ssl_conf)) != 0)
    {
        LOG_I(_3tls_6udp, "mbedtls_ssl_setup() failed, value:-0x%x\r\n", -value);

        ret = -1;
        goto exit;
    }

    mbedtls_ssl_set_bio(&ssl->ssl_ctx,
                        &ssl->net_ctx,
                        mbedtls_net_send,
                        mbedtls_net_recv,
                        NULL);

    /*
        * 3. Handshake
        */
    LOG_I(_3tls_6udp, "[FREE_HEAP_SIZE]%s : size:%d\n", "Before Handshake API", xPortGetFreeHeapSize());
    while ((ret = mbedtls_ssl_handshake(&ssl->ssl_ctx)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            LOG_I(_3tls_6udp, "mbedtls_ssl_handshake() failed, ret:-0x%x\r\n", -ret);

            ret = -1;
            goto exit;
        }
    }

    LOG_I(_3tls_6udp, "[FREE_HEAP_SIZE]%s : size:%d\n", "After Handshake API", xPortGetFreeHeapSize());

    /*
        * 4. Verify the server certificate
        */

    /* In real life, we would have used MBEDTLS_SSL_VERIFY_REQUIRED so that the
        * handshake would not succeed if the peer's cert is bad.  Even if we used
        * MBEDTLS_SSL_VERIFY_OPTIONAL, we would bail out here if ret != 0
        */

    if((flags = mbedtls_ssl_get_verify_result(&ssl->ssl_ctx)) != 0)
    {
        char vrfy_buf[512];

        LOG_I(_3tls_6udp, "svr_cert varification failed\n");

        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags);

        LOG_I(_3tls_6udp, "%s\n", vrfy_buf);
    }
    else
        LOG_I(_3tls_6udp, "svr_cert varification ok\n");

exit:
    LOG_I(_3tls_6udp, "%s : ret=%d\n", __FUNCTION__, ret);

    if (err)
    {
        *err = ret;
    }

    if (0 != ret && ssl)
    {
        mbedtls_net_free(&ssl->net_ctx);
        mbedtls_x509_crt_free(&ssl->cacert);
        mbedtls_x509_crt_free(&ssl->clicert);
        mbedtls_pk_free(&ssl->pkey);
        mbedtls_ssl_free(&ssl->ssl_ctx);
        mbedtls_ssl_config_free(&ssl->ssl_conf);
        mbedtls_ctr_drbg_free(&ssl->ctr_drbg);
        mbedtls_entropy_free(&ssl->entropy);

        vPortFree(ssl);
        ssl = NULL;

        return NULL;
    }

    return (void *)ssl;
}


static unsigned int _3tls_client_test(void)
{
    void *ssl_contx0 = NULL, *ssl_contx1 = NULL, *ssl_contx2 = NULL;
    unsigned int err = 0;
    const char * ca_crt = NULL, *svr_crt = NULL, *cli_key = NULL;
    unsigned int ca_crt_len = 0, svr_crt_len = 0, cli_key_len = 0;

    LOG_I(_3tls_6udp, "TLS test_type:%s, server:%s\r\n", _3tls_test_type, _3tls_server_name);

    if (strcmp(_3tls_test_type, "rsa") == 0)
    {
        LOG_I(_3tls_6udp, "rsa\r\n");
        ca_crt = _3tls_6udp_svr_ca_crt_rsa;
        ca_crt_len = sizeof(_3tls_6udp_svr_ca_crt_rsa);

        svr_crt = _3tls_6udp_cli_crt_rsa;
        svr_crt_len = sizeof(_3tls_6udp_cli_crt_rsa);

        cli_key = _3tls_6udp_cli_key_rsa;
        cli_key_len = sizeof(_3tls_6udp_cli_key_rsa);
    }
    else if (strcmp(_3tls_test_type, "ec") == 0)
    {
        LOG_I(_3tls_6udp, "ec\r\n");
        ca_crt = _3tls_6udp_svr_ca_crt_ec;
        ca_crt_len = sizeof(_3tls_6udp_svr_ca_crt_ec);

        svr_crt = _3tls_6udp_cli_crt_ec;
        svr_crt_len = sizeof(_3tls_6udp_cli_crt_ec);

        cli_key = _3tls_6udp_cli_key_ec;
        cli_key_len = sizeof(_3tls_6udp_cli_key_ec);
    }
    else if (strcmp(_3tls_test_type, "ec_384") == 0)
    {
        LOG_I(_3tls_6udp, "ec_384\r\n");
        ca_crt = _3tls_6udp_svr_ca_crt_384;
        ca_crt_len = sizeof(_3tls_6udp_svr_ca_crt_384);

        svr_crt = _3tls_6udp_cli_crt_384;
        svr_crt_len = sizeof(_3tls_6udp_cli_crt_384);

        cli_key = _3tls_6udp_cli_key_384;
        cli_key_len = sizeof(_3tls_6udp_cli_key_384);
    }
    else
    {
        LOG_I(_3tls_6udp, "no matched test_type\r\n");
        return -1;
    }

    LOG_I(_3tls_6udp, "[FREE_HEAP_SIZE]%s : size:%d\n", "ssl start 0", xPortGetFreeHeapSize());
    ssl_contx0 = _3tls_tls_connect(_3tls_server_name,
                                   _3TLS_SERVER_PORT,
                                   &err,
                                   ca_crt_len,
                                   ca_crt,
                                   svr_crt_len,
                                   svr_crt,
                                   cli_key_len,
                                   cli_key);
    LOG_I(_3tls_6udp, "[FREE_HEAP_SIZE]%s : size:%d\n", "ssl end 0", xPortGetFreeHeapSize());

    LOG_I(_3tls_6udp, "[FREE_HEAP_SIZE]%s : size:%d\n", "ssl start 1", xPortGetFreeHeapSize());
    ssl_contx1 = _3tls_tls_connect(_3tls_server_name,
                                   _3TLS_SERVER_PORT,
                                   &err,
                                   ca_crt_len,
                                   ca_crt,
                                   svr_crt_len,
                                   svr_crt,
                                   cli_key_len,
                                   cli_key);
    LOG_I(_3tls_6udp, "[FREE_HEAP_SIZE]%s : size:%d\n", "ssl end 1", xPortGetFreeHeapSize());

    LOG_I(_3tls_6udp, "[FREE_HEAP_SIZE]%s : size:%d\n", "ssl start 2", xPortGetFreeHeapSize());
    ssl_contx2 = _3tls_tls_connect(_3tls_server_name,
                                   _3TLS_SERVER_PORT,
                                   &err,
                                   ca_crt_len,
                                   ca_crt,
                                   svr_crt_len,
                                   svr_crt,
                                   cli_key_len,
                                   cli_key);
    LOG_I(_3tls_6udp, "[FREE_HEAP_SIZE]%s : size:%d\n", "ssl end 2", xPortGetFreeHeapSize());

    if (!ssl_contx0 || !ssl_contx1 || !ssl_contx2)
    {
      return -1;
    }

    return 0;
}


void _3tls_6udp_entry(void *args)
{
    _6udp_client_test();

    _3tls_client_test();

    while (1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}


/**
* @brief           This function creates the task for 3tls and 6udp tests.
* @return         0, if OK.\n
*                     Error code, if errors occurred.\n
*/
uint8_t _3tls_6udp_main(uint8_t len, char *param[])
{
    BaseType_t ret = pdPASS;
    
    /* Read parameters */
    memset(_3tls_test_type, 0, 5);
    memset(_3tls_server_name, 0, 50);
    memset(_6udp_server_name, 0, 50);
    memset(_6udp_server_port, 0, 6);

    if (param[0] && strlen(param[0]) < 5)
    {
        memcpy(_3tls_test_type, param[0], strlen(param[0]));
    }
    else
    {
        LOG_I(_3tls_6udp, "parameter error. ret:1");
        return 1;
    }

    if (param[1] && strlen(param[1]) < 50)
    {
        memcpy(_3tls_server_name, param[1], strlen(param[1]));
    }
    else
    {
        LOG_I(_3tls_6udp, "parameter error. ret:2");
        return 2;
    }

    if (param[2] && strlen(param[2]) < 50)
    {
        memcpy(_6udp_server_name, param[2], strlen(param[2]));
    }
    else
    {
        LOG_I(_3tls_6udp, "parameter error. ret:3");
        return 3;
    }

    if (param[3] && strlen(param[3]) < 6)
    {
        memcpy(_6udp_server_port, param[3], strlen(param[3]));
    }
    else
    {
        LOG_I(_3tls_6udp, "parameter error. ret:4");
        return 4;
    }

    ret = xTaskCreate(_3tls_6udp_entry, "3tls_6udp", 2048, NULL, 1, NULL);
    if (pdPASS != ret)
    {
        LOG_I(_3tls_6udp, "Failed to create task. ret:5");
        return 5;
    }
    
    return 0;
}


#endif
/*
 *  !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_ENTROPY_C) ||  \
 *  !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_CLI_C) || \
 *  !defined(MBEDTLS_NET_C) || !defined(MBEDTLS_RSA_C) ||         \
 *  !defined(MBEDTLS_PEM_PARSE_C) || \
 *  !defined(MBEDTLS_CTR_DRBG_C) || !defined(MBEDTLS_X509_CRT_PARSE_C)
 */
