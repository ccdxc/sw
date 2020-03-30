// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

/*
 * capri_pxb_pcie.hpp
 * Madhava Cheethirala (Pensando Systems)
 */

#ifndef __CAPRI_PXB_PCIE_HPP__
#define __CAPRI_PXB_PCIE_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/sdk/base.hpp"

namespace sdk {
namespace platform {
namespace capri {

#define CAPRI_PCIE_MAX_LIFS 1024

#define CAPRI_PCIE_ATOMIC_PAGE_ID 0x3ffUL
#define CAPRI_PCIE_ATOMIC_REGION_ID 0xfUL
#define CAPRI_PCIE_ATOMIC_BASE_ADDR_GET(_region_id, _page_id) \
            ((1UL << 35) | (_region_id << 31) | (_page_id << 12) | 0x000)
#define CAPRI_PCIE_ATOMIC_BASE_ADDR \
            CAPRI_PCIE_ATOMIC_BASE_ADDR_GET(CAPRI_PCIE_ATOMIC_REGION_ID, CAPRI_PCIE_ATOMIC_PAGE_ID)

/** capri_pxb_pcie_init
 * API to init the pxb pcie module
 *
 * @return hal_ret_t: Status of the operation
 */
sdk_ret_t capri_pxb_pcie_init(void);

/** capri_pxb_cfg_lif_bdf
 * API to configure a LIF with a BDF
 *
 * @lif: Lif number
 * @bdf: BDF number
 *
 * @return hal_ret_t: Status of the operation
 */
sdk_ret_t capri_pxb_cfg_lif_bdf (uint32_t lif, uint16_t bdf);

} // namespace capri
} // namespace platform
} // namespace sdk

#endif //__CAPRI_PXB_PCIE_HPP__
