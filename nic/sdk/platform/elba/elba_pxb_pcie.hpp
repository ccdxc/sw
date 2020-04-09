//// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_PXB_PCIE_HPP__
#define __ELBA_PXB_PCIE_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/sdk/base.hpp"

namespace sdk {
namespace platform {
namespace elba {

#define ELBA_PCIE_MAX_LIFS         1024
#define ELBA_PCIE_ATOMIC_PAGE_ID   0x3ffUL
#define ELBA_PCIE_ATOMIC_REGION_ID 0xfUL
#define ELBA_PCIE_ATOMIC_BASE_ADDR_GET(_region_id, _page_id) \
            ((1UL << 35) | (_region_id << 31) | (_page_id << 12) | 0x000)
#define ELBA_PCIE_ATOMIC_BASE_ADDR \
            ELBA_PCIE_ATOMIC_BASE_ADDR_GET(ELBA_PCIE_ATOMIC_REGION_ID, ELBA_PCIE_ATOMIC_PAGE_ID)

// elba_pxb_pcie_init
// API to init the pxb pcie module
//
//  @return hal_ret_t: Status of the operation
//
sdk_ret_t elba_pxb_pcie_init(void);

// elba_pxb_cfg_lif_bdf
// API to configure a LIF with a BDF
//
// @lif: Lif number
// @bdf: BDF number
//
// @return hal_ret_t: Status of the operation
//
sdk_ret_t elba_pxb_cfg_lif_bdf(uint32_t lif, uint16_t bdf);

}    // namespace elba
}    // namespace platform
}    // namespace sdk

#endif    //__ELBA_PXB_PCIE_HPP__
