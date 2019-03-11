#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "hal.h"
#include "memory_attribute.h"
#include "semphr.h"
#include "assert.h"
#include "hound_snsr.h"
#include "hound_led.h"
#ifdef MW_LIB
#include "mw.h"

MwHandle *inst;
//ATTR_ZIDATA_IN_CACHED_SYSRAM uint32_t MwMem[210 * 256]; /* 200 -> 210. require from MW. */
ATTR_ZIDATA_IN_NONCACHED_SYSRAM uint32_t MwMem[210 * 256]; /* 200 -> 210. require from MW. */
#endif

#include "syslog.h"
#define LOGE(fmt, arg...)	LOG_E(hound, "audio: " fmt, ##arg)
#define LOGW(fmt, arg...)	LOG_W(hound, "audio: " fmt, ##arg)
#define LOGI(fmt, arg...)	LOG_I(hound, "audio: " fmt, ##arg)

//#define Q_CNT_DBG

#define IO_AK4951_PDN		HAL_GPIO_6

#define IO_AK4951_SCL		HAL_GPIO_8
#define IO_AK4951_SDA		HAL_GPIO_9
#define I2C_AK4951_PORT	HAL_I2C_MASTER_0

#define IO_AK4951_RX		HAL_GPIO_0
#define IO_AK4951_TX		HAL_GPIO_1
#define IO_AK4951_WS		HAL_GPIO_2
#define IO_AK4951_CK		HAL_GPIO_3
#define IO_AK4951_MCLK		HAL_GPIO_4

#define AK4951_PORT			HAL_I2S_1

#define AK4951_ADDR		(0x24>>1)
#define AK4951_CMD_NUM	(80)
static const uint8_t ak4951_cfg[AK4951_CMD_NUM][2] =
{
	{0x00, 0xC7},
	{0x01, 0x32},
	{0x02, 0xAE},
	{0x03, 0x00},
	{0x04, 0x04},
	{0x05, 0x53},
	{0x06, 0xC2},
	{0x07, 0x14},
	{0x08, 0x00},
	{0x09, 0x00},
	{0x0A, 0x6C},
	{0x0B, 0x2E},
	{0x0C, 0xE1},
	{0x0D, 0xF1},
	{0x0E, 0xF1},
	{0x0F, 0x00},
	{0x10, 0xff},
	{0x11, 0xff},
	{0x12, 0x00},
	{0x13, 0x2C},
	{0x14, 0x2C},
	{0x15, 0x00},
	{0x16, 0x00},
	{0x17, 0x00},
	{0x18, 0x00},
	{0x19, 0x00},
	{0x1A, 0x00},
	{0x1B, 0x01},
	{0x1C, 0x00},
	{0x1D, 0x02},
	{0x1E, 0x00},
	{0x1F, 0x00},
	{0x20, 0x00},
	{0x21, 0x00},
	{0x22, 0x00},
	{0x23, 0x00},
	{0x24, 0x00},
	{0x25, 0x00},
	{0x26, 0x00},
	{0x27, 0x00},
	{0x28, 0x00},
	{0x29, 0x00},
	{0x2A, 0x00},
	{0x2B, 0x00},
	{0x2C, 0x00},
	{0x2D, 0x00},
	{0x2E, 0x00},
	{0x2F, 0x00},
	{0x30, 0x00},
	{0x31, 0x00},
	{0x32, 0x00},
	{0x33, 0x00},
	{0x34, 0x00},
	{0x35, 0x00},
	{0x36, 0x00},
	{0x37, 0x00},
	{0x38, 0x00},
	{0x39, 0x00},
	{0x3A, 0x00},
	{0x3B, 0x00},
	{0x3C, 0x00},
	{0x3D, 0x00},
	{0x3E, 0x00},
	{0x3F, 0x00},
	{0x40, 0x00},
	{0x41, 0x00},
	{0x42, 0x00},
	{0x43, 0x00},
	{0x44, 0x00},
	{0x45, 0x00},
	{0x46, 0x00},
	{0x47, 0x00},
	{0x48, 0x00},
	{0x49, 0x00},
	{0x4A, 0x00},
	{0x4B, 0x00},
	{0x4C, 0x00},
	{0x4D, 0x00},
	{0x4E, 0x00},
	{0x4F, 0x00}
};

