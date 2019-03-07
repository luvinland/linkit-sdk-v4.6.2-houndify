#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "mt7686.h"

#include "hal.h"

#include "sys_init.h"
#include "task_def.h"

#include "bsp_gpio_ept_config.h"
#include "hal_sleep_manager.h"

#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
#include "hal_wdt.h"
#endif

#include "wifi_nvdm_config.h"
#include "wifi_lwip_helper.h"
#include "connsys_profile.h"
#include "get_profile_string.h"
#include "wifi_api.h"
#include "nvdm.h"
#include "hal_sys.h"
#include "misc.h"
#include "syslog.h"
#include "inet.h"
#include "dhcp.h"
#include "stats.h"
#include "netif.h"
#include "lwip/sockets.h"
#include "sntp.h"

#include "hound_client.h"
#include "hound_audio.h"
#include "hound_led.h"
#include "hound_snsr.h"

#define LOGE(fmt, arg...)	LOG_E(common, "Main: " fmt, ##arg)
#define LOGW(fmt, arg...)	LOG_W(common, "Main: " fmt, ##arg)
#define LOGI(fmt, arg...)	LOG_I(common, "Main: " fmt, ##arg)

#ifdef HOUND_HEAP_CHECK
extern "C" unsigned long hound_heap_size(void);
#endif

#define CLIENT_ID	"gl-qiubgzXMf_Vk37apgmQ=="
#define CLIENT_KEY	"LH73xzm2v4hZim_DFke6g3hdiUd2ocVn4UeD1G7XeGpRebUKgqD9ZbSZqzNAFt0UcRM-u5nrAybkNzmquQpWHw=="
#define USER_ID		"root@test-ThinkPad-X220"

#define sec_DELAY ( ( portTickType ) 1000 / portTICK_RATE_MS )

static SemaphoreHandle_t m_sntp = NULL;
static bool bSntpSuccess = FALSE;
static bool bNetReady = FALSE;

static SemaphoreHandle_t m_hound = NULL;

static void sntp_proc(void *args)
{
    hal_rtc_time_t r_time;
    hal_rtc_status_t ret = HAL_RTC_STATUS_OK;
	time_t		std_time;
	struct tm std_tm;

	memset(&std_tm, 0, sizeof(std_time));

    for (int i = 0 ; i < 30; i++) {
        ret = hal_rtc_get_time(&r_time);
        if (ret == HAL_RTC_STATUS_OK && 
			(r_time.rtc_year != 0 || r_time.rtc_mon != 1 || r_time.rtc_day != 1)) {
            LOGI("SNTP success [%d]", i);
            LOGI("cur_time[y(%d),m(%d),d(%d),w(%d)]", r_time.rtc_year, r_time.rtc_mon,
											r_time.rtc_day, r_time.rtc_week);
            LOGI("[%d]cur_time[%d:%d:%d]", ret, r_time.rtc_hour + 9, r_time.rtc_min,
									r_time.rtc_sec);
            sntp_stop();

		std_tm.tm_year = r_time.rtc_year + 100;	// years since 1900
		std_tm.tm_mon = r_time.rtc_mon - 1;		// months since January, 0-11
		std_tm.tm_mday = r_time.rtc_day;		// day of the month, 1-31
		std_tm.tm_hour = r_time.rtc_hour;		// hours since midnight, 0-23
		std_tm.tm_min = r_time.rtc_min;			// minutes after the hour, 0-59
		std_tm.tm_sec = r_time.rtc_sec;			// seconds after the minute, 0-60 (C++11), 0-61(C++98)
		std_tm.tm_isdst = -1;
		std_time = mktime(&std_tm);
		// calibrate hound moudule time
		hound_set_time(std_time - time(NULL));

		bSntpSuccess = TRUE;
            break;
        }
        vTaskDelay(sec_DELAY);
    }
	// sntp finished so notify whether starting hound client or not
	assert(pdTRUE == xSemaphoreGive(m_sntp));

    vTaskDelete(NULL);
}

