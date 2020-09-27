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
#include "fds.h"
#include "app_scheduler.h"
#include "nrf_pwr_mgmt.h"

#define MEM_START 0x50000
#define MEM_END 0x51fff
#define FILE_ID         0x0001  /* The ID of the file to write the records into. */
#define RECORD_KEY_1    0x1111  /* A key for the first record. */
#define RECORD_KEY_2    0x2222  /* A key for the second record. */
static uint32_t   const m_deadbeef = 0xDEADBEEF;
// static char       const m_hello[]  = "Hello, world!";
fds_record_t        record;
fds_record_desc_t   record_desc;

// static nrf_fstorage_api_t * p_fs_api;
static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);
// static uint32_t m_data          = 0xBADC0FFE;
// static uint32_t fd_memory_buffer;

static uint32_t ant_id=0x00005DAD;
uint32_t ant_id_address =0x40000;
bool ant_id_address_found = false;

bool gc_flag = false;


/* Flag to check fds initialization. */
static bool volatile m_fds_initialized2 = false;

char const * fds_err_str[] =
{
    "FDS_SUCCESS",
    "FDS_ERR_OPERATION_TIMEOUT",
    "FDS_ERR_NOT_INITIALIZED",
    "FDS_ERR_UNALIGNED_ADDR",
    "FDS_ERR_INVALID_ARG",
    "FDS_ERR_NULL_ARG",
    "FDS_ERR_NO_OPEN_RECORDS",
    "FDS_ERR_NO_SPACE_IN_FLASH",
    "FDS_ERR_NO_SPACE_IN_QUEUES",
    "FDS_ERR_RECORD_TOO_LARGE",
    "FDS_ERR_NOT_FOUND",
    "FDS_ERR_NO_PAGES",
    "FDS_ERR_USER_LIMIT_REACHED",
    "FDS_ERR_CRC_CHECK_FAILED",
    "FDS_ERR_BUSY",
    "FDS_ERR_INTERNAL",
};

/* Array to map FDS events to strings. */
static char const * fds_evt_str[] =
{
    "FDS_EVT_INIT",
    "FDS_EVT_WRITE",
    "FDS_EVT_UPDATE",
    "FDS_EVT_DEL_RECORD",
    "FDS_EVT_DEL_FILE",
    "FDS_EVT_GC",
};

/* Keep track of the progress of a delete_all operation. */
static struct
{
    bool delete_next;   //!< Delete next record.
    bool pending;       //!< Waiting for an fds FDS_EVT_DEL_RECORD event, to delete the next record.
} m_delete_all;

/* Dummy configuration data. */
static mael_configuration_t m_mael_cfg =
{
    .ant_id  = 0,
    .power_setting  = 3,
    .offset_min  = 160,
    .offset_max = 40,
};

/* A record containing dummy configuration data. */
static fds_record_t const m_dummy_record =
{
    .file_id           = FILE_ID,
    .key               = RECORD_KEY_1,
    .data.p_data       = &m_mael_cfg,
    /* The length of a record is always expressed in 4-byte units (words). */
    .data.length_words = (sizeof(m_mael_cfg)+3) / sizeof(uint32_t),
};



NRF_FSTORAGE_DEF(nrf_fstorage_t titan_mem) =
{
    .evt_handler = fstorage_evt_handler,
    .start_addr = MEM_START,
    .end_addr   = MEM_END,
};


static void tm_fds_evt_handler(fds_evt_t const * p_evt)
{
    NRF_LOG_INFO("Event: %s received (%s)",
                  fds_evt_str[p_evt->id],
                  fds_err_str[p_evt->result]);

    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_evt->result == FDS_SUCCESS)
            {
                m_fds_initialized2 = true;
                NRF_LOG_INFO("FDS initialized");
            }
            break;

        case FDS_EVT_WRITE:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->write.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->write.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->write.record_key);
            }
        } break;

        case FDS_EVT_DEL_RECORD:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->del.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->del.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->del.record_key);
            }
            m_delete_all.pending = false;
        } break;

        case FDS_EVT_GC:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                gc_flag = false;
                NRF_LOG_INFO("GC successful");
            }
            m_delete_all.pending = false;
        } break;

        default:
            break;
    }
}

static void wait_for_fds_ready(void)
{
    while(!m_fds_initialized2)
    {
        app_sched_execute();
        nrf_pwr_mgmt_run();
    }
}



void tm_fds_init()
{
    ret_code_t ret = fds_register(tm_fds_evt_handler);
    if (ret != FDS_SUCCESS)
    {
        // Registering of the FDS event handler has failed.
    }
    NRF_LOG_INFO("MEM: registered event handler");

    ret = fds_init();
    if (ret != FDS_SUCCESS)
    {
        // Handle error.
    }
    NRF_LOG_INFO("MEM: fds initialize");

    wait_for_fds_ready();

}

