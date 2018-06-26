#ifndef __DOL_STORAGE_SEQ_P4PD_HPP__
#define __DOL_STORAGE_SEQ_P4PD_HPP__

#include "nic/gen/storage_seq/include/storage_seq_p4pd.h"

/*
 * Default P4PD table width
 */
#define STORAGE_SEQ_P4PD_TABLE_BIT_WIDTH_DFLT       512
#define STORAGE_SEQ_P4PD_TABLE_BYTE_WIDTH_DFLT      64

/*
 * Debug flags
 */ 
#define STORAGE_SEQ_ACTION_PACK_WIDTH_DEBUG         0

/*
 * Bit width to byte width
 */
#define STORAGE_SEQ_BYTE_WIDTH(bit_width)                                       \
    (((bit_width) + BITS_PER_BYTE - 1) / BITS_PER_BYTE)
    
/*
 * Actiondata scalar field assignment
 */
#define STORAGE_SEQ_ACTION_SCALAR_SET(actiondata, u, f, src)                    \
    actiondata.u.f = src
    
/*
 * Actiondata array field assignment
 */
#define STORAGE_SEQ_ACTION_ARRAY_SET(actiondata, u, f, src)                     \
    memcpy(&actiondata.u.f[0], &src, sizeof(actiondata.u.f))
    
/*
 * Actiondata P4PD entry pack
 */
#define STORAGE_SEQ_ACTION_PACK(dst_p, actiondata, table_id, action_id)         \
    do {                                                                        \
        uint8_t     *packed_p;                                                  \
        uint32_t    bit_width;                                                  \
        uint32_t    byte_width;                                                 \
        packed_p = p4pd_storage_seq_entry_pack(table_id, action_id,             \
                                               (void *)&actiondata, &bit_width);\
        byte_width = STORAGE_SEQ_BYTE_WIDTH(bit_width);                         \
        if (STORAGE_SEQ_ACTION_PACK_WIDTH_DEBUG) {                              \
            printf("table_id %u action_id %u bit_width %u byte_width %u\n",     \
                   table_id, action_id, bit_width, byte_width);                 \
        }                                                                       \
        memcpy(dst_p, packed_p, byte_width);                                    \
    } while (false)
    
/*
 * Sequencer descriptor action
 */
#define STORAGE_SEQ_DESC_DEFINE(desc)                                           \
    s1_tbl_actiondata       desc
    
#define STORAGE_SEQ_DESC_SCALAR_SET(desc, f, src)                               \
    STORAGE_SEQ_ACTION_SCALAR_SET(desc,                                         \
            s1_tbl_action_u.s1_tbl_seq_barco_entry_handler, f, src)
            
#define STORAGE_SEQ_DESC_ARRAY_SET(desc, f, src)                                \
    STORAGE_SEQ_ACTION_ARRAY_SET(desc,                                          \
            s1_tbl_action_u.s1_tbl_seq_barco_entry_handler, f, src)
            
#define STORAGE_SEQ_DESC_PACK(dst_p, desc)                                      \
    STORAGE_SEQ_ACTION_PACK(dst_p, desc, P4_STORAGE_SEQ_TBL_ID_S1_TBL,          \
                            S1_TBL_SEQ_BARCO_ENTRY_HANDLER_ID);

/*
 * Comp status descriptor0 action
 */
#define STORAGE_SEQ_CS_DESC0_DEFINE(cs_desc0)                                   \
    s1_tbl_actiondata       cs_desc0
    
#define STORAGE_SEQ_CS_DESC0_SCALAR_SET(cs_desc0, f, src)                       \
    STORAGE_SEQ_ACTION_SCALAR_SET(cs_desc0,                                     \
            s1_tbl_action_u.s1_tbl_seq_comp_status_desc0_handler, f, src)
            
#define STORAGE_SEQ_CS_DESC0_ARRAY_SET(cs_desc0, f, src)                        \
    STORAGE_SEQ_ACTION_ARRAY_SET(cs_desc0,                                      \
            s1_tbl_action_u.s1_tbl_seq_comp_status_desc0_handler, f, src)
            
