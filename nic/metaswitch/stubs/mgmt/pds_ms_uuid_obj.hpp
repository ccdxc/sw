//---------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// UUID Obj Base class
//--------------------------------------------------------------

#ifndef __PDS_MS_UUID_OBJ_HPP__
#define __PDS_MS_UUID_OBJ_HPP__

#include "nic/metaswitch/stubs/mgmt/pds_ms_mib_idx_gen.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include <memory>

namespace pds_ms {

enum class uuid_obj_type_t {
    BGP = 1,
    BGP_PEER,
    BGP_PEER_AF,
    VPC,
    SUBNET,
    INTERFACE
};

const char* uuid_obj_type_str(uuid_obj_type_t t);

class uuid_obj_t {
public:
    uuid_obj_t(uuid_obj_type_t type, const pds_obj_key_t& uuid)
        : obj_type_(type), uuid_(uuid) {};
    uuid_obj_type_t obj_type() {return obj_type_;}
    pds_obj_key_t  uuid() {return uuid_;}
    virtual std::string str()=0;
    virtual ~uuid_obj_t() {};
    // Should the UUID obj release be delayed until
    // MS HAL stub delete is invoked
    virtual bool delay_release() {return false;}
private:
    uuid_obj_type_t obj_type_;
    pds_obj_key_t          uuid_;
};

using uuid_obj_uptr_t = std::unique_ptr<uuid_obj_t>;

// BGP Global
// No slab for this one as it is a singleton
class bgp_uuid_obj_t : public uuid_obj_t {
public:
    using ms_id_t = uint32_t;
    bgp_uuid_obj_t(const pds_obj_key_t& uuid);
    ms_id_t ms_id() {return entity_id_;}
    std::string str() override {
        return std::string("BGP Entity ID ").append(std::to_string(entity_id_));
    }
private:
    // There is only a single BGP instance ever
    ms_id_t entity_id_;
};
using bgp_uuid_obj_uptr_t = std::unique_ptr<bgp_uuid_obj_t>;

// BGP Peer Object
class bgp_peer_uuid_obj_t : public slab_obj_t<bgp_peer_uuid_obj_t>,
                            public uuid_obj_t {
public:
    struct ms_id_t {
        ip_addr_t local_ip;
        ip_addr_t peer_ip;
        //types::IPAddress local_ip;
        //types::IPAddress peer_ip;
        ms_id_t(const ip_addr_t& l, const ip_addr_t& p)
            : local_ip(l), peer_ip(p) {};
    };
    bgp_peer_uuid_obj_t(const pds_obj_key_t& uuid,
                        const ip_addr_t& l,
                        const ip_addr_t& p)
        : uuid_obj_t(uuid_obj_type_t::BGP_PEER, uuid),
          mib_keys_(l,p) {};

    ms_id_t& ms_id() { return mib_keys_; }
    std::string str() override {
        return std::string("BGP Peer Local IP ").append(ipaddr2str(&mib_keys_.local_ip))
               .append(" Peer IP ").append(ipaddr2str(&mib_keys_.peer_ip));
    }
private:
    ms_id_t  mib_keys_;
};
using bgp_peer_uuid_obj_uptr_t = std::unique_ptr<bgp_peer_uuid_obj_t>;
void bgp_peer_uuid_obj_slab_init(slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id);

// BGP PeerAF Object
class bgp_peer_af_uuid_obj_t : public slab_obj_t<bgp_peer_af_uuid_obj_t>,
                            public uuid_obj_t {
public:
    struct ms_id_t {
        ip_addr_t local_ip;
        ip_addr_t peer_ip;
        uint32_t  afi;
        uint32_t  safi;
        ms_id_t(const ip_addr_t& l, const ip_addr_t& p, const uint32_t &af,
                const uint32_t &saf)
            : local_ip(l), peer_ip(p), afi(af), safi(saf) {};
    };
    bgp_peer_af_uuid_obj_t(const pds_obj_key_t& uuid, const ip_addr_t& l,
                           const ip_addr_t& p, const uint32_t &afi,
                           const uint32_t &safi)
        : uuid_obj_t(uuid_obj_type_t::BGP_PEER_AF, uuid),
          mib_keys_(l, p, afi, safi) {};

    ms_id_t& ms_id() { return mib_keys_; }
    std::string str() override {
        return std::string("BGP PeerAF Local IP ")
               .append(ipaddr2str(&mib_keys_.local_ip))
               .append(" PeerAF IP ").append(ipaddr2str(&mib_keys_.peer_ip))
               .append(std::to_string(mib_keys_.afi))
               .append(std::to_string(mib_keys_.safi));
    }
private:
    ms_id_t  mib_keys_;
};
using bgp_peer_af_uuid_obj_uptr_t = std::unique_ptr<bgp_peer_af_uuid_obj_t>;
void bgp_peer_af_uuid_obj_slab_init(slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id);

// VPC object
class vpc_uuid_obj_t : public slab_obj_t<vpc_uuid_obj_t>,
                          public uuid_obj_t {
public:
    using ms_id_t = uint32_t;
    vpc_uuid_obj_t(const pds_obj_key_t& uuid, bool is_default=false)
        : uuid_obj_t(uuid_obj_type_t::VPC, uuid), is_default_(is_default) {
        if (!is_default) {
            idx_guard.alloc();
        }
    }
    ms_id_t ms_id(void);
    ms_id_t ms_v4_rttbl_id(void);
    std::string str() override {
        return std::string("VPC VRF-ID ").append(std::to_string(ms_id()));
    }
    // Delay release until MS HAL stub delete is invoked
    // so that internal MS VRF ID is not released and reallocated until
    // all VRF state is cleaned up all the way down until MS HAL stubs
    bool delay_release() override {return true;}
private:
    bool is_default_;
    mib_idx_gen_guard_t  idx_guard = MIB_IDX_GEN_TBL_VRF;
};
using vpc_uuid_obj_uptr_t = std::unique_ptr<vpc_uuid_obj_t>;
void vpc_uuid_obj_slab_init(slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id);

// Subnet object
class subnet_uuid_obj_t : public slab_obj_t<subnet_uuid_obj_t>,
                          public uuid_obj_t {
public:
    using ms_id_t = uint32_t;
    subnet_uuid_obj_t(const pds_obj_key_t& uuid)
        : uuid_obj_t(uuid_obj_type_t::SUBNET, uuid) {
            idx_guard.alloc();
        };

    ms_id_t ms_id() { return idx_guard.idx(); }
    std::string str() override {
        return std::string("Subnet BD-ID ").append(std::to_string(ms_id()));
    }
    // Delay release until MS HAL stub delete is invoked
    // so that internal MS BD ID is not released and reallocated until
    // all BD state is cleaned up all the way down until MS HAL stubs
    bool delay_release() override {return true;}
private:
    mib_idx_gen_guard_t  idx_guard = MIB_IDX_GEN_TBL_BD;
};
using subnet_uuid_obj_uptr_t = std::unique_ptr<subnet_uuid_obj_t>;
void subnet_uuid_obj_slab_init(slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id);

// Interface Object
class interface_uuid_obj_t : public slab_obj_t<interface_uuid_obj_t>,
                            public uuid_obj_t {
public:
    using ms_id_t = uint32_t;
    struct info_t {
        ms_id_t  ms_ifindex;
        uint32_t  eth_ifindex = 0;
        info_t(uint32_t ifi, uint32_t eifi)
            : ms_ifindex(ifi), eth_ifindex (eifi) {};
    };
    interface_uuid_obj_t(const pds_obj_key_t& uuid, uint32_t ifi, uint32_t eifi)
        : uuid_obj_t(uuid_obj_type_t::INTERFACE, uuid),
          info_(ifi, eifi) {};

    ms_id_t ms_id() { return info_.ms_ifindex; }
    info_t info() { return info_; }

    std::string str() override {
        return std::string("Interface IfIdx ")
               .append(std::to_string(info_.ms_ifindex));
    }
private:
    info_t  info_;
};
using interface_uuid_obj_uptr_t = std::unique_ptr<interface_uuid_obj_t>;
void interface_uuid_obj_slab_init(slab_uptr_t slabs_[], sdk::lib::slab_id_t slab_id);

} // End namespace

#endif
