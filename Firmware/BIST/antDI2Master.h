/**
* Put some licence BS here
* Keith Wakeham 2019
*/


#ifndef ANT_DI2_MASTER_H__
#define ANT_DI2_MASTER_H__

#include "bsp.h"
#include "di2_page_def.h"
#include "di2_btn.h"

#ifdef __cplusplus
extern "C" {
#endif

enum DI2_DATA
{
   DI2_BATTERY_PERCENT,
   DI2_FD_GEAR,
   DI2_RD_GEAR,
   DI2_SHIFT_MODE,
   DI2_FD_CURRENT_ADJUSTMENT,
   DI2_RD_CURRENT_ADJUSTMENT,
   DI2_FD_MAX_ADJUSTMENT,
   DI2_FD_MIN_ADJUSTMENT,
   DI2_RD_MAX_ADJUSTMENT,
   DI2_RD_MIN_ADJUSTMENT,
   DI2_LEFT_BTN_COUNT,
   DI2_RIGHT_BTN_COUNT,
   DI2_LEFT_BTN_TYPE,
   DI2_RIGHT_BTN_TYPE
};

typedef enum
{
    ANT_TRANS_MODE,                  /**< Assign this event to an action to prevent the action from generating an event (disable the action). */
    BLE_TRANS_MODE,                      /**< Assign this event to an action to assign the default event to the action. */
} trans_mode_t;

typedef enum
{
    LEFT,                  /**< */
    RIGHT,                      /***/
} press_btn_t;

typedef enum
{
    SHORT,                  /**< */
    LONG,                      /***/
    LONG_CONT,
    DOUBLE,
} press_type_t;


typedef void (* di2_ble_keysend_t)(uint8_t,uint8_t *);

void di2_ble_register_callback(di2_ble_keysend_t callback);


/**@brief Function for initializing the data types
 *
 */
void di2_init(void);

// /**@brief Function for initializing the buttons types
//  *
//  */
// void di2_button_init(void);

/**@brief Function for configuring and starting ANT channel
 *
 */
void ant_message_types_master_setup(void);

/**@brief Handles BSP events.
 *
 * @param[in] evt   BSP event.
 */
void ant_message_types_master_bsp_evt_handler(di2btn_event_t evt);

/**@brief Sets the ant transmission page to be the shift status page
 *
 * @param[in] uint8_t* di2_payload pointer
 */
void di2_button_message_update(press_btn_t btn_side, press_type_t press_type);

/**@brief Sets the ant transmission page to be the shift status page
 *
 * @param[in] uint8_t* di2_payload pointer
 */
void di2_set_shift_status(uint8_t* di2_payload);

/**@brief Sets the ant transmission page for the system setup page
 *
 * @param[in] uint8_t* di2_payload pointer
 */
void di2_system_setup(uint8_t* di2_payload);

/**@brief Sets the ant transmission page for the buttons
 *
 * @param[in] uint8_t* di2_payload pointer
 */
void di2_set_button_page(uint8_t* di2_payload);

/**@brief 
 *
 * @param[in] evt   Data
 */
void di2_update_data(enum DI2_DATA data_update, uint8_t data);



#ifdef __cplusplus
}
#endif

#endif // ANT_DI2_MASTER_H__
