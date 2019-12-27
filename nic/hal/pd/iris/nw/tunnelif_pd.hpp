#ifndef __HAL_PD_TUNNELIF_HPP__
#define __HAL_PD_TUNNELIF_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/tnnl_rw_pd.hpp"

namespace hal {
namespace pd {

struct pd_tunnelif_s {

    // Hw Indices
    uint32_t imn_idx[3];                 // Input mapping native table idx
    uint32_t imt_idx[3];                 // Input mapping tunneled table idx
    uint32_t tunnel_rw_idx;              // Tunnel rewrite table idx
    uint32_t vf_prop_idx;                // VF properties table idx
    // vrf_id_t tid;

    void        *pi_if;                  // PI ptr
} __PACK__;

typedef enum pd_tunnelif_inp_map_tbl_type_e_ {
    PD_TUNNEL_IF_INP_MAP_NATIVE = 0,
    PD_TUNNEL_IF_INP_MAP_TUNNEL,
    PD_TUNNEL_IF_INP_MAP_NONE,
} pd_tunnelif_inp_map_tbl_type_e;

// ----------------------------------------------------------------------------
// Allocate Tunnel IF Instance
// ----------------------------------------------------------------------------
static inline pd_tunnelif_t *
pd_tunnelif_alloc (void)
{
    pd_tunnelif_t    *tunnelif;

    tunnelif = (pd_tunnelif_t *)g_hal_state_pd->tunnelif_pd_slab()->alloc();
    if (tunnelif == NULL) {
        return NULL;
    }
    return tunnelif;
}

// ----------------------------------------------------------------------------
// Initialize Tunnel IF PD instance
// ----------------------------------------------------------------------------
static inline pd_tunnelif_t *
pd_tunnelif_init (pd_tunnelif_t *tunnelif)
{
    if (!tunnelif) {
        return NULL;
    }
    for (int i = 0; i < 3; i++) {
        tunnelif->imn_idx[i] = -1;
        tunnelif->imt_idx[i] = -1;
    }
    tunnelif->tunnel_rw_idx = -1;
    return tunnelif;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Tunnel IF PD Instance
// ----------------------------------------------------------------------------
static inline pd_tunnelif_t *
pd_tunnelif_alloc_init(void)
{
    return pd_tunnelif_init(pd_tunnelif_alloc());
}

// ----------------------------------------------------------------------------
// Freeing tunnelif PD
// ----------------------------------------------------------------------------
static inline hal_ret_t
pd_tunnelif_free (pd_tunnelif_t *tunnelif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_TUNNELIF_PD, tunnelif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void
pd_tunnelif_link_pi_pd(pd_tunnelif_t *pd_tunnelif, if_t *pi_if)
{
    pd_tunnelif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_tunnelif);
}

// ----------------------------------------------------------------------------
// De-Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void
pd_tunnelif_delink_pi_pd(pd_tunnelif_t *pd_tunnelif, if_t *pi_if)
{
    pd_tunnelif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}

hal_ret_t pd_tunnelif_form_data (pd_tnnl_rw_entry_key_t *tnnl_rw_key,
                                 pd_tunnelif_t *pd_tif);
hal_ret_t pd_tunnelif_alloc_res(pd_tunnelif_t *pd_tunnelif);
hal_ret_t pd_tunnelif_program_hw(pd_tunnelif_t *pd_tunnelif,
                                 bool is_upgrade = false);
hal_ret_t pd_tunnelif_pgm_tunnel_rewrite_tbl(pd_tunnelif_t *pd_tif,
                                             bool is_upgrade = false);
hal_ret_t pd_tunnelif_depgm_tunnel_rewrite_tbl(pd_tunnelif_t *pd_tif);
hal_ret_t pd_tunnelif_pgm_inp_mapping_native_tbl(pd_tunnelif_t *pd_tunnelif,
                                                 int tunnel_type,
                                                 bool is_upgrade = false);
hal_ret_t pd_tunnelif_pgm_inp_mapping_tunneled_tbl(pd_tunnelif_t *pd_tunnelif,
                                                   int tunnel_type,
                                                   uint16_t vf_id,
                                                   bool is_upgrade = false);
hal_ret_t pd_tunnelif_deprogram_hw(pd_tunnelif_t *pd_tunnelif);
hal_ret_t pd_tunnelif_del_inp_mapp_entries(pd_tunnelif_t *pd_tunnelif,
                                           p4pd_table_id tbl_id);
hal_ret_t pd_tunnelif_del_tunnel_rw_table_entry(pd_tunnelif_t *pd_tif);
hal_ret_t pd_tunnelif_cleanup(pd_tunnelif_t *pd_tunnelif);

hal_ret_t pd_tunnelif_create(pd_if_create_args_t *args);
hal_ret_t pd_tunnelif_update(pd_if_update_args_t *args);
hal_ret_t pd_tunnelif_delete(pd_if_delete_args_t *args);
hal_ret_t pd_tunnelif_get (pd_if_get_args_t *args);
hal_ret_t pd_tunnelif_make_clone(pd_if_make_clone_args_t *args);
hal_ret_t pd_tunnelif_mem_free(pd_if_mem_free_args_t *args);
hal_ret_t pd_tunnelif_restore(pd_if_restore_args_t *args);

hal_ret_t pd_tunnelif_restore (pd_if_restore_args_t *args);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_PD_TUNNELIF_HPP__

