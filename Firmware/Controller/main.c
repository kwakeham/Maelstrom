/**
 * This software is subject to the ANT+ Shared Source License
 * www.thisisant.com/swlicenses
 * Copyright (c) Garmin Canada Inc. 2012
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
/*
 * Before compiling this example for NRF52, complete the following steps:
 * - Download the S212 SoftDevice from <a href="https://www.thisisant.com/developer/components/nrf52832" target="_blank">thisisant.com</a>.
 * - Extract the downloaded zip file and copy the S212 SoftDevice headers to <tt>\<InstallFolder\>/components/softdevice/s212/headers</tt>.
 * If you are using Keil packs, copy the files into a @c headers folder in your example folder.
 * - Make sure that @ref ANT_LICENSE_KEY in @c nrf_sdm.h is uncommented.
 */

#include <stdio.h>
#include "nrf.h"
#include "bsp.h"
#include "hardfault.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ant.h"
#include "ant_key_manager.h"
#include "ant_search_config.h"
#include "ant_bpwr.h"
#include "bsp_btn_ant.h"
#include "ant_state_indicator.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "triac.h"
#include "mael_btn_led.h"
#include "app_scheduler.h"
#include "ant_interface.h"
#include "nrf_delay.h"
#include "nrf_fstorage_sd.h" //does this need to be here? probably not
#include "titan_mem.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define SCHED_MAX_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE            /**< Maximum size of scheduler events. */
#ifdef SVCALL_AS_NORMAL_FUNCTION
#define SCHED_QUEUE_SIZE                    20                                         /**< Maximum number of events in the scheduler queue. More is needed in case of Serialization. */
#else
#define SCHED_QUEUE_SIZE                    10                                         /**< Maximum number of events in the scheduler queue. */
#endif

/** @snippet [ANT BPWR RX Instance] */
void ant_bpwr_evt_handler(ant_bpwr_profile_t * p_profile, ant_bpwr_evt_t event);


BPWR_DISP_CHANNEL_CONFIG_DEF(m_ant_bpwr,
                             BPWR_CHANNEL_NUM,
                             CHAN_ID_TRANS_TYPE,
                             CHAN_ID_DEV_NUM,
                             ANTPLUS_NETWORK_NUM);
BPWR_DISP_PROFILE_CONFIG_DEF(m_ant_bpwr,
                             ant_bpwr_evt_handler);

static ant_bpwr_profile_t m_ant_bpwr;
/** @snippet [ANT BPWR RX Instance] */

NRF_SDH_ANT_OBSERVER(m_ant_observer, ANT_BPWR_ANT_OBSERVER_PRIO,
                     ant_bpwr_disp_evt_handler, &m_ant_bpwr);


/** @snippet [ANT BPWR RX Profile handling] */

/**@brief Function for handling bsp events.
 */
// void bsp_evt_handler(bsp_event_t evt)
// {
//     ret_code_t            err_code;
//     ant_bpwr_page1_data_t page1;

//     switch (evt)
//     {
//         case BSP_EVENT_KEY_0:
//             // request to calibrating the sensor
//             page1    = ANT_BPWR_GENERAL_CALIB_REQUEST();
//             err_code = ant_bpwr_calib_request(&m_ant_bpwr, &page1);
//             APP_ERROR_CHECK(err_code);
//             break;

//         case BSP_EVENT_SLEEP:
//             nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
//             break;

//         default:
//             break;
//     }
// }

/**
 * @brief Function for shutdown events.
 *
 * @param[in]   event       Shutdown type.
 */

// static bool shutdown_handler(nrf_pwr_mgmt_evt_t event)
// {
//     ret_code_t err_code;

//     switch (event)
//     {
//         case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
//             err_code = bsp_btn_ant_sleep_mode_prepare();
//             APP_ERROR_CHECK(err_code);
//             break;

//         default:
//             break;
//     }

//     return true;
// }

