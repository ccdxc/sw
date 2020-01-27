// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>

#include "include/sdk/base.hpp"

#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "p4_api.hpp"
#include "p4_utils.hpp"

#define P4PD_CALLOC  calloc
#define P4PD_FREE    free

// key strings used in table packing json file
#define JSON_KEY_MAX                "global"
#define JSON_KEY_MAX_INGRESS        "ingress"
#define JSON_KEY_MAX_EGRESS         "egress"
#define JSON_KEY_MAX_DEPTH          "depth"
#define JSON_KEY_TABLES             "tables"
#define JSON_KEY_TABLE_NAME         "name"
#define JSON_KEY_MATCH_TYPE         "match_type"
#define JSON_KEY_DIRECTION          "direction"
#define JSON_KEY_REGION             "region"
#define JSON_KEY_STAGE              "stage"
#define JSON_KEY_STAGE_TBL_ID       "stage_table_id"
#define JSON_KEY_NUM_ENTRIES        "num_entries"
#define JSON_KEY_TCAM               "tcam"
#define JSON_KEY_SRAM               "sram"
#define JSON_KEY_HBM                "hbm"
#define JSON_KEY_OVERFLOW           "overflow"
#define JSON_KEY_OVERFLOW_PARENT    "overflow_parent"
#define JSON_KEY_ENTRY_WIDTH        "entry_width"
#define JSON_KEY_ENTRY_WIDTH_BITS   "entry_width_bits"
#define JSON_KEY_ENTRY_START_INDEX  "entry_start_index"
#define JSON_KEY_ENTRY_END_INDEX    "entry_end_index"
#define JSON_KEY_TOP_LEFT_X         "layout.top_left.x"
#define JSON_KEY_TOP_LEFT_Y         "layout.top_left.y"
#define JSON_KEY_TOP_LEFT_BLOCK     "layout.top_left.block"
#define JSON_KEY_BTM_RIGHT_X        "layout.bottom_right.x"
#define JSON_KEY_BTM_RIGHT_Y        "layout.bottom_right.y"
#define JSON_KEY_BTM_RIGHT_BLOCK    "layout.bottom_right.block"
#define JSON_KEY_HASH_TYPE          "hash_type"
#define JSON_KEY_NUM_BUCKETS        "num_buckets"
#define JSON_KEY_THREAD_TBL_IDS     "thread_tbl_ids"
#define JSON_KEY_NUM_THREADS        "num_threads"

static p4pd_table_max_cfg_t  p4table_max_cfg;
/*
 * Based on table id call appropriate table routine.
 * For now this API is only called for p4pd and rxdma
 * routines. TODO: Add other APIs as well here.
 */
void
p4pd_global_hwentry_query (uint32_t tableid,
                           uint32_t *hwkey_len,
                           uint32_t *hwkeymask_len,
                           uint32_t *hwactiondata_len)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        p4pd_hwentry_query(tableid, hwkey_len, hwkeymask_len, hwactiondata_len);
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        p4pd_rxdma_hwentry_query(tableid, hwkey_len,
            hwkeymask_len, hwactiondata_len);
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        p4pd_txdma_hwentry_query(tableid, hwkey_len,
            hwkeymask_len, hwactiondata_len);
    } else {
        SDK_ASSERT(0);
    }
    return;
}

/*
 * Based on table id call appropriate table routine.
 * For now this API is only called for p4pd and rxdma
 * routines. TODO: Add other APIs as well here.
 */
p4pd_error_t
p4pd_global_entry_write (uint32_t tableid,
                         uint32_t  index,
                         uint8_t   *hwkey,
                         uint8_t   *hwkey_mask,
                         void      *actiondata)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return (p4pd_entry_write(tableid, index, hwkey, hwkey_mask, actiondata));
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return (p4pd_rxdma_entry_write(tableid,
                index, hwkey, hwkey_mask, actiondata));
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return (p4pd_txdma_entry_write(tableid,
                index, hwkey, hwkey_mask, actiondata));
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

/*
 * Based on table id call appropriate table routine.
 * For now this API is only called for p4pd and rxdma
 * routines. TODO: Add other APIs as well here.
 */
