// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_ASIC_RW_HPP__
#define __HAL_PD_ASIC_RW_HPP__

#include "nic/include/base.h"
#include <vector>
#include <tuple>
using std::vector;
using std::tuple;

namespace hal {
namespace pd {

typedef struct asic_cfg_s {
    std::string      loader_info_file;
} asic_cfg_t;

//------------------------------------------------------------------------------
// public API for register read operations
// NOTE: this is always a blocking call and this API runs in the calling
//       thread's context
//------------------------------------------------------------------------------
hal_ret_t asic_reg_read(uint64_t addr, uint32_t *data, uint32_t num_words = 1);
uint32_t asic_reg_read(uint64_t addr);


//------------------------------------------------------------------------------
// public API for memory read operations
// NOTE: this is always a blocking call and this API runs in the calling
//       thread's context
//------------------------------------------------------------------------------
hal_ret_t asic_mem_read(uint64_t addr, uint8_t *data, uint32_t len);

//------------------------------------------------------------------------------
// public API for register write operations
// write given data at specified address in the memory
//------------------------------------------------------------------------------
hal_ret_t asic_reg_write(uint64_t addr,
                         uint32_t *data,
                         uint32_t num_words = 1,
                         bool blocking = true);

//------------------------------------------------------------------------------
// public API for memory write operations
// write given data at specified address in the memory
//------------------------------------------------------------------------------
hal_ret_t asic_mem_write(uint64_t addr, uint8_t *data,
                         uint32_t len, bool blocking = true);

//------------------------------------------------------------------------------
// public API for ringing doorbells.
//------------------------------------------------------------------------------
hal_ret_t asic_ring_doorbell(uint64_t addr, uint64_t data,
                             bool blocking = true);

//------------------------------------------------------------------------------
// public API for saving cpu packet.
//------------------------------------------------------------------------------
hal_ret_t asic_step_cpu_pkt(const uint8_t* pkt, size_t pkt_len);

// starting point for asic read-write thread
void *asic_rw_start(void *ctxt);

// initialize the asic
hal_ret_t asic_init(asic_cfg_t *asic_cfg);

// return TRUE if asic is initialized and ready for read/writes
bool is_asic_rw_ready(void);

// port related config
hal_ret_t
asic_port_cfg (uint32_t port_num,
               uint32_t speed,
               uint32_t type,
               uint32_t num_lanes,
               uint32_t val);

// check if the current thread is hal-control thread
bool is_hal_ctrl_thread();

// check if this thread is the asic-rw thread
// Returns true if:
//    this thread's id matches with asic-rw thread's id
bool
is_asic_rw_thread();

std::string asic_pd_csr_dump(char *csr_str);

std::string asic_csr_dump(char *csr_str);
vector < tuple < std::string, std::string, std::string > > asic_csr_dump_reg(char *block_name, bool exlude_mem);
vector <std::string>
asic_csr_list_get(std::string path, int level);


}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_ASIC_RW_HPP__

