#ifndef __HOUND_SNSR_H__
#define __HOUND_SNSR_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool sensoryTest(void);
bool sensorySpotted(short * audioSamples, int sampleCount);

#ifdef __cplusplus
}
#endif

#endif
