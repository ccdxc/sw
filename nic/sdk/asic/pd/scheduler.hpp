#ifndef __ASICPD_SCHEDULER_HPP_
#define __ASICPD_SCHEDULER_HPP_

typedef struct asicpd_scheduler_lif_params_s_ {
    uint32_t lif_id;
    uint32_t tx_sched_table_offset;
    uint32_t tx_sched_num_table_entries; 
    uint32_t total_qcount;
    uint32_t hw_lif_id;
    uint16_t cos_bmp;
} __PACK__ asicpd_scheduler_lif_params_t;

namespace sdk {
namespace asic {
namespace pd {

sdk_ret_t asicpd_tx_scheduler_map_alloc(asicpd_scheduler_lif_params_t *lif);
sdk_ret_t asicpd_tx_scheduler_map_free(asicpd_scheduler_lif_params_t *lif);
sdk_ret_t asicpd_tx_scheduler_map_program(asicpd_scheduler_lif_params_t *lif);
sdk_ret_t asicpd_tx_scheduler_map_cleanup(asicpd_scheduler_lif_params_t *lif);
sdk_ret_t asicpd_tx_policer_program(asicpd_scheduler_lif_params_t *lif);

}   // namespace pd
}   // namespace asic
}   // namespace sdk

using namespace sdk::platform::capri;
#endif 
