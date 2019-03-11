#ifndef __HOUND_AUDIO_H__
#define __HOUND_AUDIO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void audio_io_config(void);
void EnableAK4951(void);
void DisableAK4951(void);
void EnableI2S(void);
void DisableI2S(void);
void spk_out(uint8_t *data, uint32_t len);
void snsr_trig(void);

void hound_voice_start(void);
void hound_voice_stop(void);
short *hound_get_voice(void);
void hound_complete_voice(void);

#ifdef MW_LIB
void mw_configure(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
