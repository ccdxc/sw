/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    interrupts.hpp
 *
 * @brief   This file defines interrupt structures and methods
 */

#ifndef __SDK_INTERRUPTS_HPP__
#define __SDK_INTERRUPTS_HPP__

#include <inttypes.h>
#include <string>

#define INTR_REG_TYPE_SRC 0
typedef uint8_t intr_reg_type_t;

typedef enum intr_sev_type_e {
    INTR_SEV_TYPE_HW_RMA = 0,
    INTR_SEV_TYPE_FATAL  = 1,
    INTR_SEV_TYPE_ERR    = 2,
    INTR_SEV_TYPE_INFO   = 3,
} intr_sev_type_t;

struct intr_reg_t;
struct intr_field_t {
    public :
        const char *name;
        intr_reg_t *next_ptr;
        uint64_t id;
        intr_sev_type_t severity;
        const char *desc;
        uint64_t count;
};

struct intr_reg_t {
    public :
        const char *name;
        uint64_t addr;
        intr_reg_type_t reg_type;
        uint64_t id;
        uint16_t field_count;
        intr_field_t fields[];
};

typedef void (*intr_event_cb_t)(const intr_reg_t *reg,
                                const intr_field_t *field);
typedef struct intr_cfg_s {
    intr_event_cb_t intr_event_cb;
} intr_cfg_t;

extern intr_reg_t cap0;

// interrupt lib initialization
int intr_init(intr_cfg_t *intr_cfg);

// dump all non-zero interrupts
void print_interrupts(intr_reg_t &reg);

// interrupts poll
void traverse_interrupts(intr_reg_t &reg);

static inline std::string
get_severity_str (intr_sev_type_t sev)
{
    switch (sev) {
    case INTR_SEV_TYPE_HW_RMA:
        return "HW_RMA";

    case INTR_SEV_TYPE_ERR:
        return "ERROR";

    case INTR_SEV_TYPE_FATAL:
        return "FATAL";

    case INTR_SEV_TYPE_INFO:
    default:
        return "INFO";
    }
}

#endif    // __SDK_INTERRUPTS_HPP__
