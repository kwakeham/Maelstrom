/**
* Put some licence BS here
* Keith Wakeham 2019
*/



#include <stdint.h>
#include "string.h"
#include "app_error.h"
#include "app_timer.h"
#include "app_button.h"
#include "boards.h"
#include "sdk_config.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "mael_btn_led.h"
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "triac.h"



// APP_TIMER_DEF(m_button_action);
APP_TIMER_DEF(m_button_action);
APP_TIMER_DEF(m_repeat_action);

APP_TIMER_DEF(m_input_override);

#define BUTTON_STATE_POLL_INTERVAL_MS  5UL
#define MULTI_PRESS_INTERVAL_MS  600UL
#define LED_SHOW_INTERVAL  8000UL

#define LONG_PRESS(MS)    (uint32_t)(MS)/BUTTON_STATE_POLL_INTERVAL_MS 
#define DOUBLE_PRESS(MS)    (uint32_t)(MS)/BUTTON_STATE_POLL_INTERVAL_MS 

static app_button_cfg_t  button_cfg[]= {{BUTTON_1, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_callback}, //Hey idiot, don't forget to check the manual checkers down below
                    {BUTTON_2, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_callback},
                    {BUTTON_3, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_callback}};
static maelbtn_event_callback_t   m_registered_callback         = NULL;


// static app_button_cfg_t  button_cfg;
static uint8_t btn_1_press_count         = 0;
static uint8_t btn_2_press_count         = 0;
static uint8_t btn_3_press_count         = 0;
static bool long_press_active_1;
static bool long_press_active_2;
static bool long_press_active_3;
static int32_t cnt_1 = 0;
static int32_t cnt_2 = 0;
static int32_t cnt_3 = 0;

bool show_setting_led = false;


void button_timeout_handler (void * p_context)
{ 
    // NRF_LOG_INFO("button timeout");
    uint32_t err_code;
    bool btn_1_pushed = app_button_is_pushed(0);
    bool btn_2_pushed = app_button_is_pushed(1);
    bool btn_3_pushed = app_button_is_pushed(2);
    bool timer_run = false;

    if (btn_1_pushed && btn_2_pushed)
    {
        cnt_2++;
        cnt_1++;
        if ( cnt_1 >= LONG_PRESS(1000))
        {
            cnt_1 = cnt_1 - LONG_PRESS(1000);
            if (!long_press_active_1)
            {
                long_press_active_1 = true;
                m_registered_callback(MAEL_BTN_EVENT_1_LONG);
            } else
            {
                m_registered_callback(MAEL_BTN_EVENT_1_LONG_CONT);
            }
            btn_1_press_count = 0;
        } 

        if ( cnt_2 >= LONG_PRESS(1000))
        {
            cnt_2 = cnt_2 - LONG_PRESS(1000);
            if (!long_press_active_2)
            {
                long_press_active_2 = true;
                m_registered_callback(MAEL_BTN_EVENT_2_LONG);
            } else
            {
                m_registered_callback(MAEL_BTN_EVENT_2_LONG_CONT);
            }
            btn_2_press_count = 0;
        } 
        timer_run = true;
        cnt_3 = 0;
    }
    else if (btn_2_pushed)
    {
        // NRF_LOG_INFO("btn 2 pressed");
        cnt_2++;
        if ( cnt_2 >= LONG_PRESS(1000))
        {
            cnt_2 = cnt_2 - LONG_PRESS(1000);
            if (!long_press_active_2)
            {
                long_press_active_2 = true;
                m_registered_callback(MAEL_BTN_EVENT_2_LONG);
                
            } else
            {
                m_registered_callback(MAEL_BTN_EVENT_2_LONG_CONT);
            }
            btn_2_press_count = 0;
        } 
        timer_run = true;
        cnt_1 = 0; //These are suppose to be opposite, so if btn2 is pushed btn1 is reset
        cnt_3 = 0;
    }
    else if (btn_1_pushed)
    {
        cnt_1++;
        if ( cnt_1 >= LONG_PRESS(1000))
        {
            cnt_1 = cnt_1 - LONG_PRESS(1000);
            if (!long_press_active_1)
            {
                long_press_active_1 = true;
                m_registered_callback(MAEL_BTN_EVENT_1_LONG);
            } else
            {
                m_registered_callback(MAEL_BTN_EVENT_1_LONG_CONT);
            }
            btn_1_press_count = 0;
        } 
        timer_run = true;
        cnt_2 = 0; //These are suppose to be opposite, so if btn2 is pushed btn1 is reset
        cnt_3 = 0;
    }
    else if (btn_3_pushed)
    {
        cnt_3++;
        if ( cnt_3 >= LONG_PRESS(1000))
        {
            cnt_3 = cnt_3 - LONG_PRESS(1000);
            if (!long_press_active_3)
            {
                long_press_active_3 = true;
                m_registered_callback(MAEL_BTN_EVENT_3_LONG);
            } else
            {
                m_registered_callback(MAEL_BTN_EVENT_3_LONG_CONT);
            }
            btn_3_press_count = 0;
        } 
        timer_run = true;
        cnt_1 = 0; //These are suppose to be opposite, so if btn2 is pushed btn1 is reset
        cnt_2 = 0;
    }
    else //button isn't press1
    {
        if (long_press_active_2)
        {
            long_press_active_2 = false;
        } 
        else if (!long_press_active_2 && cnt_2 >= (LONG_PRESS(600)))  // This fixes the edge case of between 600ms time out and the 1000ms long
        {
            m_registered_callback(MAEL_BTN_EVENT_2);
            // NRF_LOG_INFO("Edgecase");
        }

        if (long_press_active_1)
        {
            long_press_active_1 = false;
        } 
        else if (!long_press_active_1 && cnt_1 >= (LONG_PRESS(600)))  // This fixes the edge case of between 600ms time out and the 1000ms long
        {
            m_registered_callback(MAEL_BTN_EVENT_1);
        }

        if (long_press_active_3)
        {
            long_press_active_3 = false;
        } 
        else if (!long_press_active_3 && cnt_3 >= (LONG_PRESS(600)))  // This fixes the edge case of between 600ms time out and the 1000ms long
        {
            m_registered_callback(MAEL_BTN_EVENT_3);
        }
        timer_run = true;
        cnt_2 = 0; // Reset the counter
        cnt_1 = 0;
        cnt_3 = 0;
    }

    if (timer_run)
    {
        err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);
    }

} // end function

