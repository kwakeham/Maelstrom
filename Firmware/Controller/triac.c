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
// #include "nrf_log.h"
#include "mael_btn_led.h"


#define NRF_LOG_MODULE_NAME triac
#if NRF_TRIAC_LOG_ENABLED
#else // ANT_BPWR_LOG_ENABLED
#define NRF_LOG_LEVEL       3
#endif // ANT_BPWR_LOG_ENABLED
#include "nrf_log.h"
#include "titan_mem.h"

NRF_LOG_MODULE_REGISTER();


#define BWPR_TIMEOUT  5000UL


APP_TIMER_DEF(m_triac_timer_id);

APP_TIMER_DEF(m_bpwr_timer_id); 

APP_TIMER_DEF(m_bpwr_timeout_timer_id); 

bool ZC_pulse = 1;
uint32_t offset = 500;
uint16_t p_avg = 0;
uint32_t time_old = 0;
uint32_t time_diff = 0;
uint32_t time = 0;
int32_t cool_down_count = 0;

uint32_t zc_time = 0;
uint32_t zc_time_old = 0;
uint32_t zc_time_diff = 0;

//Variables for storing old crank page data
uint8_t old_crank_event_count;
uint16_t old_crank_period;
uint16_t old_crank_accumulated_torque;

triac_settings_t triac_power_level = TRIAC_200;

uint16_t triac_offset_min = 140; //min power = longest time
uint16_t triac_offset_max = 60; //max power = shortest time
uint16_t triac_power_min = 20;
uint16_t triac_power_max = 200;
float triac_slope = -0.5;
uint16_t triac_offset = 150;

bool m_triac_setting = false;

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

void timeout_handler2(void * p_context) //this is to keep the timers alive!!!!
{

}

//if there is no signal then turn off the fan
void no_signal_timeout_handler(void * p_context)
{
    set_power(0);
}


void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    ret_code_t err_code;
    // nrf_drv_gpiote_out_toggle(PIN_OUT);
    zc_time = app_timer_cnt_get();
    zc_time_diff = zc_time - zc_time_old;
    zc_time_old = zc_time;
    if (offset < 500)
    {
        err_code = app_timer_start(m_triac_timer_id, offset, NULL);
        APP_ERROR_CHECK(err_code);
        ZC_pulse = 1;
    }
    // NRF_LOG_INFO("time = %d",zc_time_diff);
    
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

    //Keeps timers alive for other reasons
    err_code = app_timer_create(&m_bpwr_timer_id, APP_TIMER_MODE_REPEATED, timeout_handler2);
    NRF_LOG_INFO("Timer repeated ");
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_bpwr_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT, no_signal_timeout_handler);
    NRF_LOG_INFO("bpwr timeout repeated ");
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_bpwr_timer_id, 128, NULL);
    APP_ERROR_CHECK(err_code);
}

void set_power(uint16_t bike_power)
{

    ret_code_t err_code;
    int16_t p_diff; //power difference
    uint8_t adjuster; //The calculated adjuster difference, to be calculated
    uint8_t cool_down_adjuster; //The calculated adjuster difference, to be calculated
    uint16_t p_cool;

    time = app_timer_cnt_get();
    time_diff = time - time_old;
    time_old = time;
    // NRF_LOG_INFO("time:                  %u s", time_diff);
    adjuster = time_diff/8180;
    cool_down_adjuster=(time_diff+1638)/3277;
    // NRF_LOG_INFO("factor1:                  %u s", adjuster);
    if (adjuster > 10)
    {
        adjuster = 10;
    }
    if (cool_down_adjuster > 50)
    {
        cool_down_adjuster = 50;
    }
    // NRF_LOG_INFO("factor2:                  %u s", cool_down_adjuster);
    p_diff = bike_power - p_avg;
    // NRF_LOG_INFO("p_diff:                  %i W", p_diff);
    p_diff = (p_diff*adjuster)/10;
    // NRF_LOG_INFO("p_diff:                  %i W", p_diff);
    p_avg = p_avg + p_diff;
    p_cool = (bike_power*adjuster)/4;

    if (!m_triac_setting)
    {
        if (p_avg > triac_power_max)
        {
            offset = triac_offset_max;
            cool_down_count= cool_down_count+p_cool;
        }
        // else if (p_avg <= 200 && p_avg > 40) //(p_avg <= 200 && p_avg > 100) //This is for -0.8 and 220
        else if (p_avg <= triac_power_max && p_avg > 20) //(p_avg <= 200 && p_avg > 100) //This is for -0.8 and 220
        {
            offset = (p_avg*triac_slope)+triac_offset;
            cool_down_count= cool_down_count+p_cool;
        }
        // else if (p_avg <= 40 && p_avg > 20)
        // {
        //     if (cool_down_count > 0)
        //     {
        //         cool_down_count = cool_down_count-100;
        //         offset = 100;
        //     }
        //     else
        //     {
        //         offset = triac_offset_min;
        //     }
        // }
        else if (p_avg <= 20)
        {
            if (cool_down_count > 0)
            {
                cool_down_count = cool_down_count-(triac_power_max*cool_down_adjuster*12/10);
                offset = triac_offset_min-triac_offset_max;
            }
            else
            {
                offset = 500;
                cool_down_count = 0;
            }
        } //end else if p_avg <=20
    } //end m_triac_setting


//this sets the timer to run for power time out
    err_code =  app_timer_stop(m_bpwr_timeout_timer_id);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(m_bpwr_timeout_timer_id, APP_TIMER_TICKS(BWPR_TIMEOUT), NULL);
    APP_ERROR_CHECK(err_code);

    // NRF_LOG_INFO("offset:                  %u c", offset);
    // NRF_LOG_INFO("p_avg:                  %u W", p_avg);
    // NRF_LOG_INFO("cooldowncount            %i c",cool_down_count);
    
    NRF_LOG_INFO("basic_bike_power:                   %u c", bike_power);
}

