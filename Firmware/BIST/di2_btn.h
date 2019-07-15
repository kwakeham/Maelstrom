/**
* Put some licence BS here
* Keith Wakeham 2019
*/


#ifndef DI2_BUTTON_H__
#define DI2_BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DI2_BTN_EVENT_NOTHING = 0,                  /**< Assign this event to an action to prevent the action from generating an event (disable the action). */
    DI2_BTN_EVENT_LEFT,                      /**< Assign this event to an action to assign the default event to the action. */
    DI2_BTN_EVENT_RIGHT,           /**< Persistent bonding data should be erased. */
    DI2_BTN_EVENT_LEFT_LONG,                  /**< An alert should be cleared. */
    DI2_BTN_EVENT_RIGHT_LONG,                   /**< A link should be disconnected. */
    DI2_BTN_EVENT_LEFT_LONG_CONT,
    DI2_BTN_EVENT_RIGHT_LONG_CONT,
    DI2_BTN_EVENT_LEFT_DOUBLE,            /**< The device should start advertising. */
    DI2_BTN_EVENT_RIGHT_DOUBLE,             /**< The device should stop advertising. */
    DI2_BTN_EVENT_LEFT_TRIPLE,                /**< The device should remove its advertising whitelist. */
    DI2_BTN_EVENT_RIGHT_TRIPLE,                         /**< The device should bond to the currently connected peer. */
} di2btn_event_t;


/**@brief DI2BTN module event callback function type.
 *
 * @details     Upon an event in the BSP module, this callback function will be called to notify
 *              the application about the event.
 *
 * @param[in]   bsp_event_t BSP event type.
 */
typedef void (* di2btn_event_callback_t)(di2btn_event_t);

// typedef void (* di2_ble_keysend_t)(uint8_t,uint8_t *);

// void test_keys_send(uint8_t key_pattern_len, uint8_t * p_key_pattern);

// void di2_ble_register_callback(di2_ble_keysend_t callback);

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


/**@brief Function for initializing DI2 Button
 *
 * @details     The function initializes the DI2 button module to allow state indication and
 *              button reaction.
 *
 * @param[in]   none
 */
void di2_buttons_init(di2btn_event_callback_t callback);


/**@brief Function for disabling DI2 Button
 *
 * @details     The function initializes the DI2 button module to allow state indication and
 *              button reaction.
 *
 * @param[in]   none
 */
void di2_buttons_disable();

#ifdef __cplusplus
}
#endif

#endif // DI2_BUTTON_H__