void repeat_timeout_handler(void * p_context)
{ 
    if (!long_press_active_1  && (cnt_1 < (LONG_PRESS(600)-1) ))
    {
        // NRF_LOG_INFO("repeat_timeout_handler: btn 1 pressed %d with cnt = %d", btn_1_press_count, cnt_2 );
        if (btn_1_press_count == 1)
        {
            m_registered_callback(MAEL_BTN_EVENT_1);
            // NRF_LOG_INFO("Edgecase");
        } else if (btn_1_press_count == 2)
        {
            m_registered_callback(MAEL_BTN_EVENT_1_DOUBLE);
        } else if (btn_1_press_count == 3)
        {
            m_registered_callback(MAEL_BTN_EVENT_1_TRIPLE);
        }
        btn_1_press_count = 0;
        // btn_2_press_count = 0;
    }
    if (!long_press_active_2 && (cnt_2 < (LONG_PRESS(600)-1)))
    {
        //Button 2 stuff
        if (btn_2_press_count == 1)
        {
            m_registered_callback(MAEL_BTN_EVENT_2);
        } else if (btn_2_press_count == 2)
        {
            m_registered_callback(MAEL_BTN_EVENT_2_DOUBLE);
        } else if (btn_2_press_count == 3)
        {
            m_registered_callback(MAEL_BTN_EVENT_2_TRIPLE);
        }
        
        btn_2_press_count = 0;
    }
    if (!long_press_active_3 && (cnt_3 < (LONG_PRESS(600)-1)))
    {
        //Button 2 stuff
        if (btn_3_press_count == 1)
        {
            m_registered_callback(MAEL_BTN_EVENT_3);
        } else if (btn_3_press_count == 2)
        {
            m_registered_callback(MAEL_BTN_EVENT_3_DOUBLE);
        } else if (btn_3_press_count == 3)
        {
            m_registered_callback(MAEL_BTN_EVENT_3_TRIPLE);
        }
        
        btn_3_press_count = 0;
    }

} // end function


