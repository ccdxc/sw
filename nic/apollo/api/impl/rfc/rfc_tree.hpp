//
// Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// \file
/// RFC tree related data structures and APIs
///
//----------------------------------------------------------------------------

#ifndef __RFC_TREE_HPP__
#define __RFC_TREE_HPP__

#include <unordered_map>
#include <rte_bitmap.h>
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/impl/rfc/rfc.hpp"
#include "nic/apollo/api/impl/rfc/rte_bitmap_utils.hpp"

using std::unordered_map;

namespace rfc {

typedef struct rfc_itree_node_data_s {
    uint16_t    class_id;        ///< RFC class id
    uint16_t    rule_no : 12;    ///< rule number
    uint16_t    start   : 1;     ///< TRUE if this is start of the interval
    uint16_t    pad     : 3;     ///< pad bits
} __PACK__ rfc_itree_node_data_t;

///< interval tree node
typedef struct inode_s {
    union {
        ip_addr_t    ipaddr;    ///< IPv4/IPv6 address
        uint16_t     port;      ///< src/dst port
        uint32_t     key32;     ///< generic 32 bit key
    };
    union {
        rfc_itree_node_data_t    rfc;    ///< RFC specific data
    };
} __PACK__ inode_t;

///< interval node table
typedef struct itable_s {
    uint32_t    num_nodes;    ///< number of entries in interval table
    inode_t     *nodes;       ///< number of interval table nodes allocated
} __PACK__ itable_t;

#define RFC_MAX_EQ_CLASSES        1024
typedef unordered_map<rte_bitmap *, uint16_t,
                      rte_bitmap_hasher,
                      rte_bitmap_equal_to>    cbm_map_t;

///< class bitmap (cbm) table entry
typedef struct cbm_te_s {
    uint16_t class_id;
    rte_bitmap *cbm;
} cbm_te_t;

///< generic RFC table thats used in all phases
typedef struct rfc_table_s {
    uint16_t  num_classes;
    uint16_t  max_classes;
    cbm_map_t cbm_map;
    cbm_te_t  cbm_table[RFC_MAX_EQ_CLASSES];
} rfc_table_t;

typedef enum rfc_tree_type_e {
    RFC_TREE_TYPE_NONE       = 0,
    RFC_TREE_TYPE_SIP        = 1,
    RFC_TREE_TYPE_DIP        = 2,
    RFC_TREE_TYPE_STAG       = 3,
    RFC_TREE_TYPE_DTAG       = 4,
    RFC_TREE_TYPE_PORT       = 5,
    RFC_TREE_TYPE_PROTO_PORT = 6,
} rfc_tree_type_t;

///< RFC tree table thats used in all phases
typedef struct rfc_tree_s {
    rfc_tree_type_t    type;
    itable_t           itable;
    uint32_t           num_intervals;
    rfc_table_t        rfc_table;
} rfc_tree_t;

typedef struct rfc_ctxt_s {
    policy_t      *policy;             ///< user configured policy
    ///< phase 0 information
    rfc_tree_t     sip_tree;           ///< RFC tree for SIP prefix
    rfc_tree_t     dip_tree;           ///< RFC tree for DIP prefix
    rfc_tree_t     stag_tree;          ///< RFC tree for src tags used in policy
    rfc_tree_t     dtag_tree;          ///< RFC tree for dst tags used in policy
    rfc_tree_t     port_tree;          ///< RFC tree for port
    rfc_tree_t     proto_port_tree;    ///< RFC tree for protocol-port
    ///< phase 1 information
    rfc_table_t    p1_table;           ///< phase 1 RFC table
    rfc_table_t    p2_table;           ///< phase 2 RFC table
    rte_bitmap     *cbm;               ///< RFC class bitmap instance
                                       ///< (used as scratch pad)
    size_t         cbm_size;           ///< size of class-bit-map (CBM)
    mem_addr_t     base_addr;          ///< base address of the entire RFC block
    uint32_t       mem_size;           ///< RFC memory block size
    tag2class_cb_t tag2class_cb;       ///< class id allocater
    void           *tag2class_cb_ctxt; ///< opaque ctxt passed to above callback
} rfc_ctxt_t;

void rfc_ctxt_destroy(rfc_ctxt_t *rfc_ctxt);
void rfc_table_destroy(rfc_table_t *rfc_table);
sdk_ret_t rfc_ctxt_init(rfc_ctxt_t *rfc_ctxt, policy_params_t *policy_params);
void itable_add_address_inodes(uint32_t rule, inode_t *addr_inode,
                               ip_prefix_t *pfx);
void itable_add_address_range_inodes(uint32_t rule, inode_t *addr_inode,
                                     ipvx_range_t *range);
void itable_add_tag_inodes(uint32_t rule, inode_t *tag_inode, uint32_t tag);
void itable_update_icmp_type_code (rule_l4_match_t *l4Match);
void itable_update_l4_any (rule_l4_match_t *l4Match);
inode_t *itable_add_port_inodes(uint32_t rule, inode_t *port_inode,
                                rule_l4_match_t *l4Match);
void itable_add_proto_port_inodes(uint32_t rule, inode_t *proto_port_inode,
                                  rule_l3_match_t *l3Match,
                                  rule_l4_match_t *l4Match);
/// \brief    dump the contents of given equivalence table
/// \param[in] rfc_ctxt    RFC context carrying all of the previous phases
///                       information processed until now
void rfc_eq_class_table_dump(rfc_table_t *rfc_table);

}    // namespace rfc

#endif    // __RFC_TREE_HPP__