// NRF_PWR_MGMT_HANDLER_REGISTER(shutdown_handler, APP_SHUTDOWN_HANDLER_PRIORITY);

/**@brief Function for handling Bicycle Power profile's events
 *
 */
void ant_bpwr_evt_handler(ant_bpwr_profile_t * p_profile, ant_bpwr_evt_t event)
{
    nrf_pwr_mgmt_feed();

    switch (event)
    {
        case ANT_BPWR_PAGE_1_UPDATED:
            // calibration data received from sensor
            NRF_LOG_DEBUG("Received calibration data");
            break;
        case ANT_BPWR_PAGE_16_UPDATED:
            set_power(p_profile->BPWR_PROFILE_instantaneous_power);
            mael_led_toggle();
            /* fall through */
        case ANT_BPWR_PAGE_17_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_18_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_80_UPDATED:
            /* fall through */
        case ANT_BPWR_PAGE_81_UPDATED:
            // data actualization
            NRF_LOG_DEBUG("Page was updated");
            break;

        case ANT_BPWR_CALIB_TIMEOUT:
            // calibration request time-out
            NRF_LOG_DEBUG("ANT_BPWR_CALIB_TIMEOUT");
            break;

        case ANT_BPWR_CALIB_REQUEST_TX_FAILED:
            // Please consider retrying the request.
            NRF_LOG_DEBUG("ANT_BPWR_CALIB_REQUEST_TX_FAILED");
            break;

        default:
            // never occurred
            break;
    }
}

/**
 * @brief Function for setup all thinks not directly associated with ANT stack/protocol.
 * @desc Initialization of: @n
 *         - app_tarce for debug.
 *         - app_timer, pre-setup for bsp.
 *         - bsp for signaling LEDs and user buttons.
 */
static void utils_setup(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);

    // err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS,
    //                     bsp_evt_handler);
    // APP_ERROR_CHECK(err_code);

    // err_code = bsp_btn_ant_init(m_ant_bpwr.channel_number, BPWR_DISP_CHANNEL_TYPE);
    // APP_ERROR_CHECK(err_code);

    // err_code = ant_state_indicator_init(m_ant_bpwr.channel_number, BPWR_DISP_CHANNEL_TYPE);
    // APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for ANT stack initialization.
 */