void button_callback(uint8_t pin_no, uint8_t button_action)
{
    uint8_t button_test;
      uint32_t err_code;
      if ((pin_no == BUTTON_1) && (button_action == APP_BUTTON_PUSH))
      {
        button_test = 1;
        NRF_LOG_INFO("Button set to %d", button_test);

        err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);

        err_code = app_timer_start(m_repeat_action, APP_TIMER_TICKS(MULTI_PRESS_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);
        btn_1_press_count++;

      } else if ((pin_no == BUTTON_2) && (button_action == APP_BUTTON_PUSH))
      {
        // NRF_LOG_INFO("button 14!!!");
        button_test = 2;
        NRF_LOG_INFO("Button set to %d", button_test);

        err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);

        err_code = app_timer_start(m_repeat_action, APP_TIMER_TICKS(MULTI_PRESS_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);
        btn_2_press_count++;
      } else if ((pin_no == BUTTON_3) && (button_action == APP_BUTTON_PUSH))
      {
        // NRF_LOG_INFO("button 14!!!");
        button_test = 3;
        NRF_LOG_INFO("Button set to %d", button_test);

        err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);

        err_code = app_timer_start(m_repeat_action, APP_TIMER_TICKS(MULTI_PRESS_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);
        btn_3_press_count++;
      }
}


void mael_buttons_init(maelbtn_event_callback_t callback)
{
    uint32_t err_code;
      
    // button_cfg.pin_no         = 13;
    // button_cfg.button_handler = button_callback;
    // button_cfg.pull_cfg       = NRF_GPIO_PIN_PULLUP;
    // button_cfg.active_state   = APP_BUTTON_ACTIVE_LOW;

    //   button_cfg
    m_registered_callback = callback;

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("MAEL app timer init");

    err_code = app_button_init(button_cfg, sizeof(button_cfg) / sizeof(button_cfg[0]), APP_TIMER_TICKS(40));
    // err_code = app_button_init(&button_cfg, 1, APP_TIMER_TICKS(40));
    // NRF_LOG_ERROR("Failed to init ", err_code);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("MAEL app button init");
      
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("MAEL app enable");
    
    err_code = app_timer_create(&m_button_action, APP_TIMER_MODE_SINGLE_SHOT, button_timeout_handler );
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("MAEL app button init");
    
    err_code = app_timer_create(&m_repeat_action, APP_TIMER_MODE_SINGLE_SHOT, repeat_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_input_override, APP_TIMER_MODE_SINGLE_SHOT, override_callback);
    APP_ERROR_CHECK(err_code);

    mael_init_leds();
}

void mael_buttons_disable()
{
    uint32_t err_code;

    err_code = app_timer_stop(m_button_action);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_stop(m_repeat_action);
    APP_ERROR_CHECK(err_code);
}

void mael_init_leds()
{
    nrf_gpio_cfg_output(LED_1R);
    nrf_gpio_cfg_output(LED_1G);
    nrf_gpio_cfg_output(LED_1B);

    nrf_gpio_cfg_output(LED_2R);
    nrf_gpio_cfg_output(LED_2G);
    nrf_gpio_cfg_output(LED_2B);

    nrf_gpio_cfg_output(LED_3R);
    nrf_gpio_cfg_output(LED_3G);
    nrf_gpio_cfg_output(LED_3B);

    mael_led_clear_all();

    mael_test_leds();
}

void mael_test_leds()
{
    nrf_gpio_pin_clear(LED_1R);
    nrf_gpio_pin_clear(LED_2R);
    nrf_gpio_pin_clear(LED_3R);

    nrf_delay_ms(500);

    nrf_gpio_pin_set(LED_1R);
    nrf_gpio_pin_set(LED_2R);
    nrf_gpio_pin_set(LED_3R);

    nrf_delay_ms(500);

    nrf_gpio_pin_clear(LED_1G);
    nrf_gpio_pin_clear(LED_2G);
    nrf_gpio_pin_clear(LED_3G);

    nrf_delay_ms(500);

    nrf_gpio_pin_set(LED_1G);
    nrf_gpio_pin_set(LED_2G);
    nrf_gpio_pin_set(LED_3G);
    
    nrf_delay_ms(500);

    nrf_gpio_pin_clear(LED_1B);
    nrf_gpio_pin_clear(LED_2B);
    nrf_gpio_pin_clear(LED_3B);

    nrf_delay_ms(500);

    nrf_gpio_pin_set(LED_1B);
    nrf_gpio_pin_set(LED_2B);
    nrf_gpio_pin_set(LED_3B);

}

