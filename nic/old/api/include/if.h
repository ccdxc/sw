#ifndef __IF_H__
#define __IF_H__

#include <base.hpp>
#include <ip.hpp>

//------------------------------------------------------------------------------
// NOTE: assumption here is that h/w identifies each port of any kind, either
// SR-IOV VF ports or uplink ports or ARM cpu port(s) etc. with an id in same
// id-space, called logic port id (aka. h/w port id). That forms an index into
// the following table
//------------------------------------------------------------------------------
typedef struct lport_s {
    uint8_t       uplink:1;     // TRUE if this is uplink port
    uint8_t       cpu_port:1;   // TRUE if this is CPU port
    lport_id_t    lport_id;     // id to be used for next set of lookups
} lport_t;

//------------------------------------------------------------------------------
// vif is virtual interface that represents all kinds of interfaces in the
// system - tunnels, l2 port, l3 port, l3 sub if etc.
//------------------------------------------------------------------------------
typedef struct vif_key_s {
    union {
        struct tep_s {
            vrf_id_t           vrf_id;
            ipv4_addr_t        ipv4_addr;
            union  {
                encap_t        encap;
                uint16_t       port;
                uint32_t       misc;
            };
        } __PACK__ tep_t;

        // for packets coming from VF or .1q/untagged packets coming
        // in on the uplink port(s), (logical port id, vlan) gives the
        // corresponding vif
        struct lport_s {
            lport_id_t        lport_id;
            uint16_t          vlan_id;
        } lport_t;
    };
} __PACK__ vif_key_t;

typedef struct vif_s {
    vif_id_t        vif_id;
    policer_id_t    policer_id;
    uint8_t         cos;
    bd_id_t         bd_id;
    uint8_t         admin_status;
    uint8_t         oper_status;
} __PACK__ vif_t;

typedef enum encap_e {
    ENCAP_TYPE_NONE,
    ENCAP_TYPE_DOT1Q,
    ENCAP_TYPE_VXLAN,
    ENCAP_TYPE_IPINIP,
    ENCAP_TYPE_IPSEC,
    ENCAP_TYPE_MAX,
} encap_t;

//------------------------------------------------------------------------------
// NOTE: tunnel interface key structure
//       - assuming only IPv4 overlay encap for now
//       - can this also be mytep key ??
//------------------------------------------------------------------------------
typedef struct tunnel_if_key_s {
    vrf_id_t           vrf_id;
    ipv4_addr_t        ipv4_addr;
    union  {
        encap_t        encap;
        uint16_t       port;
        uint32_t       misc;
    };
} __PACK__ tunnel_if_key_t;

#endif    // __IF_H__