#define STORAGE_SEQ_CS_DESC0_PACK(dst_p, cs_desc0)                              \
    STORAGE_SEQ_ACTION_PACK(dst_p, cs_desc0, P4_STORAGE_SEQ_TBL_ID_S1_TBL,      \
                            S1_TBL_SEQ_COMP_STATUS_DESC0_HANDLER_ID);

/*
 * Comp status descriptor1 action
 */
#define STORAGE_SEQ_CS_DESC1_DEFINE(cs_desc1)                                   \
    s1_tbl1_actiondata       cs_desc1
    
#define STORAGE_SEQ_CS_DESC1_SCALAR_SET(cs_desc1, f, src)                       \
    STORAGE_SEQ_ACTION_SCALAR_SET(cs_desc1,                                     \
            s1_tbl1_action_u.s1_tbl1_seq_comp_status_desc1_handler, f, src)
            
#define STORAGE_SEQ_CS_DESC1_ARRAY_SET(cs_desc1, f, src)                        \
    STORAGE_SEQ_ACTION_ARRAY_SET(cs_desc1,                                      \
            s1_tbl1_action_u.s1_tbl1_seq_comp_status_desc1_handler, f, src)
            
#define STORAGE_SEQ_CS_DESC1_PACK(dst_p, cs_desc1)                              \
    STORAGE_SEQ_ACTION_PACK(dst_p, cs_desc1, P4_STORAGE_SEQ_TBL_ID_S1_TBL1,     \
                            S1_TBL1_SEQ_COMP_STATUS_DESC1_HANDLER_ID);
                            

/*
 * XTS status descriptor0 action
 */
#define STORAGE_SEQ_XS_DESC0_DEFINE(xs_desc0)                                   \
    s1_tbl_actiondata       xs_desc0
    
#define STORAGE_SEQ_XS_DESC0_SCALAR_SET(xs_desc0, f, src)                       \
    STORAGE_SEQ_ACTION_SCALAR_SET(xs_desc0,                                     \
            s1_tbl_action_u.s1_tbl_seq_xts_status_desc0_handler, f, src)
            
#define STORAGE_SEQ_XS_DESC0_ARRAY_SET(xs_desc0, f, src)                        \
    STORAGE_SEQ_ACTION_ARRAY_SET(xs_desc0,                                      \
            s1_tbl_action_u.s1_tbl_seq_xts_status_desc0_handler, f, src)
            
#define STORAGE_SEQ_XS_DESC0_PACK(dst_p, xs_desc0)                              \
    STORAGE_SEQ_ACTION_PACK(dst_p, xs_desc0, P4_STORAGE_SEQ_TBL_ID_S1_TBL,      \
                            S1_TBL_SEQ_XTS_STATUS_DESC0_HANDLER_ID);

/*
 * XTS status descriptor1 action
 */
#define STORAGE_SEQ_XS_DESC1_DEFINE(xs_desc1)                                   \
    s1_tbl1_actiondata       xs_desc1
    
#define STORAGE_SEQ_XS_DESC1_SCALAR_SET(xs_desc1, f, src)                       \
    STORAGE_SEQ_ACTION_SCALAR_SET(xs_desc1,                                     \
            s1_tbl1_action_u.s1_tbl1_seq_xts_status_desc1_handler, f, src)
            
#define STORAGE_SEQ_XS_DESC1_ARRAY_SET(xs_desc1, f, src)                        \
    STORAGE_SEQ_ACTION_ARRAY_SET(xs_desc1,                                      \
            s1_tbl1_action_u.s1_tbl1_seq_xts_status_desc1_handler, f, src)
            
#define STORAGE_SEQ_XS_DESC1_PACK(dst_p, xs_desc1)                              \
    STORAGE_SEQ_ACTION_PACK(dst_p, xs_desc1, P4_STORAGE_SEQ_TBL_ID_S1_TBL1,     \
                            S1_TBL1_SEQ_XTS_STATUS_DESC1_HANDLER_ID);
                            
#endif /* __DOL_STORAGE_SEQ_P4PD_HPP__ */

