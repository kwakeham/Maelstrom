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
#include "di2_btn.h"
#include "app_util_platform.h"



// APP_TIMER_DEF(m_button_action);
APP_TIMER_DEF(m_button_action);
APP_TIMER_DEF(m_repeat_action);

#define BUTTON_STATE_POLL_INTERVAL_MS  5UL
#define MULTI_PRESS_INTERVAL_MS  600UL

#define LONG_PRESS(MS)    (uint32_t)(MS)/BUTTON_STATE_POLL_INTERVAL_MS 
#define DOUBLE_PRESS(MS)    (uint32_t)(MS)/BUTTON_STATE_POLL_INTERVAL_MS 

static app_button_cfg_t  button_cfg[]= {{29, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_callback}, //Hey idiot, don't forget to check the manual checkers down below
                    {28, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_callback}};
static di2btn_event_callback_t   m_registered_callback         = NULL;


// static app_button_cfg_t  button_cfg;
static uint8_t btn_1_press_count         = 0;
static uint8_t btn_2_press_count         = 0;
static bool long_press_active_right;
static bool long_press_active_left;
static int32_t cnt_r = 0;
static int32_t cnt_l = 0;
// static uint8_t button_test;

// void (*di2_ble_keysend)(uint8_t key_pattern_len, uint8_t * p_key_pattern);

// void di2_ble_register_callback(di2_ble_keysend_t callback)
// {
//     di2_ble_keysend_registered_callback = callback;
//     NRF_LOG_INFO("It should have set the callback");
// }

// void test_keys_send(uint8_t key_pattern_len, uint8_t * p_key_pattern)
// {
//     NRF_LOG_INFO("Length and pattern =");
// }

void button_timeout_handler (void * p_context)
{ 
    uint32_t err_code;
    bool left_pushed = app_button_is_pushed(1);
    bool right_pushed = app_button_is_pushed(0);
    bool timer_run = false;

    if (left_pushed && right_pushed)
    {
        cnt_r++;
        cnt_l++;
        if ( cnt_l >= LONG_PRESS(1000))
        {
            cnt_l = cnt_l - LONG_PRESS(1000);
            if (!long_press_active_left)
            {
                long_press_active_left = true;
                m_registered_callback(DI2_BTN_EVENT_LEFT_LONG);
            } else
            {
                m_registered_callback(DI2_BTN_EVENT_LEFT_LONG_CONT);
            }
            btn_2_press_count = 0;
        } 

        if ( cnt_r >= LONG_PRESS(1000))
        {
            cnt_r = cnt_r - LONG_PRESS(1000);
            if (!long_press_active_right)
            {
                long_press_active_right = true;
                m_registered_callback(DI2_BTN_EVENT_RIGHT_LONG);
            } else
            {
                m_registered_callback(DI2_BTN_EVENT_RIGHT_LONG_CONT);
            }
            btn_1_press_count = 0;
        } 
        timer_run = true;
    }
    else if (right_pushed)
    {
        cnt_r++;
        if ( cnt_r >= LONG_PRESS(1000))
        {
            cnt_r = cnt_r - LONG_PRESS(1000);
            if (!long_press_active_right)
            {
                long_press_active_right = true;
                m_registered_callback(DI2_BTN_EVENT_RIGHT_LONG);
                
            } else
            {
                m_registered_callback(DI2_BTN_EVENT_RIGHT_LONG_CONT);
            }
            btn_1_press_count = 0;
        } 
        timer_run = true;
        cnt_l = 0;
    }
    else if (left_pushed)
    {
        cnt_l++;
        if ( cnt_l >= LONG_PRESS(1000))
        {
            cnt_l = cnt_l - LONG_PRESS(1000);
            if (!long_press_active_left)
            {
                long_press_active_left = true;
                // di2_ble_keysend_registered_callback(sample_key_l,&sample_key); //TEST
                m_registered_callback(DI2_BTN_EVENT_LEFT_LONG);
            } else
            {
                m_registered_callback(DI2_BTN_EVENT_LEFT_LONG_CONT);
            }
            btn_2_press_count = 0;
        } 
        timer_run = true;
        cnt_r = 0;
    }

    else //button isn't press1
    {
        if (long_press_active_right)
        {
            long_press_active_right = false;
        } 
        else if (!long_press_active_right && cnt_r >= (LONG_PRESS(600)))  // This fixes the edge case of between 600ms time out and the 1000ms long
        {
            m_registered_callback(DI2_BTN_EVENT_RIGHT);
            // NRF_LOG_INFO("Edgecase");
        }

        if (long_press_active_left)
        {
            long_press_active_left = false;
        } 
        else if (!long_press_active_left && cnt_l >= (LONG_PRESS(600)))  // This fixes the edge case of between 600ms time out and the 1000ms long
        {
            m_registered_callback(DI2_BTN_EVENT_LEFT);
        }
        cnt_r = 0; // Reset the counter
        cnt_l = 0;
    }

    if (timer_run)
    {
        err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);
    }

} // end function