#define I2S_THRESHOLD	(512)	// 8msec
#define I2S_BUF_LENGTH	(I2S_THRESHOLD * 8)	// 64msec buffering
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint32_t i2s_tx_buf[I2S_BUF_LENGTH];
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint32_t i2s_rx_buf[I2S_BUF_LENGTH];
#ifdef MW_LIB
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint32_t i2s_rx_data[I2S_THRESHOLD];	// 2 channel
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint16_t out[I2S_THRESHOLD];
#else
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint16_t i2s_rx_data[I2S_THRESHOLD];	// mono channel
#endif

static uint16_t 	*pOutData = NULL;
static uint32_t		out_data_len = 0;
static uint32_t		out_cur_idx = 0;
static bool		out_play = FALSE;
static bool		in_rec = FALSE;

static SemaphoreHandle_t	m_audio_tx_lock = NULL;
static SemaphoreHandle_t	m_audio_rx_lock = NULL;

#define Q_SIZE	(300)

typedef struct
{
	void	**ppvRead;
	void	**ppvWrite;
	void	**ppvStart;
	void	**ppvEnd;
} tsQIdx;

typedef struct
{
	uint16_t	Data[I2S_THRESHOLD];
	//uint32_t	Data[I2S_THRESHOLD];
} tsData;

typedef struct
{
	tsQIdx	sSpareQIdx;
	tsQIdx	sUsedQIdx;
	void		*apvSpareQData[Q_SIZE + 1];
	void		*apvUsedQData[Q_SIZE + 1];
	tsData	Data[Q_SIZE];
} tsQInfo;

static tsQInfo	sAudioQ;

#ifdef Q_CNT_DBG
static uint32_t	dbg_tx_cnt, dbg_rx_cnt;
#endif

static void vInitQ(tsQIdx *psQ, void **ppvData, uint32_t qsize)
{
	psQ->ppvRead = &ppvData[0];
	psQ->ppvWrite = &ppvData[0];
	psQ->ppvStart = &ppvData[0];
	psQ->ppvEnd = &ppvData[qsize];
}

static void *pvPull(tsQIdx *psQ)
{
	void *pvRet = NULL;

	if (psQ->ppvRead != psQ->ppvWrite)
	{
		pvRet = *(psQ->ppvRead);

		if (psQ->ppvRead == psQ->ppvEnd)
			psQ->ppvRead = psQ->ppvStart;
		else
			psQ->ppvRead++;
	}

	return pvRet;
}

static bool bPush(tsQIdx *psQ, void *pvData)
{
	if (psQ->ppvWrite == psQ->ppvEnd)
	{
		if (psQ->ppvRead == psQ->ppvStart)
		{
			return FALSE;
		}
	}
	else
	{
		if (psQ->ppvRead == psQ->ppvWrite + 1)
		{
			return FALSE;
		}
	}

	*(psQ->ppvWrite) = pvData;

	if (psQ->ppvWrite == psQ->ppvEnd)
		psQ->ppvWrite = psQ->ppvStart;
	else
		psQ->ppvWrite++;

	return TRUE;
}

static bool bInitQ(void)
{
	uint32_t	i;

	vInitQ(&(sAudioQ.sSpareQIdx), sAudioQ.apvSpareQData, Q_SIZE);

	for (i = 0; i < Q_SIZE; i++)
	{
		if (!bPush(&(sAudioQ.sSpareQIdx), (void *)&(sAudioQ.Data[i])))
		{
			return FALSE;
		}
	}

	vInitQ(&(sAudioQ.sUsedQIdx), sAudioQ.apvUsedQData, Q_SIZE);

#ifdef Q_CNT_DBG
	dbg_tx_cnt = 0;
	dbg_rx_cnt = 0;
#endif

	return TRUE;
}

static tsData *psGetQ(void)
{
	return (tsData *)pvPull(&(sAudioQ.sSpareQIdx));
}

static bool bPostQ(tsData *psData)
{
	if (!psData)
		return FALSE;
	
	if (bPush(&(sAudioQ.sUsedQIdx), (void *)psData))
	{
#ifdef Q_CNT_DBG
		uint32_t tt = dbg_tx_cnt++;
		LOGI("+ %d\r\n", tt);
#endif
		return TRUE;
	}

	return FALSE;
}

static tsData *psReadQ(void)
{
	tsData	*ret = (tsData *)pvPull(&(sAudioQ.sUsedQIdx));

	if (ret)
	{
#ifdef Q_CNT_DBG
		uint32_t tt = dbg_rx_cnt++;
		LOGI("- %d\r\n", tt);
#endif
	}

	return ret;
}

