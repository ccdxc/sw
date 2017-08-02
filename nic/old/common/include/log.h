#ifndef    __LOG_H__
#define    __LOG_H__

#include <base.hpp>
#include <ip.hpp>

//------------------------------------------------------------------------------
// global log levels
//------------------------------------------------------------------------------
typedef enum log_level_e {
    LOG_LEVEL_EMERGENCY   = 0,
    LOG_LEVEL_ALERT       = 1,
    LOG_LEVEL_CRITICAL    = 2,
    LOG_LEVEL_ERR         = 3,
    LOG_LEVEL_WARNING     = 4,
    LOG_LEVEL_NOTICE      = 5,
    LOG_LEVEL_INFO        = 6,
    LOG_LEVEL_DEBUG       = 7,
} log_level_t;

//------------------------------------------------------------------------------
// NIC level syslog policy
//------------------------------------------------------------------------------
#define LOG_MAX_PFX_LEN                              16
typedef struct log_policy_s {
    ip_addr_t          dst;                     // log destination
    uint16_t           dport;                   // log destination port
    uint8_t            pfx[LOG_MAX_PFX_LEN];    // pfx to add for every log msg
    log_level_t        level;                   // log level for filtering logs
} __PACK__ log_policy_t;

#endif    // __LOG_H__

