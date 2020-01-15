//---------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// UUID Table Mapping Objects
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/mgmt/pds_ms_uuid_obj.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mib_idx_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"

namespace pds_ms {

const char*
uuid_obj_type_str (uuid_obj_type_t t) {
    switch (t) {
        case uuid_obj_type_t::BGP:
            return "BGP"; break;
        case uuid_obj_type_t::BGP_PEER:
            return "BGP_PEER"; break;
        case uuid_obj_type_t::BGP_PEER_AF:
            return "BGP_PEER_AF"; break;
        case uuid_obj_type_t::VPC:
            return "VPC"; break;
        case uuid_obj_type_t::SUBNET:
            return "SUBNET"; break;
    }
    return "UNKNOWN";
}

// BGP Global object - Singleton, no slab required
bgp_uuid_obj_t::bgp_uuid_obj_t(const pds_obj_key_t& uuid) 
    : uuid_obj_t(uuid_obj_type_t::BGP, uuid),
      entity_id_(PDS_MS_BGP_RM_ENT_INDEX) {};

// BGP Peer object
template<> sdk::lib::slab* slab_obj_t<bgp_peer_uuid_obj_t>::slab_ = nullptr;
void
bgp_peer_uuid_obj_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-BGP-PEER-UUID-OBJ",
                                      slab_id, sizeof(bgp_peer_uuid_obj_t),
                                      4,
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for BGP Peer UUID Map obj");
    }
    bgp_peer_uuid_obj_t::set_slab(slabs_[slab_id].get());
}

// BGP PeerAF object
template<> sdk::lib::slab* slab_obj_t<bgp_peer_af_uuid_obj_t>::slab_ = nullptr;
void
bgp_peer_af_uuid_obj_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-BGP-PEERAF-UUID-OBJ",
                                      slab_id, sizeof(bgp_peer_af_uuid_obj_t),
                                      4,
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for BGP PeerAF UUID Map obj");
    }
    bgp_peer_af_uuid_obj_t::set_slab(slabs_[slab_id].get());
}

// Subnet object
template<> sdk::lib::slab* slab_obj_t<subnet_uuid_obj_t>::slab_ = nullptr;
void
subnet_uuid_obj_slab_init (slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id)
{
    slabs_[slab_id].
        reset(sdk::lib::slab::factory("PDS-MS-SUBNET-UUID-OBJ",
                                      slab_id, sizeof(subnet_uuid_obj_t),
                                      3,
                                      true, true, true));
    if (unlikely (!slabs_[slab_id])) {
        throw Error("SLAB creation failed for Subnet UUID mapping obj");
    }
    subnet_uuid_obj_t::set_slab(slabs_[slab_id].get());
}

} // End namespace

