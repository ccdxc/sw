#ifndef __HAL_LIF_PD_HPP__
#define __HAL_LIF_PD_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/hal/pd/utils/acl_tcam/acl_tcam.hpp"

namespace hal {
namespace pd {

using hal::pd::utils::acl_tcam_entry_handle_t;

struct pd_lif_s {
    uint32_t    hw_lif_id;                  // Id used to carve out buffers
    uint32_t    lif_lport_id;               // Index into O/PMapping table
    uint32_t    tx_sched_table_offset;
    uint32_t    tx_sched_num_table_entries;
    acl_tcam_entry_handle_t host_mgmt_acl_handle;
    uint32_t    rdma_sniff_mirr_idx;
    acl_tcam_entry_handle_t tx_handle;
    acl_tcam_entry_handle_t rx_handle;

    // pi ptr
    void        *pi_lif;
} __PACK__;


pd_lif_t *lif_pd_alloc ();
pd_lif_t *lif_pd_init (pd_lif_t *lif);
pd_lif_t *lif_pd_alloc_init ();
hal_ret_t lif_pd_free (pd_lif_t *lif);
hal_ret_t lif_pd_mem_free (pd_lif_t *lif);
hal_ret_t lif_pd_alloc_res(pd_lif_t *pd_lif,
                           pd_lif_create_args_t *args);
hal_ret_t lif_pd_program_hw(pd_lif_t *pd_lif);
hal_ret_t lif_pd_deprogram_hw (pd_lif_t *pd_lif);
if_t *pd_lif_get_enic(lif_id_t lif_id);

hal_ret_t lif_pd_pgm_output_mapping_tbl(pd_lif_t *pd_lif,
                                        pd_lif_update_args_t *args,
                                        table_oper_t oper);
hal_ret_t lif_pd_depgm_output_mapping_tbl (pd_lif_t *pd_lif);
hal_ret_t lif_pd_rx_policer_program_hw (pd_lif_t *pd_lif, bool update);
hal_ret_t lif_pd_rx_policer_deprogram_hw (pd_lif_t *pd_lif);
hal_ret_t lif_pd_tx_policer_program_hw (pd_lif_t *pd_lif, bool update);
hal_ret_t lif_pd_tx_policer_deprogram_hw (pd_lif_t *pd_lif);

void link_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif);
void delink_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif);
hal_ret_t lif_pd_cleanup(pd_lif_t *lif_pd);
hal_ret_t pd_lif_make_clone(lif_t *ten, lif_t *clone);
hal_ret_t pd_lif_handle_promiscous_filter_change (lif_t *lif,
                                                  pd_lif_update_args_t *args,
                                                  bool skip_hw_pgm);
uint32_t pd_lif_get_enic_lport(lif_id_t lif_id);
#if 0
hal_ret_t pd_lif_pgm_host_mgmt (pd_lif_t *pd_lif);
hal_ret_t pd_lif_depgm_host_mgmt (pd_lif_t *pd_lif);
#endif

hal_ret_t pd_lif_pgm_mirror_session(pd_lif_t *pd_lif, if_t *hal_if,
                                    table_oper_t oper);
hal_ret_t pd_lif_depgm_mirror_session(pd_lif_t *pd_lif);
hal_ret_t pd_lif_pgm_tx_nacl(pd_lif_t *pd_lif, if_t *hal_if);
hal_ret_t pd_lif_depgm_tx_nacl(pd_lif_t *pd_lif);
hal_ret_t pd_lif_pgm_rx_nacl(pd_lif_t *pd_lif, if_t *hal_if);
hal_ret_t pd_lif_depgm_rx_nacl(pd_lif_t *pd_lif);
hal_ret_t pd_lif_install_rdma_sniffer(pd_lif_t *pd_lif, if_t *hal_if);
hal_ret_t pd_lif_uninstall_rdma_sniffer(pd_lif_t *pd_lif);
hal_ret_t pd_lif_stats_reset(pd_lif_t *pd_lif);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_LIF_PD_HPP__

