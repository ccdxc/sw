/*
 * capri_pxb_pcie.cc
 * Madhava Cheethirala (Pensando Systems)
 */


#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <cmath>

#include "nic/include/base.h"
#include "nic/hal/pd/capri/capri_pxb_pcie.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_pxb_pcie.hpp"

#ifndef HAL_GTEST
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/cap_pcie/cap_pxb_csr.h"
#endif



hal_ret_t
capri_pxb_pcie_init ()
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pxb_csr_t &pxb_csr = cap0.pxb.pxb;

    HAL_TRACE_DEBUG("CAPRI-PXB::{}: Initializing LIF state for all of {} LIFs",
                    __func__, CAPRI_PCIE_MAX_LIFS);

    for (int i = 0; i < CAPRI_PCIE_MAX_LIFS; i++) {
        pxb_csr.dhs_itr_pcihdrt.entry[i].valid(1);
        pxb_csr.dhs_itr_pcihdrt.entry[i].write();
    }
    HAL_TRACE_DEBUG("CAPRI-PXB::{}: Initializing PCIE Atomic Region/Page as {:#x}/{:#x}\n",
                    __func__, CAPRI_PCIE_ATOMIC_REGION_ID, CAPRI_PCIE_ATOMIC_PAGE_ID);

    // axi addressing formula :  
    //     {1 const (1bit), region - 4bit , page_id - 19bit , 12bit addr with page };
    // allocate region number 0xf and page 0x3ff within region as atomic
    // above formula will create id_0_addr : 0xf803ff000 to access atomic id[0]
    // id_n_addr = id_0_addr + (64*n)
    pxb_csr.cfg_pcie_local_memaddr_decode.atomic_page_id(CAPRI_PCIE_ATOMIC_PAGE_ID);
    pxb_csr.cfg_pcie_local_memaddr_decode.atomic(CAPRI_PCIE_ATOMIC_REGION_ID);
    pxb_csr.cfg_pcie_local_memaddr_decode.rc_cfg(0);
    pxb_csr.cfg_pcie_local_memaddr_decode.write();
    

    return HAL_RET_OK;
}

hal_ret_t
capri_pxb_cfg_lif_bdf (uint32_t lif, uint16_t bdf)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pxb_csr_t &pxb_csr = cap0.pxb.pxb;

    HAL_TRACE_DEBUG("CAPRI-PXB::{}: Configuring LIF {} with BDF {}",
                    __func__, lif, bdf);

    if (lif >= CAPRI_PCIE_MAX_LIFS) {
      HAL_TRACE_DEBUG("CAPRI-PXB::{}: LIF {} exceeded MAX_LIFS {}",
                      __func__, lif, CAPRI_PCIE_MAX_LIFS);
      return HAL_RET_ERR;
    }
    pxb_csr.dhs_itr_pcihdrt.entry[lif].bdf(bdf);
    pxb_csr.dhs_itr_pcihdrt.entry[lif].write();

    HAL_TRACE_DEBUG("CAPRI-PXB::{}: Successfully configured LIF {} with BDF {}",
                    __func__, lif, bdf);

    return HAL_RET_OK;
}
