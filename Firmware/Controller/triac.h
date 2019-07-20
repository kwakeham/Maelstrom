/**
 *
 * Triac Library
 * 
 * Keith Wakeham
 * 
 * 
 */

#ifndef TRIAC__
#define TRIAC__
#include "nrf.h"
#include "app_timer.h"
#include "nrf_drv_gpiote.h"

#define PIN_ZC 18 //old 11
#define PIN_OUT 20  //old 12


void timeout_handler(void * p_context);

void timeout_handler2(void * p_context);

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void gpio_init(void);

void set_power(uint16_t bike_power);

#endif