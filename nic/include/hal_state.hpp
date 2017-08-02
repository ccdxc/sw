#ifndef __HAL_STATE_HPP__
#define __HAL_STATE_HPP__

#include <slab.hpp>
#include <indexer.hpp>
#include <ht.hpp>

namespace hal {

using hal::utils::slab;
using hal::utils::indexer;
using hal::utils::ht;

#define HAL_HANDLE_HT_SZ                             (16 << 10)

//------------------------------------------------------------------------------
// hal_state class contains all the HAL state (well, most of it) including all
// the slab memory instances, hash tables, indexers etc. used inside HAL,
// potentially across threads, and is instantiated during init time.
//
// TODO:
// 1. add hal_handle_ht here !!
// 2. add APIs to dump info about everything inside this, for debugging
// 3. add tenant HT by tenant id
// 4. add tenant HT by tenant hwid
//------------------------------------------------------------------------------
class hal_state {
public:
    static hal_state *factory(void);
    ~hal_state();

    // get APIs for tenant related state
    slab *tenant_slab(void) const { return tenant_slab_; }
    ht *tenant_id_ht(void) const { return tenant_id_ht_; }
    ht *tenant_hal_handle_ht(void) const { return tenant_hal_handle_ht_; }

    // get APIs for security profile related state
    slab *nwsec_profile_slab(void) const { return nwsec_profile_slab_; }
    ht *nwsec_profile_id_ht(void) const { return nwsec_profile_id_ht_; }
    ht *nwsec_profile_hal_handle_ht(void) const { return nwsec_profile_hal_handle_ht_ ; }

    // get APIs for L2 segment state
    slab *l2seg_slab(void) const { return l2seg_slab_; }
    ht *l2seg_id_ht(void) const { return l2seg_id_ht_; }
    ht *l2seg_hal_handle_ht(void) const { return l2seg_hal_handle_ht_; }

    // get APIs for LIF state
    slab *lif_slab(void) const { return lif_slab_; }
    ht *lif_id_ht(void) const { return lif_id_ht_; }
    ht *lif_hal_handle_ht(void) const { return lif_hal_handle_ht_; }

    // get APIs for interface state
    slab *if_slab(void) const { return if_slab_; }
    indexer *if_hwid_idxr(void) const { return if_hwid_idxr_; }
    ht *if_id_ht(void) const { return if_id_ht_; }
    ht *if_hwid_ht(void) const { return if_hwid_ht_; }
    ht *if_hal_handle_ht(void) const { return if_hal_handle_ht_; }

    // get APIs for endpoint state
    slab *ep_slab(void) const { return ep_slab_; }
    slab *ep_ip_entry_slab(void) const { return ep_ip_entry_slab_; }
    slab *ep_l3_entry_slab(void) const { return ep_l3_entry_slab_; }
    ht *ep_l2_ht(void) const { return ep_l2_ht_; }
    ht *ep_l3_entry_ht(void) const { return ep_l3_entry_ht_; }
    ht *ep_hal_handle_ht(void) const { return ep_hal_handle_ht_; }

    // get APIs for flow/session state
    slab *flow_slab(void) const { return flow_slab_; }
    slab *session_slab(void) const { return session_slab_; }
    slab *tcp_state_slab(void) const { return tcp_state_slab_; }
    ht *session_id_ht(void) const { return session_id_ht_; }
    ht *session_hal_handle_ht(void) const { return session_hal_handle_ht_; }

private:
    bool init(void);
    hal_state();

private:
    // tenant/vrf related state
    struct {
        slab       *tenant_slab_;
        ht         *tenant_id_ht_;
        ht         *tenant_hal_handle_ht_;
    } __PACK__;

    // security profile related state
    struct {
        slab       *nwsec_profile_slab_;
        ht         *nwsec_profile_id_ht_;
        ht         *nwsec_profile_hal_handle_ht_;
    } __PACK__;

    // l2segment related state
    struct {
        slab       *l2seg_slab_;
        ht         *l2seg_id_ht_;
        ht         *l2seg_hal_handle_ht_;
    } __PACK__;

    // LIF related state
    struct {
        slab       *lif_slab_;
        ht         *lif_id_ht_;
        ht         *lif_hal_handle_ht_;
    } __PACK__;

    // interface related state
    struct {
        slab       *if_slab_;
        indexer    *if_hwid_idxr_;
        ht         *if_id_ht_;
        ht         *if_hwid_ht_;
        ht         *if_hal_handle_ht_;
    } __PACK__;

    // endpoint related state
    struct {
        slab       *ep_slab_;
        slab       *ep_ip_entry_slab_;
        slab       *ep_l3_entry_slab_;
        ht         *ep_l2_ht_;
        ht         *ep_l3_entry_ht_;
        ht         *ep_hal_handle_ht_;
    } __PACK__;

    // flow/session related state
    struct {
        slab       *flow_slab_;
        slab       *tcp_state_slab_;
        slab       *session_slab_;
        ht         *flow_ht_;
        ht         *session_id_ht_;
        ht         *session_hal_handle_ht_;
    } __PACK__;
};

extern class hal_state    *g_hal_state;

}    // namespace hal

#endif    // __HAL_STATE_HPP__

