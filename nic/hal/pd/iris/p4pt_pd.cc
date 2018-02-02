#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/lif.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/src/p4pt.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/hal/pd/pd_api.hpp"

namespace hal {
namespace pd {

EXTC hal_ret_t
// p4pt_pd_init() {
p4pt_pd_init(p4pt_pd_init_args_t *args) {
    lif_id_t lif_id = SERVICE_LIF_P4PT;
    uint32_t qid = 0;

    uint8_t pgm_offset = 0;
    int ret = g_lif_manager->GetPCOffset("p4plus", "rxdma_stage0.bin",
                                         "p4pt_rx_stage0", &pgm_offset);
    HAL_ABORT(ret == 0);

    g_lif_manager->WriteQState(lif_id, 0, qid,
                               (uint8_t *)&pgm_offset, 1);
    return HAL_RET_OK;
}

} // namespace pd
} // namespace hal
