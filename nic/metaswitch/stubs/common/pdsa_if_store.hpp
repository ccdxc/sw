//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub Interface Store
//---------------------------------------------------------------

#ifndef __PDSA_INTERFACE_STORE_HPP__
#define __PDSA_INTERFACE_STORE_HPP__

// Somewhere in slab.hpp __FNAME__ is used that result in build failure 
#define __FNAME__ __FUNCTION__

#include "pdsa_ms_defs.hpp"
#include "pdsa_object_store.hpp"
#include "pdsa_slab_object.hpp"
#include "apollo/api/include/pds.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "sdk/lib/slab/slab.hpp"
#include <unordered_map>
#include <memory>

namespace pdsa_stub {

enum class ms_iftype_t
{
    PHYSICAL_INTERFACE,
    LOOPBACK,
    VXLAN_TUNNEL,
    VXLAN_PORT,
    IRB,
    UNKNOWN
};

class if_obj_t : public slab_obj_t<if_obj_t> {
public:
    struct vxlan_tunnel_properties_t {
        ms_ifindex_t ifindex;
        ip_addr_t tep_ip;
    };
    struct vxlan_port_properties_t {
        ms_ifindex_t ifindex;
        ip_addr_t tep_ip;
    };
    struct irb_properties_t {
        ms_ifindex_t ifindex;
        ms_bd_id_t bd_id;
    };

    if_obj_t(const vxlan_tunnel_properties_t& vxt)
        : prop_(vxt) {};
    if_obj_t(const vxlan_port_properties_t& vxp)
        : prop_(vxp) {};
    if_obj_t(const irb_properties_t& irb)
        : prop_(irb) {};

    ms_iftype_t type(void) {return prop_.iftype_;}
    const vxlan_tunnel_properties_t& vxlan_tunnel_properties(ms_ifindex_t ifindex) const; // throws Error if wrong type
    const vxlan_port_properties_t& vxlan_port_properties(ms_ifindex_t ifindex) const; // throws Error if wrong type
    const irb_properties_t& irb_properties(ms_ifindex_t ifindex) const; // throws Error if wrong type

private:
    struct properties_t {
        ms_iftype_t  iftype_ = ms_iftype_t::UNKNOWN;
        union {
            vxlan_tunnel_properties_t  vxt_;
            vxlan_port_properties_t    vxp_;
            irb_properties_t           irb_;
        };
        properties_t(const vxlan_tunnel_properties_t& vxt)
            : iftype_(ms_iftype_t::VXLAN_TUNNEL), vxt_(vxt) {};
        properties_t(const vxlan_port_properties_t& vxp)
            : iftype_(ms_iftype_t::VXLAN_PORT), vxp_(vxp) {};
        properties_t(const irb_properties_t& irb)
            : iftype_(ms_iftype_t::IRB), irb_(irb) {};

    };
    properties_t prop_;
};

class if_store_t : public obj_store_t <ms_ifindex_t, if_obj_t> {
};

class lif_obj_t: public slab_obj_t<lif_obj_t> {
public:
    struct properties_t {
        pds_lif_key_t lif;
        ms_bd_id_t  bd_id;
        ms_ifindex_t ifindex;
        properties_t(pds_lif_key_t lif_, ms_bd_id_t bd_id_, ms_ifindex_t ifindex_) 
            : lif(lif_), bd_id(bd_id_), ifindex(ifindex_) {};
    };
    lif_obj_t(const properties_t& prop) 
        : prop_(prop) {};

    const properties_t& properties(void) {return prop_;}
    void set_properties(const properties_t& prop) {prop_ = prop;}

private:
    properties_t prop_;
};

class lif_store_t : public obj_store_t <pds_lif_key_t, lif_obj_t> {
};

}

#endif
