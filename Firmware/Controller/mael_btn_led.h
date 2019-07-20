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

#ifdef __cplusplus
}
#endif

#endif // mael_BUTTON_H__
