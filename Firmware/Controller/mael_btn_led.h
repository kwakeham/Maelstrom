/**
* Put some licence BS here
* Keith Wakeham 2019
*/


#ifndef MAEL_BUTTON_H__
#define MAEL_BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MAEL_BTN_EVENT_NOTHING = 0,                  /**< Assign this event to an action to prevent the action from generating an event (disable the action). */
    MAEL_BTN_EVENT_1,                      /**< Assign this event to an action to assign the default event to the action. */
    MAEL_BTN_EVENT_2,           /**< Persistent bonding data should be erased. */
    MAEL_BTN_EVENT_3,           /**< Persistent bonding data should be erased. */
    MAEL_BTN_EVENT_1_LONG,                  /**< An alert should be cleared. */
    MAEL_BTN_EVENT_2_LONG,                   /**< A link should be disconnected. */
    MAEL_BTN_EVENT_3_LONG,                   /**< A link should be disconnected. */
    MAEL_BTN_EVENT_1_LONG_CONT,
    MAEL_BTN_EVENT_2_LONG_CONT,
    MAEL_BTN_EVENT_3_LONG_CONT,
    MAEL_BTN_EVENT_1_DOUBLE,            /**< The device should start advertising. */
    MAEL_BTN_EVENT_2_DOUBLE,             /**< The device should stop advertising. */
    MAEL_BTN_EVENT_3_DOUBLE,
    MAEL_BTN_EVENT_1_TRIPLE,                /**< The device should remove its advertising whitelist. */
    MAEL_BTN_EVENT_2_TRIPLE,                         /**< The device should bond to the currently connected peer. */
    MAEL_BTN_EVENT_3_TRIPLE,
} maelbtn_event_t;

typedef enum
{
    MAEL_LED_EVENT_NOTHING = 0,                  /**< Assign this event to an action to prevent the action from generating an event (disable the action). */
    MAEL_LED_POWER_1,                       /**< LED Power setting 1 (100 watt) */
    MAEL_LED_POWER_2,                       /**< LED Power setting 2 (150 watt) */
    MAEL_LED_POWER_3,                       /**< LED Power setting 3 (200 watt) */
    MAEL_LED_POWER_4,                       /**< LED Power setting 4 (250 watt) */
    MAEL_LED_POWER_5,                       /**< LED Power setting 5 (300 watt) */
    MAEL_LED_POWER_6,                       /**< LED Power setting 6 (350 watt) */
    MAEL_LED_BLUE_3,                        /**< LED 3 BLUE */
    MAEL_LED_BLUE_2,                        /**< LED Lower 2 BLUE */
    MAEL_LED_BLUE_1,                        /**< LED Lower 1 BLUE */
    MAEL_LED_GREEN_3,                       /**< LED 3 GREEN */
    MAEL_LED_GREEN_2,                       /**< LED Lower 2 GREEN */
    MAEL_LED_GREEN_1,                       /**< LED Lower 1 GREEN */
    MAEL_LED_RED_3,                         /**< LED 3 RED */
    MAEL_LED_RED_2,                         /**< LED Lower 2 RED */
    MAEL_LED_RED_1,                         /**< LED Lower 1 RED */ //15
} mael_led_event_t;



/**@brief maelBTN module event callback function type.
 *
 * @details     Upon an event in the BSP module, this callback function will be called to notify
 *              the application about the event.
 *
 * @param[in]   bsp_event_t BSP event type.
 */
typedef void (* maelbtn_event_callback_t)(maelbtn_event_t);

// typedef void (* mael_ble_keysend_t)(uint8_t,uint8_t *);

// void test_keys_send(uint8_t key_pattern_len, uint8_t * p_key_pattern);

// void mael_ble_register_callback(mael_ble_keysend_t callback);

/**@brief button_timeout_handler.
 *
 * @details     Upon an event in the BSP module, this callback function will be called to notify
 *              the application about the event.
 *
 * @param[in]   p_context .
 */
// void button_timeout_handler(void * p_context);
void button_timeout_handler(void * p_context);

/**@brief repeat_timeout_handler.
 *
 * @details     Upon an event in the BSP module, this callback function will be called to notify
 *              the application about the event.
 *
 * @param[in]   p_context .
 */
void repeat_timeout_handler(void * p_context);


/**@brief button_callback function type.
 *
 * @details     Upon an event in the BSP module, this callback function will be called to notify
 *              the application about the event.
 *
 * @param[in]   pin_no  which pin caused the callback
 * @param[in]   button_action  ?????
 */
void button_callback(uint8_t pin_no, uint8_t button_action);


/**@brief Function for initializing mael Button
 *
 * @details     The function initializes the mael button module to allow state indication and
 *              button reaction.
 *
 * @param[in]   none
 */
void mael_buttons_init(maelbtn_event_callback_t callback);


/**@brief Function for disabling mael Button
 *
 * @details     The function initializes the mael button module to allow state indication and
 *              button reaction.
 *
 * @param[in]   none
 */
void mael_buttons_disable();

/**@brief Function for enable LEDS
 *
 * @details     The function initializes the mael button module to allow state indication and
 *              button reaction.
 *
 * @param[in]   none
 */
void mael_init_leds();

/**@brief Function for testing LEDS 
 *
 * @details     The function initializes the mael button module to allow state indication and
 *              button reaction.
 *
 * @param[in]   none
 */
void mael_test_leds();

void mael_led_toggle();

void override_callback(void * p_context);

void mael_led_display(mael_led_event_t led_status);

void mael_led_clear_all(void);

#ifdef __cplusplus
}
#endif

#endif // mael_BUTTON_H__
