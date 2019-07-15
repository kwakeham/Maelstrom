/**
 * This software is subject to the ANT+ Shared Source License
 * www.thisisant.com/swlicenses
 * Copyright (c) Garmin Canada Inc. 2015
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *    1) Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *
 *    2) Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 *    3) Neither the name of Garmin nor the names of its
 *       contributors may be used to endorse or promote products
 *       derived from this software without specific prior
 *       written permission.
 *
 * The following actions are prohibited:
 *
 *    1) Redistribution of source code containing the ANT+ Network
 *       Key. The ANT+ Network Key is available to ANT+ Adopters.
 *       Please refer to http://thisisant.com to become an ANT+
 *       Adopter and access the key. 
 *
 *    2) Reverse engineering, decompilation, and/or disassembly of
 *       software provided in binary form under this license.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE HEREBY
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; DAMAGE TO ANY DEVICE, LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE. SOME STATES DO NOT ALLOW 
 * THE EXCLUSION OF INCIDENTAL OR CONSEQUENTIAL DAMAGES, SO THE
 * ABOVE LIMITATIONS MAY NOT APPLY TO YOU.
 *
 */
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "app_timer.h"
#include "app_button.h"
#include "app_gpiote.h"
#include "boards.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define APP_TIMER_PRESCALER             0  // Value of the RTC1 PRESCALER register.
#define APP_TIMER_MAX_TIMERS            1  // Maximum number of simultaneously created timers. 
#define APP_TIMER_OP_QUEUE_SIZE         2  // Size of timer operation queues. 
#define BUTTON_DEBOUNCE_DELAY			50 // Delay from a GPIOTE event until a button is reported as pushed. 
#define APP_GPIOTE_MAX_USERS            1  // Maximum number of users of the GPIOTE handler. 

/**@brief Function for ANT stack initialization.
 */
// static void softdevice_setup(void)
// {
//     ret_code_t err_code = nrf_sdh_enable_request();
//     APP_ERROR_CHECK(err_code);

//     ASSERT(nrf_sdh_is_enabled());

//     err_code = nrf_sdh_ant_enable();
//     APP_ERROR_CHECK(err_code);
// }

/**
 *@brief Function for initializing logging.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void init_clock()
{
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0); // Wait for clock to start
}	

void init_leds()
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

    nrf_gpio_pin_set(LED_1R);
    nrf_gpio_pin_set(LED_1G);
    nrf_gpio_pin_set(LED_1B);

    nrf_gpio_pin_set(LED_2R);
    nrf_gpio_pin_set(LED_2G);
    nrf_gpio_pin_set(LED_2B);

    nrf_gpio_pin_set(LED_3R);
    nrf_gpio_pin_set(LED_3G);
    nrf_gpio_pin_set(LED_3B);

    // nrf_gpio_cfg_output(LED_1);
    // nrf_gpio_cfg_output(LED_2);
    // nrf_gpio_cfg_output(LED_3);
    // nrf_gpio_cfg_output(LED_4);
    // nrf_gpio_pin_set(LED_1);
    // nrf_gpio_pin_set(LED_2);
    // nrf_gpio_pin_set(LED_3);
    // nrf_gpio_pin_set(LED_4);
}


// void ant_message_types_master_bsp_evt_handler(di2btn_event_t evt)
// {
//     // uint8_t update_data;
//     switch (evt)
//     {
//         case DI2_BTN_EVENT_NOTHING:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's nothing!");
//             break;

//         case DI2_BTN_EVENT_LEFT:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left!");
//             break;

//         case DI2_BTN_EVENT_RIGHT:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right!");
//             break;

//         case DI2_BTN_EVENT_LEFT_LONG:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left long!");
//             break;

//         case DI2_BTN_EVENT_RIGHT_LONG:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right long!");
//             break;

//         case DI2_BTN_EVENT_LEFT_LONG_CONT:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left long continued!");
//             break;

//         case DI2_BTN_EVENT_RIGHT_LONG_CONT:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right long continued!");
//             break;

//         case DI2_BTN_EVENT_LEFT_DOUBLE:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left double!");
//             break;

//         case DI2_BTN_EVENT_RIGHT_DOUBLE:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right double!");
//             break;

//         case DI2_BTN_EVENT_LEFT_TRIPLE:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left triple!");
//             break;

//         case DI2_BTN_EVENT_RIGHT_TRIPLE:
//             NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right triple!");
//             break;

//         default:
//             break;
//         }
    
    
// }

static void button_handler(uint8_t pin_no, uint8_t button_action)
{
    NRF_LOG_INFO("Into handler");
    if(button_action == APP_BUTTON_PUSH)
    {
        switch(pin_no)
        {
            case (BUTTON_1):
                nrf_gpio_pin_toggle(LED_1R);
                nrf_gpio_pin_toggle(LED_2R);
                nrf_gpio_pin_toggle(LED_3R);
                break;
            case (BUTTON_2):
                nrf_gpio_pin_toggle(LED_1G);
                nrf_gpio_pin_toggle(LED_2G);
                nrf_gpio_pin_toggle(LED_3G);
                break;
            case (BUTTON_3):
                nrf_gpio_pin_toggle(LED_1B);
                nrf_gpio_pin_toggle(LED_2B);
                nrf_gpio_pin_toggle(LED_3B);
                break;
            default:
                break;
            //             case BUTTON_1:
            //     nrf_gpio_pin_toggle(LED_1);
            //     break;
            // case BUTTON_2:
            //     nrf_gpio_pin_toggle(LED_2);
            //     break;
            // case BUTTON_3:
            //     nrf_gpio_pin_toggle(LED_3);
            //     break;
            // case BUTTON_4:
            //     nrf_gpio_pin_toggle(LED_4);
            //     break;
            // default:
            //     break;
        }
    }
}

/**@brief Function for application main entry. Does not return.
 */
int main(void)
{
    uint32_t err_code;
    log_init();
    NRF_LOG_INFO("MAELSTROM: QUICK TEST");

    init_leds();
    init_clock();

    if(!nrf_drv_gpiote_is_init())
    {
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    }
    // di2_buttons_init(ant_message_types_master_bsp_evt_handler);

    // ant_message_types_master_setup();

    static app_button_cfg_t p_button[] = {  {BUTTON_1, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_handler},
                                            {BUTTON_2, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_handler},
                                            {BUTTON_3, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_handler}};

    // APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);

    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("app timer init");

    err_code = app_button_init(p_button, sizeof(p_button) / sizeof(p_button[0]), BUTTON_DEBOUNCE_DELAY);
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);

    // softdevice_setup();


    // Enter main loop.
    for (;;)
    {
        NRF_LOG_FLUSH();
        // nrf_pwr_mgmt_run();
    }
}
