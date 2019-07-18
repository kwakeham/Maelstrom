/**
* Put some licence BS here
* Keith Wakeham 2019
*/


// MAEL SETUP information, may be in config.h already though. TODO UNIFY
#define MAEL_CHANNEL_NUM                            0
#define MAEL_DEVICE_TYPE                            1
#define MAEL_MESSAGE_PERIOD                         8192
#define MAEL_TRANSMISSION_TYPE                       5
#define MAEL_RESERVED                               0xFF
#define MAEL_NETOWRK_NUMBER                         0
#define MAEL_FREQ                                   57

// Page Defines, TODO add common pages
#define MAEL_SHIFT_STATUS                           0x00
#define MAEL_DERAILLEUR_ADJUSTMENT                  0x02
#define MAEL_BUTTON                                 0x04
#define MAEL_SHIFT_SETUP                            0x11

// Generic reserved offsets
#define MAEL_RESERVED1_OFFSET          1
#define MAEL_RESERVED2_OFFSET          2
#define MAEL_RESERVED3_OFFSET          3
#define MAEL_RESERVED4_OFFSET          4
#define MAEL_RESERVED5_OFFSET          5
#define MAEL_RESERVED6_OFFSET          6
#define MAEL_RESERVED7_OFFSET          7

// Page 0x00 (Shift stauts page)
#define MAEL_PAGE_OFFSET                            0 // Original
#define MAEL_BUTTON_STATUS_OFFSET                   1
#define MAEL_FD_CURRENT_GEAR_OFFSET                 2
#define MAEL_RD_CURRENT_GEAR_OFFSET                 3
#define MAEL_BATTERY_OFFSET                         4
#define MAEL_SHIFT_MODE_OFFSET                      5

// Page 0x11 (Shift system setup page)
// #define MAEL_PAGE_OFFSET                            0 // listed in page 0x00
// #define MAEL_BUTTON_STATUS_OFFSET                   1 // listed in page 0x00, This might not be right, but okay
#define MAEL_FD_MAX_GEAR_OFFSET                     2
#define MAEL_RD_MAX_GEAR_OFFSET                     3

// Page 0x02 (Di2 derailleur adjustment page)
// #define MAEL_PAGE_OFFSET                            0 // listed in page 0x00
// #define MAEL_BUTTON_STATUS_OFFSET                   1 // listed in page 0x00, This might not be right, but okay
// #define MAEL_FD_MAX_GEAR_OFFSET                     2
// #define MAEL_RD_MAX_GEAR_OFFSET                     3

// Page 0x04 (Shift stauts page)
// #define MAEL_PAGE_OFFSET                            0 // Original
#define MAEL_RIGHT_BUTTON                           1
#define MAEL_LEFT_BUTTON                            2

