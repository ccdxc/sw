/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_eq_table.cc
 *
 * @brief   RFC library equivalence table handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/impl/rfc/rfc.hpp"
#include "nic/apollo/api/impl/rfc/rfc_tree.hpp"
#include "nic/apollo/api/impl/rfc/rte_bitmap_utils.hpp"
#include "nic/apollo/api/impl/apulu/rfc/rfc_utils.hpp"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "nic/apollo/p4/include/apulu_sacl_defines.h"

#define RFC_RESULT_RULE_ACTION_ALLOW               0
#define RFC_RESULT_RULE_ACTION_DENY                1
#define RFC_RESULT_SET_ACTION_BIT(val, action)     ((val) |= action)
#define RFC_RESULT_BITS_PRIORITY_MASK              0x7FE
#define RFC_RESULT_SET_PRIORITY_BITS(val, prio)    ((val) |= ((prio<<1) & RFC_RESULT_BITS_PRIORITY_MASK))

namespace rfc {

typedef void (*rfc_compute_cl_addr_entry_cb_t)(mem_addr_t base_address,
                                               uint32_t class_block_number,
                                               uint32_t num_classes_per_block,
                                               mem_addr_t *next_cl_addr,
                                               uint16_t *next_entry_num);
typedef sdk_ret_t (*rfc_action_data_flush_cb_t)(mem_addr_t addr,
                                                void *action_data);
typedef sdk_ret_t (*rfc_table_entry_pack_cb_t)(uint32_t running_id,
                                               void *action_data,
                                               uint32_t entry_num,
                                               uint16_t entry_val);
typedef uint16_t rfc_compute_entry_val_cb_t(rfc_ctxt_t *rfc_ctxt,
                                            rfc_table_t *rfc_table,
                                            rte_bitmap *cbm, uint32_t cbm_size,
                                            void *ctxt);
/**
 * @brief    given a classid & entry id, fill the corresponding portion of the
 *           RFC phase 1 table entry action data
 * @param[in] running_id    running index of the entry (for debugging)
 * @param[in] actiondata    pointer to the action data
 * @param[in] entry_num     entry idx (0 to 50, inclusive), we can fit 51
 *                          entries, each 10 bits wide
 * @param[in] cid           RFC class id
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_p1_table_entry_pack (uint32_t running_id, void *actiondata,
                         uint32_t entry_num, uint16_t cid)
{
    rfc_p1_actiondata_t    *action_data;

    PDS_TRACE_DEBUG("running id %u, entry %u, class id %u",
                    running_id, entry_num, cid);
    action_data = (rfc_p1_actiondata_t *)actiondata;
    switch (entry_num) {
    case 0:
        action_data->action_u.rfc_p1_rfc_action_p1.id00 = cid;
        break;
    case 1:
        action_data->action_u.rfc_p1_rfc_action_p1.id01 = cid;
        break;
    case 2:
        action_data->action_u.rfc_p1_rfc_action_p1.id02 = cid;
        break;
    case 3:
        action_data->action_u.rfc_p1_rfc_action_p1.id03 = cid;
        break;
    case 4:
        action_data->action_u.rfc_p1_rfc_action_p1.id04 = cid;
        break;
    case 5:
        action_data->action_u.rfc_p1_rfc_action_p1.id05 = cid;
        break;
    case 6:
        action_data->action_u.rfc_p1_rfc_action_p1.id06 = cid;
        break;
    case 7:
        action_data->action_u.rfc_p1_rfc_action_p1.id07 = cid;
        break;
    case 8:
        action_data->action_u.rfc_p1_rfc_action_p1.id08 = cid;
        break;
    case 9:
        action_data->action_u.rfc_p1_rfc_action_p1.id09 = cid;
        break;
    case 10:
        action_data->action_u.rfc_p1_rfc_action_p1.id10 = cid;
        break;
    case 11:
        action_data->action_u.rfc_p1_rfc_action_p1.id11 = cid;
        break;
    case 12:
        action_data->action_u.rfc_p1_rfc_action_p1.id12 = cid;
        break;
    case 13:
        action_data->action_u.rfc_p1_rfc_action_p1.id13 = cid;
        break;
    case 14:
        action_data->action_u.rfc_p1_rfc_action_p1.id14 = cid;
        break;
    case 15:
        action_data->action_u.rfc_p1_rfc_action_p1.id15 = cid;
        break;
    case 16:
        action_data->action_u.rfc_p1_rfc_action_p1.id16 = cid;
        break;
    case 17:
        action_data->action_u.rfc_p1_rfc_action_p1.id17 = cid;
        break;
    case 18:
        action_data->action_u.rfc_p1_rfc_action_p1.id18 = cid;
        break;
    case 19:
        action_data->action_u.rfc_p1_rfc_action_p1.id19 = cid;
        break;
    case 20:
        action_data->action_u.rfc_p1_rfc_action_p1.id20 = cid;
        break;
    case 21:
        action_data->action_u.rfc_p1_rfc_action_p1.id21 = cid;
        break;
    case 22:
        action_data->action_u.rfc_p1_rfc_action_p1.id22 = cid;
        break;
    case 23:
        action_data->action_u.rfc_p1_rfc_action_p1.id23 = cid;
        break;
    case 24:
        action_data->action_u.rfc_p1_rfc_action_p1.id24 = cid;
        break;
    case 25:
        action_data->action_u.rfc_p1_rfc_action_p1.id25 = cid;
        break;
    case 26:
        action_data->action_u.rfc_p1_rfc_action_p1.id26 = cid;
        break;
    case 27:
        action_data->action_u.rfc_p1_rfc_action_p1.id27 = cid;
        break;
    case 28:
        action_data->action_u.rfc_p1_rfc_action_p1.id28 = cid;
        break;
    case 29:
        action_data->action_u.rfc_p1_rfc_action_p1.id29 = cid;
        break;
    case 30:
        action_data->action_u.rfc_p1_rfc_action_p1.id30 = cid;
        break;
    case 31:
        action_data->action_u.rfc_p1_rfc_action_p1.id31 = cid;
        break;
    case 32:
        action_data->action_u.rfc_p1_rfc_action_p1.id32 = cid;
        break;
    case 33:
        action_data->action_u.rfc_p1_rfc_action_p1.id33 = cid;
        break;
    case 34:
        action_data->action_u.rfc_p1_rfc_action_p1.id34 = cid;
        break;
    case 35:
        action_data->action_u.rfc_p1_rfc_action_p1.id35 = cid;
        break;
    case 36:
        action_data->action_u.rfc_p1_rfc_action_p1.id36 = cid;
        break;
    case 37:
        action_data->action_u.rfc_p1_rfc_action_p1.id37 = cid;
        break;
    case 38:
        action_data->action_u.rfc_p1_rfc_action_p1.id38 = cid;
        break;
    case 39:
        action_data->action_u.rfc_p1_rfc_action_p1.id39 = cid;
        break;
    case 40:
        action_data->action_u.rfc_p1_rfc_action_p1.id40 = cid;
        break;
    case 41:
        action_data->action_u.rfc_p1_rfc_action_p1.id41 = cid;
        break;
    case 42:
        action_data->action_u.rfc_p1_rfc_action_p1.id42 = cid;
        break;
    case 43:
        action_data->action_u.rfc_p1_rfc_action_p1.id43 = cid;
        break;
    case 44:
        action_data->action_u.rfc_p1_rfc_action_p1.id44 = cid;
        break;
    case 45:
        action_data->action_u.rfc_p1_rfc_action_p1.id45 = cid;
        break;
    case 46:
        action_data->action_u.rfc_p1_rfc_action_p1.id46 = cid;
        break;
    case 47:
        action_data->action_u.rfc_p1_rfc_action_p1.id47 = cid;
        break;
    case 48:
        action_data->action_u.rfc_p1_rfc_action_p1.id48 = cid;
        break;
    case 49:
        action_data->action_u.rfc_p1_rfc_action_p1.id49 = cid;
        break;
    case 50:
        action_data->action_u.rfc_p1_rfc_action_p1.id50 = cid;
        break;
    case 51:
        action_data->action_u.rfc_p1_rfc_action_p1.id51 = cid;
        break;
    case 52:
        action_data->action_u.rfc_p1_rfc_action_p1.id52 = cid;
        break;
    case 53:
        action_data->action_u.rfc_p1_rfc_action_p1.id53 = cid;
        break;
    case 54:
        action_data->action_u.rfc_p1_rfc_action_p1.id54 = cid;
        break;
    case 55:
        action_data->action_u.rfc_p1_rfc_action_p1.id55 = cid;
        break;
    case 56:
        action_data->action_u.rfc_p1_rfc_action_p1.id56 = cid;
        break;
    case 57:
        action_data->action_u.rfc_p1_rfc_action_p1.id57 = cid;
        break;
    case 58:
        action_data->action_u.rfc_p1_rfc_action_p1.id58 = cid;
        break;
    case 59:
        action_data->action_u.rfc_p1_rfc_action_p1.id59 = cid;
        break;
    case 60:
        action_data->action_u.rfc_p1_rfc_action_p1.id60 = cid;
        break;
    case 61:
        action_data->action_u.rfc_p1_rfc_action_p1.id61 = cid;
        break;
    case 62:
        action_data->action_u.rfc_p1_rfc_action_p1.id62 = cid;
        break;
    case 63:
        action_data->action_u.rfc_p1_rfc_action_p1.id63 = cid;
        break;
    default:
        PDS_TRACE_ERR("Invalid entry number %u while packing RFC P1 table",
                      entry_num);
        break;
    }

    return SDK_RET_OK;
}

/**
 * @brief    given a classid & entry id, fill the corresponding portion of the
 *           RFC phase 2 table entry action data
 * @param[in] running_id    running index of the entry (for debugging)
 * @param[in] actiondata    pointer to the action data
 * @param[in] entry_num     entry idx (0 to 50, inclusive), we can fit 51
 *                          entries, each 10 bits wide
 * @param[in] cid           RFC class id
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_p2_table_entry_pack (uint32_t running_id, void *actiondata,
                         uint32_t entry_num, uint16_t cid)
{
    rfc_p2_actiondata_t    *action_data;

    PDS_TRACE_DEBUG("running id %u, entry %u, class id %u",
                    running_id, entry_num, cid);
    action_data = (rfc_p2_actiondata_t *)actiondata;
    switch (entry_num) {
        case 0:
            action_data->action_u.rfc_p2_rfc_action_p2.id00 = cid;
            break;
        case 1:
            action_data->action_u.rfc_p2_rfc_action_p2.id01 = cid;
            break;
        case 2:
            action_data->action_u.rfc_p2_rfc_action_p2.id02 = cid;
            break;
        case 3:
            action_data->action_u.rfc_p2_rfc_action_p2.id03 = cid;
            break;
        case 4:
            action_data->action_u.rfc_p2_rfc_action_p2.id04 = cid;
            break;
        case 5:
            action_data->action_u.rfc_p2_rfc_action_p2.id05 = cid;
            break;
        case 6:
            action_data->action_u.rfc_p2_rfc_action_p2.id06 = cid;
            break;
        case 7:
            action_data->action_u.rfc_p2_rfc_action_p2.id07 = cid;
            break;
        case 8:
            action_data->action_u.rfc_p2_rfc_action_p2.id08 = cid;
            break;
        case 9:
            action_data->action_u.rfc_p2_rfc_action_p2.id09 = cid;
            break;
        case 10:
            action_data->action_u.rfc_p2_rfc_action_p2.id10 = cid;
            break;
        case 11:
            action_data->action_u.rfc_p2_rfc_action_p2.id11 = cid;
            break;
        case 12:
            action_data->action_u.rfc_p2_rfc_action_p2.id12 = cid;
            break;
        case 13:
            action_data->action_u.rfc_p2_rfc_action_p2.id13 = cid;
            break;
        case 14:
            action_data->action_u.rfc_p2_rfc_action_p2.id14 = cid;
            break;
        case 15:
            action_data->action_u.rfc_p2_rfc_action_p2.id15 = cid;
            break;
        case 16:
            action_data->action_u.rfc_p2_rfc_action_p2.id16 = cid;
            break;
        case 17:
            action_data->action_u.rfc_p2_rfc_action_p2.id17 = cid;
            break;
        case 18:
            action_data->action_u.rfc_p2_rfc_action_p2.id18 = cid;
            break;
        case 19:
            action_data->action_u.rfc_p2_rfc_action_p2.id19 = cid;
            break;
        case 20:
            action_data->action_u.rfc_p2_rfc_action_p2.id20 = cid;
            break;
        case 21:
            action_data->action_u.rfc_p2_rfc_action_p2.id21 = cid;
            break;
        case 22:
            action_data->action_u.rfc_p2_rfc_action_p2.id22 = cid;
            break;
        case 23:
            action_data->action_u.rfc_p2_rfc_action_p2.id23 = cid;
            break;
        case 24:
            action_data->action_u.rfc_p2_rfc_action_p2.id24 = cid;
            break;
        case 25:
            action_data->action_u.rfc_p2_rfc_action_p2.id25 = cid;
            break;
        case 26:
            action_data->action_u.rfc_p2_rfc_action_p2.id26 = cid;
            break;
        case 27:
            action_data->action_u.rfc_p2_rfc_action_p2.id27 = cid;
            break;
        case 28:
            action_data->action_u.rfc_p2_rfc_action_p2.id28 = cid;
            break;
        case 29:
            action_data->action_u.rfc_p2_rfc_action_p2.id29 = cid;
            break;
        case 30:
            action_data->action_u.rfc_p2_rfc_action_p2.id30 = cid;
            break;
        case 31:
            action_data->action_u.rfc_p2_rfc_action_p2.id31 = cid;
            break;
        case 32:
            action_data->action_u.rfc_p2_rfc_action_p2.id32 = cid;
            break;
        case 33:
            action_data->action_u.rfc_p2_rfc_action_p2.id33 = cid;
            break;
        case 34:
            action_data->action_u.rfc_p2_rfc_action_p2.id34 = cid;
            break;
        case 35:
            action_data->action_u.rfc_p2_rfc_action_p2.id35 = cid;
            break;
        case 36:
            action_data->action_u.rfc_p2_rfc_action_p2.id36 = cid;
            break;
        case 37:
            action_data->action_u.rfc_p2_rfc_action_p2.id37 = cid;
            break;
        case 38:
            action_data->action_u.rfc_p2_rfc_action_p2.id38 = cid;
            break;
        case 39:
            action_data->action_u.rfc_p2_rfc_action_p2.id39 = cid;
            break;
        case 40:
            action_data->action_u.rfc_p2_rfc_action_p2.id40 = cid;
            break;
        case 41:
            action_data->action_u.rfc_p2_rfc_action_p2.id41 = cid;
            break;
        case 42:
            action_data->action_u.rfc_p2_rfc_action_p2.id42 = cid;
            break;
        case 43:
            action_data->action_u.rfc_p2_rfc_action_p2.id43 = cid;
            break;
        case 44:
            action_data->action_u.rfc_p2_rfc_action_p2.id44 = cid;
            break;
        case 45:
            action_data->action_u.rfc_p2_rfc_action_p2.id45 = cid;
            break;
        case 46:
            action_data->action_u.rfc_p2_rfc_action_p2.id46 = cid;
            break;
        case 47:
            action_data->action_u.rfc_p2_rfc_action_p2.id47 = cid;
            break;
        case 48:
            action_data->action_u.rfc_p2_rfc_action_p2.id48 = cid;
            break;
        case 49:
            action_data->action_u.rfc_p2_rfc_action_p2.id49 = cid;
            break;
        case 50:
            action_data->action_u.rfc_p2_rfc_action_p2.id50 = cid;
            break;
        case 51:
            action_data->action_u.rfc_p2_rfc_action_p2.id51 = cid;
            break;
        case 52:
            action_data->action_u.rfc_p2_rfc_action_p2.id52 = cid;
            break;
        case 53:
            action_data->action_u.rfc_p2_rfc_action_p2.id53 = cid;
            break;
        case 54:
            action_data->action_u.rfc_p2_rfc_action_p2.id54 = cid;
            break;
        case 55:
            action_data->action_u.rfc_p2_rfc_action_p2.id55 = cid;
            break;
        case 56:
            action_data->action_u.rfc_p2_rfc_action_p2.id56 = cid;
            break;
        case 57:
            action_data->action_u.rfc_p2_rfc_action_p2.id57 = cid;
            break;
        case 58:
            action_data->action_u.rfc_p2_rfc_action_p2.id58 = cid;
            break;
        case 59:
            action_data->action_u.rfc_p2_rfc_action_p2.id59 = cid;
            break;
        case 60:
            action_data->action_u.rfc_p2_rfc_action_p2.id60 = cid;
            break;
        case 61:
            action_data->action_u.rfc_p2_rfc_action_p2.id61 = cid;
            break;
        case 62:
            action_data->action_u.rfc_p2_rfc_action_p2.id62 = cid;
            break;
        case 63:
            action_data->action_u.rfc_p2_rfc_action_p2.id63 = cid;
            break;
        default:
        PDS_TRACE_ERR("Invalid entry number %u while packing RFC P2 table",
                      entry_num);
            break;
    }

    return SDK_RET_OK;
}

/**
 * @brief    write the current contents of RFC P1 action data buffer to memory
 * @param[in] addr        address to write the action data to
 * @param[in] actiondata    action data buffer
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_p1_action_data_flush (mem_addr_t addr, void *actiondata)
{
    sdk_ret_t                        ret;
    rfc_p1_actiondata_t    *action_data;

    PDS_TRACE_DEBUG("Flushing action data to 0x%llx", addr);
    action_data = (rfc_p1_actiondata_t *)actiondata;
    ret = impl_base::pipeline_impl()->write_to_txdma_table(addr,
                                   P4_P4PLUS_TXDMA_TBL_ID_RFC_P1,
                                   RFC_P1_RFC_ACTION_P1_ID,
                                   action_data);
    // reset the action data after flushing it
    memset(action_data, 0, sizeof(*action_data));
    return ret;
}

/**
 * @brief    write the current contents of RFC P2 action data buffer to memory
 * @param[in] addr        address to write the action data to
 * @param[in] actiondata    action data buffer
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_p2_action_data_flush (mem_addr_t addr, void *actiondata)
{
    sdk_ret_t                        ret;
    rfc_p2_actiondata_t    *action_data;

    PDS_TRACE_DEBUG("Flushing action data to 0x%llx", addr);
    action_data = (rfc_p2_actiondata_t *)actiondata;
    ret = impl_base::pipeline_impl()->write_to_txdma_table(addr,
              P4_P4PLUS_TXDMA_TBL_ID_RFC_P2,
              RFC_P2_RFC_ACTION_P2_ID,
              action_data);
    // reset the action data after flushing it
    memset(action_data, 0, sizeof(*action_data));
    return ret;
}

/**
 * @brief    dump the contents of P1 equivalence table(s)
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 */
static inline void
rfc_p1_eq_class_tables_dump (rfc_ctxt_t *rfc_ctxt)
{
    rfc_table_t    *rfc_table = &rfc_ctxt->p1_table;

    PDS_TRACE_DEBUG("RFC P1 equivalence class table dump : ");
    rfc_eq_class_table_dump(rfc_table);
}

/**
 * @brief    dump the contents of P1 equivalence table(s)
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 */
static inline void
rfc_p2_eq_class_tables_dump (rfc_ctxt_t *rfc_ctxt)
{
    rfc_table_t    *rfc_table = &rfc_ctxt->p2_table;

    PDS_TRACE_DEBUG("RFC P2 equivalence class table dump : ");
    rfc_eq_class_table_dump(rfc_table);
}

/**
 * @brief    compute the cache line address where the given class block
 *           starts from and the entry index where the 1st entry should
 *           be written to
 * @param[in] base_address            RFC phase 1 table's base address
 * @param[in] class_block_number      number of the class block
 * @param[in] num_classes_per_block   number of classes per block
 * @param[out] next_cl_addr           next cache line address computed
 * @param[out] next_entry_num         next entry number computed
 */
static inline void
rfc_compute_p1_next_cl_addr_entry_num (mem_addr_t base_address,
                                       uint32_t class_block_number,
                                       uint32_t num_classes_per_block,
                                       mem_addr_t *next_cl_addr,
                                       uint16_t *next_entry_num)
{
    uint32_t    num_classes, num_cache_lines;

    // TODO: @cruzj, do we really need ceil() here ?
    num_classes = class_block_number * num_classes_per_block;
    num_cache_lines = num_classes/SACL_P1_ENTRIES_PER_CACHE_LINE;
    *next_cl_addr = base_address + (num_cache_lines << CACHE_LINE_SIZE_SHIFT);
    *next_entry_num = num_classes%SACL_P1_ENTRIES_PER_CACHE_LINE;
}

/**
 * @brief    compute the cache line address where the given class block
 *           starts from and the entry index where the 1st entry should
 *           be written to
 * @param[in] base_address            RFC phase 2 table's base address
 * @param[in] class_block_number      number of the class block
 * @param[in] num_classes_per_block   number of classes per block
 * @param[out] next_cl_addr           next cache line address computed
 * @param[out] next_entry_num         next entry number computed
 */
static inline void
rfc_compute_p2_next_cl_addr_entry_num (mem_addr_t base_address,
                                       uint32_t class_block_number,
                                       uint32_t num_classes_per_block,
                                       mem_addr_t *next_cl_addr,
                                       uint16_t *next_entry_num)
{
    uint32_t    num_classes, num_cache_lines;

    // TODO: @cruzj, do we really need ceil() here ?
    num_classes = class_block_number * num_classes_per_block;
    num_cache_lines = num_classes/SACL_P2_ENTRIES_PER_CACHE_LINE;
    *next_cl_addr = base_address + (num_cache_lines << CACHE_LINE_SIZE_SHIFT);
    *next_entry_num = num_classes%SACL_P2_ENTRIES_PER_CACHE_LINE;
}

/**
 * @brief    given 10 bits of priority, one bit of drop/allow, and the
 *           entry id, fill corresponding portion of the RFC phase 3
 *           table entry action data.
 * @param[in] running_id     running index of the entry (for debugging)
 * @param[in] action_data    pointer to the action data
 * @param[in] entry_num      entry idx (0 to 46, inclusive), we can fit 46
 *                           entries, each 11 bits wide
 * @param[in] prio           priority
 * @param[in] result         drop/allow
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_p3_table_entry_pack (uint32_t running_id, void *actiondata,
                         uint32_t entry_num, uint16_t result)
{
    rfc_p3_actiondata_t    *action_data;

    action_data = (rfc_p3_actiondata_t *)actiondata;
    result &= (uint16_t)0x7FF; // Only 11 bits are valid
    PDS_TRACE_DEBUG("running id %u, entry %u, result 0x%x",
                    running_id, entry_num, result);
    switch (entry_num) {
    case 0:
        action_data->action_u.rfc_p3_rfc_action_p3.pr00 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res00 = result&0x01;
        break;
    case 1:
        action_data->action_u.rfc_p3_rfc_action_p3.pr01 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res01 = result&0x01;
        break;
    case 2:
        action_data->action_u.rfc_p3_rfc_action_p3.pr02 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res02 = result&0x01;
        break;
    case 3:
        action_data->action_u.rfc_p3_rfc_action_p3.pr03 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res03 = result&0x01;
        break;
    case 4:
        action_data->action_u.rfc_p3_rfc_action_p3.pr04 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res04 = result&0x01;
        break;
    case 5:
        action_data->action_u.rfc_p3_rfc_action_p3.pr05 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res05 = result&0x01;
        break;
    case 6:
        action_data->action_u.rfc_p3_rfc_action_p3.pr06 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res06 = result&0x01;
        break;
    case 7:
        action_data->action_u.rfc_p3_rfc_action_p3.pr07 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res07 = result&0x01;
        break;
    case 8:
        action_data->action_u.rfc_p3_rfc_action_p3.pr08 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res08 = result&0x01;
        break;
    case 9:
        action_data->action_u.rfc_p3_rfc_action_p3.pr09 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res09 = result&0x01;
        break;
    case 10:
        action_data->action_u.rfc_p3_rfc_action_p3.pr10 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res10 = result&0x01;
        break;
    case 11:
        action_data->action_u.rfc_p3_rfc_action_p3.pr11 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res11 = result&0x01;
        break;
    case 12:
        action_data->action_u.rfc_p3_rfc_action_p3.pr12 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res12 = result&0x01;
        break;
    case 13:
        action_data->action_u.rfc_p3_rfc_action_p3.pr13 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res13 = result&0x01;
        break;
    case 14:
        action_data->action_u.rfc_p3_rfc_action_p3.pr14 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res14 = result&0x01;
        break;
    case 15:
        action_data->action_u.rfc_p3_rfc_action_p3.pr15 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res15 = result&0x01;
        break;
    case 16:
        action_data->action_u.rfc_p3_rfc_action_p3.pr16 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res16 = result&0x01;
        break;
    case 17:
        action_data->action_u.rfc_p3_rfc_action_p3.pr17 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res17 = result&0x01;
        break;
    case 18:
        action_data->action_u.rfc_p3_rfc_action_p3.pr18 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res18 = result&0x01;
        break;
    case 19:
        action_data->action_u.rfc_p3_rfc_action_p3.pr19 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res19 = result&0x01;
        break;
    case 20:
        action_data->action_u.rfc_p3_rfc_action_p3.pr20 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res20 = result&0x01;
        break;
    case 21:
        action_data->action_u.rfc_p3_rfc_action_p3.pr21 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res21 = result&0x01;
        break;
    case 22:
        action_data->action_u.rfc_p3_rfc_action_p3.pr22 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res22 = result&0x01;
        break;
    case 23:
        action_data->action_u.rfc_p3_rfc_action_p3.pr23 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res23 = result&0x01;
        break;
    case 24:
        action_data->action_u.rfc_p3_rfc_action_p3.pr24 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res24 = result&0x01;
        break;
    case 25:
        action_data->action_u.rfc_p3_rfc_action_p3.pr25 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res25 = result&0x01;
        break;
    case 26:
        action_data->action_u.rfc_p3_rfc_action_p3.pr26 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res26 = result&0x01;
        break;
    case 27:
        action_data->action_u.rfc_p3_rfc_action_p3.pr27 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res27 = result&0x01;
        break;
    case 28:
        action_data->action_u.rfc_p3_rfc_action_p3.pr28 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res28 = result&0x01;
        break;
    case 29:
        action_data->action_u.rfc_p3_rfc_action_p3.pr29 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res29 = result&0x01;
        break;
    case 30:
        action_data->action_u.rfc_p3_rfc_action_p3.pr30 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res30 = result&0x01;
        break;
    case 31:
        action_data->action_u.rfc_p3_rfc_action_p3.pr31 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res31 = result&0x01;
        break;
    case 32:
        action_data->action_u.rfc_p3_rfc_action_p3.pr32 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res32 = result&0x01;
        break;
    case 33:
        action_data->action_u.rfc_p3_rfc_action_p3.pr33 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res33 = result&0x01;
        break;
    case 34:
        action_data->action_u.rfc_p3_rfc_action_p3.pr34 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res34 = result&0x01;
        break;
    case 35:
        action_data->action_u.rfc_p3_rfc_action_p3.pr35 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res35 = result&0x01;
        break;
    case 36:
        action_data->action_u.rfc_p3_rfc_action_p3.pr36 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res36 = result&0x01;
        break;
    case 37:
        action_data->action_u.rfc_p3_rfc_action_p3.pr37 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res37 = result&0x01;
        break;
    case 38:
        action_data->action_u.rfc_p3_rfc_action_p3.pr38 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res38 = result&0x01;
        break;
    case 39:
        action_data->action_u.rfc_p3_rfc_action_p3.pr39 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res39 = result&0x01;
        break;
    case 40:
        action_data->action_u.rfc_p3_rfc_action_p3.pr40 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res40 = result&0x01;
        break;
    case 41:
        action_data->action_u.rfc_p3_rfc_action_p3.pr41 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res41 = result&0x01;
        break;
    case 42:
        action_data->action_u.rfc_p3_rfc_action_p3.pr42 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res42 = result&0x01;
        break;
    case 43:
        action_data->action_u.rfc_p3_rfc_action_p3.pr43 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res43 = result&0x01;
        break;
    case 44:
        action_data->action_u.rfc_p3_rfc_action_p3.pr44 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res44 = result&0x01;
        break;
    case 45:
        action_data->action_u.rfc_p3_rfc_action_p3.pr45 = result>>1;
        action_data->action_u.rfc_p3_rfc_action_p3.res45 = result&0x01;
        break;
    default:
        break;
    }

    return SDK_RET_OK;
}

/**
 * @brief    write the current contents of RFC P2 action data buffer to memory
 * @param[in] addr        address to write the action data to
 * @param[in] actiondata action data buffer
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_p3_action_data_flush (mem_addr_t addr, void *actiondata)
{
    sdk_ret_t               ret;
    rfc_p3_actiondata_t    *action_data;

    PDS_TRACE_DEBUG("Flushing action data to 0x%llx", addr);
    action_data = (rfc_p3_actiondata_t *)actiondata;
    ret = impl_base::pipeline_impl()->write_to_txdma_table(addr,
              P4_P4PLUS_TXDMA_TBL_ID_RFC_P3,
              RFC_P3_RFC_ACTION_P3_ID, action_data);
    // reset the action data after flushing it
    memset(action_data, 0, sizeof(*action_data));
    return ret;
}

/**
 * @brief    compute the cache line address where the given class block
 *           starts from and the entry index where the 1st entry should
 *           be written to
 * @param[in] base_address            RFC phase 3 table's base address
 * @param[in] class_block_number      number of the class block
 * @param[in] num_classes_per_block   number of classes per block
 * @param[out] next_cl_addr           next cache line address computed
 * @param[out] next_entry_num         next entry number computed
 */
static inline void
rfc_compute_p3_next_cl_addr_entry_num (mem_addr_t base_address,
                                       uint32_t class_block_number,
                                       uint32_t num_classes_per_block,
                                       mem_addr_t *next_cl_addr,
                                       uint16_t *next_entry_num)
{
    uint32_t    num_classes, num_cache_lines;

    num_classes = class_block_number * num_classes_per_block;
    num_cache_lines = num_classes/SACL_P3_ENTRIES_PER_CACHE_LINE;
    *next_cl_addr = base_address + (num_cache_lines << CACHE_LINE_SIZE_SHIFT);
    *next_entry_num = num_classes%SACL_P3_ENTRIES_PER_CACHE_LINE;
}

#define fw_act    action_data.fw_action.action
/**
 * @brief    given two equivalence class tables, compute the new equivalence
 *           class table or the result table by doing cross product of class
 *           bitmaps of the two tables
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @param[in] rfc_table   result RFC table
 * @param[in] cbm         class bitmap from which is result is computed from
 * @param[in] cbm_size    class bitmap size
 * @return    result bits of P2 table corresponding to the class bitmap provided
 */
uint16_t
rfc_compute_p3_result (rfc_ctxt_t *rfc_ctxt, rfc_table_t *rfc_table,
                       rte_bitmap *cbm, uint32_t cbm_size, void *ctxt)
{
    int         rv;
    uint16_t    result = 0, priority = SACL_PRIORITY_INVALID;
    uint32_t    ruleidx, posn, start_posn = 0, new_posn = 0;
    uint64_t    slab = 0;

    // TODO: remove
    std::stringstream    a1ss, a2ss;
    rte_bitmap2str(cbm, a1ss, a2ss);
    PDS_TRACE_DEBUG("a1ss %s\nbitmap %s",
                    a1ss.str().c_str(), a2ss.str().c_str());

    // remember the start position of the scan
    rv = rte_bitmap_scan(cbm, &start_posn, &slab);
    if (rv == 0) {
        // no bit is set in the bitmap
        PDS_TRACE_DEBUG("No bits set in bitmap, setting lowest priority");
        RFC_RESULT_SET_PRIORITY_BITS(result, SACL_PRIORITY_LOWEST);
        if (rfc_ctxt->policy->default_action.fw_action.action ==
                SECURITY_RULE_ACTION_ALLOW) {
            RFC_RESULT_SET_ACTION_BIT(result, RFC_RESULT_RULE_ACTION_ALLOW);
        } else {
            RFC_RESULT_SET_ACTION_BIT(result, RFC_RESULT_RULE_ACTION_DENY);
        }
    } else {
        do {
            posn = RTE_BITMAP_START_SLAB_SCAN_POS;
            while (rte_bitmap_slab_scan(slab, posn, &new_posn) != 0) {
                ruleidx =
                    rte_bitmap_get_global_bit_pos(cbm->index2-1, new_posn);
                if (priority > rfc_ctxt->policy->rules[ruleidx].attrs.priority) {
                    priority = rfc_ctxt->policy->rules[ruleidx].attrs.priority;
                    PDS_TRACE_DEBUG("Picked high priority rule %u", ruleidx);
                    result = 0;
                    if (rfc_ctxt->policy->rules[ruleidx].attrs.fw_act ==
                            SECURITY_RULE_ACTION_ALLOW) {
                        RFC_RESULT_SET_ACTION_BIT(result,
                                                  RFC_RESULT_RULE_ACTION_ALLOW);
                    } else {
                        RFC_RESULT_SET_ACTION_BIT(result,
                                                  RFC_RESULT_RULE_ACTION_DENY);
                    }
                    RFC_RESULT_SET_PRIORITY_BITS(result, priority);
                } else {
                    PDS_TRACE_VERBOSE("rule %u priority %u < current %u"
                        ", skipping", ruleidx,
                        rfc_ctxt->policy->rules[ruleidx].attrs.priority,
                        priority);
                }
                posn = new_posn;
            }
            rte_bitmap_scan(cbm, &posn, &slab);
        } while (posn != start_posn);
    }
    return result;
}

/**
 * @brief    given two equivalence class tables, compute the new equivalence
 *           class table or the result table by doing cross product of class
 *           bitmaps of the two tables
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @param[in] rfc_table1                RFC equivalance class table 1
 * @param[in] rfc_table2                RFC equivalence class table 2
 * @param[in] result_table              pointer to the result table
 * @param[in] result_table_base_addr    base address of the result table
 * @param[in] action_data               pointer to action data where entries are
 *                                      computed and packed
 * @param[in] entries_per_cl            number of entries that can fit in each
 *                                      cache line
 * @param[in] cl_addr_entry_cb          callback function that computes next
 *                                      cacheline address and entry number in
 *                                      the cache line to write to
 * @param[in] compute_entry_val_cb      callback function to compute the entry
 *                                      value
 * @param[in] entry_pack_cb             callback function to pack each entry
 *                                      value with in the action data at the
 *                                      given entry number
 * @param[in] action_data_flush_cb      callback function to flush the action
 *                                      data contents
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_compute_eq_class_tables (rfc_ctxt_t *rfc_ctxt, rfc_table_t *rfc_table1,
                             rfc_table_t *rfc_table2, rfc_table_t *result_table,
                             mem_addr_t result_table_base_addr,
                             void *action_data, uint32_t entries_per_cl,
                             rfc_compute_cl_addr_entry_cb_t cl_addr_entry_cb,
                             rfc_compute_entry_val_cb_t compute_entry_val_cb,
                             rfc_table_entry_pack_cb_t entry_pack_cb,
                             rfc_action_data_flush_cb_t action_data_flush_cb)
{
    uint16_t      entry_val, entry_num = 0, next_entry_num;
    uint32_t      running_id = 0;
    mem_addr_t    cl_addr, next_cl_addr;

    // do cross product of bitmaps, compute the entries (e.g., class ids or
    // final table result etc.), pack them into appropriate cache lines and
    // flush them
    for (uint32_t i = 0; i < rfc_table1->num_classes; i++) {
        cl_addr_entry_cb(result_table_base_addr, i, rfc_table2->max_classes,
                         &next_cl_addr, &next_entry_num);
        if (entry_num) {
            if (cl_addr != next_cl_addr) {
                // flush the current partially filled cache line
                action_data_flush_cb(cl_addr, action_data);
                cl_addr = next_cl_addr;
                entry_num = next_entry_num;
            } else {
                // cache line didn't change, so no need to flush the cache line
                // but we need to set the entry_num to right value to start the
                // next class block with
                entry_num = next_entry_num;
            }
        } else {
            // we filled earlier cache line, need to start on new cache
            // line and entry number computed
            cl_addr = next_cl_addr;
            entry_num = next_entry_num;
        }

        for (uint32_t j = 0; j < rfc_table2->num_classes; j++) {
            rte_bitmap_reset(rfc_ctxt->cbm);
            rte_bitmap_and(rfc_table1->cbm_table[i].cbm,
                           rfc_table2->cbm_table[j].cbm,
                           rfc_ctxt->cbm);
            entry_val = compute_entry_val_cb(rfc_ctxt, result_table,
                                             rfc_ctxt->cbm,
                                             rfc_ctxt->cbm_size, NULL);
            entry_pack_cb(running_id, action_data, entry_num, entry_val);
            running_id++;
            entry_num++;
            if (entry_num == entries_per_cl) {
                // write this full entry to the table
                action_data_flush_cb(cl_addr, action_data);
                entry_num = 0;
                cl_addr += CACHE_LINE_SIZE;
            }
        }
    }
    if (entry_num) {
        // flush the partially filled last cache line as well
        action_data_flush_cb(cl_addr, action_data);
    }
    return SDK_RET_OK;
}

/**
 * @brief    given the class bitmap tables of phase0, compute class
 *           bitmap tables of RFC phase 1
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @param[in] table1      Phase0 bitmap table 1
 * @param[in] table2      Phase0 bitmap table 2
 * @param[in] addr_offset Offset of the RFC table wrt base addr
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_compute_p1_tables (rfc_ctxt_t *rfc_ctxt,
                       rfc_table_t *table1,
                       rfc_table_t *table2,
                       uint64_t addr_offset)
{
    rfc_p1_actiondata_t action_data = { 0 };

    rfc_ctxt->p1_table.num_classes = 0;
    rfc_compute_eq_class_tables(rfc_ctxt,
                                table1,
                                table2,
                                &rfc_ctxt->p1_table,
                                rfc_ctxt->base_addr + addr_offset,
                                &action_data, SACL_P1_ENTRIES_PER_CACHE_LINE,
                                rfc_compute_p1_next_cl_addr_entry_num,
                                rfc_compute_class_id_cb,
                                rfc_p1_table_entry_pack,
                                rfc_p1_action_data_flush);
    return SDK_RET_OK;
}

/**
 * @brief    given the class bitmap tables of phase0, compute class
 *           bitmap table(s) of RFC phase 2, and set the results bits
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @param[in] table3      Phase0 bitmap table 3
 * @param[in] table4      Phase0 bitmap table 4
 * @param[in] addr_offset Offset of the RFC table wrt base addr
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_compute_p2_tables (rfc_ctxt_t *rfc_ctxt,
                       rfc_table_t *table3,
                       rfc_table_t *table4,
                       uint64_t addr_offset)
{
    rfc_p2_actiondata_t     action_data = { 0 };

    rfc_ctxt->p2_table.num_classes = 0;
    rfc_compute_eq_class_tables(rfc_ctxt,
                                table3,
                                table4,
                                &rfc_ctxt->p2_table,
                                rfc_ctxt->base_addr + addr_offset,
                                &action_data, SACL_P2_ENTRIES_PER_CACHE_LINE,
                                rfc_compute_p2_next_cl_addr_entry_num,
                                rfc_compute_class_id_cb,
                                rfc_p2_table_entry_pack,
                                rfc_p2_action_data_flush);
    return SDK_RET_OK;
}

/**
 * @brief    given the class bitmap tables of phase1 and Phase2 compute class
 *           bitmap table(s) of RFC phase 3, and set the results bits
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @param[in] addr_offset Offset of the RFC table wrt base addr
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_compute_p3_tables (rfc_ctxt_t *rfc_ctxt, uint64_t addr_offset)
{
    rfc_p3_actiondata_t     action_data = { 0 };

    rfc_compute_eq_class_tables(rfc_ctxt,
                                &rfc_ctxt->p1_table,
                                &rfc_ctxt->p2_table,
                                NULL,
                                rfc_ctxt->base_addr + addr_offset,
                                &action_data, SACL_P3_ENTRIES_PER_CACHE_LINE,
                                rfc_compute_p3_next_cl_addr_entry_num,
                                rfc_compute_p3_result, rfc_p3_table_entry_pack,
                                rfc_p3_action_data_flush);
    return SDK_RET_OK;
}

/**
 * @brief    given the class bitmap tables of phase0 & phase1, compute class
 *           bitmap table(s) of RFC phase 2, and set the results bits
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
rfc_build_eqtables (rfc_ctxt_t *rfc_ctxt)
{
    // combination 1
    PDS_TRACE_DEBUG("Starting RFC combination 1");
    rfc_compute_p1_tables(rfc_ctxt,
                          &rfc_ctxt->sip_tree.rfc_table,
                          &rfc_ctxt->port_tree.rfc_table,
                          SACL_P1_1_TABLE_OFFSET);
    rfc_p1_eq_class_tables_dump(rfc_ctxt);
    rfc_compute_p2_tables(rfc_ctxt,
                          &rfc_ctxt->dip_tree.rfc_table,
                          &rfc_ctxt->proto_port_tree.rfc_table,
                          SACL_P2_1_TABLE_OFFSET);
    rfc_p2_eq_class_tables_dump(rfc_ctxt);
    rfc_compute_p3_tables(rfc_ctxt, SACL_P3_1_TABLE_OFFSET);
    rfc_table_destroy(&rfc_ctxt->p1_table);
    rfc_table_destroy(&rfc_ctxt->p2_table);

    // combination 2
    PDS_TRACE_DEBUG("Starting RFC combination 2");
    rfc_compute_p1_tables(rfc_ctxt,
                          &rfc_ctxt->stag_tree.rfc_table,
                          &rfc_ctxt->dip_tree.rfc_table,
                          SACL_P1_2_TABLE_OFFSET);
    rfc_p1_eq_class_tables_dump(rfc_ctxt);
    rfc_compute_p2_tables(rfc_ctxt,
                          &rfc_ctxt->port_tree.rfc_table,
                          &rfc_ctxt->proto_port_tree.rfc_table,
                          SACL_P2_2_TABLE_OFFSET);
    rfc_p2_eq_class_tables_dump(rfc_ctxt);
    rfc_compute_p3_tables(rfc_ctxt, SACL_P3_2_TABLE_OFFSET);
    rfc_table_destroy(&rfc_ctxt->p1_table);
    rfc_table_destroy(&rfc_ctxt->p2_table);

    // combination 3
    PDS_TRACE_DEBUG("Starting RFC combination 3");
    rfc_compute_p1_tables(rfc_ctxt,
                          &rfc_ctxt->sip_tree.rfc_table,
                          &rfc_ctxt->dtag_tree.rfc_table,
                          SACL_P1_3_TABLE_OFFSET);
    rfc_p1_eq_class_tables_dump(rfc_ctxt);
    rfc_compute_p2_tables(rfc_ctxt,
                          &rfc_ctxt->port_tree.rfc_table,
                          &rfc_ctxt->proto_port_tree.rfc_table,
                          SACL_P2_3_TABLE_OFFSET);
    rfc_p2_eq_class_tables_dump(rfc_ctxt);
    rfc_compute_p3_tables(rfc_ctxt, SACL_P3_3_TABLE_OFFSET);
    rfc_table_destroy(&rfc_ctxt->p1_table);
    rfc_table_destroy(&rfc_ctxt->p2_table);

    // combination 4
    PDS_TRACE_DEBUG("Starting RFC combination 4");
    PDS_TRACE_DEBUG("Computing P1 tables");
    rfc_compute_p1_tables(rfc_ctxt,
                          &rfc_ctxt->stag_tree.rfc_table,
                          &rfc_ctxt->port_tree.rfc_table,
                          SACL_P1_4_TABLE_OFFSET);
    rfc_p1_eq_class_tables_dump(rfc_ctxt);
    PDS_TRACE_DEBUG("Computing P2 tables");
    rfc_compute_p2_tables(rfc_ctxt,
                          &rfc_ctxt->dtag_tree.rfc_table,
                          &rfc_ctxt->proto_port_tree.rfc_table,
                          SACL_P2_4_TABLE_OFFSET);
    rfc_p2_eq_class_tables_dump(rfc_ctxt);
    PDS_TRACE_DEBUG("Computing P3 tables");
    rfc_compute_p3_tables(rfc_ctxt, SACL_P3_4_TABLE_OFFSET);
    rfc_table_destroy(&rfc_ctxt->p1_table);
    rfc_table_destroy(&rfc_ctxt->p2_table);

    return SDK_RET_OK;
}

}    // namespace rfc
