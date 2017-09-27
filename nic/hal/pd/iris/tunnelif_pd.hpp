#ifndef __HAL_PD_TUNNELIF_HPP__
#define __HAL_PD_TUNNELIF_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"

namespace hal {
namespace pd {

struct pd_tunnelif_s {
    // Hw Indices
    // Input mapping native table idx
    int imn_idx[3];
    // Input mapping tunneled table idx
    int imt_idx[3];
    // Tunnel rewrite table idx
    int tunnel_rw_idx;
    tenant_id_t tid;
    // PI ptr
    void        *pi_if;
} __PACK__;

typedef enum pd_tunnelif_inp_map_tbl_type_e_ {
    PD_TUNNEL_IF_INP_MAP_NATIVE = 0,
    PD_TUNNEL_IF_INP_MAP_TUNNEL,
    PD_TUNNEL_IF_INP_MAP_NONE,
} pd_tunnelif_inp_map_tbl_type_e;

hal_ret_t pd_tunnelif_create(pd_if_args_t *args);
pd_tunnelif_t *pd_tunnelif_alloc_init(void);
pd_tunnelif_t *pd_tunnelif_alloc (void);
pd_tunnelif_t *pd_tunnelif_init (pd_tunnelif_t *tunnelif);
hal_ret_t pd_tunnelif_alloc_res(pd_tunnelif_t *pd_tunnelif);
hal_ret_t pd_tunnelif_program_hw(pd_tunnelif_t *pd_tunnelif);
hal_ret_t pd_tunnelif_free (pd_tunnelif_t *tunnelif);
void link_pi_pd(pd_tunnelif_t *pd_upif, if_t *pi_if);
void unlink_pi_pd(pd_tunnelif_t *pd_upif, if_t *pi_if);
hal_ret_t pd_tunnelif_pgm_tunnel_rewrite_tbl(pd_tunnelif_t *pd_tif);
hal_ret_t pd_tunnelif_pgm_inp_mapping_native_tbl(pd_tunnelif_t *pd_tunnelif);
hal_ret_t pd_tunnelif_pgm_inp_mapping_tunneled_tbl(pd_tunnelif_t *pd_tunnelif);
hal_ret_t pd_tunnelif_deprogram_hw(pd_tunnelif_t *pd_tunnelif);
hal_ret_t pd_tunnelif_del_inp_mapp_entries(pd_tunnelif_t *pd_tunnelif,
                                           p4pd_table_id tbl_id);
hal_ret_t pd_tunnelif_del_tunnel_rw_table_entry(pd_tunnelif_t *pd_tif);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_PD_TUNNELIF_HPP__