static bool bReturnQ(tsData *psData)
{
	bool	bRet = FALSE;

	if (psData)
	{
		bRet = bPush(&(sAudioQ.sSpareQIdx), (void *)psData);
	}

	return bRet;
}

void EnableAK4951(void)
{
	hal_i2c_status_t	ret;
	hal_i2c_config_t	i2c_init = { HAL_I2C_FREQUENCY_400K };
	uint8_t			i, retry = 0;

	ret = hal_i2c_master_init(I2C_AK4951_PORT, &i2c_init);
	assert (ret == HAL_I2C_STATUS_OK);

	hal_gpio_set_output(IO_AK4951_PDN, HAL_GPIO_DATA_LOW);
	hal_gpt_delay_ms(1);
	hal_gpio_set_output(IO_AK4951_PDN, HAL_GPIO_DATA_HIGH);

	hal_gpt_delay_ms(100);

	for (i = 0; i < AK4951_CMD_NUM; i++)
	{
		while (TRUE)
		{
			ret = hal_i2c_master_send_polling(I2C_AK4951_PORT, AK4951_ADDR,
											ak4951_cfg[i], 2);
			if (ret != HAL_I2C_STATUS_OK)
			{
				retry++;
				assert(retry < 10);
				continue;
			}
			else
			{
				retry = 0;
				break;
			}
		}
	}

	hal_gpt_delay_ms(100);

	ret = hal_i2c_master_deinit(I2C_AK4951_PORT);
	assert (ret == HAL_I2C_STATUS_OK); 

	hal_gpt_delay_ms(100);
}

void DisableAK4951(void)
{
	hal_gpio_set_output(IO_AK4951_PDN, HAL_GPIO_DATA_LOW);
}

static void ResetRxSem(void)
{
	vSemaphoreDelete(m_audio_rx_lock);
	m_audio_rx_lock = xSemaphoreCreateCounting(Q_SIZE, 0);
}

static void ak4951_i2s_tx_cb(hal_i2s_event_t event, void *user_data)
{
	BaseType_t	xHigherPriorityTaskWoken;

	if (event == HAL_I2S_EVENT_DATA_REQUEST)
	{
		uint32_t	count, i;

		hal_i2s_disable_tx_dma_interrupt_ex(AK4951_PORT);

		hal_i2s_get_tx_sample_count_ex(AK4951_PORT, &count);
		for (i = 0; i < count; i++)
		{
			if (out_play)
			{
				if (out_cur_idx < out_data_len)
				{
					uint32_t	data = (uint32_t)pOutData[out_cur_idx++];
					uint32_t	out_data = (data << 16) | data;	// duplicate mono data for 2 channel
					hal_i2s_tx_write_ex(AK4951_PORT, out_data);
				}
				else
				{
					out_play = FALSE;
					xSemaphoreGiveFromISR(m_audio_tx_lock, &xHigherPriorityTaskWoken);
					hal_i2s_tx_write_ex(AK4951_PORT, 0);
				}
			}
			else
			{
				hal_i2s_tx_write_ex(AK4951_PORT, 0);
			}
		}

		hal_i2s_enable_tx_dma_interrupt_ex(AK4951_PORT);
	}
}

static void ak4951_i2s_rx_cb(hal_i2s_event_t event, void *user_data)
{
	tsData		*psData;
	BaseType_t	xHigherPriorityTaskWoken = pdFALSE;
	BaseType_t	ret;
#ifdef MW_LIB
    int         mwStatus;
#endif

	if (event == HAL_I2S_EVENT_DATA_NOTIFICATION)
	{
		uint32_t	count, i;
		uint32_t	data;

		hal_i2s_disable_rx_dma_interrupt_ex(AK4951_PORT);

		hal_i2s_get_rx_sample_count_ex(AK4951_PORT, &count);

		for (i = 0; i < count; i++)
		{
			hal_i2s_rx_read_ex(AK4951_PORT, &data);
#ifdef MW_LIB
            i2s_rx_data[i] = data;
#else
            i2s_rx_data[i] = (uint16_t)data;        //change 2 ch data to mono data
#endif
		}

#ifdef MW_LIB
        mwStatus = Mw_PbProcess(inst, (int16_t *)i2s_rx_data, (int16_t *)out);
        
        if(mwStatus == -1)
        {
            LOGE("MW Lib time out!!.\n");
        }
#endif

		if (in_rec)
		{
			psData = psGetQ();
			if (psData)
			{
#ifdef MW_LIB
                memcpy(psData->Data, out, sizeof(out));
#else
                memcpy(psData->Data, i2s_rx_data, sizeof(i2s_rx_data));
#endif
				assert(bPostQ(psData));
				ret = xSemaphoreGiveFromISR(m_audio_rx_lock, &xHigherPriorityTaskWoken);
				assert(ret == pdPASS);
			}
			else
			{
				LOGE("Q is FULL\r\n");
			}
		}

		hal_i2s_enable_rx_dma_interrupt_ex(AK4951_PORT);
	}
}

