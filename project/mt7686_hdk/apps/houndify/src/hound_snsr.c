#include "snsr.h"

#include "syslog.h"
#define LOGE(fmt, arg...)	LOG_E(hound, "snsr: " fmt, ##arg)
#define LOGW(fmt, arg...)	LOG_W(hound, "snsr: " fmt, ##arg)
#define LOGI(fmt, arg...)	LOG_I(hound, "snsr: " fmt, ##arg)

#define RING_BUFFER_SIZE		(32768)	//32 * 1024
#define MAX_RING_BUFFER_SIZE	(65536)	//32 * 1024 * 2

extern unsigned char thfft_alexa_enus_v2_31kb_snsr[];
extern unsigned int thfft_alexa_enus_v2_31kb_snsr_len;

static SnsrSession snsrS = NULL;
static SnsrStream audioStream = NULL;

static SnsrRC resultEvent(const char *key, void *privateData)
{
	SnsrRC rc;
	const char *phrase;
	double begin, end;

	snsrGetDouble(snsrS, SNSR_RES_BEGIN_SAMPLE, &begin);
	snsrGetDouble(snsrS, SNSR_RES_END_SAMPLE, &end);
	rc = snsrGetString(snsrS, SNSR_RES_TEXT, &phrase);

	if (rc != SNSR_RC_OK)
		return rc;

	return SNSR_RC_STOP;
}

bool sensoryTest(void)
{
	SnsrRC		rc;
	SnsrStream	spotterStream = NULL;

	rc = snsrNew(&snsrS);
	if (rc != SNSR_RC_OK)
	{
		const char * err = snsrS ? snsrErrorDetail(snsrS) : snsrRCMessage(rc);
		LOGE("Sensory: Error on init: %d - %s\r\n", rc, err);
		return FALSE;
	}

	spotterStream = snsrStreamFromMemory(thfft_alexa_enus_v2_31kb_snsr,
											thfft_alexa_enus_v2_31kb_snsr_len,
											SNSR_ST_MODE_READ);
	snsrLoad(snsrS, spotterStream);
	if (snsrRequire(snsrS, SNSR_TASK_TYPE, SNSR_PHRASESPOT) != SNSR_RC_OK)
	{
		LOGE("Sensory: Error loading spotter: %d - %s\r\n", rc, snsrRCMessage(rc)); 
		return FALSE;
	}

	snsrSetHandler(snsrS, SNSR_RESULT_EVENT, snsrCallback((SnsrHandler)resultEvent, NULL, NULL));
	snsrSetInt(snsrS, SNSR_AUTO_FLUSH, 0);
	audioStream = snsrStreamFromBuffer(RING_BUFFER_SIZE, MAX_RING_BUFFER_SIZE);
	snsrSetStream(snsrS, SNSR_SOURCE_AUDIO_PCM, audioStream);

	return TRUE;
}

bool sensorySpotted(short * audioSamples, int sampleCount)
{
	SnsrRC	rc;

	snsrStreamWrite(audioStream, audioSamples, sizeof(short), sampleCount);
	rc = snsrStreamRC(audioStream);
	if (rc != SNSR_RC_OK)
	{
		LOGE("Sensory: error on stream: %d - %s\r\n", rc,
												snsrStreamErrorDetail(audioStream));
		LOGE("Sensory: error %s\r\n", snsrErrorDetail(snsrS));
		return FALSE;
	}

	if (snsrRun(snsrS) == SNSR_RC_STREAM_END)
	{
		snsrClearRC(snsrS);
	}

	rc = snsrRC(snsrS);
	if (rc == SNSR_RC_STOP)
	{
		snsrClearRC(snsrS);
		return TRUE;
	}

	return FALSE;
}
