#ifndef _NOISE_H_
#define _NOISE_H_ 1

#include "deinterlace.h"

extern BOOL UseTemporalNoiseFilter;
extern long TemporalLuminanceThreshold;
extern long TemporalChromaThreshold;

DEINTERLACE_FUNC NoiseFilter_Temporal;

#endif /* _NOISE_H_ */