void DisableI2S(void)
{
	hal_i2s_status_t	ret;

	ret = hal_i2s_disable_tx_dma_interrupt_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);
	ret = hal_i2s_disable_rx_dma_interrupt_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);

	ret = hal_i2s_disable_tx_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);
	ret = hal_i2s_disable_rx_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);

	ret = hal_i2s_disable_audio_top_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);
	ret = hal_i2s_deinit_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);

	ret = hal_i2s_stop_tx_vfifo_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);
	ret = hal_i2s_stop_rx_vfifo_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);

	vSemaphoreDelete(m_audio_tx_lock);
	m_audio_tx_lock = NULL;
	vSemaphoreDelete(m_audio_rx_lock);
	m_audio_rx_lock = NULL;
}

void EnableI2S(void)
{
	hal_i2s_status_t	ret;
	hal_i2s_config_t	i2s_config;

	assert(m_audio_tx_lock == NULL);
	assert(m_audio_rx_lock == NULL);

	m_audio_tx_lock = xSemaphoreCreateBinary();
	m_audio_rx_lock = xSemaphoreCreateCounting(Q_SIZE, 0);

	ret = hal_i2s_init_ex(AK4951_PORT, HAL_I2S_TYPE_EXTERNAL_MODE);
	assert (ret == HAL_I2S_STATUS_OK);

	i2s_config.clock_mode = HAL_I2S_MASTER;
	i2s_config.sample_width = HAL_I2S_SAMPLE_WIDTH_16BIT;
	i2s_config.frame_sync_width = HAL_I2S_FRAME_SYNC_WIDTH_32;
	i2s_config.rx_down_rate = HAL_I2S_RX_DOWN_RATE_DISABLE;
	i2s_config.tx_mode = HAL_I2S_TX_MONO_DUPLICATE_DISABLE;

	i2s_config.i2s_in.channel_number = HAL_I2S_STEREO;
	i2s_config.i2s_out.channel_number = HAL_I2S_STEREO;

	i2s_config.i2s_in.sample_rate = HAL_I2S_SAMPLE_RATE_16K;
	i2s_config.i2s_out.sample_rate = HAL_I2S_SAMPLE_RATE_16K;
	i2s_config.i2s_in.msb_offset = 0;
	i2s_config.i2s_out.msb_offset = 0;
	i2s_config.i2s_in.word_select_inverse = HAL_I2S_WORD_SELECT_INVERSE_DISABLE;
	i2s_config.i2s_out.word_select_inverse = HAL_I2S_WORD_SELECT_INVERSE_DISABLE;
	i2s_config.i2s_in.lr_swap = HAL_I2S_LR_SWAP_DISABLE;
	i2s_config.i2s_out.lr_swap = HAL_I2S_LR_SWAP_DISABLE;

	ret = hal_i2s_set_config_ex(AK4951_PORT, &i2s_config);
	assert (ret == HAL_I2S_STATUS_OK);

	ret = hal_i2s_setup_tx_vfifo_ex(AK4951_PORT, i2s_tx_buf, I2S_THRESHOLD,
														I2S_BUF_LENGTH);
	assert (ret == HAL_I2S_STATUS_OK);
	ret = hal_i2s_setup_rx_vfifo_ex(AK4951_PORT, i2s_rx_buf, I2S_THRESHOLD,
														I2S_BUF_LENGTH);
	assert (ret == HAL_I2S_STATUS_OK);

	ret = hal_i2s_register_tx_vfifo_callback_ex(AK4951_PORT, ak4951_i2s_tx_cb,
													NULL);
		assert (ret == HAL_I2S_STATUS_OK);
	ret = hal_i2s_register_rx_vfifo_callback_ex(AK4951_PORT, ak4951_i2s_rx_cb,
												NULL);
	assert (ret == HAL_I2S_STATUS_OK);

	ret = hal_i2s_enable_tx_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);
	ret = hal_i2s_enable_rx_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);

	ret = hal_i2s_enable_audio_top_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);

	ret = hal_i2s_enable_tx_dma_interrupt_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);
	ret = hal_i2s_enable_rx_dma_interrupt_ex(AK4951_PORT);
	assert (ret == HAL_I2S_STATUS_OK);
}

