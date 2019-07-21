/**
 *
 * Triac Library
 * 
 * Keith Wakeham
 * 
 * 
 */


#include "nrf.h"
#include "app_timer.h"
#include "nrf_drv_gpiote.h"
#include "triac.h"
#include "nrf_log.h"


APP_TIMER_DEF(m_triac_timer_id);

APP_TIMER_DEF(m_bpwr_timer_id); 

bool ZC_pulse = 1;
uint32_t offset = 500;
uint16_t p_avg = 0;
uint32_t time_old = 0;
uint32_t time_diff = 0;
uint32_t time = 0;
int32_t cool_down_count = 0;

triac_settings_t triac_power_level = TRIAC_200;

uint16_t triac_offset_min = 140; //min power = longest time
uint16_t triac_offset_max = 60; //max power = shortest time

void timeout_handler(void * p_context)
{
    // nrf_drv_gpiote_out_set(BSP_LED_1);
    nrf_drv_gpiote_out_toggle(PIN_OUT);
    ret_code_t err_code;
    if (ZC_pulse == 1) {
        nrf_drv_gpiote_out_set(PIN_OUT);
        err_code = app_timer_start(m_triac_timer_id, 32, NULL);
        APP_ERROR_CHECK(err_code);
        ZC_pulse = 0;
    } else {
        nrf_drv_gpiote_out_clear(PIN_OUT);
    }
}

void timeout_handler2(void * p_context)
{

}

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    ret_code_t err_code;
    // nrf_drv_gpiote_out_toggle(PIN_OUT);
    if (offset < 500)
    {
        err_code = app_timer_start(m_triac_timer_id, offset, NULL);
        APP_ERROR_CHECK(err_code);
        ZC_pulse = 1;
    }
    // NRF_LOG_INFO("Input");
    
}

/**
 * @brief Function for configuring: PIN_IN pin for input, PIN_OUT pin for output,
 * and configures GPIOTE to give an interrupt on pin change.
 */
void gpio_init(void)
{
    ret_code_t err_code;
    if(!nrf_drv_gpiote_is_init())
    {
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    }
    NRF_LOG_INFO("Init'd GPIOTE");
    // err_code = nrfx_gpiote_init();
    // APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

    err_code = nrf_drv_gpiote_out_init(PIN_OUT, &out_config);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("Phase output setup");

    // err_code = nrf_drv_gpiote_out_init(BSP_LED_1, &out_config);
    // APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(PIN_ZC, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("ZC input  setup");

    nrf_drv_gpiote_in_event_enable(PIN_ZC, true);

    // APP_TIMER_DEF(m_triac_timer_id); 
    err_code = app_timer_create(&m_triac_timer_id, APP_TIMER_MODE_SINGLE_SHOT, timeout_handler);
    NRF_LOG_INFO("Timer Create");
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_bpwr_timer_id, APP_TIMER_MODE_REPEATED, timeout_handler2);
    NRF_LOG_INFO("Timer repeated ");
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_bpwr_timer_id, 128, NULL);
    APP_ERROR_CHECK(err_code);
}

void set_power(uint16_t bike_power){

    int16_t p_diff; //power difference
    uint8_t adjuster; //The calculated adjuster difference, to be calculated
    uint16_t p_cool;

    time = app_timer_cnt_get();
    time_diff = time - time_old;
    time_old = time;
    NRF_LOG_INFO("time:                  %u s", time_diff);
    adjuster = time_diff/8180;
    NRF_LOG_INFO("factor1:                  %u s", adjuster);
    if (adjuster > 10)
    {
        adjuster = 10;
    }
    NRF_LOG_INFO("factor2:                  %u s", adjuster);
    p_diff = bike_power - p_avg;
    NRF_LOG_INFO("p_diff:                  %i W", p_diff);
    p_diff = (p_diff*adjuster)/10;
    NRF_LOG_INFO("p_diff:                  %i W", p_diff);
    p_avg = p_avg + p_diff;

    p_cool = (bike_power*adjuster)/200;


    if (p_avg > 200)
    {
        offset = triac_offset_max;
        cool_down_count= cool_down_count+p_cool;
    } else if (p_avg <= 200 && p_avg > 40) //(p_avg <= 200 && p_avg > 100) //This is for -0.8 and 220
    {
        offset = (p_avg*-0.5)+160;
        cool_down_count= cool_down_count+p_cool;
    } else if (p_avg <= 40 && p_avg > 20)
    {
        if (cool_down_count > 0)
        {
            cool_down_count = cool_down_count-100;
            offset = 100;
        }
        else
        {
            offset = triac_offset_min;
        }
        
        
    } else if (p_avg <= 20)
    {
        if (cool_down_count > 0)
        {
            cool_down_count = cool_down_count-100;
            offset = 100;
        }
        else
        {
            offset = 500;
        }
    } 
    NRF_LOG_INFO("offset:                  %u c", offset);
    NRF_LOG_INFO("p_avg:                  %u W", p_avg);
    NRF_LOG_INFO("cooldowncount            %i c",cool_down_count);
    
}

void cool_down(uint16_t bike_power)
{
    if (bike_power > 40)
    {
        
    }

}

void set_power_mode (triac_select_t t_up_down)
{
    switch (t_up_down)
    {
        case TRIAC_POWER_UP:
            triac_power_level++;
            if(triac_power_level > 5)
            {
                triac_power_level = TRIAC_350;
            }
            break;

        case TRIAC_POWER_DOWN:
            triac_power_level--;
            if(triac_power_level > 5)
            {
                triac_power_level = TRIAC_100;
            }
            break;

        default:
            break;
    }
    NRF_LOG_INFO("Triac powerlevel          %u",triac_power_level);
}



//150 = 4.683ms, 4.697ms
//75 = 2.454ms
//1800 = 59
//4000 = 586952301