void mael_led_toggle()
{
    nrf_gpio_pin_toggle(LED_1R);
}



void override_callback(void * p_context)
{
    mael_led_display(MAEL_LED_EVENT_NOTHING);
    NRF_LOG_INFO ("sent to clear");
    triac_set_normal();
}

void mael_led_display(mael_led_event_t led_status)
{
    NRF_LOG_INFO("ENUM:          %d",led_status);
    uint32_t err_code;
    if(!show_setting_led)
    {
        mael_led_clear_all();
        switch (led_status)
        {
        case MAEL_LED_EVENT_NOTHING:
            break;

        case MAEL_LED_POWER_1:
            nrf_gpio_pin_clear(LED_3G);
            break;

        case MAEL_LED_POWER_2:
            nrf_gpio_pin_clear(LED_2G);
            nrf_gpio_pin_clear(LED_3G);
            break;

        case MAEL_LED_POWER_3:
            nrf_gpio_pin_clear(LED_1G);
            nrf_gpio_pin_clear(LED_2G);
            nrf_gpio_pin_clear(LED_3G);
            break;

        case MAEL_LED_POWER_4:
            nrf_gpio_pin_clear(LED_3B);
            break;

        case MAEL_LED_POWER_5:
            nrf_gpio_pin_clear(LED_2B);
            nrf_gpio_pin_clear(LED_3B);
            break;

        case MAEL_LED_POWER_6:
            nrf_gpio_pin_clear(LED_1B);
            nrf_gpio_pin_clear(LED_2B);
            nrf_gpio_pin_clear(LED_3B);
            break;

        case MAEL_LED_BLUE_3:
            nrf_gpio_pin_clear(LED_1B);
            nrf_gpio_pin_clear(LED_2B);
            nrf_gpio_pin_clear(LED_3B);
            break;

        case MAEL_LED_BLUE_2:
            nrf_gpio_pin_clear(LED_2B);
            nrf_gpio_pin_clear(LED_3B);
            break;

        case MAEL_LED_BLUE_1:
            nrf_gpio_pin_clear(LED_3B);
            break;

        case MAEL_LED_GREEN_3:
            nrf_gpio_pin_clear(LED_1G);
            nrf_gpio_pin_clear(LED_2G);
            nrf_gpio_pin_clear(LED_3G);
            break;

        case MAEL_LED_GREEN_2:
            nrf_gpio_pin_clear(LED_2G);
            nrf_gpio_pin_clear(LED_3G);
            break;

        case MAEL_LED_GREEN_1:
            nrf_gpio_pin_clear(LED_3G);
            break;

        case MAEL_LED_RED_3:
            nrf_gpio_pin_clear(LED_1R);
            nrf_gpio_pin_clear(LED_2R);
            nrf_gpio_pin_clear(LED_3R);
            break;

        case MAEL_LED_RED_2:
            nrf_gpio_pin_clear(LED_2R);
            nrf_gpio_pin_clear(LED_3R);
            break;

        case MAEL_LED_RED_1:
            nrf_gpio_pin_clear(LED_3R);
            break;
        
        default:
            break;
        } // End switch
        if (led_status == MAEL_LED_EVENT_NOTHING)
        {
            err_code =  app_timer_stop(m_input_override);
            APP_ERROR_CHECK(err_code);
        } else
        {
            err_code =  app_timer_stop(m_input_override);
            APP_ERROR_CHECK(err_code);
            err_code = app_timer_start(m_input_override, APP_TIMER_TICKS(LED_SHOW_INTERVAL), NULL);
            APP_ERROR_CHECK(err_code);
        }
        
    } else
    { // This all needs fixing below this
        switch (led_status)
        {
        case MAEL_LED_EVENT_NOTHING:
            mael_led_clear_all();
            break;
        
        default:
            break;
        }
        show_setting_led = false;
    } //end show_setting LED, this isn't in use properly yet
}

void mael_led_clear_all(void)
{
    nrf_gpio_pin_set(LED_1R);
    nrf_gpio_pin_set(LED_2R);
    nrf_gpio_pin_set(LED_3R);
    nrf_gpio_pin_set(LED_1G);
    nrf_gpio_pin_set(LED_2G);
    nrf_gpio_pin_set(LED_3G);
    nrf_gpio_pin_set(LED_1B);
    nrf_gpio_pin_set(LED_2B);
    nrf_gpio_pin_set(LED_3B);
}