void audio_io_config(void)
{
	hal_gpio_init(IO_AK4951_PDN);
	hal_pinmux_set_function(IO_AK4951_PDN, 0);
	hal_gpio_set_direction(IO_AK4951_PDN,HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_pull_up(IO_AK4951_PDN);	

	hal_gpio_init(IO_AK4951_SCL);
	hal_gpio_init(IO_AK4951_SDA);
	hal_pinmux_set_function(IO_AK4951_SCL, 4);
	hal_pinmux_set_function(IO_AK4951_SDA, 4);

	hal_gpio_init(IO_AK4951_RX);
	hal_gpio_init(IO_AK4951_TX);
	hal_gpio_init(IO_AK4951_WS);
	hal_gpio_init(IO_AK4951_CK);
	hal_gpio_init(IO_AK4951_MCLK);
	hal_pinmux_set_function(IO_AK4951_RX, 5);
	hal_pinmux_set_function(IO_AK4951_TX, 5);
	hal_pinmux_set_function(IO_AK4951_WS, 5);
	hal_pinmux_set_function(IO_AK4951_CK, 5);
	hal_pinmux_set_function(IO_AK4951_MCLK, 5);
}

/* Hound calls spk_out */
void spk_out(uint8_t *data, uint32_t len)
{
	out_cur_idx = 0;
	pOutData = (uint16_t *)data;	// change to 16bit little endian data
	out_data_len = len >> 1;

	out_play = TRUE;
	blue_led(TRUE);

	xSemaphoreTake(m_audio_tx_lock, portMAX_DELAY);

	blue_led(FALSE);
}

void snsr_trig(void)
{
	tsData	*psData;

	LOGI("snsr_trig\r\n");

	bInitQ();
	ResetRxSem();
	in_rec = TRUE;

#ifdef MW_LIB
    Mw_Set_Parameter(inst, PB_UPDATE, "1");
#endif

	while (TRUE)
	{
		xSemaphoreTake(m_audio_rx_lock, portMAX_DELAY);
		psData = psReadQ();
		if (psData)
		{
			if (sensorySpotted((short *)psData->Data, I2S_THRESHOLD))
			{
				bReturnQ(psData);
				in_rec = FALSE;
				LOGI("sensory spotted\r\n");
#ifdef MW_LIB
                Mw_Set_Parameter(inst, PB_UPDATE, "0");
#endif
				break;
			}
			else
				bReturnQ(psData);
		}
		else
		{
			LOGW("no data\r\n");
		}
	}
}

static tsData	*psHoundAudio = NULL;
void hound_voice_start(void)
{
	LOGI("hound_voice_start\r\n");

	bInitQ();
	ResetRxSem();
	in_rec = TRUE;
}

void hound_voice_stop(void)
{
	in_rec = FALSE;

	LOGI("hound_voice_stop\r\n");
}

/* Hound module calls hound_get_voice to get voice data */
/* Hound task context */
short *hound_get_voice(void)
{
	do
	{
		xSemaphoreTake(m_audio_rx_lock, portMAX_DELAY);
		psHoundAudio = psReadQ();
	} while (psHoundAudio == NULL);

	return (short *)psHoundAudio->Data;
}

/* Hound module calls hound_complete_voice to release voice data buffer */
/* Hound task context */
void hound_complete_voice(void)
{
	bReturnQ(psHoundAudio);
}

#ifdef MW_LIB
void mw_configure(void)
{
    int real_memory = Mw_Create(&inst, MwMem);
    LOGI("MW Memory Size = %d bytes\n", real_memory);

	int mw_err = Mw_Init(inst);
	
    if(mw_err == -1) 
        LOGE("MwInit Failed = %d\n", mw_err);
    else
        LOGI("MwInit Success\n");
}
#endif