static void sntp_client(void)
{
    hal_rtc_time_t r_time = {6,6,6,1,1,6,0};
    hal_rtc_status_t ret = HAL_RTC_STATUS_OK;

    ret = hal_rtc_set_time(&r_time);
	assert(ret == HAL_RTC_STATUS_OK);
    LOGI("Start SNTP Client\r\n");

    sntp_setservername(0, (char *)"time.windows.com");
    sntp_setservername(1, (char *)"1.kr.pool.ntp.org");
    sntp_init();

    xTaskCreate(sntp_proc, APP_SNTP_TASK_NAME,
							APP_SNTP_TASK_STACKSIZE / sizeof(portSTACK_TYPE), NULL,
							APP_SNTP_TASK_PRIO, NULL);
}

static int32_t wifi_handler(wifi_event_t event, uint8_t *payload, uint32_t length)
{
	if (event == WIFI_EVENT_IOT_INIT_COMPLETE)
	{
		LOGI("wifi_handler, %s init\r\n", payload[6] == WIFI_PORT_STA ? "STA" : "AP");
	}
	else if (event == WIFI_EVENT_IOT_CONNECTED)
	{
		LOGI("wifi_handler, connected %d\r\n", length);

		if (length >= 6)
		{
			LOGI("C %02X %02X %02X %02X %02X %02X\r\n",
										payload[0], payload[1], payload[2], payload[3],
										payload[4], payload[5]);
		}
	}
	else if (event == WIFI_EVENT_IOT_PORT_SECURE)
	{
		LOGI("wifi_handler, port secure %d\r\n", length);
	}
	else if (event == WIFI_EVENT_IOT_DISCONNECTED)
	{
		bNetReady = FALSE;
		LOGI("wifi_handler, disconnected %d\r\n", length);

		if (length >= 6)
		{
			LOGI("D %02X %02X %02X %02X %02X %02X %02X\r\n",
										payload[0], payload[1], payload[2], payload[3],
										payload[4], payload[5], payload[6]);
		}
	}

	return 1;
}

static int32_t scan_handler(wifi_event_t event, uint8_t *payload, uint32_t length)
{
	return 1;
}

static void wifi_init(void)
{
	wifi_cfg_t			wifi_config = {0};
	wifi_config_t		config = {0};
	wifi_config_ext_t	config_ext = {0};

	if (wifi_config_init(&wifi_config) != 0)
	{
		LOGE("wifi config init fail\r\n");
		return;
	}

	config.opmode = WIFI_MODE_STA_ONLY;

	memcpy(config.sta_config.ssid, wifi_config.sta_ssid, 32);
	config.sta_config.ssid_length = wifi_config.sta_ssid_len;
	config.sta_config.bssid_present = 0;
	memcpy(config.sta_config.password, wifi_config.sta_wpa_psk, 64);
	config.sta_config.password_length = wifi_config.sta_wpa_psk_len;
	config_ext.sta_wep_key_index_present = 1;
	config_ext.sta_wep_key_index = wifi_config.sta_default_key_id;
	config_ext.sta_auto_connect_present = 1;
	config_ext.sta_auto_connect = 1;

	memcpy(config.ap_config.ssid, wifi_config.ap_ssid, 32);
	config.ap_config.ssid_length = wifi_config.ap_ssid_len;
	memcpy(config.ap_config.password, wifi_config.ap_wpa_psk, 64);
	config.ap_config.password_length = wifi_config.ap_wpa_psk_len;
	config.ap_config.auth_mode = (wifi_auth_mode_t)wifi_config.ap_auth_mode;
	config.ap_config.encrypt_type = (wifi_encrypt_type_t)wifi_config.ap_encryp_type;
	config.ap_config.channel = wifi_config.ap_channel;
	config.ap_config.bandwidth = wifi_config.ap_bw;
	config.ap_config.bandwidth_ext = WIFI_BANDWIDTH_EXT_40MHZ_UP;
	config_ext.ap_wep_key_index_present = 1;
	config_ext.ap_wep_key_index = wifi_config.ap_default_key_id;
	config_ext.ap_hidden_ssid_enable_present = 1;
	config_ext.ap_hidden_ssid_enable = wifi_config.ap_hide_ssid;
	config_ext.sta_power_save_mode = (wifi_power_saving_mode_t)wifi_config.sta_power_save_mode;

	wifi_init(&config, &config_ext);
	if (wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE,
											wifi_handler) < 0)
	{
		LOGE("init complete reg error\r\n");
	}
	if (wifi_connection_register_event_handler(WIFI_EVENT_IOT_SCAN_COMPLETE,
											scan_handler) < 0)
	{
		LOGE("scan complete reg error\r\n");
	}
	if (wifi_connection_register_event_handler(WIFI_EVENT_IOT_PORT_SECURE,
											wifi_handler) < 0)
	{
		LOGE("port secure reg error\r\n");
	}
	if (wifi_connection_register_event_handler(WIFI_EVENT_IOT_CONNECTED,
											wifi_handler) < 0)
	{
		LOGE("connect reg error\r\n");
	}
	if (wifi_connection_register_event_handler(WIFI_EVENT_IOT_DISCONNECTED,
											wifi_handler) < 0)
	{
		LOGE("disconnect reg error\r\n");
	}

	lwip_network_init(config.opmode);
	lwip_net_start(config.opmode);
}

