/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm.cc
 *
 * @brief   LPM library implementation
 */

#include <math.h>
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/api/impl/lpm/lpm_priv.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_sport.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv4_acl.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv6_acl.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv4_tag.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv6_tag.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv4_sip.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv6_sip.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv4_meter.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv6_meter.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv4_route.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv6_route.hpp"
#include "nic/apollo/api/impl/lpm/lpm_proto_dport.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv4_peer_route.hpp"
#include "nic/apollo/api/impl/lpm/lpm_ipv6_peer_route.hpp"

/**
 * key size is 4 bytes for IPv4, 8 bytes (assuming max prefix length of 64)
 * for IPv6, 2 bytes for port tree and 4 bytes for proto-port tree
 */
uint32_t
lpm_entry_key_size (itree_type_t tree_type)
{
    if (tree_type == ITREE_TYPE_IPV4) {
        return lpm_ipv4_route_key_size();
    } else if (tree_type == ITREE_TYPE_IPV6) {
        return lpm_ipv6_route_key_size();
    } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
        return lpm_proto_dport_key_size();
    } else if (tree_type == ITREE_TYPE_PORT) {
        return lpm_sport_key_size();
    } else if (tree_type == ITREE_TYPE_IPV4_DIP_ACL) {
        return lpm_ipv4_acl_key_size();
    } else if (tree_type == ITREE_TYPE_IPV6_DIP_ACL) {
        return lpm_ipv6_acl_key_size();
    } else if (tree_type == ITREE_TYPE_METER_V4) {
        return lpm_ipv4_meter_key_size();
    } else if (tree_type == ITREE_TYPE_METER_V6) {
        return lpm_ipv6_meter_key_size();
    } else if (tree_type == ITREE_TYPE_TAG_V4) {
        return lpm_ipv4_tag_key_size();
    } else if (tree_type == ITREE_TYPE_TAG_V6) {
        return lpm_ipv6_tag_key_size();
    } else if (tree_type == ITREE_TYPE_IPV4_SIP_ACL) {
        return lpm_ipv4_sip_key_size();
    } else if (tree_type == ITREE_TYPE_IPV6_SIP_ACL) {
        return lpm_ipv6_sip_key_size();
    } else if (tree_type == ITREE_TYPE_IPV4_PEER_ROUTE) {
        return lpm_ipv4_peer_route_key_size();
    } else if (tree_type == ITREE_TYPE_IPV6_PEER_ROUTE) {
        return lpm_ipv6_peer_route_key_size();
    }
    return 0;
}

/**
 * number of keys per table is CACHE_LINE_SIZE/(sizeof(ipv4_addr_t) = 16 for
 * IPv4 and CACHE_LINE_SIZE/8 = 8 for IPv6 (assuming max prefix length of 64)
 * NOTE: this doesn't apply to last stage where each node also has 2 byte
 * nexthop
 */
uint32_t
lpm_keys_per_table (itree_type_t tree_type)
{
    if (tree_type == ITREE_TYPE_IPV4) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_route_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV6) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_route_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
        return ((LPM_TABLE_SIZE / lpm_proto_dport_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_PORT) {
        return ((LPM_TABLE_SIZE / lpm_sport_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV4_DIP_ACL) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_acl_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV6_DIP_ACL) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_acl_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_METER_V4) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_meter_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_METER_V6) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_meter_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_TAG_V4) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_tag_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_TAG_V6) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_tag_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV4_SIP_ACL) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_sip_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV6_SIP_ACL) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_sip_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV4_PEER_ROUTE) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_peer_route_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV6_PEER_ROUTE) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_peer_route_key_size()) - 1);
    }
    return 0;
}

/**
 * @brief    compute the number of stages needed for LPM lookup given the
 *           interval table scale
 * @param[in]    tree_type     type of the tree being built
 * @param[in]    num_intrvls   number of intervals in the interval table
 * @return       number of lookup stages (aka. depth of the interval tree)
 *
 * for IPv6:
 *     The last stage gives an 4-way decision. The other stages each give
 *     a 8-way decision.
 *     #stages = 1 + log8(num_intrvls/4.0)
 *             = 1 + log2(num_intrvls/4.0)/log2(8)
 *             = 1 + log2(num_intrvls/4.0)/3.0
 */
