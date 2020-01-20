/**
 *
 * Flash memory interface Library
 * TITANLAB INC 2019
 * Keith Wakeham
 * 
 * 
 */

#include "titan_mem.h"
#include "nrf_fstorage_sd.h"
// #include "nrf_fstorage.h"
#include "nrf_log.h"
#include "nrf_sdh_ant.h"
#include "app_error.h"
#include "ant_interface.h"
#include "nrf_delay.h"

#define MEM_START 0x40000
#define MEM_END 0x40fff

// static nrf_fstorage_api_t * p_fs_api;
static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);
// static uint32_t m_data          = 0xBADC0FFE;
static uint32_t fd_memory_buffer;

static uint32_t ant_id=0x00005DAD;
uint32_t ant_id_address =0x40000;
bool ant_id_address_found = false;





NRF_FSTORAGE_DEF(nrf_fstorage_t titan_mem) =
{
    .evt_handler = fstorage_evt_handler,
    .start_addr = MEM_START,
    .end_addr   = MEM_END,
};


void storage_init()
{
    ret_code_t rc;

    // p_fs_api = &nrf_fstorage_sd;
    
    rc = nrf_fstorage_init(
        &titan_mem,       /* You fstorage instance, previously defined. */
        &nrf_fstorage_sd,   /* Name of the backend. */
        NULL                /* Optional parameter, backend-dependant. */
    );

    APP_ERROR_CHECK(rc);

}

static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_INFO("--> Event received: ERROR while executing an fstorage operation.");
        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: wrote %d bytes at address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
            NRF_LOG_INFO("--> Event received: erased %d page from address 0x%x.",
                         p_evt->len, p_evt->addr);
        } break;

        default:
            break;
    }
}

// void mem_ant_id_write()
// {
//     ant_id_address = ant_id_address + 32;
//     ret_code_t rc = nrf_fstorage_write(
//         &titan_mem,   /* The instance to use. */
//         ant_id_address,     /* The address in flash where to store the data. */
//         &ant_id,        /* A pointer to the data. */
//         sizeof(m_data), /* Lenght of the data, in bytes. */
//         NULL            /* Optional parameter, backend-dependent. */
//     );
//     if (rc == NRF_SUCCESS)
//     {
//         /* The operation was accepted.
//         Upon completion, the NRF_FSTORAGE_WRITE_RESULT event
//         is sent to the callback function registered by the instance. */
//     }
//     else
//     {
//         /* Handle error.*/
//     }
// }

void mem_update_ant_id(uint16_t mem_ant_id)
{
    ant_id = 0x00000000 | mem_ant_id;
}

void mem_mael_triac_update(uint16_t mem_triac_offset_min, uint16_t mem_triac_offset_max, uint8_t mem_triac_power_level)
{
    uint16_t mael_devicenumber;
    uint8_t mael_devicetype;
    uint8_t mael_TransmitType;
    sd_ant_channel_id_get(BPWR_CHANNEL_NUM, &mael_devicenumber, &mael_devicetype, &mael_TransmitType);
    // fd_memory_buffer[0] = mem_triac_offset_min<<16|mem_triac_offset_max;
    // fd_memory_buffer = mem_triac_power_level << 16| mael_devicenumber;
    fd_memory_buffer = mael_devicenumber;
    // NRF_LOG_INFO("triac offset min: %d",mem_triac_offset_min);
    // NRF_LOG_INFO("triac offset max: %d",mem_triac_offset_max);
    // NRF_LOG_INFO("triac power level: %d",mem_triac_power_level);
    NRF_LOG_INFO("triac ant id: %d",mael_devicenumber);
    NRF_LOG_INFO("fd_mem_length: %d",sizeof(fd_memory_buffer));
    NRF_LOG_INFO("ant address before incre: %d",ant_id_address);
    ant_id_address = ant_id_address + 32;
    if(ant_id_address > MEM_END)
    {
        ant_id_address = MEM_START;
        mem_ant_id_erase();
    }
    // nrf_delay_ms(100);
}

void mem_mael_write()
{
    ret_code_t rc = nrf_fstorage_write(
        &titan_mem,   /* The instance to use. */
        ant_id_address,     /* The address in flash where to store the data. */
        &fd_memory_buffer,        /* A pointer to the data. */
        sizeof(fd_memory_buffer), /* Length of the data, in bytes. */
        NULL            /* Optional parameter, backend-dependent. */
    );
    if (rc == NRF_SUCCESS)
    {
        /* The operation was accepted.
        Upon completion, the NRF_FSTORAGE_WRITE_RESULT event
        is sent to the callback function registered by the instance. */
    }
    else
    {
        /* Handle error.*/
        NRF_LOG_INFO("ERROR in write");
    }


}

void mem_ant_id_erase()
{
     ret_code_t rc = nrf_fstorage_erase(
        &titan_mem,   /* The instance to use. */
        ant_id_address,     /* The address of the flash pages to erase. */
        1, /* The number of pages to erase. */
        NULL            /* Optional parameter, backend-dependent. */
    );
    if (rc == NRF_SUCCESS)
    {
        /* The operation was accepted.
        Upon completion, the NRF_FSTORAGE_ERASE_RESULT event
        is sent to the callback function registered by the instance. */
        NRF_LOG_INFO("MEM: ERASED %d", ant_id_address);
    }
    else
    {
        NRF_LOG_INFO("MEM:ERASE FAILED");
        /* Handle error.*/
    }
}