void repeat_timeout_handler(void * p_context)
{ 
    if (!long_press_active_right  && (cnt_r < (LONG_PRESS(600)-1) ))
    {
        // NRF_LOG_INFO("repeat_timeout_handler: btn 1 pressed %d with cnt = %d", btn_1_press_count, cnt_r );
        if (btn_1_press_count == 1)
        {
            m_registered_callback(DI2_BTN_EVENT_RIGHT);
            // NRF_LOG_INFO("Edgecase");
        } else if (btn_1_press_count == 2)
        {
            m_registered_callback(DI2_BTN_EVENT_RIGHT_DOUBLE);
        } else if (btn_1_press_count == 3)
        {
            m_registered_callback(DI2_BTN_EVENT_RIGHT_TRIPLE);
        }
        btn_1_press_count = 0;
        // btn_2_press_count = 0;
    }
    if (!long_press_active_left && (cnt_l < (LONG_PRESS(600)-1)))
    {
        //Button 2 stuff
        if (btn_2_press_count == 1)
        {
            m_registered_callback(DI2_BTN_EVENT_LEFT);
        } else if (btn_2_press_count == 2)
        {
            m_registered_callback(DI2_BTN_EVENT_LEFT_DOUBLE);
        } else if (btn_2_press_count == 3)
        {
            m_registered_callback(DI2_BTN_EVENT_LEFT_TRIPLE);
        }
        
        btn_2_press_count = 0;
    }
    
    

} // end function


void button_callback(uint8_t pin_no, uint8_t button_action)
{
    uint8_t button_test;
      uint32_t err_code;
      if ((pin_no == 29) && (button_action == APP_BUTTON_PUSH))
      {
        button_test = 0;
        NRF_LOG_INFO("Button set to %d", button_test);

        err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);

        err_code = app_timer_start(m_repeat_action, APP_TIMER_TICKS(MULTI_PRESS_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);
        btn_1_press_count++;

      } else if ((pin_no == 28) && (button_action == APP_BUTTON_PUSH))
      {
        // NRF_LOG_INFO("button 14!!!");
        button_test = 1;
        NRF_LOG_INFO("Button set to %d", button_test);

        err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);

        err_code = app_timer_start(m_repeat_action, APP_TIMER_TICKS(MULTI_PRESS_INTERVAL_MS), NULL);
        APP_ERROR_CHECK(err_code);
        btn_2_press_count++;
      }
}


void di2_buttons_init(di2btn_event_callback_t callback)
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
    NRF_LOG_INFO("DI2 app timer init");

    err_code = app_button_init(button_cfg, sizeof(button_cfg) / sizeof(button_cfg[0]), APP_TIMER_TICKS(40));
    // err_code = app_button_init(&button_cfg, 1, APP_TIMER_TICKS(40));
    // NRF_LOG_ERROR("Failed to init ", err_code);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("DI2 app button init");
      
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("DI2 app enable");
    
    err_code = app_timer_create(&m_button_action, APP_TIMER_MODE_SINGLE_SHOT, button_timeout_handler );
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("DI2 app button init");
    
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
