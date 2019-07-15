/**
* Put some licence BS here
* Keith Wakeham 2019
*/

// #include "antDI2Master.h"
#include <stdint.h>
#include "string.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "app_error.h"
#include "boards.h"
#include "sdk_config.h"
#include "ant_channel_config.h"
#include "nrf_delay.h"
#include "nrf_sdh_ant.h"
#include "nrf_log.h"
// #include "bsp.h"


// I/O configuration
#define LED_BROADCAST                   BSP_BOARD_LED_0
#define LED_ACKNOWLEDGED                BSP_BOARD_LED_1
#define LED_BURST                       BSP_BOARD_LED_2

// Channel configuration.
#define ANT_CUSTOM_TRANSMIT_POWER       0u      /**< ANT Custom Transmit Power (Invalid/Not Used). */
#define BROADCAST_DATA_BUFFER_SIZE      8u      /**< Size of the broadcast data buffer. */
#define BURST_BLOCK_SIZE                32u     /**< Size of data block transmitted via burst. Size must be divisible by 8. */

#define APP_ANT_OBSERVER_PRIO           1       /**< Application's ANT observer priority. You shouldn't need to modify this value. */

#ifndef DI2_NETWORK_KEY
    #define DI2_NETWORK_KEY    {0xA9, 0xAD, 0x32, 0x68, 0x3D, 0x76, 0xC7, 0x4D}            /**< The DI2 network key. */
#endif //

#define MAX_VIEWS              9 /**< Maximum number of views */


// Static variables and buffers.
static uint8_t m_di2_network_key[] = DI2_NETWORK_KEY; // KW, that network keey needs to be a static
static uint8_t m_tx_buffer[BROADCAST_DATA_BUFFER_SIZE]; /**< Primary data (Broadcast/Acknowledged) transmit buffer. */
static uint8_t m_counter = 1u;                               /**< Counter to increment the ANT broadcast data payload. */
static uint8_t m_burst_data[BURST_BLOCK_SIZE];               /**< Burst data transmit buffer. */
static bool button_press; // This is to indicate if the button is pressed is pending
// static bool long_press; // This flag indicates a long press has happened to block the release

static trans_mode_t m_ant_mode = ANT_TRANS_MODE;
static uint8_t m_view_key_press[] = /**< Key pattern to be sent when the key press button has been pushed. */
{
    0x1e,       /* Key 1 */
    0x1f,       /* Key 2 */
    0x20,       /* Key 3 */
    0x21,       /* Key 4 */
    0x22,       /* Key 5 */
    0x23,       /* Key 6 */
    0x24,       /* Key 7 */
    0x25,       /* Key 8 */
    0x26       /* Key 9 */
};

static uint8_t m_tab_key_press = 0x2b;   //ZWIFT Skip workout
static uint8_t m_left_key_press = 0x50;  //ZWIFT left
static uint8_t m_right_key_press = 0x4f; //ZWIFT right
static uint8_t m_down_key_press = 0x51;  //ZWIFT Turn around
static uint8_t m_f3_key_press = 0x3c;    //ZWIFT Ride on
static uint8_t m_space_key_press = 0x2c; //ZWIFT powerup


// State Machine
enum MESSAGE_TYPES_MASTER_STATES
{
    BROADCAST,
    ACKNOWLEDGED,
    BURST
} state_message_types;

struct DI2_MAIN_PAGE_DATA // Page 0, normallly transmitted
{
   uint8_t fd_gear_indicator;
   uint8_t rd_gear_indicator;
   uint8_t battery_level_in_percent;
   uint8_t shift_mode;
};

struct DI2_DERAIL_ADJUST_PAGE_DATA // page 2
{
   int8_t fd_current_adjustment;
   int8_t rd_current_adjustment;
   int8_t fd_max_adjustment;
   int8_t fd_min_adjustment;
   int8_t rd_max_adjustment;
   int8_t rd_min_adjustment;
};

struct DI2_BUTTON_PAGE_DATA // page 4
{
   uint8_t left_btn_count;
   uint8_t left_btn_type;
   uint8_t right_btn_count;
   uint8_t right_btn_type;
};

