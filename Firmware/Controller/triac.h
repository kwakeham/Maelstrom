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
// #include "titan_mem.h"

#define PIN_ZC 18 //old 11
#define PIN_OUT 20  //old 12

#define NRF_TRIAC_LOG_ENABLED 1

typedef enum
{
    TRIAC_100 = 0,                       //
    TRIAC_150,                       //
    TRIAC_200,                       //
    TRIAC_250,                       //
    TRIAC_300,                       //
    TRIAC_350                        //
} triac_settings_t;

typedef enum
{
    TRIAC_POWER_UP,
    TRIAC_POWER_DOWN 
} triac_select_t;


void timeout_handler(void * p_context);

void timeout_handler2(void * p_context);

void no_signal_timeout_handler(void * p_context);

// void led_timeout_handler(void * p_context);

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void gpio_init(void);

void set_power(uint16_t bike_power);

void set_crank_power(uint8_t crank_event_count, uint16_t crank_period, uint16_t crank_accumulated_torque);

void cool_down(uint16_t bike_power);

void triac_set_normal(void);

void triac_set_setting_mode(void);

void set_power_mode (triac_select_t t_up_down);

void triac_settings(bool store_settings);

void set_triac_offset_max(void);

void set_triac_offset_max_reset(void);

void set_triac_offset_min(void);

void set_triac_offset_min_reset(void);

void triac_store_settings(void);

void triac_retrive_settings();




#endif