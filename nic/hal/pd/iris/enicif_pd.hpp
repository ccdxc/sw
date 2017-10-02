#ifndef __HAL_PD_ENICIF_HPP__
#define __HAL_PD_ENICIF_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/common/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

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

// ----------------------------------------------------------------------------
// Allocate EnicIf Instance
// ----------------------------------------------------------------------------
static inline pd_enicif_t *
pd_enicif_alloc (void)
{
    pd_enicif_t    *enicif;

    enicif = (pd_enicif_t *)g_hal_state_pd->enicif_pd_slab()->alloc();
    if (enicif == NULL) {
        return NULL;
    }
    return enicif;
}

// ----------------------------------------------------------------------------
// Initialize EnicIF PD instance
// ----------------------------------------------------------------------------
static inline pd_enicif_t *
pd_enicif_init (pd_enicif_t *enicif)
{
    // Nothing to do currently
    if (!enicif) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return enicif;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize EnicIf PD Instance
// ----------------------------------------------------------------------------
static inline pd_enicif_t *
pd_enicif_alloc_init(void)
{
    return pd_enicif_init(pd_enicif_alloc());
}

// ----------------------------------------------------------------------------
// Freeing EnicIF PD
// ----------------------------------------------------------------------------
static inline hal_ret_t
pd_enicif_free (pd_enicif_t *enicif)
{
    g_hal_state_pd->enicif_pd_slab()->free(enicif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void 
pd_enicif_link_pi_pd(pd_enicif_t *pd_enicif, if_t *pi_if)
{
    pd_enicif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_enicif);
}

// ----------------------------------------------------------------------------
// De-Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void 
pd_enicif_delink_pi_pd(pd_enicif_t *pd_enicif, if_t *pi_if)
{
    pd_enicif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}

hal_ret_t pd_enicif_alloc_res(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_program_hw(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_free (pd_enicif_t *enicif);
void link_pi_pd(pd_enicif_t *pd_upif, if_t *pi_if);
void unlink_pi_pd(pd_enicif_t *pd_upif, if_t *pi_if);
hal_ret_t pd_enicif_pgm_inp_prop_mac_vlan_tbl(pd_enicif_t *pd_enicif, 
        nwsec_profile_t *nwsec_prof);
hal_ret_t pd_enicif_pd_pgm_output_mapping_tbl(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_cleanup(pd_enicif_t *pd_enicif);

uint32_t pd_enicif_get_l4_prof_idx(pd_enicif_t *pd_enicif);
pd_lif_t *pd_enicif_get_pd_lif(pd_enicif_t *pd_enicif);
hal_ret_t
pd_enicif_inp_prop_form_data (pd_enicif_t *pd_enicif,
                              nwsec_profile_t *nwsec_prof,
                              input_properties_mac_vlan_actiondata &data,
                              bool host_entry);
hal_ret_t
pd_enicif_upd_inp_prop_mac_vlan_tbl (pd_enicif_t *pd_enicif, 
                                     nwsec_profile_t *nwsec_prof);
hal_ret_t
pd_enicif_depgm_inp_prop_mac_vlan_tbl(pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_pd_depgm_output_mapping_tbl (pd_enicif_t *pd_enicif);
hal_ret_t pd_enicif_deprogram_hw (pd_enicif_t *pd_enicif);

hal_ret_t pd_enicif_create(pd_if_args_t *args);
hal_ret_t pd_enicif_update(pd_if_args_t *args);
hal_ret_t pd_enicif_delete(pd_if_args_t *args);
hal_ret_t pd_enicif_make_clone(if_t *hal_if, if_t *clone);
hal_ret_t pd_enicif_mem_free(pd_if_args_t *args);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_PD_ENICIF_HPP__

