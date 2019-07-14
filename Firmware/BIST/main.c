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
#include "bsp.h"
#include "boards.h"
#include "hardfault.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ant.h"
#include "nrf_pwr_mgmt.h"
#include "antDI2Master.h"
#include "di2_btn.h"
#include "mpos.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define BT1_LONG_PUSH BSP_EVENT_KEY_4
#define BT3_LONG_PUSH  BSP_EVENT_KEY_5

// APP_TIMER_DEF(m_button_action);
// #define BUTTON_STATE_POLL_INTERVAL_MS  100UL
// #define LONG_PRESS2(MS)    (uint32_t)(MS)/BUTTON_STATE_POLL_INTERVAL_MS 

/**@brief Function for the Timer and BSP initialization.
 */
// static void utils_setup(void)
// {
//     ret_code_t err_code = app_timer_init();
//     APP_ERROR_CHECK(err_code);

//     err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS,
//                         ant_message_types_master_bsp_evt_handler);
//     APP_ERROR_CHECK(err_code);

//     err_code = bsp_event_to_button_action_assign(BSP_BOARD_BUTTON_1, BSP_BUTTON_ACTION_PUSH, BSP_EVENT_NOTHING);
//     APP_ERROR_CHECK(err_code);

//     err_code = bsp_event_to_button_action_assign(BSP_BOARD_BUTTON_1, BSP_BUTTON_ACTION_RELEASE, BSP_EVENT_KEY_1);
//     APP_ERROR_CHECK(err_code);

//     err_code = bsp_event_to_button_action_assign(BSP_BOARD_BUTTON_3, BSP_BUTTON_ACTION_PUSH, BSP_EVENT_NOTHING);
//     APP_ERROR_CHECK(err_code);

//     err_code = bsp_event_to_button_action_assign(BSP_BOARD_BUTTON_3, BSP_BUTTON_ACTION_RELEASE, BSP_EVENT_KEY_3);
//     APP_ERROR_CHECK(err_code);

//     err_code = bsp_event_to_button_action_assign(BSP_BOARD_BUTTON_1, BSP_BUTTON_ACTION_LONG_PUSH, BT1_LONG_PUSH);
//     APP_ERROR_CHECK(err_code);

//     err_code = bsp_event_to_button_action_assign(BSP_BOARD_BUTTON_3, BSP_BUTTON_ACTION_LONG_PUSH, BT3_LONG_PUSH);
//     APP_ERROR_CHECK(err_code);

//     err_code = nrf_pwr_mgmt_init();
//     APP_ERROR_CHECK(err_code);
// }

/**@brief Function for ANT stack initialization.
 */
static void softdevice_setup(void)
{
    ret_code_t err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    ASSERT(nrf_sdh_is_enabled());

    err_code = nrf_sdh_ant_enable();
    APP_ERROR_CHECK(err_code);
}

/**
 *@brief Function for initializing logging.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

// void test_callback(di2btn_event_t event_list)
// {
//     // if (event_list == DI2_BTN_EVENT_LEFT)
//     // {
//     //     NRF_LOG_INFO ("it's a left event");
//     // } else
//     // {
//     //     // NRF_LOG_INFO ("it's not left! Oh noes!!!");
//     // }
//     switch (event_list)
//         {
//             case DI2_BTN_EVENT_NOTHING:
//                 NRF_LOG_INFO ("m_callback: it's nothing!");
//                 break;

//             case DI2_BTN_EVENT_LEFT:
//                 NRF_LOG_INFO ("m_callback: it's left!");
//                 break;

//             case DI2_BTN_EVENT_RIGHT:
//                 NRF_LOG_INFO ("m_callback: it's right!");
//                 break;

//             case DI2_BTN_EVENT_LEFT_LONG:
//                 NRF_LOG_INFO ("m_callback: it's left long!");
//                 break;

//             case DI2_BTN_EVENT_RIGHT_LONG:
//                 NRF_LOG_INFO ("m_callback: it's right long!");
//                 break;

//             case DI2_BTN_EVENT_LEFT_DOUBLE:
//                 NRF_LOG_INFO ("m_callback: it's left double!");
//                 break;

//             case DI2_BTN_EVENT_RIGHT_DOUBLE:
//                 NRF_LOG_INFO ("m_callback: it's right double!");
//                 break;

//             case DI2_BTN_EVENT_LEFT_TRIPLE:
//                 NRF_LOG_INFO ("m_callback: it's left triple!");
//                 break;

//             case DI2_BTN_EVENT_RIGHT_TRIPLE:
//                 NRF_LOG_INFO ("m_callback: it's right triple!");
//                 break;

//         }
    

// }

/**@brief Function for application main entry. Does not return.
 */
int main(void)
{
    log_init();
    di2_buttons_init(ant_message_types_master_bsp_evt_handler);
    // utils_setup();
    softdevice_setup();
    ant_message_types_master_setup();
    mpos_init();

    NRF_LOG_INFO("ANT Message Types example started.");

    // Enter main loop.
    for (;;)
    {
        NRF_LOG_FLUSH();
        nrf_pwr_mgmt_run();
    }
}