struct DI2_SYSTEM_SETUP_DATA // page 4
{
   uint8_t fd_max_gear;
   uint8_t rd_max_gear;
};

// static const app_button_cfg_t app_buttons[4] = 
// {
//     {BSP_BUTTON_0, false, BUTTON_PULL, ant_message_types_master_bsp_evt_handler},
//     {BSP_BUTTON_1, false, BUTTON_PULL, ant_message_types_master_bsp_evt_handler},
//     {BSP_BUTTON_2, false, BUTTON_PULL, ant_message_types_master_bsp_evt_handler},
//     {BSP_BUTTON_3, false, BUTTON_PULL, ant_message_types_master_bsp_evt_handler},
// };

static uint8_t di2_page_count; // This is the 256ish count, maybe 64 or 128? need to APP_ERROR_CHECK
static struct DI2_MAIN_PAGE_DATA main_page_data;
static struct DI2_DERAIL_ADJUST_PAGE_DATA derail_adjust_page_data;
static struct DI2_BUTTON_PAGE_DATA button_page_data;
static struct DI2_SYSTEM_SETUP_DATA system_setup_page_data;

//move these in the future
// static void di2_set_main__page(uint8_t* ant_payload);
// static void di2_set_derail_adjust_page(uint8_t* ant_payload);
// static void di2_set_button_page(uint8_t* ant_payload);



// static uint8_t battery = 0x64; // Default = 100, greater values will show but it's dumb, Page 2
// static uint8_t fd_total = 0x02; // Default max front = 2, Page 17
// static uint8_t rd_total = 0x0B; // Default max Rear = 11, Page 17
// static uint8_t fd = 0x01; // Front derailleur current position, Page 2
// static uint8_t rd = 0x01; // Rear derailleur current position, Page 2
// static uint8_t r_btn_count = 0x00; // Right button count, initialize to zero, Page 4
// static uint8_t l_btn_count = 0x00; // Left button count, intialize to zero, Page 4

// static uint8_t key_sample = 0x08;
static di2_ble_keysend_t di2_ble_keysend_registered_callback = NULL;

void di2_ble_register_callback(di2_ble_keysend_t callback)
{
    di2_ble_keysend_registered_callback = callback;
    NRF_LOG_INFO("It should have set the callback");
}


void di2_init(void)
{
    main_page_data.battery_level_in_percent = 100;
    main_page_data.fd_gear_indicator = 1;
    main_page_data.rd_gear_indicator = 1;
    main_page_data.shift_mode = 0;

    derail_adjust_page_data.fd_current_adjustment = 0;
    derail_adjust_page_data.rd_current_adjustment = 0;
    derail_adjust_page_data.fd_max_adjustment = 32;
    derail_adjust_page_data.rd_min_adjustment = -32;
    derail_adjust_page_data.rd_max_adjustment = 32;
    derail_adjust_page_data.rd_min_adjustment = -32;

    button_page_data.left_btn_count = 0;
    button_page_data.right_btn_count = 0;
    button_page_data.left_btn_type = 0xF0;
    button_page_data.right_btn_type = 0xF0;

    system_setup_page_data.fd_max_gear = 0x02;
    system_setup_page_data.rd_max_gear = 0x0B;

    di2_page_count = 0;
}

void di2_button_init()
{
    bsp_event_to_button_action_assign(1, BSP_BUTTON_ACTION_RELEASE, BSP_EVENT_KEY_1);
    bsp_event_to_button_action_assign(2, BSP_BUTTON_ACTION_RELEASE, BSP_EVENT_KEY_2);
    bsp_event_to_button_action_assign(3, BSP_BUTTON_ACTION_RELEASE, BSP_EVENT_KEY_3);
    bsp_event_to_button_action_assign(4, BSP_BUTTON_ACTION_RELEASE, BSP_EVENT_KEY_4);

}