void mem_ant_id_read()
{
    uint32_t stored_number = 0; 
    ant_id_address = MEM_START;
    ant_id_address_found = false;
    while(!ant_id_address_found)
    {
        if (ant_id_address < MEM_END)
        {
            stored_number = mem_read(ant_id_address);
            if (stored_number != 0xFFFFFFFF)
            {
                ant_id_address = ant_id_address+32;
            }
            else
            {
                if (ant_id_address>MEM_START)
                {
                    ant_id_address = ant_id_address-32;
                } else
                {
                    ant_id_address = MEM_START;
                }
                ant_id_address_found = true;
            }
        }
        else
        {
            NRF_LOG_INFO("too high, What to do, erase everything and start again");
            ant_id_address = MEM_START;
            ant_id_address_found = true;
            mem_ant_id_erase();
        }
        NRF_LOG_INFO("final read");
        stored_number = mem_read(ant_id_address);
    }
    NRF_LOG_INFO("ANT+ ID FOUND: %d @ %d",stored_number,ant_id_address);
}

uint32_t mem_read(uint32_t addresss)
{
        static uint32_t number;
        ret_code_t rc = nrf_fstorage_read(
            &titan_mem,   /* The instance to use. */
            ant_id_address,     /* The address in flash where to read data from. */
            &number,        /* A buffer to copy the data into. */
            sizeof(number)  /* Lenght of the data, in bytes. */
        );
        if (rc == NRF_SUCCESS)
        {
            /* The operation was accepted. */
            // NRF_LOG_INFO("MEM1 Value: %X", number);
        }
        else
        {
            /* Handle error.*/
            NRF_LOG_INFO("MEM1 ERROR: %X", number);
        }
        return number;
}



// void mem_test ()
// {

//     ret_code_t rc = nrf_fstorage_erase(
//         &titan_mem,   /* The instance to use. */
//         0x40000,     /* The address of the flash pages to erase. */
//         1, /* The number of pages to erase. */
//         NULL            /* Optional parameter, backend-dependent. */
//     );
//     if (rc == NRF_SUCCESS)
//     {
//         /* The operation was accepted.
//         Upon completion, the NRF_FSTORAGE_ERASE_RESULT event
//         is sent to the callback function registered by the instance. */
//         NRF_LOG_INFO("MEM: written");
//     }
//     else
//     {
//         NRF_LOG_INFO("MEM:She's fucked by");
//         /* Handle error.*/
//     }

//     static uint32_t number;
//     rc = nrf_fstorage_read(
//         &titan_mem,   /* The instance to use. */
//         0x40000,     /* The address in flash where to read data from. */
//         &number,        /* A buffer to copy the data into. */
//         sizeof(number)  /* Lenght of the data, in bytes. */
//     );
//     if (rc == NRF_SUCCESS)
//     {
//         /* The operation was accepted. */
//         NRF_LOG_INFO("MEM1 Value: %X", number);
//     }
//     else
//     {
//         /* Handle error.*/
//     }

//     rc = nrf_fstorage_write(
//         &titan_mem,   /* The instance to use. */
//         0x40000,     /* The address in flash where to store the data. */
//         &m_data,        /* A pointer to the data. */
//         sizeof(m_data), /* Lenght of the data, in bytes. */
//         NULL            /* Optional parameter, backend-dependent. */
//     );
//     if (rc == NRF_SUCCESS)
//     {
//         /* The operation was accepted.
//         Upon completion, the NRF_FSTORAGE_WRITE_RESULT event
//         is sent to the callback function registered by the instance. */
//     }
//     else
//     {
//         /* Handle error.*/
//     }

//     rc = nrf_fstorage_read(
//         &titan_mem,   /* The instance to use. */
//         0x40000,     /* The address in flash where to read data from. */
//         &number,        /* A buffer to copy the data into. */
//         sizeof(number)  /* Lenght of the data, in bytes. */
//     );
//     if (rc == NRF_SUCCESS)
//     {
//         /* The operation was accepted. */
//         NRF_LOG_INFO("MEM2 Value: %X", number);
//     }
//     else
//     {
//         /* Handle error.*/
//     }

//     m_data = 0xDEADBEEF;

//     rc = nrf_fstorage_write(
//         &titan_mem,   /* The instance to use. */
//         0x40100,     /* The address in flash where to store the data. */
//         &m_data,        /* A pointer to the data. */
//         sizeof(m_data), /* Lenght of the data, in bytes. */
//         NULL            /* Optional parameter, backend-dependent. */
//     );
//     if (rc == NRF_SUCCESS)
//     {
//         /* The operation was accepted.
//         Upon completion, the NRF_FSTORAGE_WRITE_RESULT event
//         is sent to the callback function registered by the instance. */
//     }
//     else
//     {
//         /* Handle error.*/
//     }

//     rc = nrf_fstorage_read(
//         &titan_mem,   /* The instance to use. */
//         0x40100,     /* The address in flash where to read data from. */
//         &number,        /* A buffer to copy the data into. */
//         sizeof(number)  /* Lenght of the data, in bytes. */
//     );
//     if (rc == NRF_SUCCESS)
//     {
//         /* The operation was accepted. */
//         NRF_LOG_INFO("MEM3 Value: %X", number);
//     }
//     else
//     {
//         /* Handle error.*/
//     }
// }