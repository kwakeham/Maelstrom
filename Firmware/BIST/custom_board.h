/**
*
 */

#ifndef MAELSTROM_H
#define MAELSTROM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"


#define LED_1R          16
#define LED_1G          12
#define LED_1B          14

#define LED_2R          11
#define LED_2G          9
#define LED_2B          10

#define LED_3R          19
#define LED_3G          30
#define LED_3B          31

#define BUTTON_1       29
#define BUTTON_2       28
#define BUTTON_3       27
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define RX_PIN_NUMBER  8
#define TX_PIN_NUMBER  6
#define CTS_PIN_NUMBER 7
#define RTS_PIN_NUMBER 5
#define HWFC           true


// LEDs definitions for PCA10040
#define LEDS_NUMBER    4

#define LED_START      17
#define LED_1          17
#define LED_2          18
#define LED_3          19
#define LED_4          20
#define LED_STOP       20

#define LEDS_ACTIVE_STATE 0

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_LIST { LED_1, LED_2, LED_3, LED_4 }

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4

#define BUTTONS_NUMBER 4

#define BUTTON_START   13
// #define BUTTON_1       13
// #define BUTTON_2       14
// #define BUTTON_3       15
#define BUTTON_4       16
#define BUTTON_STOP    16
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4 }

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2
#define BSP_BUTTON_2   BUTTON_3
#define BSP_BUTTON_3   BUTTON_4


#ifdef __cplusplus
}
#endif

#endif // PCA10040_H