p4pd_error_t
p4pd_global_entry_write_with_datamask (uint32_t tableid,
                                       uint32_t  index,
                                       uint8_t   *hwkey,
                                       uint8_t   *hwkey_mask,
                                       void      *actiondata,
                                       void      *actiondata_mask)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return (p4pd_entry_write_with_datamask(tableid, index, hwkey, hwkey_mask,
                                               actiondata, actiondata_mask));
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return (p4pd_rxdma_entry_write_with_datamask(tableid,
                index, hwkey, hwkey_mask, actiondata, actiondata_mask));
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return (p4pd_txdma_entry_write_with_datamask(tableid,
                index, hwkey, hwkey_mask, actiondata, actiondata_mask));
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

/*
 * Based on table id call appropriate table routine.
 * For now this API is only called for p4pd and rxdma
 * routines. TODO: Add other APIs as well here.
 */
p4pd_error_t
p4pd_global_entry_read (uint32_t tableid,
                        uint32_t   index,
                        void       *swkey,
                        void       *swkey_mask,
                        void       *actiondata)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return (p4pd_entry_read(tableid, index, swkey, swkey_mask, actiondata));
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return (p4pd_rxdma_entry_read(tableid,
                index, swkey, swkey_mask, actiondata));
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return (p4pd_txdma_entry_read(tableid,
                index, swkey, swkey_mask, actiondata));
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

p4pd_error_t
p4pd_global_table_ds_decoded_string_get (uint32_t   tableid,
                                         uint32_t   index,
                                         void*      sw_key,
                                         /* Valid only in case of TCAM;
                                          * Otherwise can be NULL) */
                                         void*      sw_key_mask,
                                         void*      action_data,
                                         char*      buffer,
                                         uint16_t   buf_len)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return (p4pd_table_ds_decoded_string_get(tableid,
                index, sw_key, sw_key_mask, action_data, buffer, buf_len));
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return (p4pd_rxdma_table_ds_decoded_string_get(tableid,
                index, sw_key, sw_key_mask, action_data, buffer, buf_len));
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return (p4pd_txdma_table_ds_decoded_string_get(tableid,
                index, sw_key, sw_key_mask, action_data, buffer, buf_len));
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

/* P4PD API that uses tableID to return table properties that HAL
 * layer can use to construct, initialize P4 tables in local memory.
 *
 * Arguments:
 *
 *  IN  : uint32_t          tableid    : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *  OUT : p4pd_table_ctx_t *table_ctx  : Returns a structure of data
 *                                       that contains table properties.
 * Return Value:
 *  P4PD_SUCCESS                       : Table properties copied into tbl_ctx
 *                                       Memory for tbl_ctx is provided by
 *                                       API caller.
 *
 *  P4PD_FAIL                          : If tableid is not valid
 */
p4pd_error_t
p4pd_global_table_properties_get (uint32_t tableid, void *tbl_ctx)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return (p4pd_table_properties_get(tableid,
                (p4pd_table_properties_t*)tbl_ctx));
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return (p4pluspd_rxdma_table_properties_get(tableid,
               (p4pd_table_properties_t*) tbl_ctx));
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return (p4pluspd_txdma_table_properties_get(tableid,
               (p4pd_table_properties_t*) tbl_ctx));
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

static p4pd_table_properties_t *_p4tbls;
namespace pt = boost::property_tree;

static uint16_t
p4pd_get_tableid_from_tablename (const char *tablename)
{
    for (uint32_t i = p4pd_tableid_min_get(); i < p4pd_tableid_max_get(); i++) {
        if (!strcmp(p4pd_tbl_names[i], tablename)) {
            return i;
        }
    }
    return -1;
}

static p4pd_table_dir_en
p4pd_get_table_direction (const char *direction)
{
    if (!strcmp(direction, "ingress")) {
        return(P4_GRESS_INGRESS);
    }
    if (!strcmp(direction, "egress")) {
        return(P4_GRESS_EGRESS);
    }
    return P4_GRESS_INVALID;
}

