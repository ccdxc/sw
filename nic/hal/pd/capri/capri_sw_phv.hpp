//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// sw PHV injection
//-----------------------------------------------------------------------------


#ifndef __CAPRI_SW_PHV_HPP__
#define __CAPRI_SW_PHV_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "nic/include/base.h"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"

namespace hal {
namespace pd {

/** capri_sw_phv_init
 * API to init the sw php module
 *
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t capri_sw_phv_init();

/** capri_sw_phv_inject
 * API to inject a software PHV into the pipeline
 *
 * type: Which pipeline to inject the PHV
 * prof_num: profile number to use
 * start_idx: Index of first flit
 * num_flits: number of flits to address
 * data: raw PHV data to inject (needs to contain num_flits * sizeof(flit) bytes of data)
 * @return hal_ret_t: Status of the operation
 */
hal_ret_t capri_sw_phv_inject(asicpd_swphv_type_t type, uint8_t prof_num, 
		uint8_t start_idx, uint8_t num_flits, void *data);

/*
 * capri_sw_phv_get
 * 
 * type: Which pipeline to inject the PHV
 * prof_num: profile number to use
 * state: current state of swphv injection to return
 */
hal_ret_t capri_sw_phv_get (asicpd_swphv_type_t type, uint8_t prof_num, 
	asicpd_sw_phv_state_t *state);

}    // namespace pd
}    // namespace hal

#endif //__CAPRI_SW_PHV_HPP__
