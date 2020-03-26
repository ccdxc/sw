////-----------------------------------------------------------------------------
//// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
////-----------------------------------------------------------------------------

#include "include/sdk/base.hpp"
#include "platform/elba/elba_pxb_pcie.hpp"
#include "third-party/asic/elba/model/utils/elb_blk_reg_model.h"
#include "third-party/asic/elba/model/elb_top/elb_top_csr.h"
#include "third-party/asic/elba/model/elb_pcie/elb_pxb_csr.h"

namespace sdk {
namespace platform {
namespace elba {

extern "C" sdk_ret_t
elba_pxb_pcie_init ()
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pxb_csr_t &pxb_csr = elb0.pxb.pxb;

    SDK_TRACE_DEBUG("Initializing LIF state for all of %d LIFs", ELBA_PCIE_MAX_LIFS);

    for (int i = 0; i < ELBA_PCIE_MAX_LIFS; i++) {
        pxb_csr.dhs_itr_pcihdrt.entry[i].valid(1);
        pxb_csr.dhs_itr_pcihdrt.entry[i].write();
    }
    SDK_TRACE_DEBUG("Initializing PCIE Atomic Region/Page as 0x%x/0x%x",
                     ELBA_PCIE_ATOMIC_REGION_ID, ELBA_PCIE_ATOMIC_PAGE_ID);
    // axi addressing formula :  
    //     {1 const (1bit), region - 4bit , page_id - 19bit , 12bit addr with page };
    // allocate region number 0xf and page 0x3ff within region as atomic
    // above formula will create id_0_addr : 0xf803ff000 to access atomic id[0]
    // id_n_addr = id_0_addr + (64*n)
    pxb_csr.cfg_pcie_local_memaddr_decode.atomic_page_id(ELBA_PCIE_ATOMIC_PAGE_ID);
    pxb_csr.cfg_pcie_local_memaddr_decode.atomic(ELBA_PCIE_ATOMIC_REGION_ID);
    pxb_csr.cfg_pcie_local_memaddr_decode.rc_cfg(0);
    pxb_csr.cfg_pcie_local_memaddr_decode.write();
    return SDK_RET_OK;
}

extern "C" sdk_ret_t
elba_pxb_cfg_lif_bdf (uint32_t lif, uint16_t bdf)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pxb_csr_t &pxb_csr = elb0.pxb.pxb;

    SDK_TRACE_DEBUG("Configuring LIF %u with BDF %u",lif, bdf);

    if (lif >= ELBA_PCIE_MAX_LIFS) {
      SDK_TRACE_DEBUG("LIF %u exceeded MAX_LIFS %u",lif, ELBA_PCIE_MAX_LIFS);
      return sdk::SDK_RET_ERR;
    }
    pxb_csr.dhs_itr_pcihdrt.entry[lif].bdf(bdf);
    pxb_csr.dhs_itr_pcihdrt.entry[lif].write();

    SDK_TRACE_DEBUG("Successfully configured LIF %u with BDF %u",lif, bdf);
    return SDK_RET_OK;
}

}    // namespace elba
}    // namespace platform
}    // namespace sdk
