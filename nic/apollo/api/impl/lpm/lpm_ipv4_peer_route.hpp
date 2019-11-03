/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm_ipv4_peer_route.hpp
 *
 * @brief   LPM for IPv4 Peer VNET Peer Route
 */

#if !defined (__LPM_IPV4_PEER_ROUTE_HPP__)
#define __LPM_IPV4_PEER_ROUTE_HPP__

#include "nic/apollo/api/impl/lpm/lpm.hpp"

/* key size is 4 bytes for IPv4 Peer Route */
#define LPM_IPV4_PEER_ROUTE_KEY_SIZE    (4)

sdk_ret_t lpm_ipv4_peer_route_add_key_to_stage (uint8_t *bytes, uint32_t idx,
                                                lpm_inode_t *lpm_inode);
sdk_ret_t lpm_ipv4_peer_route_set_default_data (uint8_t *bytes,
                                                uint32_t default_data);
sdk_ret_t lpm_ipv4_peer_route_add_key_to_last_stage (uint8_t *bytes, uint32_t idx,
                                                     lpm_inode_t *lpm_inode);
sdk_ret_t lpm_ipv4_peer_route_write_stage_table (mem_addr_t addr, uint8_t *bytes);
sdk_ret_t lpm_ipv4_peer_route_write_last_stage_table (mem_addr_t addr,
                                                      uint8_t *bytes);
uint32_t lpm_ipv4_peer_route_key_size (void);

uint32_t lpm_ipv4_peer_route_stages (uint32_t num_intrvls);

extern sdk_ret_t
lpm_write_txdma_table (mem_addr_t addr, uint32_t tableid,
                       uint8_t action_id, void *actiondata);

#endif //__LPM_IPV4_PEER_ROUTE_HPP__
