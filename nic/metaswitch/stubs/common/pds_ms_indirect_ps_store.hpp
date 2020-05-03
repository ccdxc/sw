//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub indirect pathset store
//---------------------------------------------------------------

#ifndef __PDS_MS_INDIRECT_PS_STORE_HPP__
#define __PDS_MS_INDIRECT_PS_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pds_ms {

// Indirect Pathset (Cascaded mode) that points to direct pathsets
// In cascaded mode, MS creates unique Indirect pathset for each
// destination IP that its tracking
// This object holds the back-ref to the destination IP that this
// indirect pathset is tracking
class indirect_ps_obj_t : public slab_obj_t<indirect_ps_obj_t>,
                             public base_obj_t {
public:
    indirect_ps_obj_t();

    ms_hw_tbl_id_t direct_ps_dpcorr(void) {return direct_ps_dpcorr_;}
    ip_addr_t& destip(void) {return destip_;}
    bool is_ms_evpn_tep_ip(void) {return ms_evpn_tep_ip_;}

    void set_ms_evpn_tepip(const ip_addr_t& destip) {
        destip_ = destip; ms_evpn_tep_ip_ = true;
    }
    void set_destip(const ip_addr_t& destip) {
        destip_ = destip; ms_evpn_tep_ip_ = false;
    }
    void set_direct_ps_dpcorr(ms_hw_tbl_id_t direct_ps_dpcorr) {
        direct_ps_dpcorr_ = direct_ps_dpcorr;
    }

    void reset_destip(void);
private:
    ms_hw_tbl_id_t direct_ps_dpcorr_ = 0; // direct pathset DP correlator
    ip_addr_t  destip_;       // Dest IP that this Pathset tracks
    bool  ms_evpn_tep_ip_ = true; // Is the IP being tracked a TEP IP
                                   // created from MS EVPN
};

class indirect_ps_store_t : public obj_store_t <ms_ps_id_t, indirect_ps_obj_t> {
};

void indirect_ps_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

} // End namespace

#endif