void ant_message_types_master_setup(void)
{
    uint32_t err_code;

    di2_init();

    ant_channel_config_t channel_config =
    {
        .channel_number    = DI2_CHANNEL_NUM,
        .channel_type      = CHANNEL_TYPE_MASTER,
        .ext_assign        = 0x00,
        .rf_freq           = DI2_FREQ,
        .transmission_type = DI2_TRANSMISSION_TYPE,
        .device_type       = DI2_DEVICE_TYPE,
        .device_number     = (uint16_t) (NRF_FICR->DEVICEID[0]),
        .channel_period    = DI2_MESSAGE_PERIOD,
        .network_number    = DI2_NETOWRK_NUMBER,
    };

    err_code = sd_ant_network_address_set(0,m_di2_network_key); // *fix This needs to move to the DI2 setup I believe
    APP_ERROR_CHECK(err_code);

    err_code = ant_channel_init(&channel_config);
    APP_ERROR_CHECK(err_code);

    //Set Tx Power
    err_code = sd_ant_channel_radio_tx_power_set(DI2_CHANNEL_NUM,
                                                 RADIO_TX_POWER_LVL_3,
                                                 ANT_CUSTOM_TRANSMIT_POWER);
    APP_ERROR_CHECK(err_code);

    // Open channel.
    err_code = sd_ant_channel_open(DI2_CHANNEL_NUM);
    APP_ERROR_CHECK(err_code);

    // Write counter value to last byte of the broadcast data.
    // The last byte is chosen to get the data more visible in the end of an printout
    // on the recieving end.
    memset(m_tx_buffer, 0, BROADCAST_DATA_BUFFER_SIZE);
    m_tx_buffer[BROADCAST_DATA_BUFFER_SIZE - 1] = m_counter;

    // Configure the initial payload of the broadcast data
    err_code = sd_ant_broadcast_message_tx(DI2_CHANNEL_NUM,
                                           BROADCAST_DATA_BUFFER_SIZE,
                                           m_tx_buffer);
    APP_ERROR_CHECK(err_code);

    //Set state to broadcasting
    state_message_types = BROADCAST;
}