static p4pd_table_type_en
p4pd_get_table_type (const char *match_type)
{
    if (!strcmp(match_type, "hash")) {
        return(P4_TBL_TYPE_HASH);
    }
    if (!strcmp(match_type, "hash_tcam")) {
        return(P4_TBL_TYPE_HASHTCAM);
    }
    if (!strcmp(match_type, "tcam_sram")) {
        return(P4_TBL_TYPE_TCAM);
    }
    if (!strcmp(match_type, "indexed")) {
        return(P4_TBL_TYPE_INDEX);
    }
    if (!strcmp(match_type, "mpu")) {
        return(P4_TBL_TYPE_MPU);
    }
    return P4_TBL_TYPE_INVALID;
}

static p4pd_error_t
p4pd_tbl_packing_json_parse (p4pd_cfg_t *p4pd_cfg)
{
    pt::ptree                  json_pt;
    p4pd_table_properties_t    *tbl;
    std::string                full_path;
    int                        tableid, num_tables = p4pd_tableid_max_get();

    full_path = std::string(p4pd_cfg->cfg_path) + "/" +
                    std::string(p4pd_cfg->table_map_cfg_file);
    std::ifstream tbl_json(full_path.c_str());
    read_json(tbl_json, json_pt);

    boost::optional<pt::ptree&>table_gl =
        json_pt.get_child_optional(JSON_KEY_MAX);
    if (!table_gl) {
        return P4PD_FAIL;
    }

    BOOST_FOREACH (pt::ptree::value_type &obj, json_pt.get_child(JSON_KEY_MAX)) {
        if (obj.first == JSON_KEY_TCAM) {
            boost::optional<pt::ptree&>ing = obj.second.get_child_optional(JSON_KEY_MAX_INGRESS);
            boost::optional<pt::ptree&>egr = obj.second.get_child_optional(JSON_KEY_MAX_EGRESS);
            if (ing) {
                p4table_max_cfg.tcam_ingress_depth = ing.get().get<int>(JSON_KEY_MAX_DEPTH);
            }
            if (egr) {
                p4table_max_cfg.tcam_egress_depth = egr.get().get<int>(JSON_KEY_MAX_DEPTH);
            }
        } else if (obj.first == JSON_KEY_SRAM) {
            boost::optional<pt::ptree&>ing = obj.second.get_child_optional(JSON_KEY_MAX_INGRESS);
            boost::optional<pt::ptree&>egr = obj.second.get_child_optional(JSON_KEY_MAX_EGRESS);
            if (ing) {
                p4table_max_cfg.sram_ingress_depth = ing.get().get<int>(JSON_KEY_MAX_DEPTH);
            }
            if (egr) {
                p4table_max_cfg.sram_egress_depth = egr.get().get<int>(JSON_KEY_MAX_DEPTH);
            }
        }
    }

    boost::optional<pt::ptree&>table_pt =
        json_pt.get_child_optional(JSON_KEY_TABLES);
    if (!table_pt) {
        return P4PD_FAIL;
    }
    _p4tbls =
        (p4pd_table_properties_t *)P4PD_CALLOC(num_tables,
                                               sizeof(p4pd_table_properties_t));
    if (!_p4tbls) {
        return P4PD_FAIL;
    }

    // iterator over all p4 tables packing data and build book keeping data
    // structures used to read/write to device.
    BOOST_FOREACH(pt::ptree::value_type &p4_tbl, json_pt.get_child(JSON_KEY_TABLES)) {
        std::string tablename = p4_tbl.second.get<std::string>(JSON_KEY_TABLE_NAME);
        std::string match_type = p4_tbl.second.get<std::string>(JSON_KEY_MATCH_TYPE);
        std::string direction = p4_tbl.second.get<std::string>(JSON_KEY_DIRECTION);
        std::string overflow  = p4_tbl.second.get<std::string>(JSON_KEY_OVERFLOW);
        std::string overflow_parent  = p4_tbl.second.get<std::string>(JSON_KEY_OVERFLOW_PARENT);

        tableid = p4pd_get_tableid_from_tablename(tablename.c_str());
        if (tableid == -1) {
            return P4PD_FAIL;
        }

        tbl = _p4tbls + tableid;
        tbl->key_struct_size = p4pd_tbl_swkey_size[tableid];
        tbl->actiondata_struct_size = p4pd_tbl_sw_action_data_size[tableid];

        if (strlen(overflow.c_str())) {
            tbl->has_oflow_table = true;
            tbl->oflow_table_id =
                p4pd_get_tableid_from_tablename(overflow.c_str());
        } else {
            tbl->has_oflow_table = false;
        }
        if (strlen(overflow_parent.c_str())) {
            tbl->is_oflow_table = true;
            tbl->oflow_parent_table_id =
                p4pd_get_tableid_from_tablename(overflow_parent.c_str());
        } else {
            tbl->is_oflow_table = false;
        }

        tbl->tablename = (char*)p4pd_tbl_names[tableid];
        tbl->tableid = tableid;

        tbl->table_type = p4pd_get_table_type(match_type.c_str());
        tbl->gress = p4pd_get_table_direction(direction.c_str());
        tbl->hash_type = p4_tbl.second.get<int>(JSON_KEY_HASH_TYPE);
        tbl->stage = p4_tbl.second.get<int>(JSON_KEY_STAGE);
        tbl->stage_tableid = p4_tbl.second.get<int>(JSON_KEY_STAGE_TBL_ID);
        tbl->tabledepth = p4_tbl.second.get<int>(JSON_KEY_NUM_ENTRIES);

        // set pipeline
        if (tbl->gress == P4_GRESS_INGRESS) {
            tbl->pipe = P4_PIPELINE_INGRESS;
        } else {
            tbl->pipe = P4_PIPELINE_EGRESS;
        }
        //default table write mode is blocking.
        tbl->wr_mode = P4_TBL_WRITE_MODE_BLOCKING;

        //default table read thru mode is false.
        tbl->read_thru_mode = false;

        // memory units used by the table
        boost::optional<pt::ptree&>_tcam = p4_tbl.second.get_child_optional(JSON_KEY_TCAM);
        if (_tcam) {
            tbl->tcam_layout.entry_width = _tcam.get().get<int>(JSON_KEY_ENTRY_WIDTH);
            tbl->tcam_layout.entry_width_bits = _tcam.get().get<int>(JSON_KEY_ENTRY_WIDTH_BITS);
            tbl->tcam_layout.start_index = _tcam.get().get<int>(JSON_KEY_ENTRY_START_INDEX);
            tbl->tcam_layout.end_index = _tcam.get().get<int>(JSON_KEY_ENTRY_END_INDEX);
            tbl->tcam_layout.top_left_x = _tcam.get().get<int>(JSON_KEY_TOP_LEFT_X);
            tbl->tcam_layout.top_left_y = _tcam.get().get<int>(JSON_KEY_TOP_LEFT_Y);
            tbl->tcam_layout.top_left_block = _tcam.get().get<int>(JSON_KEY_TOP_LEFT_BLOCK);
            tbl->tcam_layout.btm_right_x = _tcam.get().get<int>(JSON_KEY_BTM_RIGHT_X);
            tbl->tcam_layout.btm_right_y = _tcam.get().get<int>(JSON_KEY_BTM_RIGHT_Y);
            tbl->tcam_layout.btm_right_block = _tcam.get().get<int>(JSON_KEY_BTM_RIGHT_BLOCK);
            tbl->tcam_layout.num_buckets = _tcam.get().get<int>(JSON_KEY_NUM_BUCKETS);
        }
        boost::optional<pt::ptree&>_sram = p4_tbl.second.get_child_optional(JSON_KEY_SRAM);
        if (_sram) {
            tbl->sram_layout.entry_width = _sram.get().get<int>(JSON_KEY_ENTRY_WIDTH);
            tbl->sram_layout.entry_width_bits = _sram.get().get<int>(JSON_KEY_ENTRY_WIDTH_BITS);
            tbl->sram_layout.start_index = _sram.get().get<int>(JSON_KEY_ENTRY_START_INDEX);
            tbl->sram_layout.end_index = _sram.get().get<int>(JSON_KEY_ENTRY_END_INDEX);
            tbl->sram_layout.top_left_x = _sram.get().get<int>(JSON_KEY_TOP_LEFT_X);
            tbl->sram_layout.top_left_y = _sram.get().get<int>(JSON_KEY_TOP_LEFT_Y);
            tbl->sram_layout.top_left_block = _sram.get().get<int>(JSON_KEY_TOP_LEFT_BLOCK);
            tbl->sram_layout.btm_right_x = _sram.get().get<int>(JSON_KEY_BTM_RIGHT_X);
            tbl->sram_layout.btm_right_y = _sram.get().get<int>(JSON_KEY_BTM_RIGHT_Y);
            tbl->sram_layout.btm_right_block = _sram.get().get<int>(JSON_KEY_BTM_RIGHT_BLOCK);
            tbl->sram_layout.num_buckets = _sram.get().get<int>(JSON_KEY_NUM_BUCKETS);
        }
        boost::optional<pt::ptree&>_hbm = p4_tbl.second.get_child_optional(JSON_KEY_HBM);
        if (_hbm) {
            tbl->table_location = P4_TBL_LOCATION_HBM;
            tbl->hbm_layout.entry_width = _hbm.get().get<int>(JSON_KEY_ENTRY_WIDTH);
            tbl->hbm_layout.start_index = _hbm.get().get<int>(JSON_KEY_ENTRY_START_INDEX);
            tbl->hbm_layout.end_index = _hbm.get().get<int>(JSON_KEY_ENTRY_END_INDEX);
        } else {
            tbl->table_location = P4_TBL_LOCATION_PIPE;
        }
        tbl->table_thread_count = p4_tbl.second.get<int>(JSON_KEY_NUM_THREADS);
        boost::optional<pt::ptree&>_table_threads = p4_tbl.second.get_child_optional(JSON_KEY_THREAD_TBL_IDS);
        if (tbl->table_thread_count > 1 && _table_threads) {
            for (int k = 1; k < tbl->table_thread_count; k++) {
                auto s = std::to_string(k);
                tbl->thread_table_id[k] = _table_threads.get().get<int>(s.c_str());
            }
        }
    }
    return P4PD_SUCCESS;
}

