/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_eq_table.cc
 *
 * @brief   RFC library equivalence table handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/rfc/rfc.hpp"
#include "nic/apollo/rfc/rfc_tree.hpp"
#include "nic/apollo/rfc/rfc_utils.hpp"
#include "nic/apollo/rfc/rte_bitmap_utils.hpp"
#include "nic/apollo/p4/include/slacl_defines.h"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"

namespace rfc {

/**
 * @brief    given a classid & entry id, fill the corresponding portion of the
 *           RFC phase 1 table buffer
 * @param[in] action_data    pointer to the action data buffer
 * @param[in] entry_num      entry idx (0 to 50, inclusive), we can fit 51
 *                           entries, each 10 bits wide
 * @param[in] cid            RFC class id
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_p1_class_id_pack (slacl_ip_sport_p1_actiondata_t *action_data,
                      uint32_t entry_num, uint16_t cid)
{
    switch (entry_num) {
    case 0:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id00 = cid;
        break;
    case 1:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id01 = cid;
        break;
    case 2:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id02 = cid;
        break;
    case 3:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id03 = cid;
        break;
    case 4:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id04 = cid;
        break;
    case 5:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id05 = cid;
        break;
    case 6:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id06 = cid;
        break;
    case 7:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id07 = cid;
        break;
    case 8:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id08 = cid;
        break;
    case 9:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id09 = cid;
        break;
    case 10:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id10 = cid;
        break;
    case 11:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id11 = cid;
        break;
    case 12:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id12 = cid;
        break;
    case 13:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id13 = cid;
        break;
    case 14:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id14 = cid;
        break;
    case 15:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id15 = cid;
        break;
    case 16:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id16 = cid;
        break;
    case 17:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id17 = cid;
        break;
    case 18:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id18 = cid;
        break;
    case 19:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id19 = cid;
        break;
    case 20:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id20 = cid;
        break;
    case 21:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id21 = cid;
        break;
    case 22:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id22 = cid;
        break;
    case 23:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id23 = cid;
        break;
    case 24:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id24 = cid;
        break;
    case 25:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id25 = cid;
        break;
    case 26:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id26 = cid;
        break;
    case 27:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id27 = cid;
        break;
    case 28:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id28 = cid;
        break;
    case 29:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id29 = cid;
        break;
    case 30:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id30 = cid;
        break;
    case 31:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id31 = cid;
        break;
    case 32:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id32 = cid;
        break;
    case 33:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id33 = cid;
        break;
    case 34:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id34 = cid;
        break;
    case 35:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id35 = cid;
        break;
    case 36:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id36 = cid;
        break;
    case 37:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id37 = cid;
        break;
    case 38:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id38 = cid;
        break;
    case 39:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id39 = cid;
        break;
    case 40:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id40 = cid;
        break;
    case 41:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id41 = cid;
        break;
    case 42:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id42 = cid;
        break;
    case 43:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id43 = cid;
        break;
    case 44:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id44 = cid;
        break;
    case 45:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id45 = cid;
        break;
    case 46:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id46 = cid;
        break;
    case 47:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id47 = cid;
        break;
    case 48:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id48 = cid;
        break;
    case 49:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id49 = cid;
        break;
    case 50:
        action_data->action_u.slacl_ip_sport_p1_slacl_ip_sport_p1.id50 = cid;
        break;
    default:
        OCI_TRACE_ERR("Invalid entry number %u while packing RFC P1 table",
                      entry_num);
        break;
    }
    return SDK_RET_OK;
}

/**
 * @brief    write the current contents of RFC P1 action data buffer to memory
 * @param[in] addr        address to write the action data to
 * @param[in] action_data action data buffer
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_p1_action_data_flush (mem_addr_t addr,
                          slacl_ip_sport_p1_actiondata_t *action_data)
{
    return impl::impl_base::pipeline_impl()->write_to_rxdma_table(addr,
               P4_APOLLO_RXDMA_TBL_ID_SLACL_IP_SPORT_P1,
               SLACL_IP_SPORT_P1_SLACL_IP_SPORT_P1_ID,
               action_data);
}

/**
 * @brief    given the class bitmap tables of phase0, compute class
 *           bitmap tables of RFC phase 1
 * @param[in] policy      user specified policy
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
rfc_compute_p1_eq_class_tables (policy_t *policy, rfc_ctxt_t *rfc_ctxt)
{
    //uint16_t      class_id;
    uint32_t      num_entries;
    rfc_tree_t    *rfc_tree1, *rfc_tree2;
    rte_bitmap    *cbm = rfc_ctxt->cbm;
    
    rfc_tree1 = &rfc_ctxt->pfx_tree;
    rfc_tree2 = &rfc_ctxt->port_tree;
    num_entries = rfc_tree1->rfc_table.num_classes *
                      rfc_tree2->rfc_table.num_classes;
    if (num_entries > SLACL_P1_TABLE_NUM_ENTRIES) {
        OCI_TRACE_ERR("RFC P1 table size %u > expected size %u",
                      num_entries, SLACL_P1_TABLE_NUM_ENTRIES);
        return SDK_RET_ERR;
    }

    /**< do cross product of bitmaps and pick unique bmaps */
    for (uint32_t i = 0; i < rfc_tree1->rfc_table.num_classes; i++) {
        for (uint32_t j = 0; j < rfc_tree2->rfc_table.num_classes; j++) {
            rte_bitmap_and(rfc_tree1->rfc_table.cbm_table[i],
                           rfc_tree2->rfc_table.cbm_table[j], cbm);
            (void)rfc_compute_class_id(policy, &rfc_ctxt->p1_table,
                                 cbm, rfc_ctxt->cbm_size);
        }
    }
    return SDK_RET_OK;
}

}    // namespace rfc
