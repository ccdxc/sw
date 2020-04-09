//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
// sw PHV injection
//-----------------------------------------------------------------------------


#ifndef __ELBA_SW_PHV_HPP__
#define __ELBA_SW_PHV_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/sdk/base.hpp"
#include "asic/asic.hpp"

using namespace sdk::asic;

namespace sdk {
namespace platform {
namespace elba {

//elba_sw_phv_init
//API to init the sw php module

//@return int: Status of the operation

sdk_ret_t elba_sw_phv_init(void);

// elba_sw_phv_inject
// API to inject a software PHV into the pipeline
//
// type: Which pipeline to inject the PHV
// prof_num: profile number to use
// start_idx: Index of first flit
// num_flits: number of flits to address
// data: raw PHV data to inject (needs to contain num_flits * sizeof(flit) bytes of data)
// @return int: Status of the operation
//
sdk_ret_t elba_sw_phv_inject(asic_swphv_type_t type, uint8_t prof_num,
                             uint8_t start_idx, uint8_t num_flits, void *data);

//
// elba_sw_phv_get
//
// type: Which pipeline to inject the PHV
// prof_num: profile number to use
// state: current state of swphv injection to return
//
sdk_ret_t elba_sw_phv_get(asic_swphv_type_t type, uint8_t prof_num,
                           asic_sw_phv_state_t *state);

}    // namespace capri
}    // namespace platform
}    // namespace sdk

#endif    //__ELBA_SW_PHV_HPP__