//-----------------------------------------------------------------------------
// cleanup function for p4pd meta
//-----------------------------------------------------------------------------
void
p4pd_cleanup (void)
{
    P4PD_FREE(_p4tbls);
}

//-----------------------------------------------------------------------------
// do common initialization
//-----------------------------------------------------------------------------
p4pd_error_t
p4pd_init (p4pd_cfg_t *p4pd_cfg)
{
    p4pd_prep_p4tbl_names();
    p4pd_prep_p4tbl_sw_struct_sizes();
    if (p4pd_tbl_packing_json_parse(p4pd_cfg) != P4PD_SUCCESS) {
        p4pd_cleanup();
        return P4PD_FAIL;
    }
    return P4PD_SUCCESS;
}

//----------------------------------------------------------------------------
// set hbm address and mapped address
//----------------------------------------------------------------------------
static void
p4pd_hbm_table_address_set (uint32_t tableid, mem_addr_t pa, mem_addr_t va,
                            p4pd_table_cache_t cache)
{
    p4pd_table_properties_t *tbl;

    tbl = _p4tbls + tableid;
    tbl->base_mem_pa = pa;
    tbl->base_mem_va = va;
    tbl->cache = cache;
}

//-----------------------------------------------------------------------------
// P4PD API that uses tableID to return table properties that app
// layer can use to construct, initialize P4 tables in local memory.
//
// Arguments:
//
//  IN  : uint32_t          tableid    : Table Id that identifies
//                                       P4 table. This id is obtained
//                                       from p4pd_table_id_enum.
//  OUT : p4pd_table_ctx_t *table_ctx  : Returns a structure of data
//                                       that contains table properties.
// Return Value:
//  P4PD_SUCCESS                       : Table properties copied into tbl_ctx
//                                       Memory for tbl_ctx is provided by
//                                       API caller.
//
//  P4PD_FAIL                          : If tableid is not valid
//-----------------------------------------------------------------------------
p4pd_error_t
p4pd_table_properties_get (uint32_t tableid, p4pd_table_properties_t *tbl_ctx)
{
    if (tableid >= p4pd_tableid_max_get() || !_p4tbls) {
        return P4PD_FAIL;
    }
    memcpy(tbl_ctx, _p4tbls + tableid, sizeof(p4pd_table_properties_t));
    return P4PD_SUCCESS;
}


