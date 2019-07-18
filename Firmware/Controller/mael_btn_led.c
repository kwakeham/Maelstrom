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



// APP_TIMER_DEF(m_button_action);
APP_TIMER_DEF(m_button_action);
APP_TIMER_DEF(m_repeat_action);

#define BUTTON_STATE_POLL_INTERVAL_MS  5UL
#define MULTI_PRESS_INTERVAL_MS  600UL

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
static bool long_press_active_2;
static bool long_press_active_1;
static int32_t cnt_1 = 0;
static int32_t cnt_2 = 0;
// static int32_t cnt_3 = 0;


void button_timeout_handler (void * p_context)
{ 
    uint32_t err_code;
    bool btn_1_pushed = app_button_is_pushed(0);
    bool btn_2_pushed = app_button_is_pushed(1);
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
    }
    else if (btn_2_pushed)
    {
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
        cnt_2 = 0;
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
                // di2_ble_keysend_registered_callback(sample_key_l,&sample_key); //TEST
                m_registered_callback(MAEL_BTN_EVENT_1_LONG);
            } else
            {
                m_registered_callback(MAEL_BTN_EVENT_1_LONG_CONT);
            }
            btn_1_press_count = 0;
        } 
        timer_run = true;
        cnt_1 = 0;
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
        cnt_2 = 0; // Reset the counter
        cnt_1 = 0;
    }

    if (timer_run)
    {
        err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);
    }

} // end function

void repeat_timeout_handler(void * p_context)
{ 
    if (!long_press_active_2  && (cnt_2 < (LONG_PRESS(600)-1) ))
    {
        // NRF_LOG_INFO("repeat_timeout_handler: btn 1 pressed %d with cnt = %d", btn_1_press_count, cnt_2 );
        if (btn_1_press_count == 1)
        {
            m_registered_callback(MAEL_BTN_EVENT_2);
            // NRF_LOG_INFO("Edgecase");
        } else if (btn_1_press_count == 2)
        {
            m_registered_callback(MAEL_BTN_EVENT_2_DOUBLE);
        } else if (btn_1_press_count == 3)
        {
            m_registered_callback(MAEL_BTN_EVENT_2_TRIPLE);
        }
        btn_1_press_count = 0;
        // btn_2_press_count = 0;
    }
    if (!long_press_active_1 && (cnt_1 < (LONG_PRESS(600)-1)))
    {
        //Button 2 stuff
        if (btn_2_press_count == 1)
        {
            m_registered_callback(MAEL_BTN_EVENT_1);
        } else if (btn_2_press_count == 2)
        {
            m_registered_callback(MAEL_BTN_EVENT_1_DOUBLE);
        } else if (btn_2_press_count == 3)
        {
            m_registered_callback(MAEL_BTN_EVENT_1_TRIPLE);
        }
        
        btn_2_press_count = 0;
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
}

void di2_buttons_disable()
{
    uint32_t err_code;

    err_code = app_timer_stop(m_button_action);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_stop(m_repeat_action);
    APP_ERROR_CHECK(err_code);
}
