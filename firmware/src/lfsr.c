#include "lfsr.h"

void LFSRInit(LFSR* lfsr, uint32_t seed) {
    if (seed == 0) {
        *lfsr = 1;
    } else {
        *lfsr = seed;
    }
}

#define LFSR_POLY_TAPS 0x80000003UL

uint32_t LFSRGet(LFSR* lfsr) {
    uint32_t lsb = *lfsr & 1; 
    *lfsr >>= 1;
    if (lsb) {
        *lfsr ^= LFSR_POLY_TAPS;
    }
    return *lfsr;
}