//-----------------------------------------------------------------------------
// P4PD API that uses tableID to set the table write mode that ASIC
// layer use when writing to P4 tables.
//
// Arguments:
//
//  IN  : uint32_t          tableid    : Table Id that identifies
//                                       P4 table. This id is obtained
//                                       from p4pd_table_id_enum.
//
//  IN  : p4pd_table_write_mode_t wr_mode  : Table write mode.
//
// Return Value:
//  P4PD_SUCCESS                       : Table write mode is set successfully
//                                       in p4tbls context that was initialized earlier.
//
//  P4PD_FAIL                          : If tableid is not valid
//-----------------------------------------------------------------------------
p4pd_error_t p4pd_table_properties_set_write_mode(uint32_t tableid,
                                                  p4pd_table_write_mode_t wr_mode)
{
    p4pd_table_properties_t *tbl_ctx;
    if (tableid >= p4pd_tableid_max_get() || !_p4tbls) {
        return P4PD_FAIL;
    }

    tbl_ctx = _p4tbls + tableid;

    tbl_ctx->wr_mode = wr_mode;
    return P4PD_SUCCESS;
}

//-----------------------------------------------------------------------------
// P4PD API that uses tableID to set the table read thru mode property that ASIC
// layer can uses when reading P4 tables.
//
// Arguments:
//
//  IN  : uint32_t tableid            : Table Id that identifies
//                                      P4 table. This id is obtained
//                                      from p4pd_table_id_enum.
//
//  IN  : bool     read_thru_mode     : Table reade thru mode.
//
// Return Value:
//  P4PD_SUCCESS                      : Table read thru mode is set successfully
//                                      in p4tbls context that was initialized earlier.
//
//  P4PD_FAIL                         : If tableid is not valid
//-----------------------------------------------------------------------------
p4pd_error_t p4pd_table_properties_set_read_thru_mode(uint32_t tableid,
                                                      bool read_thru_mode)
{
    p4pd_table_properties_t *tbl_ctx;
    if (tableid >= p4pd_tableid_max_get() || !_p4tbls) {
        return P4PD_FAIL;
    }

    tbl_ctx = _p4tbls + tableid;

    tbl_ctx->read_thru_mode = read_thru_mode;
    return P4PD_SUCCESS;
}


