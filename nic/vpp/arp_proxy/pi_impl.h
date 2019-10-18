//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Apollo pipeline interface

#ifndef __VPP_ARP_PROXY_APOLLO_IMPL_H__
#define __VPP_ARP_PROXY_APOLLO_IMPL_H__

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes

typedef struct vnic_s {
    char vr_mac[6];
    int vlan_id;
} vnic_t;

int egress_vnic_read(int vnic_id, vnic_t *vnic);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_ARP_PROXY_APOLLO_IMPL_H__
