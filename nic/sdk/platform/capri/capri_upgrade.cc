//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "asic/pd/pd_upgrade.hpp"
#include "lib/p4/p4_api.hpp"
#include "platform/capri/capri_state.hpp"
#include "platform/capri/capri_upgrade.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "third-party/asic/capri/model/cap_pic/cap_pict_csr.h"
#include "third-party/asic/capri/model/cap_pic/cap_pics_csr.h"
#include "third-party/asic/capri/model/cap_te/cap_te_csr.h"

namespace sdk {
namespace platform {
namespace capri {

// called during upgrade scenarios to update the table property
// based on the new configuration.
sdk_ret_t
capri_upg_table_property_set (p4pd_pipeline_t pipeline,
                              p4_upg_table_property_t *cfg,
                              uint32_t ncfgs)
{
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();
    cap_te_csr_t *te_csr = NULL;
    int cstage = -1;

    // called only in quiesced state
    // no error checks or trace here, as it has to be executed fast
    if (!sdk::asic::is_quiesced()) {
        return SDK_RET_ERR;
    }

    for (uint32_t i = 0; i < ncfgs; i++) {
        if (cstage != cfg->stage) {
            if (pipeline == P4_PIPELINE_INGRESS) {
                te_csr = &cap0.sgi.te[cfg->stage];
            } else if (pipeline == P4_PIPELINE_EGRESS) {
                te_csr = &cap0.sge.te[cfg->stage];
            } else if (pipeline == P4_PIPELINE_RXDMA) {
                te_csr = &cap0.pcr.te[cfg->stage];
            } else if (pipeline == P4_PIPELINE_TXDMA) {
                te_csr = &cap0.pct.te[cfg->stage];
            } else {
                SDK_ASSERT(0);
            }
            cstage = cfg->stage;
        }

        te_csr->cfg_table_property[cfg->stage_tableid].read();
        te_csr->cfg_table_property[cfg->stage_tableid]
                .mpu_pc(((cfg->asm_base) >> 6));
        if (pipeline == P4_PIPELINE_INGRESS || pipeline == P4_PIPELINE_EGRESS) {
            te_csr->cfg_table_property[cfg->stage_tableid]
                .mpu_pc_ofst_err(cfg->asm_err_offset);
        }
        if (cfg->mem_offset != INVALID_MEM_ADDRESS) {
            te_csr->cfg_table_property[cfg->stage_tableid].addr_base(cfg->mem_offset);
        }
        if (cfg->pc_dyn) {
            te_csr->cfg_table_property[cfg->stage_tableid].mpu_pc_dyn(1);
            te_csr->cfg_table_property[cfg->stage_tableid].addr_base(cfg->pc_offset);
        }
        te_csr->cfg_table_property[cfg->stage_tableid].write();
        cfg++;
    }
    return SDK_RET_OK;
}

}    // namespace upg
}    // namespace asic
}    // namespace sdk