uint32_t
p4pd_global_actiondata_appdata_size_get (uint32_t tableid, uint8_t actionid)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return p4pd_actiondata_appdata_size_get(tableid, actionid);
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return p4pd_rxdma_actiondata_appdata_size_get(tableid, actionid);
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return p4pd_txdma_actiondata_appdata_size_get(tableid, actionid);
    } else {
        SDK_ASSERT(0);
    }
    return 0;
}

p4pd_error_t
p4pd_global_actiondata_appdata_set (uint32_t tableid, uint8_t actionid,
                                    void *appdata, void *actiondata)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return p4pd_actiondata_appdata_set(tableid, actionid,
                                           appdata, actiondata);
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return p4pd_rxdma_actiondata_appdata_set(tableid, actionid,
                                                 appdata, actiondata);
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return p4pd_txdma_actiondata_appdata_set(tableid, actionid,
                                                 appdata, actiondata);
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

p4pd_error_t
p4pd_global_actiondata_appdata_get (uint32_t tableid, uint8_t actionid,
                                    void *appdata, void *actiondata)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return p4pd_actiondata_appdata_get(tableid, actionid,
                                           appdata, actiondata);
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return p4pd_rxdma_actiondata_appdata_get(tableid, actionid,
                                                 appdata, actiondata);
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return p4pd_txdma_actiondata_appdata_get(tableid, actionid,
                                                 appdata, actiondata);
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

p4pd_error_t
p4pd_global_entry_install (uint32_t tableid, uint32_t index,
                           void *swkey, void *swkey_mask,
                           void *actiondata)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return p4pd_entry_install(tableid, index, swkey,
                                  swkey_mask, actiondata);
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return p4pd_rxdma_entry_install(tableid, index, swkey,
                                        swkey_mask, actiondata);
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return p4pd_txdma_entry_install(tableid, index, swkey,
                                        swkey_mask, actiondata);
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

