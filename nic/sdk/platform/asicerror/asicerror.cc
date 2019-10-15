/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    asicerror.cc
 *
 * @brief   This file handles and dumps interrupts
 */

#include "lib/pal/pal.hpp"
#include "interrupts.hpp"

static intr_cfg_t intr_cfg;

void
print_interrupts (intr_reg_t &reg)
{
    intr_field_t *field;

    for(int i=0; i < reg.field_count; i++) {
        field = &reg.fields[i];

        if (field->count != 0) {
            SDK_TRACE_ERR("name: %s_%s, count: %lu, severity: %s, desc: %s",
                          reg.name, field->name, field->count,
                          get_severity_str(field->severity).c_str(), field->desc);
        }
        if(field->next_ptr) {
            print_interrupts(*field->next_ptr);
        }
    }
}

void
traverse_interrupts (intr_reg_t &reg)
{
    pal_ret_t ret;
    uint32_t data = 0x0;
    intr_reg_type_t reg_type = reg.reg_type;
    uint64_t reg_addr = reg.addr;
    uint16_t field_count = reg.field_count;
    intr_field_t *field;

    if (reg_type == INTR_REG_TYPE_SRC) {
        ret = sdk::lib::pal_reg_read(reg_addr, &data, 1);
        if (ret != 0) {
        }
    }

    for(int i=0; i < field_count; i++) {
        field = &reg.fields[i];

        if (reg_type == INTR_REG_TYPE_SRC) {
            if (data & (1 << i)) {
                field->count += 1;
                if (intr_cfg.intr_event_cb) {
                    intr_cfg.intr_event_cb(&reg, field);
                }
            }
        }
        if(field->next_ptr) {
            traverse_interrupts(*field->next_ptr);
        }
    }

    if (reg_type == INTR_REG_TYPE_SRC) {
        // clear interrupts
        ret = sdk::lib::pal_reg_write(reg_addr, &data, 1);
        if (ret != 0) {
        }
    }
}

int
intr_init (intr_cfg_t *cfg)
{
    intr_cfg = *cfg;
    return 0;
}
