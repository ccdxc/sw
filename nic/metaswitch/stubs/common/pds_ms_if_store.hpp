//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS stub Interface Store
//---------------------------------------------------------------

#ifndef __PDS_MS_INTERFACE_STORE_HPP__
#define __PDS_MS_INTERFACE_STORE_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_object_store.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include <unordered_map>
#include <memory>

namespace pds_ms {

enum class ms_iftype_t
{
    PHYSICAL_PORT,
    LOOPBACK,
    VXLAN_TUNNEL,
    VXLAN_PORT,
    IRB,
    UNKNOWN
};

class if_obj_t : public slab_obj_t<if_obj_t>, 
                 public base_obj_t {
public:
    // *** WARNING 
    // Ensure that ifindex is always the first member in any of the 
    // xxx_properties_t struct below sinhce the key() member function 
    // directly accesses this without checking for the struct type

    struct phy_port_properties_t { // Uplink L3 ports
        ms_ifindex_t ifindex;
        bool         admin_state;
        mac_addr_t   mac_addr;
        uint32_t     lnx_ifindex;
        void         *fri_worker; // FRI worker context
        bool         hal_created; // Intf created in HAL ?
        bool         switchport;  // Switchport ?
    };
    struct vxlan_tunnel_properties_t { // All TEPs
        ms_ifindex_t ifindex;
        ip_addr_t tep_ip;
    };
    struct vxlan_port_properties_t { // Type5 TEP, VNI combo
        ms_ifindex_t   ifindex;
        ip_addr_t      tep_ip;
        pds_vnid_id_t  vni;
        pds_tep_id_t   hal_tep_idx;
        mac_addr_t     dmaci;        // Inner DMAC
        vxlan_port_properties_t(ms_ifindex_t ifi, ip_addr_t tip, pds_vnid_id_t v, pds_tep_id_t ht) 
            : ifindex(ifi), tep_ip(tip), vni(v), hal_tep_idx(ht) {
            memset(dmaci, 0, ETH_ADDR_LEN);
        }
    };
    struct irb_properties_t {  // Subnet SVI
        ms_ifindex_t ifindex;
        ms_bd_id_t bd_id;
    };

    if_obj_t(const phy_port_properties_t& port)
        : prop_(port) {};
    if_obj_t(const vxlan_tunnel_properties_t& vxt)
        : prop_(vxt) {};
    if_obj_t(const vxlan_port_properties_t& vxp)
        : prop_(vxp) {};
    if_obj_t(const irb_properties_t& irb)
        : prop_(irb) {};

    ms_iftype_t type(void) {return prop_.iftype_;}
    phy_port_properties_t& phy_port_properties() {
        SDK_ASSERT (prop_.iftype_ == ms_iftype_t::PHYSICAL_PORT);
        return prop_.phy_port_;
    }
    vxlan_tunnel_properties_t& vxlan_tunnel_properties() {
        SDK_ASSERT (prop_.iftype_ == ms_iftype_t::VXLAN_TUNNEL);
        return prop_.vxt_;
    }
    vxlan_port_properties_t& vxlan_port_properties() {
        SDK_ASSERT (prop_.iftype_ == ms_iftype_t::VXLAN_PORT);
        return prop_.vxp_;
    }
    irb_properties_t& irb_properties() {
        SDK_ASSERT (prop_.iftype_ == ms_iftype_t::IRB);
        return prop_.irb_;
    }
    ms_ifindex_t key(void) const {return prop_.vxt_.ifindex;} // Accessing any of the union types 
                                                          // should be fine since this is always
                                                          // the first member
    void update_store(state_t* state, bool op_delete) override;
    void print_debug_str(void) override;

private:
    struct properties_t {
        ms_iftype_t  iftype_ = ms_iftype_t::UNKNOWN;
        union {
            phy_port_properties_t      phy_port_;
            vxlan_tunnel_properties_t  vxt_;
            vxlan_port_properties_t    vxp_;
            irb_properties_t           irb_;
        };
        properties_t(const phy_port_properties_t& phy_port)
            : iftype_(ms_iftype_t::PHYSICAL_PORT), phy_port_(phy_port) {};
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

void if_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

class host_lif_obj_t: public slab_obj_t<host_lif_obj_t>, 
                 public base_obj_t {
public:
    struct properties_t {
        pds_lif_key_t host_lif;
        ms_bd_id_t  bd_id;
        ms_ifindex_t ifindex;
        properties_t(pds_lif_key_t host_lif_, ms_bd_id_t bd_id_, ms_ifindex_t ifindex_) 
            : host_lif(host_lif_), bd_id(bd_id_), ifindex(ifindex_) {};
    };
    host_lif_obj_t(const properties_t& prop) 
        : prop_(prop) {};

    properties_t& properties(void) {return prop_;}
    const properties_t& properties(void) const {return prop_;}
    void set_properties(const properties_t& prop) {prop_ = prop;}
    pds_lif_key_t key (void) const {return prop_.host_lif;}
    void update_store(state_t* state, bool op_delete) override;
    void print_debug_str(void) override {};

private:
    properties_t prop_;
};

class host_lif_store_t : public obj_store_t <pds_lif_key_t, host_lif_obj_t> {
};

void host_lif_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

}

#endif
