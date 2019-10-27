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
#include "app_error.h"

// static nrf_fstorage_api_t * p_fs_api;
static void fstorage_evt_handler(nrf_fstorage_evt_t * p_evt);
static uint32_t m_data          = 0xBADC0FFE;
// static char     m_hello_world[] = "hello world";

NRF_FSTORAGE_DEF(nrf_fstorage_t titan_mem) =
{
    .evt_handler = fstorage_evt_handler,
    .start_addr = 0x40000,
    .end_addr   = 0x40fff,
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

void mem_test ()
{

    ret_code_t rc = nrf_fstorage_erase(
        &titan_mem,   /* The instance to use. */
        0x40000,     /* The address of the flash pages to erase. */
        1, /* The number of pages to erase. */
        NULL            /* Optional parameter, backend-dependent. */
    );
    if (rc == NRF_SUCCESS)
    {
        /* The operation was accepted.
        Upon completion, the NRF_FSTORAGE_ERASE_RESULT event
        is sent to the callback function registered by the instance. */
        NRF_LOG_INFO("MEM: written");
    }
    else
    {
        NRF_LOG_INFO("MEM:She's fucked by");
        /* Handle error.*/
    }

    static uint32_t number;
    rc = nrf_fstorage_read(
        &titan_mem,   /* The instance to use. */
        0x40000,     /* The address in flash where to read data from. */
        &number,        /* A buffer to copy the data into. */
        sizeof(number)  /* Lenght of the data, in bytes. */
    );
    if (rc == NRF_SUCCESS)
    {
        /* The operation was accepted. */
        NRF_LOG_INFO("MEM1 Value: %X", number);
    }
    else
    {
        /* Handle error.*/
    }

    rc = nrf_fstorage_write(
        &titan_mem,   /* The instance to use. */
        0x40000,     /* The address in flash where to store the data. */
        &m_data,        /* A pointer to the data. */
        sizeof(m_data), /* Lenght of the data, in bytes. */
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
    }

    rc = nrf_fstorage_read(
        &titan_mem,   /* The instance to use. */
        0x40000,     /* The address in flash where to read data from. */
        &number,        /* A buffer to copy the data into. */
        sizeof(number)  /* Lenght of the data, in bytes. */
    );
    if (rc == NRF_SUCCESS)
    {
        /* The operation was accepted. */
        NRF_LOG_INFO("MEM2 Value: %X", number);
    }
    else
    {
        /* Handle error.*/
    }

    m_data = 0xDEADBEEF;

    rc = nrf_fstorage_write(
        &titan_mem,   /* The instance to use. */
        0x40100,     /* The address in flash where to store the data. */
        &m_data,        /* A pointer to the data. */
        sizeof(m_data), /* Lenght of the data, in bytes. */
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
    }

    rc = nrf_fstorage_read(
        &titan_mem,   /* The instance to use. */
        0x40100,     /* The address in flash where to read data from. */
        &number,        /* A buffer to copy the data into. */
        sizeof(number)  /* Lenght of the data, in bytes. */
    );
    if (rc == NRF_SUCCESS)
    {
        /* The operation was accepted. */
        NRF_LOG_INFO("MEM3 Value: %X", number);
    }
    else
    {
        /* Handle error.*/
    }
}

// static void fstorage_read(nrf_cli_t const * p_cli, uint32_t addr, uint32_t len, data_fmt_t fmt)
// {
//     ret_code_t rc;
//     uint8_t    data[256] = {0};

//     if (len > sizeof(data))
//     {
//         len = sizeof(data);
//     }

//     /* Read data. */
//     rc = nrf_fstorage_read(&fstorage, addr, data, len);
//     if (rc != NRF_SUCCESS)
//     {
//         nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "nrf_fstorage_read() returned: %s\n",
//                         nrf_strerror_get(rc));
//         return;
//     }

//     switch (fmt)
//     {
//         case DATA_FMT_HEX:
//         {
//             /* Print bytes. */
//             for (uint32_t i = 0; i < len; i++)
//             {
//                 nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "0x%x ", data[i]);
//             }
//             nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "\n");
//         } break;

//         case DATA_FMT_STR:
//         {
//             nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "%s\n", data);
//         } break;

//         default:
//             break;
//     }
// }

// static uint32_t round_up_u32(uint32_t len)
// {
//     if (len % sizeof(uint32_t))
//     {
//         return (len + sizeof(uint32_t) - (len % sizeof(uint32_t)));
//     }

//     return len;
// }


// static void fstorage_write(nrf_cli_t const * p_cli, uint32_t addr, void const * p_data)
// {
//     /* The following code snippet make sure that the length of the data we are writing to flash
//      * is a multiple of the program unit of the flash peripheral (4 bytes).
//      *
//      * In case of non-string piece of data, use the sizeof operator instead of strlen.
//      */
//     uint32_t len = round_up_u32(strlen(p_data));

//     ret_code_t rc = nrf_fstorage_write(&fstorage, addr, p_data, len, NULL);
//     if (rc != NRF_SUCCESS)
//     {
//         nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "nrf_fstorage_write() returned: %s\n",
//                         nrf_strerror_get(rc));
//     }
// }

// static void fstorage_erase(nrf_cli_t const * p_cli, uint32_t addr, uint32_t pages_cnt)
// {
//     ret_code_t rc = nrf_fstorage_erase(&fstorage, addr, pages_cnt, NULL);
//     if (rc != NRF_SUCCESS)
//     {
//         nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "nrf_fstorage_erase() returned: %s\n",
//                         nrf_strerror_get(rc));
//     }
// }

// static void read_cmd_hex(nrf_cli_t const * p_cli, size_t argc, char ** argv)
// {
//     if (nrf_cli_help_requested(p_cli))
//     {
//         nrf_cli_help_print(p_cli, NULL, 0);
//     }
//     else if (argc != 3)
//     {
//         cli_missing_param_help(p_cli, "read hex");
//     }
//     else
//     {
//         uint32_t const addr = strtol(argv[1], NULL, 16);
//         uint32_t const len  = strtol(argv[2], NULL, 10);

//         fstorage_read(p_cli, addr, len, DATA_FMT_HEX);
//     }
// }


// static void read_cmd_str(nrf_cli_t const * p_cli, size_t argc, char ** argv)
// {
//     if (nrf_cli_help_requested(p_cli))
//     {
//         nrf_cli_help_print(p_cli, NULL, 0);
//     }
//     else if (argc != 3)
//     {
//         cli_missing_param_help(p_cli, "read str");
//     }
//     else
//     {
//         uint32_t const addr = strtol(argv[1], NULL, 16);
//         uint32_t const len  = strtol(argv[2], NULL, 10);

//         fstorage_read(p_cli, addr, len, DATA_FMT_STR);
//     }
// }


// static void write_cmd(nrf_cli_t const * p_cli, size_t argc, char ** argv)
// {
//     static uint8_t m_data[256];

//     if (nrf_cli_help_requested(p_cli))
//     {
//         nrf_cli_help_print(p_cli, NULL, 0);
//     }
//     else if (argc != 3)
//     {
//         cli_missing_param_help(p_cli, "write");
//     }
//     else
//     {
//         uint32_t const addr = strtol(argv[1], NULL, 16);
//         uint32_t const len  = strlen(argv[2]) < sizeof(m_data) ?
//                               strlen(argv[2]) : sizeof(m_data);

//         /* Copy data to a static variable. */
//         memset(m_data, 0x00, sizeof(m_data));
//         memcpy(m_data, argv[2], len);

//         fstorage_write(p_cli, addr, m_data);
//     }

// }


// static void erase_cmd(nrf_cli_t const * p_cli, size_t argc, char ** argv)
// {
//     if (nrf_cli_help_requested(p_cli))
//     {
//         nrf_cli_help_print(p_cli, NULL, 0);
//     }
//     else if (argc != 3)
//     {
//         cli_missing_param_help(p_cli, argv[0]);
//     }
//     else
//     {
//         uint32_t const addr      = strtol(argv[1], NULL, 16);
//         uint32_t const pages_cnt = strtol(argv[2], NULL, 10);

//         fstorage_erase(p_cli, addr, pages_cnt);
//     }
// }