static void softdevice_setup(void)
{
    ret_code_t err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    ASSERT(nrf_sdh_is_enabled());

    err_code = nrf_sdh_ant_enable();
    APP_ERROR_CHECK(err_code);

    err_code = ant_plus_key_set(ANTPLUS_NETWORK_NUM);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for Bicycle Power profile initialization.
 *
 * @details Initializes the Bicycle Power profile and open ANT channel.
 */
static void profile_setup(void)
{
/** @snippet [ANT BPWR RX Profile Setup] */
    ret_code_t err_code = ant_bpwr_disp_init(&m_ant_bpwr,
                                             BPWR_DISP_CHANNEL_CONFIG(m_ant_bpwr),
                                             BPWR_DISP_PROFILE_CONFIG(m_ant_bpwr));
    APP_ERROR_CHECK(err_code);

    const ant_search_config_t bs_search_config =
    {
        .channel_number        = BPWR_CHANNEL_NUM,
        .low_priority_timeout  = ANT_LOW_PRIORITY_TIMEOUT_DISABLE,
        .high_priority_timeout = ANT_DEFAULT_HIGH_PRIORITY_TIMEOUT,
        .search_sharing_cycles = ANT_SEARCH_SHARING_CYCLES_DISABLE,
        .search_priority       = ANT_SEARCH_PRIORITY_DEFAULT,
        .waveform              = ANT_WAVEFORM_DEFAULT,
    };

    err_code = ant_search_init(&bs_search_config);
    APP_ERROR_CHECK(err_code);

    err_code = ant_bpwr_disp_open(&m_ant_bpwr);
    APP_ERROR_CHECK(err_code);

    err_code = ant_state_indicator_channel_opened();
    APP_ERROR_CHECK(err_code);
/** @snippet [ANT BPWR RX Profile Setup] */
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

static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

void test_callback(maelbtn_event_t event_list)
{
    uint16_t mael_devicenumber;
    uint8_t mael_devicetype;
    uint8_t mael_TransmitType;
    switch (event_list)
        {
            case MAEL_BTN_EVENT_NOTHING:
                NRF_LOG_INFO ("m_callback: it's nothing!");
                break;

            case MAEL_BTN_EVENT_1:
                NRF_LOG_INFO ("m_callback: it's btn 1!");
                sd_ant_channel_close(BPWR_CHANNEL_NUM);
                sd_ant_channel_id_get(BPWR_CHANNEL_NUM, &mael_devicenumber, &mael_devicetype, &mael_TransmitType);
                NRF_LOG_INFO ("Device number %d",mael_devicenumber);
                mael_devicenumber = 0;
                nrf_delay_ms(200);
                sd_ant_channel_id_set(BPWR_CHANNEL_NUM, mael_devicenumber, mael_devicetype, mael_TransmitType);
                ret_code_t err_code = ant_bpwr_disp_open(&m_ant_bpwr);
                APP_ERROR_CHECK(err_code);
                break;

            case MAEL_BTN_EVENT_2:
                NRF_LOG_INFO ("m_callback: it's btn 2!");
                set_power_mode (TRIAC_POWER_UP);
                break;

            case MAEL_BTN_EVENT_3:
                NRF_LOG_INFO ("m_callback: it's btn 3!");
                set_power_mode (TRIAC_POWER_DOWN);
                break;

            case MAEL_BTN_EVENT_1_LONG:
                NRF_LOG_INFO ("m_callback: it's long 1!");
                break;

            case MAEL_BTN_EVENT_2_LONG:
                NRF_LOG_INFO ("m_callback: it's long 2!");
                set_triac_offset_max_reset();
                break;

            case MAEL_BTN_EVENT_3_LONG:
                NRF_LOG_INFO ("m_callback: it's long 3!");
                set_triac_offset_min_reset();
                break;

            case MAEL_BTN_EVENT_1_LONG_CONT:
                NRF_LOG_INFO ("m_callback: it's long 1 cont!");
                break;

            case MAEL_BTN_EVENT_2_LONG_CONT:
                NRF_LOG_INFO ("m_callback: it's long 2 cont!");
                set_triac_offset_max();
                break;

            case MAEL_BTN_EVENT_3_LONG_CONT:
                NRF_LOG_INFO ("m_callback: it's long 3 cont!");
                set_triac_offset_min();
                break;

            case MAEL_BTN_EVENT_1_DOUBLE:
                NRF_LOG_INFO ("m_callback: it's 1 double!");
                break;

            case MAEL_BTN_EVENT_2_DOUBLE:
                NRF_LOG_INFO ("m_callback: it's 2 double!");
                break;

            case MAEL_BTN_EVENT_3_DOUBLE:
                NRF_LOG_INFO ("m_callback: it's 3 double!");
                break;

            case MAEL_BTN_EVENT_1_TRIPLE:
                NRF_LOG_INFO ("m_callback: it's 1 triple!");
                break;

            case MAEL_BTN_EVENT_2_TRIPLE:
                NRF_LOG_INFO ("m_callback: it's 2 triple!");
                break;

            case MAEL_BTN_EVENT_3_TRIPLE:
                NRF_LOG_INFO ("m_callback: it's 3 triple!");
                break;
        }
}

/**@brief Function for application main entry, does not return.
 */
int main(void)
{
    log_init();
    utils_setup();

    storage_init();
    mem_test();

    softdevice_setup();
    profile_setup();
    scheduler_init();



    mael_buttons_init(test_callback);

    NRF_LOG_INFO("ANT+ Bicycle Power RX example started.");

    gpio_init();

    for (;;)
    {
        NRF_LOG_FLUSH();
        app_sched_execute();
        nrf_pwr_mgmt_run();
    }
}
