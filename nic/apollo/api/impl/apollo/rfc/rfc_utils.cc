/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_utils.cc
 *
 * @brief   RFC library internal helper functions
 */

#include "include/sdk/platform.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/api/impl/rfc/rfc.hpp"
#include "nic/apollo/api/impl/rfc/rfc_tree.hpp"

namespace rfc {

/**
 * @brief    compare two proto-port tree keys and return -1 or 1 based on where
 *           they fit on the number line
 * @param[in]    n1    pointer to 1st node
 * @param[in]    n2    pointer to 2nd node
 * @param[in]    ctxt  context carrying tree_type information
 * @return  -1 or 1 based on the comparison result
 * NOTE: when keys are same, return -1
 */
static int
inode_compare_cb (const void *n1, const void *n2, void *ctxt)
{
    inode_t         *inode1 = (inode_t *)n1;
    inode_t         *inode2 = (inode_t *)n2;
    itree_type_t    tree_type = reinterpret_cast<itree_type_t&>(ctxt);

    if ((tree_type == ITREE_TYPE_IPV4) ||
        (tree_type == ITREE_TYPE_IPV6)) {
        if (IPADDR_GT(&inode1->ipaddr, &inode2->ipaddr)) {
            return 1;
        }
        return -1;
    } else if (tree_type == ITREE_TYPE_PORT) {
        if (inode1->port > inode2->port) {
            return 1;
        }
        return  -1;
    } else {
        if (inode1->key32 > inode2->key32) {
            return 1;
        }
        return -1;
    }
}

/**
 * @brief    sort interval table entries in each of the phase 0
 *           LPM trees
 * @param[in]    rfc_ctxt    RFC context carrying all the intermediate state for
 *                           this policy
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
rfc_sort_itables (rfc_ctxt_t *rfc_ctxt)
{
    if (rfc_ctxt->policy->af == IP_AF_IPV4) {
        qsort_r(rfc_ctxt->sip_tree.itable.nodes,
                rfc_ctxt->sip_tree.itable.num_nodes,
                sizeof(inode_t), inode_compare_cb,
                (void *)ITREE_TYPE_IPV4);
    } else if (rfc_ctxt->policy->af == IP_AF_IPV6) {
        qsort_r(rfc_ctxt->sip_tree.itable.nodes,
                rfc_ctxt->sip_tree.itable.num_nodes,
                sizeof(inode_t), inode_compare_cb,
                (void *)ITREE_TYPE_IPV6);
    }
    qsort_r(rfc_ctxt->port_tree.itable.nodes,
            rfc_ctxt->port_tree.itable.num_nodes,
            sizeof(inode_t), inode_compare_cb,
            (void *)ITREE_TYPE_PORT);
    qsort_r(rfc_ctxt->proto_port_tree.itable.nodes,
            rfc_ctxt->proto_port_tree.itable.num_nodes,
            sizeof(inode_t), inode_compare_cb,
            (void *)ITREE_TYPE_PROTO_PORT);
    return SDK_RET_OK;
}

/**
 * @brief    given a class bitmap (cbm), check if that exists in the RFC table
 *           already and if not assign new class-id, if it exists already,
 *           use the current class id for that class bitmap
 * @param[in]    rfc_ctxt  RFC context carrying all the intermediate state for
 *                         this policy
 * @param[in]    rfc_table RFC table to add the class id to
 * @param[in]    cbm       class bitmap that needs class id to be computed for
 * @param[in]    cbm_size  class bitmap size
 * @return    SDK_RET_OK on success, failure status code on error
 */
uint16_t
rfc_compute_class_id (rfc_ctxt_t *rfc_ctxt, rfc_table_t *rfc_table,
                      rte_bitmap *cbm, uint32_t cbm_size)
{
    uint8_t       *bits;
    uint16_t      class_id;
    rte_bitmap    *cbm_new;

    auto it = rfc_table->cbm_map.find(cbm);
    if (it != rfc_table->cbm_map.end()) {
        /**< found the bit combination, return corresponding class id */
        return it->second;
    }
    class_id = rfc_table->num_classes++;
    SDK_ASSERT(class_id < RFC_MAX_EQ_CLASSES);
    posix_memalign((void **)&bits, CACHE_LINE_SIZE, cbm_size);
    cbm_new = rte_bitmap_init(rfc_ctxt->policy->max_rules, bits, cbm_size);
    rte_bitmap_or(cbm, cbm_new, cbm_new);
    rfc_table->cbm_table[class_id].class_id = class_id;
    rfc_table->cbm_table[class_id].cbm = cbm_new;
    rfc_table->cbm_map[cbm_new] = class_id;

    return class_id;
}

}    // namespace rfc