p4pd_error_t
p4pd_global_hwkey_hwmask_build (uint32_t tableid, void *swkey,
                                void *swkey_mask, uint8_t *hw_key,
                                uint8_t *hw_key_mask)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return p4pd_hwkey_hwmask_build(tableid, swkey,
                                       swkey_mask, hw_key, hw_key_mask);
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return p4pd_rxdma_hwkey_hwmask_build(tableid, swkey,
                                             swkey_mask, hw_key, hw_key_mask);
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return p4pd_txdma_hwkey_hwmask_build(tableid, swkey,
                                             swkey_mask, hw_key, hw_key_mask);
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

p4pd_error_t
p4pd_global_actiondata_hwfield_set (uint32_t tableid, uint8_t actionid,
                                    uint32_t argument_slotid,
                                    uint8_t *argumentvalue,
                                    void *actiondata)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return p4pd_actiondata_hwfield_set(tableid, actionid,
                                           argument_slotid,
                                           argumentvalue,
                                           actiondata);
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return p4pd_rxdma_actiondata_hwfield_set(tableid, actionid,
                                                 argument_slotid,
                                                 argumentvalue,
                                                 actiondata);
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return p4pd_txdma_actiondata_hwfield_set(tableid, actionid,
                                                 argument_slotid,
                                                 argumentvalue,
                                                 actiondata);
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

p4pd_error_t
p4pd_global_actiondata_hwfield_get (uint32_t tableid, uint8_t actionid,
                                    uint32_t argument_slotid,
                                    uint8_t *argumentvalue,
                                    void *actiondata)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        return p4pd_actiondata_hwfield_get(tableid, actionid,
                                           argument_slotid,
                                           argumentvalue,
                                           actiondata);
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        return p4pd_rxdma_actiondata_hwfield_get(tableid, actionid,
                                                 argument_slotid,
                                                 argumentvalue,
                                                 actiondata);
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        return p4pd_txdma_actiondata_hwfield_get(tableid, actionid,
                                                 argument_slotid,
                                                 argumentvalue,
                                                 actiondata);
    } else {
        SDK_ASSERT(0);
    }
    return P4PD_SUCCESS;
}

void
p4pd_global_hbm_table_address_set (uint32_t tableid, mem_addr_t pa,
                                   mem_addr_t va, p4pd_table_cache_t cache)
{
    if ((tableid >= p4pd_tableid_min_get()) &&
        (tableid <= p4pd_tableid_max_get())) {
        p4pd_hbm_table_address_set(tableid, pa, va, cache);
    } else if ((tableid >= p4pd_rxdma_tableid_min_get()) &&
               (tableid <= p4pd_rxdma_tableid_max_get())) {
        p4pd_rxdma_hbm_table_address_set(tableid, pa, va, cache);
    } else if ((tableid >= p4pd_txdma_tableid_min_get()) &&
               (tableid <= p4pd_txdma_tableid_max_get())) {
        p4pd_txdma_hbm_table_address_set(tableid, pa, va, cache);
    } else {
        SDK_ASSERT(0);
    }
}

uint32_t
p4pd_sram_table_depth_get(p4pd_pipeline_t pipeline)
{
    if (pipeline == P4_PIPELINE_INGRESS) {
        return p4table_max_cfg.sram_ingress_depth;
    } else if (pipeline == P4_PIPELINE_EGRESS) {
        return p4table_max_cfg.sram_egress_depth;
    } else  {
        // TODO for RXDMA and TXDMA
        SDK_ASSERT(0);
    }
}

uint32_t
p4pd_tcam_table_depth_get(p4pd_pipeline_t pipeline)
{
    if (pipeline == P4_PIPELINE_INGRESS) {
        return p4table_max_cfg.tcam_ingress_depth;
    } else if (pipeline == P4_PIPELINE_EGRESS) {
        return p4table_max_cfg.tcam_egress_depth;
    } else  {
        // No TCAM for RXDMA and TXDMA
        SDK_ASSERT(0);
    }
}
