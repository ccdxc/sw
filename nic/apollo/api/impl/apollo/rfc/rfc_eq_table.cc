/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_eq_table.cc
 *
 * @brief   RFC library equivalence table handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/apollo/api/impl/rfc/rfc.hpp"
#include "nic/apollo/api/impl/rfc/rfc_tree.hpp"
#include "nic/apollo/api/impl/rfc/rte_bitmap_utils.hpp"
#include "nic/apollo/api/impl/apollo/rfc/rfc_utils.hpp"
#include "nic/apollo/p4/include/sacl_defines.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"

#define RFC_RESULT_STATELESS_BIT_MASK           0x1
#define RFC_RESULT_STATEFUL_BIT_MASK            0x2
#define RFC_RESULT_BITS_MASK                    0x3
#define RFC_RESULT_SET_STATEFUL_BIT(val)        ((val) |= RFC_RESULT_STATEFUL_BIT_MASK)
#define RFC_RESULT_SET_STATELESS_BIT(val)       ((val) |= RFC_RESULT_STATELESS_BIT_MASK)
#define RFC_RESULT_IS_STATEFUL_BIT_SET(val)     ((val) & RFC_RESULT_STATEFUL_BIT_MASK)
#define RFC_RESULT_IS_STATELESS_BIT_SET(val)    ((val) & RFC_RESULT_STATELESS_BIT_MASK)
#define RFC_RESULT_BOTH_BITS_SET(val)           (((val) & RFC_RESULT_BITS_MASK) == RFC_RESULT_BITS_MASK)

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
                                            rte_bitmap *cbm, uint32_t cbm_size);
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
    sacl_ip_sport_p1_actiondata_t    *action_data;

    PDS_TRACE_DEBUG("running id %u, entry %u, class 0x%x",
                    running_id, entry_num, cid);
    action_data = (sacl_ip_sport_p1_actiondata_t *)actiondata;
    switch (entry_num) {
        case 0:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id00 = cid;
            break;
        case 1:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id01 = cid;
            break;
        case 2:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id02 = cid;
            break;
        case 3:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id03 = cid;
            break;
        case 4:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id04 = cid;
            break;
        case 5:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id05 = cid;
            break;
        case 6:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id06 = cid;
            break;
        case 7:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id07 = cid;
            break;
        case 8:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id08 = cid;
            break;
        case 9:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id09 = cid;
            break;
        case 10:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id10 = cid;
            break;
        case 11:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id11 = cid;
            break;
        case 12:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id12 = cid;
            break;
        case 13:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id13 = cid;
            break;
        case 14:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id14 = cid;
            break;
        case 15:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id15 = cid;
            break;
        case 16:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id16 = cid;
            break;
        case 17:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id17 = cid;
            break;
        case 18:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id18 = cid;
            break;
        case 19:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id19 = cid;
            break;
        case 20:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id20 = cid;
            break;
        case 21:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id21 = cid;
            break;
        case 22:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id22 = cid;
            break;
        case 23:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id23 = cid;
            break;
        case 24:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id24 = cid;
            break;
        case 25:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id25 = cid;
            break;
        case 26:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id26 = cid;
            break;
        case 27:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id27 = cid;
            break;
        case 28:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id28 = cid;
            break;
        case 29:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id29 = cid;
            break;
        case 30:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id30 = cid;
            break;
        case 31:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id31 = cid;
            break;
        case 32:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id32 = cid;
            break;
        case 33:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id33 = cid;
            break;
        case 34:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id34 = cid;
            break;
        case 35:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id35 = cid;
            break;
        case 36:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id36 = cid;
            break;
        case 37:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id37 = cid;
            break;
        case 38:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id38 = cid;
            break;
        case 39:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id39 = cid;
            break;
        case 40:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id40 = cid;
            break;
        case 41:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id41 = cid;
            break;
        case 42:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id42 = cid;
            break;
        case 43:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id43 = cid;
            break;
        case 44:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id44 = cid;
            break;
        case 45:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id45 = cid;
            break;
        case 46:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id46 = cid;
            break;
        case 47:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id47 = cid;
            break;
        case 48:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id48 = cid;
            break;
        case 49:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id49 = cid;
            break;
        case 50:
            action_data->action_u.sacl_ip_sport_p1_sacl_ip_sport_p1.id50 = cid;
            break;
        default:
        PDS_TRACE_ERR("Invalid entry number %u while packing RFC P1 table",
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
    sacl_ip_sport_p1_actiondata_t    *action_data;

    PDS_TRACE_DEBUG("Flushing action data to 0x%llx", addr);
    action_data = (sacl_ip_sport_p1_actiondata_t *)actiondata;
    ret = impl_base::pipeline_impl()->write_to_rxdma_table(addr,
                                                           P4_P4PLUS_RXDMA_TBL_ID_SACL_IP_SPORT_P1,
                                                           SACL_IP_SPORT_P1_SACL_IP_SPORT_P1_ID,
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
 * @brief    given two bits of data (stateful rule hit, stateless rule hit) and
 *           the entry id, fill corresponding portion of the RFC phase 2 table
 *           entry action data
 * @param[in] running_id     running index of the entry (for debugging)
 * @param[in] action_data    pointer to the action data
 * @param[in] entry_num      entry idx (0 to 50, inclusive), we can fit 51
 *                           entries, each 10 bits wide
 * @param[in] data           data to be packed
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_p2_table_entry_pack (uint32_t running_id, void *actiondata,
                         uint32_t entry_num, uint16_t data)
{
    sacl_p2_actiondata_t    *action_data;

    action_data = (sacl_p2_actiondata_t *)actiondata;
    data &= 0x3;    // only last 2 bits of the result are relevant
    PDS_TRACE_DEBUG("running id %u, result 0x%x", running_id, data);
    switch (entry_num) {
        case 0:
            action_data->action_u.sacl_p2_sacl_p2.id000 = data;
            break;
        case 1:
            action_data->action_u.sacl_p2_sacl_p2.id001 = data;
            break;
        case 2:
            action_data->action_u.sacl_p2_sacl_p2.id002 = data;
            break;
        case 3:
            action_data->action_u.sacl_p2_sacl_p2.id003 = data;
            break;
        case 4:
            action_data->action_u.sacl_p2_sacl_p2.id004 = data;
            break;
        case 5:
            action_data->action_u.sacl_p2_sacl_p2.id005 = data;
            break;
        case 6:
            action_data->action_u.sacl_p2_sacl_p2.id006 = data;
            break;
        case 7:
            action_data->action_u.sacl_p2_sacl_p2.id007 = data;
            break;
        case 8:
            action_data->action_u.sacl_p2_sacl_p2.id008 = data;
            break;
        case 9:
            action_data->action_u.sacl_p2_sacl_p2.id009 = data;
            break;
        case 10:
            action_data->action_u.sacl_p2_sacl_p2.id010 = data;
            break;
        case 11:
            action_data->action_u.sacl_p2_sacl_p2.id011 = data;
            break;
        case 12:
            action_data->action_u.sacl_p2_sacl_p2.id012 = data;
            break;
        case 13:
            action_data->action_u.sacl_p2_sacl_p2.id013 = data;
            break;
        case 14:
            action_data->action_u.sacl_p2_sacl_p2.id014 = data;
            break;
        case 15:
            action_data->action_u.sacl_p2_sacl_p2.id015 = data;
            break;
        case 16:
            action_data->action_u.sacl_p2_sacl_p2.id016 = data;
            break;
        case 17:
            action_data->action_u.sacl_p2_sacl_p2.id017 = data;
            break;
        case 18:
            action_data->action_u.sacl_p2_sacl_p2.id018 = data;
            break;
        case 19:
            action_data->action_u.sacl_p2_sacl_p2.id019 = data;
            break;
        case 20:
            action_data->action_u.sacl_p2_sacl_p2.id020 = data;
            break;
        case 21:
            action_data->action_u.sacl_p2_sacl_p2.id021 = data;
            break;
        case 22:
            action_data->action_u.sacl_p2_sacl_p2.id022 = data;
            break;
        case 23:
            action_data->action_u.sacl_p2_sacl_p2.id023 = data;
            break;
        case 24:
            action_data->action_u.sacl_p2_sacl_p2.id024 = data;
            break;
        case 25:
            action_data->action_u.sacl_p2_sacl_p2.id025 = data;
            break;
        case 26:
            action_data->action_u.sacl_p2_sacl_p2.id026 = data;
            break;
        case 27:
            action_data->action_u.sacl_p2_sacl_p2.id027 = data;
            break;
        case 28:
            action_data->action_u.sacl_p2_sacl_p2.id028 = data;
            break;
        case 29:
            action_data->action_u.sacl_p2_sacl_p2.id029 = data;
            break;
        case 30:
            action_data->action_u.sacl_p2_sacl_p2.id030 = data;
            break;
        case 31:
            action_data->action_u.sacl_p2_sacl_p2.id031 = data;
            break;
        case 32:
            action_data->action_u.sacl_p2_sacl_p2.id032 = data;
            break;
        case 33:
            action_data->action_u.sacl_p2_sacl_p2.id033 = data;
            break;
        case 34:
            action_data->action_u.sacl_p2_sacl_p2.id034 = data;
            break;
        case 35:
            action_data->action_u.sacl_p2_sacl_p2.id035 = data;
            break;
        case 36:
            action_data->action_u.sacl_p2_sacl_p2.id036 = data;
            break;
        case 37:
            action_data->action_u.sacl_p2_sacl_p2.id037 = data;
            break;
        case 38:
            action_data->action_u.sacl_p2_sacl_p2.id038 = data;
            break;
        case 39:
            action_data->action_u.sacl_p2_sacl_p2.id039 = data;
            break;
        case 40:
            action_data->action_u.sacl_p2_sacl_p2.id040 = data;
            break;
        case 41:
            action_data->action_u.sacl_p2_sacl_p2.id041 = data;
            break;
        case 42:
            action_data->action_u.sacl_p2_sacl_p2.id042 = data;
            break;
        case 43:
            action_data->action_u.sacl_p2_sacl_p2.id043 = data;
            break;
        case 44:
            action_data->action_u.sacl_p2_sacl_p2.id044 = data;
            break;
        case 45:
            action_data->action_u.sacl_p2_sacl_p2.id045 = data;
            break;
        case 46:
            action_data->action_u.sacl_p2_sacl_p2.id046 = data;
            break;
        case 47:
            action_data->action_u.sacl_p2_sacl_p2.id047 = data;
            break;
        case 48:
            action_data->action_u.sacl_p2_sacl_p2.id048 = data;
            break;
        case 49:
            action_data->action_u.sacl_p2_sacl_p2.id049 = data;
            break;
        case 50:
            action_data->action_u.sacl_p2_sacl_p2.id050 = data;
            break;
        case 51:
            action_data->action_u.sacl_p2_sacl_p2.id051 = data;
            break;
        case 52:
            action_data->action_u.sacl_p2_sacl_p2.id052 = data;
            break;
        case 53:
            action_data->action_u.sacl_p2_sacl_p2.id053 = data;
            break;
        case 54:
            action_data->action_u.sacl_p2_sacl_p2.id054 = data;
            break;
        case 55:
            action_data->action_u.sacl_p2_sacl_p2.id055 = data;
            break;
        case 56:
            action_data->action_u.sacl_p2_sacl_p2.id056 = data;
            break;
        case 57:
            action_data->action_u.sacl_p2_sacl_p2.id057 = data;
            break;
        case 58:
            action_data->action_u.sacl_p2_sacl_p2.id058 = data;
            break;
        case 59:
            action_data->action_u.sacl_p2_sacl_p2.id059 = data;
            break;
        case 60:
            action_data->action_u.sacl_p2_sacl_p2.id060 = data;
            break;
        case 61:
            action_data->action_u.sacl_p2_sacl_p2.id061 = data;
            break;
        case 62:
            action_data->action_u.sacl_p2_sacl_p2.id062 = data;
            break;
        case 63:
            action_data->action_u.sacl_p2_sacl_p2.id063 = data;
            break;
        case 64:
            action_data->action_u.sacl_p2_sacl_p2.id064 = data;
            break;
        case 65:
            action_data->action_u.sacl_p2_sacl_p2.id065 = data;
            break;
        case 66:
            action_data->action_u.sacl_p2_sacl_p2.id066 = data;
            break;
        case 67:
            action_data->action_u.sacl_p2_sacl_p2.id067 = data;
            break;
        case 68:
            action_data->action_u.sacl_p2_sacl_p2.id068 = data;
            break;
        case 69:
            action_data->action_u.sacl_p2_sacl_p2.id069 = data;
            break;
        case 70:
            action_data->action_u.sacl_p2_sacl_p2.id070 = data;
            break;
        case 71:
            action_data->action_u.sacl_p2_sacl_p2.id071 = data;
            break;
        case 72:
            action_data->action_u.sacl_p2_sacl_p2.id072 = data;
            break;
        case 73:
            action_data->action_u.sacl_p2_sacl_p2.id073 = data;
            break;
        case 74:
            action_data->action_u.sacl_p2_sacl_p2.id074 = data;
            break;
        case 75:
            action_data->action_u.sacl_p2_sacl_p2.id075 = data;
            break;
        case 76:
            action_data->action_u.sacl_p2_sacl_p2.id076 = data;
            break;
        case 77:
            action_data->action_u.sacl_p2_sacl_p2.id077 = data;
            break;
        case 78:
            action_data->action_u.sacl_p2_sacl_p2.id078 = data;
            break;
        case 79:
            action_data->action_u.sacl_p2_sacl_p2.id079 = data;
            break;
        case 80:
            action_data->action_u.sacl_p2_sacl_p2.id080 = data;
            break;
        case 81:
            action_data->action_u.sacl_p2_sacl_p2.id081 = data;
            break;
        case 82:
            action_data->action_u.sacl_p2_sacl_p2.id082 = data;
            break;
        case 83:
            action_data->action_u.sacl_p2_sacl_p2.id083 = data;
            break;
        case 84:
            action_data->action_u.sacl_p2_sacl_p2.id084 = data;
            break;
        case 85:
            action_data->action_u.sacl_p2_sacl_p2.id085 = data;
            break;
        case 86:
            action_data->action_u.sacl_p2_sacl_p2.id086 = data;
            break;
        case 87:
            action_data->action_u.sacl_p2_sacl_p2.id087 = data;
            break;
        case 88:
            action_data->action_u.sacl_p2_sacl_p2.id088 = data;
            break;
        case 89:
            action_data->action_u.sacl_p2_sacl_p2.id089 = data;
            break;
        case 90:
            action_data->action_u.sacl_p2_sacl_p2.id090 = data;
            break;
        case 91:
            action_data->action_u.sacl_p2_sacl_p2.id091 = data;
            break;
        case 92:
            action_data->action_u.sacl_p2_sacl_p2.id092 = data;
            break;
        case 93:
            action_data->action_u.sacl_p2_sacl_p2.id093 = data;
            break;
        case 94:
            action_data->action_u.sacl_p2_sacl_p2.id094 = data;
            break;
        case 95:
            action_data->action_u.sacl_p2_sacl_p2.id095 = data;
            break;
        case 96:
            action_data->action_u.sacl_p2_sacl_p2.id096 = data;
            break;
        case 97:
            action_data->action_u.sacl_p2_sacl_p2.id097 = data;
            break;
        case 98:
            action_data->action_u.sacl_p2_sacl_p2.id098 = data;
            break;
        case 99:
            action_data->action_u.sacl_p2_sacl_p2.id099 = data;
            break;
        case 100:
            action_data->action_u.sacl_p2_sacl_p2.id100 = data;
            break;
        case 101:
            action_data->action_u.sacl_p2_sacl_p2.id101 = data;
            break;
        case 102:
            action_data->action_u.sacl_p2_sacl_p2.id102 = data;
            break;
        case 103:
            action_data->action_u.sacl_p2_sacl_p2.id103 = data;
            break;
        case 104:
            action_data->action_u.sacl_p2_sacl_p2.id104 = data;
            break;
        case 105:
            action_data->action_u.sacl_p2_sacl_p2.id105 = data;
            break;
        case 106:
            action_data->action_u.sacl_p2_sacl_p2.id106 = data;
            break;
        case 107:
            action_data->action_u.sacl_p2_sacl_p2.id107 = data;
            break;
        case 108:
            action_data->action_u.sacl_p2_sacl_p2.id108 = data;
            break;
        case 109:
            action_data->action_u.sacl_p2_sacl_p2.id109 = data;
            break;
        case 110:
            action_data->action_u.sacl_p2_sacl_p2.id110 = data;
            break;
        case 111:
            action_data->action_u.sacl_p2_sacl_p2.id111 = data;
            break;
        case 112:
            action_data->action_u.sacl_p2_sacl_p2.id112 = data;
            break;
        case 113:
            action_data->action_u.sacl_p2_sacl_p2.id113 = data;
            break;
        case 114:
            action_data->action_u.sacl_p2_sacl_p2.id114 = data;
            break;
        case 115:
            action_data->action_u.sacl_p2_sacl_p2.id115 = data;
            break;
        case 116:
            action_data->action_u.sacl_p2_sacl_p2.id116 = data;
            break;
        case 117:
            action_data->action_u.sacl_p2_sacl_p2.id117 = data;
            break;
        case 118:
            action_data->action_u.sacl_p2_sacl_p2.id118 = data;
            break;
        case 119:
            action_data->action_u.sacl_p2_sacl_p2.id119 = data;
            break;
        case 120:
            action_data->action_u.sacl_p2_sacl_p2.id120 = data;
            break;
        case 121:
            action_data->action_u.sacl_p2_sacl_p2.id121 = data;
            break;
        case 122:
            action_data->action_u.sacl_p2_sacl_p2.id122 = data;
            break;
        case 123:
            action_data->action_u.sacl_p2_sacl_p2.id123 = data;
            break;
        case 124:
            action_data->action_u.sacl_p2_sacl_p2.id124 = data;
            break;
        case 125:
            action_data->action_u.sacl_p2_sacl_p2.id125 = data;
            break;
        case 126:
            action_data->action_u.sacl_p2_sacl_p2.id126 = data;
            break;
        case 127:
            action_data->action_u.sacl_p2_sacl_p2.id127 = data;
            break;
        case 128:
            action_data->action_u.sacl_p2_sacl_p2.id128 = data;
            break;
        case 129:
            action_data->action_u.sacl_p2_sacl_p2.id129 = data;
            break;
        case 130:
            action_data->action_u.sacl_p2_sacl_p2.id130 = data;
            break;
        case 131:
            action_data->action_u.sacl_p2_sacl_p2.id131 = data;
            break;
        case 132:
            action_data->action_u.sacl_p2_sacl_p2.id132 = data;
            break;
        case 133:
            action_data->action_u.sacl_p2_sacl_p2.id133 = data;
            break;
        case 134:
            action_data->action_u.sacl_p2_sacl_p2.id134 = data;
            break;
        case 135:
            action_data->action_u.sacl_p2_sacl_p2.id135 = data;
            break;
        case 136:
            action_data->action_u.sacl_p2_sacl_p2.id136 = data;
            break;
        case 137:
            action_data->action_u.sacl_p2_sacl_p2.id137 = data;
            break;
        case 138:
            action_data->action_u.sacl_p2_sacl_p2.id138 = data;
            break;
        case 139:
            action_data->action_u.sacl_p2_sacl_p2.id139 = data;
            break;
        case 140:
            action_data->action_u.sacl_p2_sacl_p2.id140 = data;
            break;
        case 141:
            action_data->action_u.sacl_p2_sacl_p2.id141 = data;
            break;
        case 142:
            action_data->action_u.sacl_p2_sacl_p2.id142 = data;
            break;
        case 143:
            action_data->action_u.sacl_p2_sacl_p2.id143 = data;
            break;
        case 144:
            action_data->action_u.sacl_p2_sacl_p2.id144 = data;
            break;
        case 145:
            action_data->action_u.sacl_p2_sacl_p2.id145 = data;
            break;
        case 146:
            action_data->action_u.sacl_p2_sacl_p2.id146 = data;
            break;
        case 147:
            action_data->action_u.sacl_p2_sacl_p2.id147 = data;
            break;
        case 148:
            action_data->action_u.sacl_p2_sacl_p2.id148 = data;
            break;
        case 149:
            action_data->action_u.sacl_p2_sacl_p2.id149 = data;
            break;
        case 150:
            action_data->action_u.sacl_p2_sacl_p2.id150 = data;
            break;
        case 151:
            action_data->action_u.sacl_p2_sacl_p2.id151 = data;
            break;
        case 152:
            action_data->action_u.sacl_p2_sacl_p2.id152 = data;
            break;
        case 153:
            action_data->action_u.sacl_p2_sacl_p2.id153 = data;
            break;
        case 154:
            action_data->action_u.sacl_p2_sacl_p2.id154 = data;
            break;
        case 155:
            action_data->action_u.sacl_p2_sacl_p2.id155 = data;
            break;
        case 156:
            action_data->action_u.sacl_p2_sacl_p2.id156 = data;
            break;
        case 157:
            action_data->action_u.sacl_p2_sacl_p2.id157 = data;
            break;
        case 158:
            action_data->action_u.sacl_p2_sacl_p2.id158 = data;
            break;
        case 159:
            action_data->action_u.sacl_p2_sacl_p2.id159 = data;
            break;
        case 160:
            action_data->action_u.sacl_p2_sacl_p2.id160 = data;
            break;
        case 161:
            action_data->action_u.sacl_p2_sacl_p2.id161 = data;
            break;
        case 162:
            action_data->action_u.sacl_p2_sacl_p2.id162 = data;
            break;
        case 163:
            action_data->action_u.sacl_p2_sacl_p2.id163 = data;
            break;
        case 164:
            action_data->action_u.sacl_p2_sacl_p2.id164 = data;
            break;
        case 165:
            action_data->action_u.sacl_p2_sacl_p2.id165 = data;
            break;
        case 166:
            action_data->action_u.sacl_p2_sacl_p2.id166 = data;
            break;
        case 167:
            action_data->action_u.sacl_p2_sacl_p2.id167 = data;
            break;
        case 168:
            action_data->action_u.sacl_p2_sacl_p2.id168 = data;
            break;
        case 169:
            action_data->action_u.sacl_p2_sacl_p2.id169 = data;
            break;
        case 170:
            action_data->action_u.sacl_p2_sacl_p2.id170 = data;
            break;
        case 171:
            action_data->action_u.sacl_p2_sacl_p2.id171 = data;
            break;
        case 172:
            action_data->action_u.sacl_p2_sacl_p2.id172 = data;
            break;
        case 173:
            action_data->action_u.sacl_p2_sacl_p2.id173 = data;
            break;
        case 174:
            action_data->action_u.sacl_p2_sacl_p2.id174 = data;
            break;
        case 175:
            action_data->action_u.sacl_p2_sacl_p2.id175 = data;
            break;
        case 176:
            action_data->action_u.sacl_p2_sacl_p2.id176 = data;
            break;
        case 177:
            action_data->action_u.sacl_p2_sacl_p2.id177 = data;
            break;
        case 178:
            action_data->action_u.sacl_p2_sacl_p2.id178 = data;
            break;
        case 179:
            action_data->action_u.sacl_p2_sacl_p2.id179 = data;
            break;
        case 180:
            action_data->action_u.sacl_p2_sacl_p2.id180 = data;
            break;
        case 181:
            action_data->action_u.sacl_p2_sacl_p2.id181 = data;
            break;
        case 182:
            action_data->action_u.sacl_p2_sacl_p2.id182 = data;
            break;
        case 183:
            action_data->action_u.sacl_p2_sacl_p2.id183 = data;
            break;
        case 184:
            action_data->action_u.sacl_p2_sacl_p2.id184 = data;
            break;
        case 185:
            action_data->action_u.sacl_p2_sacl_p2.id185 = data;
            break;
        case 186:
            action_data->action_u.sacl_p2_sacl_p2.id186 = data;
            break;
        case 187:
            action_data->action_u.sacl_p2_sacl_p2.id187 = data;
            break;
        case 188:
            action_data->action_u.sacl_p2_sacl_p2.id188 = data;
            break;
        case 189:
            action_data->action_u.sacl_p2_sacl_p2.id189 = data;
            break;
        case 190:
            action_data->action_u.sacl_p2_sacl_p2.id190 = data;
            break;
        case 191:
            action_data->action_u.sacl_p2_sacl_p2.id191 = data;
            break;
        case 192:
            action_data->action_u.sacl_p2_sacl_p2.id192 = data;
            break;
        case 193:
            action_data->action_u.sacl_p2_sacl_p2.id193 = data;
            break;
        case 194:
            action_data->action_u.sacl_p2_sacl_p2.id194 = data;
            break;
        case 195:
            action_data->action_u.sacl_p2_sacl_p2.id195 = data;
            break;
        case 196:
            action_data->action_u.sacl_p2_sacl_p2.id196 = data;
            break;
        case 197:
            action_data->action_u.sacl_p2_sacl_p2.id197 = data;
            break;
        case 198:
            action_data->action_u.sacl_p2_sacl_p2.id198 = data;
            break;
        case 199:
            action_data->action_u.sacl_p2_sacl_p2.id199 = data;
            break;
        case 200:
            action_data->action_u.sacl_p2_sacl_p2.id200 = data;
            break;
        case 201:
            action_data->action_u.sacl_p2_sacl_p2.id201 = data;
            break;
        case 202:
            action_data->action_u.sacl_p2_sacl_p2.id202 = data;
            break;
        case 203:
            action_data->action_u.sacl_p2_sacl_p2.id203 = data;
            break;
        case 204:
            action_data->action_u.sacl_p2_sacl_p2.id204 = data;
            break;
        case 205:
            action_data->action_u.sacl_p2_sacl_p2.id205 = data;
            break;
        case 206:
            action_data->action_u.sacl_p2_sacl_p2.id206 = data;
            break;
        case 207:
            action_data->action_u.sacl_p2_sacl_p2.id207 = data;
            break;
        case 208:
            action_data->action_u.sacl_p2_sacl_p2.id208 = data;
            break;
        case 209:
            action_data->action_u.sacl_p2_sacl_p2.id209 = data;
            break;
        case 210:
            action_data->action_u.sacl_p2_sacl_p2.id210 = data;
            break;
        case 211:
            action_data->action_u.sacl_p2_sacl_p2.id211 = data;
            break;
        case 212:
            action_data->action_u.sacl_p2_sacl_p2.id212 = data;
            break;
        case 213:
            action_data->action_u.sacl_p2_sacl_p2.id213 = data;
            break;
        case 214:
            action_data->action_u.sacl_p2_sacl_p2.id214 = data;
            break;
        case 215:
            action_data->action_u.sacl_p2_sacl_p2.id215 = data;
            break;
        case 216:
            action_data->action_u.sacl_p2_sacl_p2.id216 = data;
            break;
        case 217:
            action_data->action_u.sacl_p2_sacl_p2.id217 = data;
            break;
        case 218:
            action_data->action_u.sacl_p2_sacl_p2.id218 = data;
            break;
        case 219:
            action_data->action_u.sacl_p2_sacl_p2.id219 = data;
            break;
        case 220:
            action_data->action_u.sacl_p2_sacl_p2.id220 = data;
            break;
        case 221:
            action_data->action_u.sacl_p2_sacl_p2.id221 = data;
            break;
        case 222:
            action_data->action_u.sacl_p2_sacl_p2.id222 = data;
            break;
        case 223:
            action_data->action_u.sacl_p2_sacl_p2.id223 = data;
            break;
        case 224:
            action_data->action_u.sacl_p2_sacl_p2.id224 = data;
            break;
        case 225:
            action_data->action_u.sacl_p2_sacl_p2.id225 = data;
            break;
        case 226:
            action_data->action_u.sacl_p2_sacl_p2.id226 = data;
            break;
        case 227:
            action_data->action_u.sacl_p2_sacl_p2.id227 = data;
            break;
        case 228:
            action_data->action_u.sacl_p2_sacl_p2.id228 = data;
            break;
        case 229:
            action_data->action_u.sacl_p2_sacl_p2.id229 = data;
            break;
        case 230:
            action_data->action_u.sacl_p2_sacl_p2.id230 = data;
            break;
        case 231:
            action_data->action_u.sacl_p2_sacl_p2.id231 = data;
            break;
        case 232:
            action_data->action_u.sacl_p2_sacl_p2.id232 = data;
            break;
        case 233:
            action_data->action_u.sacl_p2_sacl_p2.id233 = data;
            break;
        case 234:
            action_data->action_u.sacl_p2_sacl_p2.id234 = data;
            break;
        case 235:
            action_data->action_u.sacl_p2_sacl_p2.id235 = data;
            break;
        case 236:
            action_data->action_u.sacl_p2_sacl_p2.id236 = data;
            break;
        case 237:
            action_data->action_u.sacl_p2_sacl_p2.id237 = data;
            break;
        case 238:
            action_data->action_u.sacl_p2_sacl_p2.id238 = data;
            break;
        case 239:
            action_data->action_u.sacl_p2_sacl_p2.id239 = data;
            break;
        case 240:
            action_data->action_u.sacl_p2_sacl_p2.id240 = data;
            break;
        case 241:
            action_data->action_u.sacl_p2_sacl_p2.id241 = data;
            break;
        case 242:
            action_data->action_u.sacl_p2_sacl_p2.id242 = data;
            break;
        case 243:
            action_data->action_u.sacl_p2_sacl_p2.id243 = data;
            break;
        case 244:
            action_data->action_u.sacl_p2_sacl_p2.id244 = data;
            break;
        case 245:
            action_data->action_u.sacl_p2_sacl_p2.id245 = data;
            break;
        case 246:
            action_data->action_u.sacl_p2_sacl_p2.id246 = data;
            break;
        case 247:
            action_data->action_u.sacl_p2_sacl_p2.id247 = data;
            break;
        case 248:
            action_data->action_u.sacl_p2_sacl_p2.id248 = data;
            break;
        case 249:
            action_data->action_u.sacl_p2_sacl_p2.id249 = data;
            break;
        case 250:
            action_data->action_u.sacl_p2_sacl_p2.id250 = data;
            break;
        case 251:
            action_data->action_u.sacl_p2_sacl_p2.id251 = data;
            break;
        case 252:
            action_data->action_u.sacl_p2_sacl_p2.id252 = data;
            break;
        case 253:
            action_data->action_u.sacl_p2_sacl_p2.id253 = data;
            break;
        case 254:
            action_data->action_u.sacl_p2_sacl_p2.id254 = data;
            break;
        case 255:
            action_data->action_u.sacl_p2_sacl_p2.id255 = data;
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
rfc_p2_action_data_flush (mem_addr_t addr, void *actiondata)
{
    sdk_ret_t               ret;
    sacl_p2_actiondata_t    *action_data;

    PDS_TRACE_DEBUG("Flushing action data to 0x%llx", addr);
    action_data = (sacl_p2_actiondata_t *)actiondata;
    ret = impl_base::pipeline_impl()->write_to_rxdma_table(addr,
                                                           P4_P4PLUS_RXDMA_TBL_ID_SACL_P2,
                                                           SACL_P2_SACL_P2_ID, action_data);
    // reset the action data after flushing it
    memset(action_data, 0, sizeof(*action_data));

    return ret;
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
rfc_compute_p2_next_cl_addr_entry_num (mem_addr_t base_address,
                                       uint32_t class_block_number,
                                       uint32_t num_classes_per_block,
                                       mem_addr_t *next_cl_addr,
                                       uint16_t *next_entry_num)
{
    uint32_t    num_classes, num_cache_lines;

    num_classes = class_block_number * num_classes_per_block;
    num_cache_lines = num_classes/SACL_P2_ENTRIES_PER_CACHE_LINE;
    *next_cl_addr = base_address + (num_cache_lines << CACHE_LINE_SIZE_SHIFT);
    *next_entry_num = num_classes%SACL_P2_ENTRIES_PER_CACHE_LINE;
}

/**
 * @brief    given two equivalence class tables, compute the new equivalence
 *           class table or the result table by doing cross product of class
 *           bitmaps of the two tables
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @param[in] rfc_table                 result RFC table
 * @param[in] cbm         class bitmap from which is result is computed from
 * @param[in] cbm_size  class bitmap size
 * @return    result bits of P2 table corresponding to the class bitmap provided
 */
uint16_t
rfc_compute_p2_result (rfc_ctxt_t *rfc_ctxt, rfc_table_t *rfc_table,
                       rte_bitmap *cbm, uint32_t cbm_size)
{
    int         rv;
    uint16_t    result = 0;
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
    } else {
        do {
            posn = RTE_BITMAP_START_SLAB_SCAN_POS;
            while (rte_bitmap_slab_scan(slab, posn, &new_posn) != 0) {
                ruleidx =
                        rte_bitmap_get_global_bit_pos(cbm->index2 - 1, new_posn);
                PDS_TRACE_DEBUG("ruleidx = %u", ruleidx);
                if (rfc_ctxt->policy->rules[ruleidx].attrs.stateful) {
                    PDS_TRACE_DEBUG("rule %u is SF", ruleidx);
                    RFC_RESULT_SET_STATEFUL_BIT(result);
                } else {
                    PDS_TRACE_DEBUG("rule %u is SL", ruleidx);
                    RFC_RESULT_SET_STATELESS_BIT(result);
                }
                if (RFC_RESULT_BOTH_BITS_SET(result)) {
                    PDS_TRACE_DEBUG("Both SF and SL are set");
                    return result;
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
                                             rfc_ctxt->cbm, rfc_ctxt->cbm_size);
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
 * @return    SDK_RET_OK on success, failure status code on error
 */
    static inline sdk_ret_t
    rfc_compute_p1_eq_class_tables (rfc_ctxt_t *rfc_ctxt)
    {
        sacl_ip_sport_p1_actiondata_t action_data = { 0 };

        PDS_TRACE_DEBUG("Starting RFC P1");
        rfc_compute_eq_class_tables(rfc_ctxt,
                                    &rfc_ctxt->port_tree.rfc_table,
                                    &rfc_ctxt->sip_tree.rfc_table,
                                    &rfc_ctxt->p1_table,
                                    rfc_ctxt->base_addr + SACL_P1_TABLE_OFFSET,
                                    &action_data, SACL_P1_ENTRIES_PER_CACHE_LINE,
                                    rfc_compute_p1_next_cl_addr_entry_num,
                                    rfc_compute_class_id, rfc_p1_table_entry_pack,
                                    rfc_p1_action_data_flush);
        return SDK_RET_OK;
    }

/**
 * @brief    given the class bitmap tables of phase0 & phase1, compute class
 *           bitmap table(s) of RFC phase 2, and set the results bits
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_compute_p2_tables (rfc_ctxt_t *rfc_ctxt)
{
    sacl_p2_actiondata_t     action_data = { 0 };

    PDS_TRACE_DEBUG("Starting RFC P2");
    rfc_compute_eq_class_tables(rfc_ctxt, &rfc_ctxt->p1_table,
                                &rfc_ctxt->proto_port_tree.rfc_table, NULL,
                                rfc_ctxt->base_addr + SACL_P2_TABLE_OFFSET,
                                &action_data, SACL_P2_ENTRIES_PER_CACHE_LINE,
                                rfc_compute_p2_next_cl_addr_entry_num,
                                rfc_compute_p2_result, rfc_p2_table_entry_pack,
                                rfc_p2_action_data_flush);
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
    rfc_compute_p1_eq_class_tables(rfc_ctxt);
    rfc_p1_eq_class_tables_dump(rfc_ctxt);
    rfc_compute_p2_tables(rfc_ctxt);
    return SDK_RET_OK;
}

}    // namespace rfc