void tm_fds_test_write()
{
    record.file_id           = FILE_ID;
    record.key               = RECORD_KEY_1;
    record.data.p_data       = &m_deadbeef;
    record.data.length_words = 1;   /* one word is four bytes. */
    ret_code_t rc;
    rc = fds_record_write(&record_desc, &record);
    if (rc != FDS_SUCCESS)
    {
        /* Handle error. */
    }
}

mael_configuration_t tm_fds_mael_config (void)
{
    return m_mael_cfg;
}

void tm_fds_test_retrieve()
{
    // ret_code_t rc;
    fds_flash_record_t  flash_record;
    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok;
    uint32_t test_data;
    /* It is required to zero the token before first use. */
    memset(&ftok, 0x00, sizeof(fds_find_token_t));
    /* Loop until all records with the given key and file ID have been found. */
    while (fds_record_find(FILE_ID, RECORD_KEY_1, &record_desc, &ftok) == FDS_SUCCESS)
    {
        if (fds_record_open(&record_desc, &flash_record) != FDS_SUCCESS)
        {
            /* Handle error. */
        }
        /* Access the record through the flash_record structure. */
        memcpy(&test_data, flash_record.p_data, sizeof(mael_configuration_t));

        NRF_LOG_INFO("The record is: %d",test_data);
        /* Close the record when done. */
        if (fds_record_close(&record_desc) != FDS_SUCCESS)
        {
            /* Handle error. */
        }
    }
    
}

void tm_fds_test_delete()
{
    ret_code_t ret = fds_record_delete(&record_desc);
    if (ret != FDS_SUCCESS)
    {
        /* Error. */
    }
}

void tm_fds_config_init()
{
    ret_code_t rc;
    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};

    fds_stat_t stat = {0};
    rc = fds_stat(&stat);
    APP_ERROR_CHECK(rc);

    NRF_LOG_INFO("Found %d valid records.", stat.valid_records);
    NRF_LOG_INFO("Found %d dirty records (ready to be garbage collected).", stat.dirty_records);

    tm_fds_gc();

    rc = fds_record_find(FILE_ID, RECORD_KEY_1, &desc, &tok);

    if (rc == FDS_SUCCESS)
    {
        /* A config file is in flash. Let's update it. */
        fds_flash_record_t config = {0};

        /* Open the record and read its contents. */
        rc = fds_record_open(&desc, &config);
        APP_ERROR_CHECK(rc);

        /* Copy the configuration from flash into m_dummy_cfg. */
        memcpy(&m_mael_cfg, config.p_data, sizeof(mael_configuration_t));

        /* Close the record when done reading. */
        rc = fds_record_close(&desc);
        APP_ERROR_CHECK(rc);
        NRF_LOG_INFO("ANT ID: %d",m_mael_cfg.ant_id);
        NRF_LOG_INFO("ANT POWER LEVEL %d", m_mael_cfg.power_setting);
        NRF_LOG_INFO("ANT offset min %d", m_mael_cfg.offset_min);
        NRF_LOG_INFO("ANT offset max %d", m_mael_cfg.offset_max);

    }
    else
    {
        /* System config not found; write a new one. */
        NRF_LOG_INFO("Writing config file...");

        rc = fds_record_write(&desc, &m_dummy_record);
        APP_ERROR_CHECK(rc);
    }
}

void tm_fds_gc()
{
    ret_code_t rc;
    fds_stat_t stat = {0};
    rc = fds_stat(&stat);
    APP_ERROR_CHECK(rc);

    if (!gc_flag)
    {
        if(stat.dirty_records > 60)
        {
            gc_flag = true;
            fds_gc();
            NRF_LOG_INFO("Garbage Collecting");
        }
        else
        {
            gc_flag = false;
        }
        
    }
}

