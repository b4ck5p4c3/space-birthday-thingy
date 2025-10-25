#include <ch32v00x.h>

#include <stdbool.h>

#include "esig.h"
#include "lfsr.h"
#include "mcu_config.h"

#define LED_COUNT 5
#define RANDOM_SEED 0x01337008u

#define MIN(a, b) (((a) < (b)) ? a : b);

void GPIOInitPin(GPIO_TypeDef* gpio, uint16_t pin, GPIOSpeed_TypeDef speed,
                 GPIOMode_TypeDef mode) {
  GPIO_InitTypeDef init = {0};
  init.GPIO_Pin = pin;
  init.GPIO_Speed = speed;
  init.GPIO_Mode = mode;
  GPIO_Init(gpio, &init);
}

GPIO_TypeDef* kLEDPorts[LED_COUNT] = {LED_1_PORT, LED_2_PORT, LED_3_PORT,
                                      LED_5_PORT, LED_4_PORT};
uint16_t kLEDPins[LED_COUNT] = {LED_1_PIN, LED_2_PIN, LED_3_PIN, LED_5_PIN,
                                LED_4_PIN};

void GPIOInit() {
  RCC_APB2PeriphClockCmd(
      RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,
      ENABLE);

  for (uint32_t i = 0; i < LED_COUNT; i++) {
    GPIOInitPin(kLEDPorts[i], kLEDPins[i], GPIO_Speed_50MHz, GPIO_Mode_Out_PP);
    GPIO_WriteBit(kLEDPorts[i], kLEDPins[i], Bit_RESET);
  }
}

void TimerInit() {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = 500 - 1;
  TIM_TimeBaseStructure.TIM_Prescaler = 8 - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  TIM_Cmd(TIM2, ENABLE);
}

uint8_t current_time = 0;
uint8_t led_values[5] = {0};

void IteratePWM() {
  for (uint32_t i = 0; i < LED_COUNT; i++) {
    GPIOInitPin(kLEDPorts[i], kLEDPins[i], GPIO_Speed_50MHz, GPIO_Mode_Out_PP);
    GPIO_WriteBit(kLEDPorts[i], kLEDPins[i], current_time < led_values[i]);
  }
  current_time++;
}

LFSR random;

uint32_t led_intensity[LED_COUNT];
uint32_t led_intensity_speed[LED_COUNT];
bool led_intensity_up[LED_COUNT];

uint8_t led_brightness_curve[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05,
    0x05, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09,
    0x0A, 0x0A, 0x0B, 0x0B, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0F, 0x0F, 0x10,
    0x11, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
    0x1C, 0x1D, 0x1F, 0x20, 0x21, 0x23, 0x24, 0x26, 0x27, 0x29, 0x2B, 0x2C,
    0x2E, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E, 0x40, 0x43, 0x45,
    0x47, 0x4A, 0x4C, 0x4F, 0x51, 0x54, 0x57, 0x59, 0x5C, 0x5F, 0x62, 0x64,
    0x67, 0x6A, 0x6D, 0x70, 0x73, 0x76, 0x79, 0x7C, 0x7F, 0x82, 0x85, 0x88,
    0x8B, 0x8E, 0x91, 0x94, 0x97, 0x9A, 0x9C, 0x9F, 0xA2, 0xA5, 0xA7, 0xAA,
    0xAD, 0xAF, 0xB2, 0xB4, 0xB7, 0xB9, 0xBB, 0xBE, 0xC0, 0xC2, 0xC4, 0xC6,
    0xC8, 0xCA, 0xCC, 0xCE, 0xD0, 0xD2, 0xD3, 0xD5, 0xD7, 0xD8, 0xDA, 0xDB,
    0xDD, 0xDE, 0xDF, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
    0xEA, 0xEB, 0xEC, 0xED, 0xED, 0xEE, 0xEF, 0xEF, 0xF0, 0xF1, 0xF1, 0xF2,
    0xF2, 0xF3, 0xF3, 0xF4, 0xF4, 0xF5, 0xF5, 0xF6, 0xF6, 0xF6, 0xF7, 0xF7,
    0xF7, 0xF8, 0xF8, 0xF8, 0xF9, 0xF9, 0xF9, 0xF9, 0xFA, 0xFA, 0xFA, 0xFA,
    0xFA, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
    0xFC, 0xFC, 0xFC, 0xFC, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD,
    0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
    0xFE, 0xFE, 0xFF, 0xFF};

void GenerateLEDChange(uint32_t i) {
  led_intensity_speed[i] = MIN(0x01000000, LFSRGet(&random) >> 8);
}

int main() {
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  GPIOInit();
  TimerInit();
  LFSRInit(&random, RANDOM_SEED ^ ESigGetID1() ^ ESigGetID2() ^ ESigGetID3());
  for (uint32_t i = 0; i < LED_COUNT; i++) {
    led_intensity[i] = LFSRGet(&random);
    led_intensity_up[i] = LFSRGet(&random) & 1;
    GenerateLEDChange(i);
  }

  for (;;) {
    IteratePWM();
  }
}

void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void UpdateLED(uint32_t i) {
  if (led_intensity_up[i]) {
    if ((UINT32_MAX - led_intensity[i]) < led_intensity_speed[i]) {
      led_intensity[i] = UINT32_MAX;
      led_intensity_up[i] = !led_intensity_up[i];
      GenerateLEDChange(i);
    } else {
      led_intensity[i] += led_intensity_speed[i];
    }
  } else {
    if (led_intensity[i] < led_intensity_speed[i]) {
      led_intensity[i] = 0;
      led_intensity_up[i] = !led_intensity_up[i];
      GenerateLEDChange(i);
    } else {
      led_intensity[i] -= led_intensity_speed[i];
    }
  }

  led_values[i] = led_brightness_curve[led_intensity[i] >> 24];
}

void TIM2_IRQHandler(void) {
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    for (uint32_t i = 0; i < LED_COUNT; i++) {
      UpdateLED(i);
    }
  }
}