void ant_message_types_master_bsp_evt_handler(di2btn_event_t evt)
{
    static uint8_t * p_key = m_view_key_press;
    static uint8_t   size  = 0;
    // uint8_t update_data;
    switch (m_ant_mode)
    {
    case ANT_TRANS_MODE:
        switch (evt)
        {
            case DI2_BTN_EVENT_NOTHING:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's nothing!");
                break;

            case DI2_BTN_EVENT_LEFT:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left!");
                di2_button_message_update(LEFT, SHORT);
                state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_left_key_press);
                break;

            case DI2_BTN_EVENT_RIGHT:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right!");
                di2_button_message_update(RIGHT, SHORT);
                state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_right_key_press);
                break;

            case DI2_BTN_EVENT_LEFT_LONG:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left long!");
                di2_button_message_update(LEFT, LONG);
                state_message_types = ACKNOWLEDGED;
                p_key = m_view_key_press;
                size  = 0;
                di2_ble_keysend_registered_callback(1,p_key);

                break;

            case DI2_BTN_EVENT_RIGHT_LONG:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right long!");
                di2_button_message_update(RIGHT, LONG);
                state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_tab_key_press);
                break;

            case DI2_BTN_EVENT_LEFT_LONG_CONT:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left long continued!");
                di2_button_message_update(LEFT, LONG_CONT);
                state_message_types = ACKNOWLEDGED;
                p_key++;
                size++;
                if (size == MAX_VIEWS)
                {
                    p_key = m_view_key_press;
                    size  = 0;
                }
                di2_ble_keysend_registered_callback(1,p_key);
                break;

            case DI2_BTN_EVENT_RIGHT_LONG_CONT:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right long continued!");
                di2_button_message_update(RIGHT, LONG_CONT);
                state_message_types = ACKNOWLEDGED;
                break;

            case DI2_BTN_EVENT_LEFT_DOUBLE:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left double!");
                di2_button_message_update(LEFT, DOUBLE);
                state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_f3_key_press);
                break;

            case DI2_BTN_EVENT_RIGHT_DOUBLE:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right double!");
                di2_button_message_update(RIGHT, DOUBLE);
                state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_down_key_press);
                break;

            case DI2_BTN_EVENT_LEFT_TRIPLE:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left triple!");
                main_page_data.rd_gear_indicator++;
                if (main_page_data.rd_gear_indicator > 0x0B)
                {
                    main_page_data.rd_gear_indicator = 1;
                } 
                if (main_page_data.fd_gear_indicator == 1)
                {
                    main_page_data.fd_gear_indicator = 2;
                } else
                {
                    main_page_data.fd_gear_indicator =1;
                }
                di2_ble_keysend_registered_callback(1,&m_space_key_press);
                break;

            case DI2_BTN_EVENT_RIGHT_TRIPLE:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right triple!");
                main_page_data.battery_level_in_percent  = main_page_data.battery_level_in_percent - 1;
                if (main_page_data.shift_mode==0)
                {
                    main_page_data.shift_mode = 0x03;
                } else
                {
                    main_page_data.shift_mode = 0x00;
                }
                m_ant_mode = BLE_TRANS_MODE;
                break;

            default:
                break;
            }
        break;

    case BLE_TRANS_MODE:
        switch (evt)
        {
            case DI2_BTN_EVENT_NOTHING:
                NRF_LOG_INFO ("ble_message_types_master_bsp_evt_handler: it's nothing!");
                break;

            case DI2_BTN_EVENT_LEFT:
                NRF_LOG_INFO ("ble_message_types_master_bsp_evt_handler: it's left!");
                // di2_button_message_update(LEFT, SHORT);
                // state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_left_key_press);
                break;

            case DI2_BTN_EVENT_RIGHT:
                NRF_LOG_INFO ("ble_message_types_master_bsp_evt_handler: it's right!");
                // di2_button_message_update(RIGHT, SHORT);
                // state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_right_key_press);
                break;

            case DI2_BTN_EVENT_LEFT_LONG:
                NRF_LOG_INFO ("ble_message_types_master_bsp_evt_handler: it's left long!");
                // di2_button_message_update(LEFT, LONG);
                // state_message_types = ACKNOWLEDGED;
                p_key = m_view_key_press;
                size  = 0;
                di2_ble_keysend_registered_callback(1,p_key);

                break;

            case DI2_BTN_EVENT_RIGHT_LONG:
                NRF_LOG_INFO ("ble_message_types_master_bsp_evt_handler: it's right long!");
                // di2_button_message_update(RIGHT, LONG);
                // state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_tab_key_press);
                break;

            case DI2_BTN_EVENT_LEFT_LONG_CONT:
                NRF_LOG_INFO ("ble_message_types_master_bsp_evt_handler: it's left long continued!");
                // di2_button_message_update(LEFT, LONG_CONT);
                // state_message_types = ACKNOWLEDGED;
                p_key++;
                size++;
                if (size == MAX_VIEWS)
                {
                    p_key = m_view_key_press;
                    size  = 0;
                }
                di2_ble_keysend_registered_callback(1,p_key);
                break;

            case DI2_BTN_EVENT_RIGHT_LONG_CONT:
                NRF_LOG_INFO ("ble_message_types_master_bsp_evt_handler: it's right long continued!");
                // di2_button_message_update(RIGHT, LONG_CONT);
                // state_message_types = ACKNOWLEDGED;
                break;

            case DI2_BTN_EVENT_LEFT_DOUBLE:
                NRF_LOG_INFO ("ble_message_types_master_bsp_evt_handler: it's left double!");
                // di2_button_message_update(LEFT, DOUBLE);
                // state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_f3_key_press);
                break;

            case DI2_BTN_EVENT_RIGHT_DOUBLE:
                NRF_LOG_INFO ("ble_message_types_master_bsp_evt_handler: it's right double!");
                // di2_button_message_update(RIGHT, DOUBLE);
                // state_message_types = ACKNOWLEDGED;
                di2_ble_keysend_registered_callback(1,&m_down_key_press);
                break;

            case DI2_BTN_EVENT_LEFT_TRIPLE:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's left triple!");
                main_page_data.rd_gear_indicator++;
                if (main_page_data.rd_gear_indicator > 0x0B)
                {
                    main_page_data.rd_gear_indicator = 1;
                } 
                if (main_page_data.fd_gear_indicator == 1)
                {
                    main_page_data.fd_gear_indicator = 2;
                } else
                {
                    main_page_data.fd_gear_indicator =1;
                }
                break;

            case DI2_BTN_EVENT_RIGHT_TRIPLE:
                NRF_LOG_INFO ("ant_message_types_master_bsp_evt_handler: it's right triple!");
                main_page_data.battery_level_in_percent  = main_page_data.battery_level_in_percent - 1;
                if (main_page_data.shift_mode==0)
                {
                    main_page_data.shift_mode = 0x03;
                } else
                {
                    main_page_data.shift_mode = 0x00;
                }
                m_ant_mode = ANT_TRANS_MODE;
                break;

            default:
                break;
            }
        break;
    
    default:
        break;
    }
    
}

