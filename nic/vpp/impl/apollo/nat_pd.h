//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APOLLO_NAT_H__
#define __VPP_IMPL_APOLLO_NAT_H__

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes

int pds_snat_tbl_write_ip4(int nat_index, uint32_t ip, uint16_t port);
int pds_dnat_tbl_write_ip4(int nat_index, uint32_t ip, uint16_t port);
int pds_snat_tbl_read_ip4(int nat_index, uint32_t *ip, uint16_t *port);
int pds_dnat_tbl_read_ip4(int nat_index, uint32_t *ip, uint16_t *port);


#ifdef __cplusplus
}
#endif
#endif    // __VPP_IMPL_APOLLO_NAT_H__
