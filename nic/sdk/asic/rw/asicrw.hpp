// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_ASICRW_HPP__
#define __SDK_ASICRW_HPP__

#include "nic/sdk/include/sdk/catalog.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"

namespace sdk {
namespace asic {

//------------------------------------------------------------------------------
// different modes of writing to ASIC
// 1. non-blocking - adds write operation to asicrw thread's work queue &
//                   returns
// 2. blocking     - adds write operation to asicrw thread's work queue & blocks
//                   until the operation is done by asicrw thread
// 3. write-thru   - non-blocking version that bypasses asicrw thread completely
//                   and writes in the caller thread's context
//------------------------------------------------------------------------------
typedef enum asic_write_mode_e {
    ASIC_WRITE_MODE_NON_BLOCKING = 0,
    ASIC_WRITE_MODE_BLOCKING     = 1,
    ASIC_WRITE_MODE_WRITE_THRU   = 2,
} asic_write_mode_t;

//------------------------------------------------------------------------------
// public API for register read operations
//------------------------------------------------------------------------------
sdk_ret_t asic_reg_read(uint64_t addr, uint32_t *data, uint32_t num_words = 1,
                        bool read_thru=false);

//------------------------------------------------------------------------------
// public API for memory read operations
//------------------------------------------------------------------------------
sdk_ret_t asic_mem_read(uint64_t addr, uint8_t *data, uint32_t len,
                        bool read_thru=false);

//------------------------------------------------------------------------------
// public API for register write operations
// write given data at specified address in the memory
//------------------------------------------------------------------------------
sdk_ret_t asic_reg_write(uint64_t addr, uint32_t *data, uint32_t num_words = 1,
                         asic_write_mode_t mode = ASIC_WRITE_MODE_BLOCKING);

//------------------------------------------------------------------------------
// public API for memory write operations
// write given data at specified address in the memory
//------------------------------------------------------------------------------
sdk_ret_t asic_mem_write(uint64_t addr, uint8_t *data, uint32_t len,
                         asic_write_mode_t mode = ASIC_WRITE_MODE_BLOCKING);

//------------------------------------------------------------------------------
// public API for ringing doorbells
//------------------------------------------------------------------------------
sdk_ret_t asic_ring_doorbell(uint64_t addr, uint64_t data,
                             asic_write_mode_t mode = ASIC_WRITE_MODE_BLOCKING);

// starting point for asic read-write thread
void *asic_rw_start(void *ctxt);

// return TRUE if asic is initialized and ready for read/writes
bool is_asic_rw_ready(void);

}    // namespace asic
}    // namespace sdk

#endif    // __SDK_ASICRW_HPP__