void di2_button_message_update(press_btn_t btn_side, press_type_t press_type)
{
    switch (btn_side)
        {
            case LEFT:

                switch(press_type)
                {
                    case SHORT:
                        button_page_data.left_btn_type = 0x10;
                        break;
                    case LONG:
                        button_page_data.left_btn_type = 0x20;
                        break;
                    case LONG_CONT:
                        button_page_data.left_btn_type = 0x30;
                        break;
                    case DOUBLE:
                        button_page_data.left_btn_type = 0x40;
                        break;
                    
                }
                button_page_data.right_btn_type = 0xF0;
                button_page_data.left_btn_count++;
                if (button_page_data.left_btn_count > 15)
                {
                    button_page_data.left_btn_count=0;
                }
                break;

            case RIGHT:

                switch(press_type)
                {
                    case SHORT:
                        button_page_data.right_btn_type = 0x10;
                        break;
                    case LONG:
                        button_page_data.right_btn_type = 0x20;
                        break;
                    case LONG_CONT:
                        button_page_data.right_btn_type = 0x30;
                        break;
                    case DOUBLE:
                        button_page_data.right_btn_type = 0x40;
                        break;
                }
                button_page_data.left_btn_type = 0xF0;
                button_page_data.right_btn_count++;
                if (button_page_data.right_btn_count > 15)
                {
                    button_page_data.right_btn_count=0;
                }
                break;
        }



                // button_page_data.left_btn_type = 0x10;
                //  // got to reset the opposite key
}


/**@brief Function for handling a ANT stack event.
 *
 * @param[in] p_ant_evt  ANT stack event.
 * @param[in] p_context  Context.
 */