static void HoundInit(void)
{
	m_sntp = xSemaphoreCreateBinary();
	assert(m_sntp != NULL);
	m_hound = xSemaphoreCreateBinary();
	assert(m_hound != NULL);
}

static void HoundTask(void *pvParameters)
{
	/* GPIO configure for LED/I2C/I2S */
	led_io_config();
	audio_io_config();
#ifdef MW_LIB
	mw_configure();
#endif

	EnableI2S();

	red_led(TRUE);

	if (!sensoryTest())
	{
		while (1)
		{
			/* wait forever */
			red_led(TRUE);
			LOGE("sensoryTest failed\r\n");
			vTaskDelay( ( portTickType ) 200 / portTICK_RATE_MS );
			red_led(FALSE);
		}
	}

	EnableAK4951();

	LOGI("AK4951 OK\r\n");

	wifi_init();

	lwip_net_ready();

	LOGI("WIFI connected\r\n");

	while (TRUE)
	{
		sntp_client();
		assert(pdTRUE == xSemaphoreTake(m_sntp, portMAX_DELAY));
		if (bSntpSuccess)
		{
			bNetReady = TRUE;
			LOGI("SNTP success\r\n");
			break;
		}
		else
		{
			LOGW("SNTP failed, retry again\r\n");
			vTaskDelay(sec_DELAY);
		}
	}

	/* to start snsr_trig in UI */
	assert(pdTRUE == xSemaphoreGive(m_hound));

	LOGI("HOUND LIB VER %s\r\n", hound_lib_version());

	/* start HOUND module */
	hound_client(CLIENT_ID, CLIENT_KEY, USER_ID);

	while (1)
	{
		/* hound_client shall not be returned */
		red_led(TRUE);
		LOGE("Hound returned\r\n");
		vTaskDelay( ( portTickType ) 200 / portTICK_RATE_MS );
		red_led(FALSE);
	}
}

/* Hound module calls hound_start_input for notification to UI */
/* Hound task context */
void hound_start_input(void)
{
	blue_led(TRUE);
	/* start mic recording */
	hound_voice_start();
}

/* Hound module calls hound_stop_input for notification to UI */
/* Hound task context */
void hound_stop_input(void)
{
	blue_led(FALSE);
	/* stop mic recording */
	hound_voice_stop();
}

/* Hound module calls hound_query_complete for notification to UI */
/* Hound task context */
void hound_query_complete(bool bSuccess)
{
	assert(pdTRUE == xSemaphoreGive(m_hound));
}

#define LED_GPT		HAL_GPT_2
static bool bToggleBlue = FALSE;
static bool bBlueLed = FALSE;

static void gpt_toggle_blue_cb(void *param)
{
	if (bToggleBlue)
	{
		bBlueLed = !bBlueLed;
		blue_led(bBlueLed);
	}
	else
	{
		hal_gpt_status_t	ret;

		bBlueLed = FALSE;
		
		ret = hal_gpt_stop_timer(LED_GPT);
		assert(ret == HAL_GPT_STATUS_OK);
		ret = hal_gpt_deinit(LED_GPT);
		assert(ret == HAL_GPT_STATUS_OK);
	}
}

