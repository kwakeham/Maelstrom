/**
 *
 * Flash memory interface Library
 * TITANLAB INC 2019
 * Keith Wakeham
 * 
 * 
 */

#ifndef TITAN_MEM__
#define TITAN_MEM__
#include "nrf.h"
#include "app_timer.h"
#include "nrf_fstorage_sd.h"
#include "nrf_fstorage.h"

/* A dummy structure to save in flash. */
typedef struct
{
    uint16_t ant_id;
    uint8_t power_setting;
    uint16_t offset_min;
    uint16_t offset_max;
} mael_configuration_t;

// void mem_ant_id_write(void);

void tm_fds_init();

void tm_fds_test_write();

mael_configuration_t tm_fds_mael_config ();

void tm_fds_test_retrieve();
void tm_fds_test_delete();

void tm_fds_config_init();

void tm_fds_config_update();


void mem_update_ant_id(uint16_t mem_ant_id);

void mem_mael_triac_update(uint16_t mem_triac_offset_min, uint16_t mem_triac_offset_max, uint8_t mem_triac_power_level);


// void mem_mael_write();

// void mem_ant_id_erase(void);

// void mem_ant_id_read(void);

// uint32_t mem_read(uint32_t addresss);

// void storage_init(void);

// void mem_test(void);

#endif