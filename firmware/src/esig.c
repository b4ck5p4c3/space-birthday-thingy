#include "esig.h"

#ifndef ROM_CFG_USERADR_ID
#define ROM_CFG_USERADR_ID 0x1FFFF7E8
#endif

uint32_t ESigGetID1() {
  return *(volatile uint32_t*)(ROM_CFG_USERADR_ID);
}

uint32_t ESigGetID2() {
  return *(volatile uint32_t*)(ROM_CFG_USERADR_ID + 4);
}

uint32_t ESigGetID3() {
  return *(volatile uint32_t*)(ROM_CFG_USERADR_ID + 8);
}