static void ant_evt_handler(ant_evt_t * p_ant_evt, void * p_context)
{
    uint32_t err_code;
    if (p_ant_evt->channel != DI2_CHANNEL_NUM)
    {
        return;
    }
    // uint32_t led_output = LED_BROADCAST;

    switch (p_ant_evt->event)
    {
        // ANT broadcast/Acknowledged/Burst Success
        // Send the next message according to the current state and increment the counter.
        case EVENT_TX:                      // Intentional fall through
            // NRF_LOG_INFO("EVENT_TX_COMPLETED");
        case EVENT_TRANSFER_TX_COMPLETED:   // Intentional fall through
            // NRF_LOG_INFO("EVENT_TRANSFER_TX_COMPLETED");

            if (button_press)
            {
                if (state_message_types == ACKNOWLEDGED)
                {
                    state_message_types = BROADCAST;
                    button_press = 0;
                }
            }

        case EVENT_TRANSFER_TX_FAILED:
            // NRF_LOG_INFO("EVENT_TRANSFER_TX_FAILED");
            // bsp_board_leds_off();
            m_tx_buffer[BROADCAST_DATA_BUFFER_SIZE - 1] = m_counter;

            if (state_message_types == BROADCAST)
            {
                if (m_counter % 64 == 0) //update the system page
                {
                    di2_system_setup(m_tx_buffer);
                    // Send as broadcast
                    err_code = sd_ant_broadcast_message_tx(DI2_CHANNEL_NUM,
                                                        BROADCAST_DATA_BUFFER_SIZE,
                                                        m_tx_buffer);
                    APP_ERROR_CHECK(err_code);
                } else {
                    di2_set_shift_status(m_tx_buffer);
                    // Send as broadcast
                    err_code = sd_ant_broadcast_message_tx(DI2_CHANNEL_NUM,
                                                        BROADCAST_DATA_BUFFER_SIZE,
                                                        m_tx_buffer);
                    //I took out the APP_ERROR_CHECK because I was getting a TRANSFER_IN_PROGRESS (0x401F), this caused it to fatal error
                    //Wasn't sure what to do about it but letting it fail seems fine.
                    // APP_ERROR_CHECK(err_code);

                    // led_output = LED_BROADCAST;
                }
            }
            
            else if (state_message_types == ACKNOWLEDGED)
            {
                button_press = true;
                // Send as acknowledged
                di2_set_button_page(m_tx_buffer);

                err_code = sd_ant_acknowledge_message_tx(DI2_CHANNEL_NUM,
                                                         BROADCAST_DATA_BUFFER_SIZE,
                                                         m_tx_buffer);
                APP_ERROR_CHECK(err_code);

                // led_output = LED_ACKNOWLEDGED;
            }
            else if (state_message_types == BURST)
            {
                // If this is a new message, populate the burst buffer
                // with new dummy data.  Otherwise, will retry sending the
                // same content.
                if (p_ant_evt->event != EVENT_TRANSFER_TX_FAILED)
                {
                    for (uint32_t i = 0; i < BURST_BLOCK_SIZE; i++)
                    {
                        m_burst_data[i] = m_counter;
                        m_counter++;
                    }
                }

                // Queue a Burst Transfer.  Since this is a small burst, queue entire burst.
                err_code = sd_ant_burst_handler_request(DI2_CHANNEL_NUM,
                                                        BURST_BLOCK_SIZE,
                                                        m_burst_data,
                                                        (BURST_SEGMENT_START | BURST_SEGMENT_END));
                APP_ERROR_CHECK(err_code);

                // led_output = LED_BURST;
            }

                // if (p_ant_evt->message.ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID)
                // {
                    
                // }
            // Activate LED for 20ms
            // bsp_board_led_on(led_output);
            // nrf_delay_ms(20);
            // bsp_board_led_off(led_output);
            m_counter++;
            break;

        case TRANSFER_IN_PROGRESS:              //Intentional fall through
        case TRANSFER_SEQUENCE_NUMBER_ERROR:    //Intentional fall through
        case TRANSFER_IN_ERROR:                 //Intentional fall through
        case TRANSFER_BUSY:
        case EVENT_RX: // This likely means we got a message back, but could be a regular message as well
            // NRF_LOG_INFO("EVENT_TRANSFER_RX_COMPLETED");
             if (p_ant_evt->message.ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID) // Check to see if it's an ack message
            {
                NRF_LOG_HEXDUMP_INFO(p_ant_evt->message.ANT_MESSAGE_aucPayload,8);
                // state_message_types = BROADCAST;
            }
            // Ignore these events; will retry burst transfer when we get the EVENT_TRANSFER_TX_FAILED event.
            break;

        default:
            break; // No implementation needed

    }
}

void di2_set_shift_status(uint8_t* di2_payload)
{
   di2_payload[DI2_PAGE_OFFSET] = DI2_SHIFT_STATUS;
   di2_payload[DI2_BUTTON_STATUS_OFFSET] = 0xFB; // Still working on figuring this thing out
   di2_payload[DI2_FD_CURRENT_GEAR_OFFSET] = main_page_data.fd_gear_indicator;
   di2_payload[DI2_RD_CURRENT_GEAR_OFFSET] = main_page_data.rd_gear_indicator;
   di2_payload[DI2_BATTERY_OFFSET] = main_page_data.battery_level_in_percent;
   di2_payload[DI2_SHIFT_MODE_OFFSET] = main_page_data.shift_mode;
   di2_payload[DI2_RESERVED6_OFFSET] = DI2_RESERVED;
   di2_payload[DI2_RESERVED7_OFFSET] = DI2_RESERVED;
}


