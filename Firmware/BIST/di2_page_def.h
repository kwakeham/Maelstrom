/**
* Put some licence BS here
* Keith Wakeham 2019
*/


// DI2 SETUP information, may be in config.h already though. TODO UNIFY
#define DI2_CHANNEL_NUM                            0
#define DI2_DEVICE_TYPE                            1
#define DI2_MESSAGE_PERIOD                         8192
#define DI2_TRANSMISSION_TYPE                       5
#define DI2_RESERVED                               0xFF
#define DI2_NETOWRK_NUMBER                         0
#define DI2_FREQ                                   57

// Page Defines, TODO add common pages
#define DI2_SHIFT_STATUS                           0x00
#define DI2_DERAILLEUR_ADJUSTMENT                  0x02
#define DI2_BUTTON                                 0x04
#define DI2_SHIFT_SETUP                            0x11

// Generic reserved offsets
#define DI2_RESERVED1_OFFSET          1
#define DI2_RESERVED2_OFFSET          2
#define DI2_RESERVED3_OFFSET          3
#define DI2_RESERVED4_OFFSET          4
#define DI2_RESERVED5_OFFSET          5
#define DI2_RESERVED6_OFFSET          6
#define DI2_RESERVED7_OFFSET          7

// Page 0x00 (Shift stauts page)
#define DI2_PAGE_OFFSET                            0 // Original
#define DI2_BUTTON_STATUS_OFFSET                   1
#define DI2_FD_CURRENT_GEAR_OFFSET                 2
#define DI2_RD_CURRENT_GEAR_OFFSET                 3
#define DI2_BATTERY_OFFSET                         4
#define DI2_SHIFT_MODE_OFFSET                      5

// Page 0x11 (Shift system setup page)
// #define DI2_PAGE_OFFSET                            0 // listed in page 0x00
// #define DI2_BUTTON_STATUS_OFFSET                   1 // listed in page 0x00, This might not be right, but okay
#define DI2_FD_MAX_GEAR_OFFSET                     2
#define DI2_RD_MAX_GEAR_OFFSET                     3

// Page 0x02 (Di2 derailleur adjustment page)
// #define DI2_PAGE_OFFSET                            0 // listed in page 0x00
// #define DI2_BUTTON_STATUS_OFFSET                   1 // listed in page 0x00, This might not be right, but okay
// #define DI2_FD_MAX_GEAR_OFFSET                     2
// #define DI2_RD_MAX_GEAR_OFFSET                     3

// Page 0x04 (Shift stauts page)
// #define DI2_PAGE_OFFSET                            0 // Original
#define DI2_RIGHT_BUTTON                           1
#define DI2_LEFT_BUTTON                            2

