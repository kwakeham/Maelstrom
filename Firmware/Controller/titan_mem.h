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

// void mem_ant_id_write(void);

void mem_update_ant_id(uint16_t mem_ant_id);

void mem_mael_triac_update(uint16_t mem_triac_offset_min, uint16_t mem_triac_offset_max, uint8_t mem_triac_power_level);

void mem_mael_write();

void mem_ant_id_erase(void);

void mem_ant_id_read(void);

uint32_t mem_read(uint32_t addresss);

void storage_init(void);

// void mem_test(void);

#endif