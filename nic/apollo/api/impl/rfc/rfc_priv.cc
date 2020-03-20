//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// RFC library internal/private datastructures & APIs shared across pipelines
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/api/impl/rfc/rfc_tree.hpp"

namespace rfc {

bool
rfc_p0_pfx_tree_inode_eq_cb (inode_t *inode1, inode_t *inode2) {
    if (IPADDR_EQ(&inode1->ipaddr, &inode2->ipaddr)) {
        return true;
    }
    return false;
}

bool
rfc_p0_port_tree_inode_eq_cb (inode_t *inode1, inode_t *inode2) {
    if (inode1->port == inode2->port) {
        return true;
    }
    return false;
}

bool
rfc_p0_proto_port_tree_inode_eq_cb (inode_t *inode1, inode_t *inode2) {
    if (inode1->key32 == inode2->key32) {
        return true;
    }
    return false;
}

void
rfc_itree_dump (rfc_tree_t *rfc_tree, itree_type_t tree_type)
{
    itable_t    *itable = &rfc_tree->itable;
    inode_t     *inode;

    PDS_TRACE_DEBUG("No. of interval nodes in itree %u",
                    rfc_tree->num_intervals);
    for (uint32_t i = 0; i < rfc_tree->num_intervals; i++) {
        inode = &itable->nodes[i];
        if ((tree_type == ITREE_TYPE_IPV4_SIP_ACL) ||
            (tree_type == ITREE_TYPE_IPV6_SIP_ACL)) {
            PDS_TRACE_DEBUG("inode %u, SIP %s, classid %u, rule# %u, start %s",
                            i, ipaddr2str(&inode->ipaddr), inode->rfc.class_id,
                            inode->rfc.rule_no,
                            inode->rfc.start ? "true" : "false");
        } else if ((tree_type == ITREE_TYPE_IPV4_DIP_ACL) ||
                   (tree_type == ITREE_TYPE_IPV6_DIP_ACL)) {
            PDS_TRACE_DEBUG("inode %u, DIP %s, classid %u, rule# %u, start %s",
                            i, ipaddr2str(&inode->ipaddr), inode->rfc.class_id,
                            inode->rfc.rule_no,
                            inode->rfc.start ? "true" : "false");
        } else if (tree_type == ITREE_TYPE_PORT) {
            PDS_TRACE_DEBUG("inode %u, port %u, classid %u, rule# %u, start %s",
                            i, inode->port, inode->rfc.class_id,
                            inode->rfc.rule_no,
                            inode->rfc.start ? "true" : "false");
        } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
            PDS_TRACE_DEBUG("inode %u, proto %u, port %u, classid %u, "
                            "rule# %u, start %s", i,
                            (inode->key32 >> 24) & 0xFF,
                            inode->key32 & 0xFFFFFF, inode->rfc.class_id,
                            inode->rfc.rule_no,
                            inode->rfc.start ? "true" : "false");
        }
    }
}

void
rfc_eq_class_table_dump (rfc_table_t *rfc_table)
{
    std::stringstream    a1ss, a2ss;

    PDS_TRACE_DEBUG("Number of equivalence classes %u", rfc_table->num_classes);
    for (uint32_t i = 0; i < rfc_table->num_classes; i++) {
        rte_bitmap2str(rfc_table->cbm_table[i].cbm, a1ss, a2ss);
        PDS_TRACE_DEBUG("class id %u, a1ss %s\na2ss %s",
                        rfc_table->cbm_table[i].class_id,
                        a1ss.str().c_str(), a2ss.str().c_str());
        a1ss.clear();
        a1ss.str("");
        a2ss.clear();
        a2ss.str("");
    }
}

}    // namespace rfc