uint32_t
lpm_stages (itree_type_t tree_type, uint32_t num_intrvls)
{
    if (tree_type == ITREE_TYPE_IPV4) {
        return lpm_ipv4_route_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV6) {
        return lpm_ipv6_route_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
        return lpm_proto_dport_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_PORT) {
        return lpm_sport_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV4_DIP_ACL) {
        return lpm_ipv4_acl_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV6_DIP_ACL) {
        return lpm_ipv6_acl_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_METER_V4) {
        return lpm_ipv4_meter_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_METER_V6) {
        return lpm_ipv6_meter_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_TAG_V4) {
        return lpm_ipv4_tag_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_TAG_V6) {
        return lpm_ipv6_tag_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV4_SIP_ACL) {
        return lpm_ipv4_sip_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV6_SIP_ACL) {
        return lpm_ipv6_sip_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV4_PEER_ROUTE) {
        return lpm_ipv4_peer_route_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV6_PEER_ROUTE) {
        return lpm_ipv6_peer_route_stages(num_intrvls);
    }

    return 0;
}

sdk_ret_t
lpm_add_key_to_stage (itree_type_t tree_type, lpm_stage_info_t *stage,
                      lpm_inode_t *lpm_inode)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_add_key_to_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_add_key_to_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_add_key_to_stage(stage->curr_table,
                                   stage->curr_index,
                                   lpm_inode);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_add_key_to_stage(stage->curr_table,
                                         stage->curr_index,
                                         lpm_inode);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_add_key_to_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_add_key_to_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_add_key_to_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_add_key_to_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }

    stage->curr_index++;
    return SDK_RET_OK;
}

sdk_ret_t
lpm_write_stage_table (itree_type_t tree_type, lpm_stage_info_t *stage)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_write_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_write_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_write_stage_table(stage->curr_table_addr,
                                    stage->curr_table);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_write_stage_table(stage->curr_table_addr,
                                          stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_write_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_write_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_write_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_write_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }
    /**< update this stage meta for next time */
    stage->curr_index = 0;
    stage->curr_table_addr += LPM_TABLE_SIZE;
    memset(stage->curr_table, 0xFF, LPM_TABLE_SIZE);
    return SDK_RET_OK;
}

sdk_ret_t
lpm_add_key_to_last_stage (itree_type_t tree_type, lpm_stage_info_t *stage,
                           lpm_inode_t *lpm_inode)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_add_key_to_last_stage(stage->curr_table,
                                       stage->curr_index,
                                       lpm_inode);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_add_key_to_last_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_add_key_to_last_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_add_key_to_last_stage(stage->curr_table,
                                              stage->curr_index,
                                              lpm_inode);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_add_key_to_last_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_add_key_to_last_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_add_key_to_last_stage(stage->curr_table,
                                                  stage->curr_index,
                                                  lpm_inode);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_add_key_to_last_stage(stage->curr_table,
                                                  stage->curr_index,
                                                  lpm_inode);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }

    stage->curr_index++;
    return SDK_RET_OK;
}

sdk_ret_t
lpm_set_default_data (itree_type_t tree_type, lpm_stage_info_t *stage,
                      uint32_t default_data)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_set_default_data(stage->curr_table, default_data);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
lpm_write_last_stage_table (itree_type_t tree_type, lpm_stage_info_t *stage)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_write_last_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_write_last_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_write_last_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_write_last_stage_table(stage->curr_table_addr,
                                               stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_write_last_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_write_last_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_write_last_stage_table(stage->curr_table_addr,
                                                   stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_write_last_stage_table(stage->curr_table_addr,
                                                   stage->curr_table);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }
    /**< update last stage meta for next time */
    stage->curr_index = 0;
    stage->curr_table_addr += LPM_TABLE_SIZE;
    memset(stage->curr_table, 0xFF, LPM_TABLE_SIZE);
    return SDK_RET_OK;
}
