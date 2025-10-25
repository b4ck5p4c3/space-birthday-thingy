#ifndef LFSR_H_
#define LFSR_H_

#include <stdint.h>

typedef uint32_t LFSR;

void LFSRInit(LFSR* lfsr, uint32_t seed);
uint32_t LFSRGet(LFSR* lfsr);

#endif // LFSR_H_