void set_crank_power(uint8_t crank_event_count, uint16_t crank_period, uint16_t crank_accumulated_torque)
{
    float crank_average_angular_vel;
    float crank_average_torque;
    float crank_average_power;
    if (crank_event_count == old_crank_event_count)
    {
        // NRF_LOG_INFO ("nothing to see here by's");
    } else
    {
        // NRF_LOG_INFO("crank_event_count:                  %u c", crank_event_count);
        // NRF_LOG_INFO("crank_period:                  %u W", crank_period);
        // NRF_LOG_INFO("crank_accumulated_torque            %u c",crank_accumulated_torque);

        uint8_t crank_event_count_diff = crank_event_count - old_crank_event_count;
        // NRF_LOG_INFO("crank_event_count_diff:                   %u c", crank_event_count_diff);
        uint16_t crank_period_diff = crank_period - old_crank_period;
        // NRF_LOG_INFO("crank_period_diff:                   %u c", crank_period_diff);
        uint16_t crank_accumulated_torque_diff = crank_accumulated_torque - old_crank_accumulated_torque;
        // NRF_LOG_INFO("crank_accumulated_torque_diff:                   %u c", crank_accumulated_torque_diff);

        crank_average_angular_vel = (2.0*3.141592654*((float)crank_event_count_diff))/((float)crank_period_diff/2048.0);
        // NRF_LOG_INFO("crank_average_angular_vel:                   "NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(crank_average_angular_vel));
        crank_average_torque =  (float)crank_accumulated_torque_diff/(32.0*(float)crank_event_count_diff);
        // NRF_LOG_INFO("crank_average_torque:                   "NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(crank_average_torque));
        crank_average_power = crank_average_angular_vel*crank_average_torque;
        NRF_LOG_INFO("crank_average_power:                   "NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(crank_average_power));

        uint16_t bike_power = (crank_average_power+0.5); //round this for more consistent power
        old_crank_event_count = crank_event_count;
        old_crank_period = crank_period;
        old_crank_accumulated_torque = crank_accumulated_torque;
        NRF_LOG_DEBUG("crank_bike_power:                   %u c", bike_power);
        set_power(bike_power);
        mael_led_toggle();
    }
}

void cool_down(uint16_t bike_power)
{
    if (bike_power > 40)
    {
        
    }

}

void triac_set_normal(void)
{
    m_triac_setting = false;
    offset = 500;
}

void triac_set_setting_mode(void)
{
    m_triac_setting = true;
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
            mael_led_display(triac_power_level+1);
            break;

        case TRIAC_POWER_DOWN:
            triac_power_level--;
            if(triac_power_level > 5)
            {
                triac_power_level = TRIAC_100;
            }
            mael_led_display(triac_power_level+1);
            break;

        default:
            break;
    }
    NRF_LOG_INFO("Triac powerlevel          %u",triac_power_level);
    triac_settings();
}

void triac_settings(void)
{
    triac_power_max = triac_power_level * 50 + 100;
    NRF_LOG_INFO("Triac Max Power        %u",triac_power_max);
    triac_offset = ((triac_power_min*triac_offset_max)-(triac_power_max*triac_offset_min))/(triac_power_min - triac_power_max);
    triac_slope = (float)((float)triac_offset_min - (float)triac_offset)/(float)(triac_power_min);
    NRF_LOG_INFO( "slope: "NRF_LOG_FLOAT_MARKER , NRF_LOG_FLOAT(triac_slope));
    NRF_LOG_INFO( "offset: %u", triac_offset);
    triac_store_settings(); //store settings
    mem_mael_write();
}

void set_triac_offset_max(void)
{
    offset = offset - 5;
    if (offset < 20)
    {
        offset = 60;
    }
    triac_offset_max = offset;
    NRF_LOG_INFO("triac max offset =         %d",offset);
    triac_settings(); //update the triac settings (do the maths!)
    mael_led_display((triac_offset_max/5)+3); //20 = 4 ~ 7,  60 = 12 ~ 15
    triac_set_setting_mode(); //Block the set_power_mode
}

void set_triac_offset_max_reset(void)
{
    offset = 60;
    triac_offset_max = offset;
    NRF_LOG_INFO("triac max offset reset =         %d",offset);
    triac_settings();
    mael_led_display((triac_offset_max/5)+3); //20 = 4 ~ 7,  60 = 12 ~ 15
    triac_set_setting_mode(); //Block the set_power_mode
}

void set_triac_offset_min(void)
{
    offset = offset + 5;
    if (offset > 180)
    {
        offset = 140;
    }
    triac_offset_min = offset;
    NRF_LOG_INFO("triac min offset =         %d",offset);
    triac_settings();
    mael_led_display( (triac_offset_min/5)-21); //180 = 36 ~ 15,  140 = 28 ~ 7
    triac_set_setting_mode(); //Block the set_power_mode

}

void set_triac_offset_min_reset(void)
{
    offset = 140;
    triac_offset_min = offset;
    NRF_LOG_INFO("triac max offset reset =         %d",offset);
    triac_settings();
    mael_led_display( (triac_offset_min/5)-21); //180 = 36 ~ 15,  140 = 28 ~ 7
    triac_set_setting_mode(); //Block the set_power_mode
    
}

void triac_store_settings(void)
{
    mem_mael_triac_update(triac_offset_min, triac_offset_max, triac_power_level);
}



//150 = 4.683ms, 4.697ms
//75 = 2.454ms
//1800 = 59
//4000 = 586952301