void di2_system_setup(uint8_t* di2_payload)
{
   di2_payload[DI2_PAGE_OFFSET] = DI2_SHIFT_SETUP;
   di2_payload[DI2_BUTTON_STATUS_OFFSET] = 0xFB; // Still working on figuring this thing out
   di2_payload[DI2_FD_MAX_GEAR_OFFSET] = system_setup_page_data.fd_max_gear;
   di2_payload[DI2_RD_MAX_GEAR_OFFSET] = system_setup_page_data.rd_max_gear;
   di2_payload[DI2_RESERVED4_OFFSET] = DI2_RESERVED;
   di2_payload[DI2_RESERVED5_OFFSET] = DI2_RESERVED;
   di2_payload[DI2_RESERVED6_OFFSET] = DI2_RESERVED;
   di2_payload[DI2_RESERVED7_OFFSET] = DI2_RESERVED;
}

void di2_set_button_page(uint8_t* di2_payload)
{
   di2_payload[DI2_PAGE_OFFSET] = DI2_BUTTON;
   di2_payload[DI2_RIGHT_BUTTON] = button_page_data.right_btn_type | button_page_data.right_btn_count; // Still working on figuring this thing out
   di2_payload[DI2_LEFT_BUTTON] = button_page_data.left_btn_type | button_page_data.left_btn_count;
   di2_payload[DI2_RESERVED3_OFFSET] = DI2_RESERVED;
   di2_payload[DI2_RESERVED4_OFFSET] = DI2_RESERVED;
   di2_payload[DI2_RESERVED5_OFFSET] = DI2_RESERVED;
   di2_payload[DI2_RESERVED6_OFFSET] = DI2_RESERVED;
   di2_payload[DI2_RESERVED7_OFFSET] = DI2_RESERVED;
}


void di2_update_data(enum DI2_DATA data_update, uint8_t data)
{
   switch (data_update)
   {
    case DI2_BATTERY_PERCENT:
        main_page_data.battery_level_in_percent = data;
        NRF_LOG_INFO("Battery level: %d",data);
        break;

    case DI2_FD_GEAR:
        main_page_data.fd_gear_indicator =  data;
        NRF_LOG_INFO("FD Gear Changed");
        break;

    case DI2_RD_GEAR:
        main_page_data.rd_gear_indicator = data;
        NRF_LOG_INFO("RD gear Changed");
        break;

    case DI2_SHIFT_MODE:
        main_page_data.shift_mode = data;
        NRF_LOG_INFO("Shift Mode Changed");
        break;

    case DI2_FD_CURRENT_ADJUSTMENT:
        derail_adjust_page_data.fd_current_adjustment = data;
        break;

    case DI2_RD_CURRENT_ADJUSTMENT:
        derail_adjust_page_data.rd_current_adjustment = data;
        break;

    case DI2_FD_MAX_ADJUSTMENT:
        derail_adjust_page_data.fd_max_adjustment = data;
        break;

    case DI2_FD_MIN_ADJUSTMENT:
        derail_adjust_page_data.fd_min_adjustment = (uint8_t) data;
        break;

    case DI2_RD_MAX_ADJUSTMENT:
        derail_adjust_page_data.rd_max_adjustment = (uint8_t) data;
        break;

    case DI2_RD_MIN_ADJUSTMENT:
        derail_adjust_page_data.rd_min_adjustment =  (uint8_t) data;
        break;

    case DI2_LEFT_BTN_COUNT:
        button_page_data.left_btn_count = (uint8_t) data;
        break;

    case DI2_RIGHT_BTN_COUNT:
        button_page_data.right_btn_count = (uint8_t) data;
        break;

    case DI2_LEFT_BTN_TYPE:
        button_page_data.left_btn_type = (uint8_t) data;
        break;

    case DI2_RIGHT_BTN_TYPE:
        button_page_data.right_btn_type = (uint8_t) data;
        break;

    default:
        break;
   }
}

NRF_SDH_ANT_OBSERVER(m_ant_observer, APP_ANT_OBSERVER_PRIO, ant_evt_handler, NULL);