static void toggle_blue(void)
{
	hal_gpt_status_t	ret;
	uint32_t			count;

	if (bToggleBlue == FALSE)
	{
		bToggleBlue = TRUE;
		ret = hal_gpt_init(LED_GPT);
		assert(ret == HAL_GPT_STATUS_OK);

		ret = hal_gpt_register_callback(LED_GPT, gpt_toggle_blue_cb, NULL);
		assert(ret == HAL_GPT_STATUS_OK);

		ret = hal_gpt_start_timer_ms(LED_GPT, 250, HAL_GPT_TIMER_TYPE_REPEAT);
		assert(ret == HAL_GPT_STATUS_OK);

		hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
	}
}

static void UiTask(void *pvParameters)
{
	assert(pdTRUE == xSemaphoreTake(m_hound, portMAX_DELAY));
	/* STNP success */
	red_led(FALSE);

	while (1)
	{
		if (!bNetReady)
		{
			red_led(TRUE);
			blue_led(FALSE);

			lwip_net_ready();
			bNetReady = TRUE;
			red_led(FALSE);
		}

		toggle_blue();
		snsr_trig();

		/* sensory spotted */
		bToggleBlue = FALSE;
		blue_led(FALSE);
		hound_query_start();

		assert(pdTRUE == xSemaphoreTake(m_hound, portMAX_DELAY));
		/* hound_query_complete is called */
	}
}

#ifdef HOUND_HEAP_CHECK
static void HoundHeapCheckTask(void *pvParameters)
{
	while (TRUE)
	{
		unsigned long hsize = hound_heap_size();
		vTaskDelay(3 * sec_DELAY);
		if (hsize > 1024)
			LOGI("Hound Heap: %lu KB\r\n", hsize/1024);
		else
			LOGI("Hound Heap: %lu B\r\n", hsize);
	}
}
#endif

#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
#ifdef HAL_WDT_MODULE_ENABLED
void wdt_timeout_handle(hal_wdt_reset_status_t wdt_reset_status)
{
	LOGE("%s: stattus:%u\r\n", __FUNCTION__, (unsigned int)wdt_reset_status);

	configASSERT(0);
}

static void wdt_init(void)
{
	hal_wdt_config_t wdt_init;
	wdt_init.mode = HAL_WDT_MODE_INTERRUPT;
	wdt_init.seconds = 15;
	hal_wdt_init(&wdt_init);
	hal_wdt_register_callback(wdt_timeout_handle);
	hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
}
#endif
#endif

extern "C"
{
/* for idle task feed wdt (DO NOT enter sleep mode)*/
void vApplicationIdleHook(void)
{
#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
#ifdef HAL_WDT_MODULE_ENABLED
	hal_wdt_feed(HAL_WDT_FEED_MAGIC);
#endif
#endif
}

//static bool bPrn = FALSE;
void vApplicationMallocFailedHook(void)
{
	/* heap memory is ehausted */
	/* Long response from hound server, it needs more heap memory */
	LOGE("vApplicationMallocFailedHook\r\n");
	vTaskDelay(sec_DELAY * 5);
	configASSERT(0);
}
}

int main(void)
{
	/* Do system initialization, eg: hardware, nvdm. */
	system_init();

	HoundInit();

	xTaskCreate(UiTask, UI_TASK_NAME,
								UI_TASK_STACKSIZE / sizeof(StackType_t), NULL,
								UI_TASK_PRIO, NULL);

	xTaskCreate(HoundTask, HOUND_TASK_NAME,
								HOUND_TASK_STACKSIZE / sizeof(StackType_t), NULL,
								HOUND_TASK_PRIO, NULL);

#ifdef HOUND_HEAP_CHECK
	xTaskCreate(HoundHeapCheckTask, HEAP_CHK_TASK_NAME,
								HEAP_CHK_TASK_STACKSIZE / sizeof(StackType_t), NULL,
								HEAP_CHK_TASK_PRIO, NULL);
#endif

	/* Call this function to indicate the system initialize done. */
	SysInitStatus_Set();

#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
#ifdef HAL_WDT_MODULE_ENABLED
	wdt_init();
#endif
#endif

	/* Start the scheduler. */
	vTaskStartScheduler();
	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for (;;);

	return 0;
}