void tm_fds_config_update()
{
    ret_code_t rc;
    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};

    tm_fds_gc();

    rc = fds_record_find(FILE_ID, RECORD_KEY_1, &desc, &tok);

    if (rc == FDS_SUCCESS)
    {
        /* A config file is in flash. Let's update it. */
        // fds_flash_record_t config = {0};

        /* Open the record and read its contents. */
        // rc = fds_record_open(&desc, &config);
        // APP_ERROR_CHECK(rc);

        // /* Copy the configuration from flash into m_dummy_cfg. */
        // memcpy(&m_mael_cfg, config.p_data, sizeof(mael_configuration_t));

        // /* Close the record when done reading. */
        // rc = fds_record_close(&desc);
        // APP_ERROR_CHECK(rc);
        NRF_LOG_INFO("ANT ID: %d",m_mael_cfg.ant_id);
        NRF_LOG_INFO("ANT POWER LEVEL %d", m_mael_cfg.power_setting);
        NRF_LOG_INFO("ANT offset min %d", m_mael_cfg.offset_min);
        NRF_LOG_INFO("ANT offset max %d", m_mael_cfg.offset_max);

        rc = fds_record_update(&desc, &m_dummy_record);
        APP_ERROR_CHECK(rc);
    }
    else
    {
        /* System config not found; write a new one. */
        NRF_LOG_INFO("Writing config file...");

        rc = fds_record_write(&desc, &m_dummy_record);
        APP_ERROR_CHECK(rc);
    }
}


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
    NRF_LOG_INFO("triac ant id: %d",mael_devicenumber);
    NRF_LOG_INFO("triac power level: %d",mem_triac_power_level);
    NRF_LOG_INFO("triac offset min: %d",mem_triac_offset_min);
    NRF_LOG_INFO("triac offset max: %d",mem_triac_offset_max);


    m_mael_cfg.ant_id = mael_devicenumber;
    m_mael_cfg.power_setting = mem_triac_power_level;
    m_mael_cfg.offset_min = mem_triac_offset_min;
    m_mael_cfg.offset_max = mem_triac_offset_max;

    tm_fds_config_update();
}

// void mem_mael_write()
// {
//     ret_code_t rc = nrf_fstorage_write(
//         &titan_mem,   /* The instance to use. */
//         ant_id_address,     /* The address in flash where to store the data. */
//         &fd_memory_buffer,        /* A pointer to the data. */
//         sizeof(fd_memory_buffer), /* Length of the data, in bytes. */
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
//         NRF_LOG_INFO("ERROR in write");
//     }


// }

// void mem_ant_id_erase()
// {
//      ret_code_t rc = nrf_fstorage_erase(
//         &titan_mem,   /* The instance to use. */
//         ant_id_address,     /* The address of the flash pages to erase. */
//         1, /* The number of pages to erase. */
//         NULL            /* Optional parameter, backend-dependent. */
//     );
//     if (rc == NRF_SUCCESS)
//     {
//         /* The operation was accepted.
//         Upon completion, the NRF_FSTORAGE_ERASE_RESULT event
//         is sent to the callback function registered by the instance. */
//         NRF_LOG_INFO("MEM: ERASED %d", ant_id_address);
//     }
//     else
//     {
//         NRF_LOG_INFO("MEM:ERASE FAILED");
//         /* Handle error.*/
//     }
// }

// void mem_ant_id_read()
// {
//     uint32_t stored_number = 0; 
//     ant_id_address = MEM_START;
//     ant_id_address_found = false;
//     while(!ant_id_address_found)
//     {
//         if (ant_id_address < MEM_END)
//         {
//             stored_number = mem_read(ant_id_address);
//             if (stored_number != 0xFFFFFFFF)
//             {
//                 ant_id_address = ant_id_address+32;
//             }
//             else
//             {
//                 if (ant_id_address>MEM_START)
//                 {
//                     ant_id_address = ant_id_address-32;
//                 } else
//                 {
//                     ant_id_address = MEM_START;
//                 }
//                 ant_id_address_found = true;
//             }
//         }
//         else
//         {
//             NRF_LOG_INFO("too high, What to do, erase everything and start again");
//             ant_id_address = MEM_START;
//             ant_id_address_found = true;
//             mem_ant_id_erase();
//         }
//         NRF_LOG_INFO("final read");
//         stored_number = mem_read(ant_id_address);
//     }
//     NRF_LOG_INFO("ANT+ ID FOUND: %d @ %d",stored_number,ant_id_address);
//     NRF_LOG_INFO("ANT+ POWER LEVEL: %d @ %d",stored_number>>16,ant_id_address);
// }

// uint32_t mem_read(uint32_t addresss)
// {
//         static uint32_t number;
//         ret_code_t rc = nrf_fstorage_read(
//             &titan_mem,   /* The instance to use. */
//             ant_id_address,     /* The address in flash where to read data from. */
//             &number,        /* A buffer to copy the data into. */
//             sizeof(number)  /* Lenght of the data, in bytes. */
//         );
//         if (rc == NRF_SUCCESS)
//         {
//             /* The operation was accepted. */
//             // NRF_LOG_INFO("MEM1 Value: %X", number);
//         }
//         else
//         {
//             /* Handle error.*/
//             NRF_LOG_INFO("MEM1 ERROR: %X", number);
//         }
//         return number;
// }



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