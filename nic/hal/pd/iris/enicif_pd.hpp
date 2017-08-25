#ifndef __HAL_PD_ENICIF_HPP__
#define __HAL_PD_ENICIF_HPP__

#include <base.h>
#include <pd.hpp>

namespace hal {
namespace pd {

struct pd_enicif_s {
    // Hw Indices
    uint32_t    inp_prop_mac_vlan_idx_host;     // Pkts from Host
    uint32_t    inp_prop_mac_vlan_idx_upl;      // Pkts from Uplink
    uint32_t    enic_lport_id;                  // lport

    // pi ptr
    void        *pi_if;
} __PACK__;

hal_ret_t pd_enicif_create(pd_if_args_t *args);
pd_enicif_t *pd_enicif_alloc_init(void);
pd_enicif_t *pd_enicif_alloc (void);
pd_enicif_t *pd_enicif_init (pd_enicif_t *enicif);
hal_ret_t pd_enicif_alloc_res(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_program_hw(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_free (pd_enicif_t *enicif);
void link_pi_pd(pd_enicif_t *pd_upif, if_t *pi_if);
void unlink_pi_pd(pd_enicif_t *pd_upif, if_t *pi_if);
hal_ret_t pd_enicif_pgm_inp_prop_mac_vlan_tbl(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_pd_pgm_output_mapping_tbl(pd_enicif_t *pd_enicif);

uint32_t pd_enicif_get_l4_prof_idx(pd_enicif_t *pd_enicif);
pd_lif_t *pd_enicif_get_pd_lif(pd_enicif_t *pd_enicif);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_PD_ENICIF_HPP__

