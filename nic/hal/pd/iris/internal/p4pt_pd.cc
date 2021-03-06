#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/src/internal/p4pt.hpp"
#include "nic/hal/pd/pd_api.hpp"

namespace hal {
namespace pd {

hal_ret_t
// p4pt_pd_init() {
p4pt_pd_init(pd_func_args_t *pd_func_args) {
    // p4pt_pd_init_args_t *args = pd_func_args->p4pt_pd_init;
    lif_id_t lif_id = SERVICE_LIF_P4PT;
    uint32_t qid = 0;

    uint8_t pgm_offset = 0;
    int ret = hal_get_pc_offset("rxdma_stage0.bin",
                                "p4pt_rx_stage0", &pgm_offset);
#if 0
    int ret = lif_manager()->GetPCOffset("p4plus", "rxdma_stage0.bin",
                                         "p4pt_rx_stage0", &pgm_offset);
#endif
    if (ret == 0) {
        lif_manager()->write_qstate(lif_id, 0, qid,
                                    (uint8_t *)&pgm_offset, 1);
#if 0
        lif_manager()->WriteQState(lif_id, 0, qid,
                                   (uint8_t *)&pgm_offset, 1);
#endif
        return HAL_RET_OK;
    }
    return HAL_RET_ERR;
}

} // namespace pd
} // namespace hal
