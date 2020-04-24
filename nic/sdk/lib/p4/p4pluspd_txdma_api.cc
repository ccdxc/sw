// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "lib/p4/p4_api.hpp"

#define P4PD_CALLOC  calloc
#define P4PD_FREE    free

// key strings used in table packing json file
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

static p4pd_table_properties_t *_p4plus_txdma_tbls;

namespace pt = boost::property_tree;

static int
p4pluspd_txdma_get_tableid_from_tablename (const char *tablename)
{
    for (uint32_t i = 0; i < p4pd_txdma_tableid_max_get(); i++) {
        if (!strcmp(p4pd_txdma_tbl_names[i], tablename)) {
            return i;
        }
    }
    return -1;
}

static p4pd_table_dir_en
p4pluspd_txdma_get_table_direction (const char *direction)
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
p4pluspd_txdma_get_table_type (const char *match_type)
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
p4pluspd_txdma_tbl_packing_json_parse (p4pd_cfg_t *cfg)
{
    pt::ptree                  json_pt;
    p4pd_table_properties_t    *tbl;
    std::string                full_path;

    full_path =  std::string(cfg->cfg_path) + "/" +
                     std::string(cfg->table_map_cfg_file);

    std::ifstream tbl_json(full_path.c_str());
    read_json(tbl_json, json_pt);
    boost::optional<pt::ptree&>table_pt = json_pt.get_child_optional(JSON_KEY_TABLES);
    if (!table_pt) {
        // Error
        return P4PD_FAIL;
    }

    int num_tables = p4pd_txdma_tableid_max_get();
    _p4plus_txdma_tbls = (p4pd_table_properties_t*)P4PD_CALLOC(num_tables,
                                                sizeof(p4pd_table_properties_t));
    if (!_p4plus_txdma_tbls) {
        // Error : TODO: Tracing...
        return P4PD_FAIL;
    }

    // Iterator over all p4 tables packing data and build book keeping
    // DS used to read/write to device.
    BOOST_FOREACH(pt::ptree::value_type &p4_tbl, json_pt.get_child(JSON_KEY_TABLES)) {
        std::string tablename = p4_tbl.second.get<std::string>(JSON_KEY_TABLE_NAME);
        std::string match_type = p4_tbl.second.get<std::string>(JSON_KEY_MATCH_TYPE);
        std::string direction = p4_tbl.second.get<std::string>(JSON_KEY_DIRECTION);
        std::string overflow  = p4_tbl.second.get<std::string>(JSON_KEY_OVERFLOW);
        std::string overflow_parent  = p4_tbl.second.get<std::string>(JSON_KEY_OVERFLOW_PARENT);

        int tableid = p4pluspd_txdma_get_tableid_from_tablename(tablename.c_str());
        if (tableid == -1) {
            // Error..
            return P4PD_FAIL;
        }

        tbl = _p4plus_txdma_tbls + tableid;

        tbl->key_struct_size = p4pd_txdma_tbl_swkey_size[tableid];
        tbl->actiondata_struct_size = p4pd_txdma_tbl_sw_action_data_size[tableid];

        if (strlen(overflow.c_str())) {
            tbl->has_oflow_table = true;
            tbl->oflow_table_id = p4pluspd_txdma_get_tableid_from_tablename(overflow.c_str());
        } else {
            tbl->has_oflow_table = false;
        }
        if (strlen(overflow_parent.c_str())) {
            tbl->is_oflow_table = true;
        } else {
            tbl->is_oflow_table = false;
        }


        tbl->tablename = (char*)p4pd_txdma_tbl_names[tableid];
        tbl->tableid = tableid;

        tbl->table_type = p4pluspd_txdma_get_table_type(match_type.c_str());
        tbl->gress = p4pluspd_txdma_get_table_direction(direction.c_str());
        tbl->hash_type = p4_tbl.second.get<int>(JSON_KEY_HASH_TYPE);

        tbl->stage = p4_tbl.second.get<int>(JSON_KEY_STAGE);
        tbl->stage_tableid = p4_tbl.second.get<int>(JSON_KEY_STAGE_TBL_ID); 
        tbl->tabledepth = p4_tbl.second.get<int>(JSON_KEY_NUM_ENTRIES); 
        tbl->pipe = P4_PIPELINE_TXDMA;

        /* Memory units used by the table */
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

void
p4pluspd_txdma_cleanup (void)
{
    P4PD_FREE(_p4plus_txdma_tbls);
    _p4plus_txdma_tbls = NULL;
}

p4pd_error_t
p4pluspd_txdma_init (p4pd_cfg_t *cfg)
{
    if (_p4plus_txdma_tbls) {
        return P4PD_SUCCESS;
    }

    p4pd_txdma_prep_p4tbl_names();
    p4pd_txdma_prep_p4tbl_sw_struct_sizes();

    if (p4pluspd_txdma_tbl_packing_json_parse(cfg) != P4PD_SUCCESS) {
        P4PD_FREE(_p4plus_txdma_tbls);
        _p4plus_txdma_tbls = NULL;
        return P4PD_FAIL;
    }
    return P4PD_SUCCESS;
}

//----------------------------------------------------------------------------
// set hbm address and mapped address
//----------------------------------------------------------------------------
void
p4pd_txdma_hbm_table_address_set (uint32_t tableid, mem_addr_t pa,
                                  mem_addr_t va, p4pd_table_cache_t cache)
{
    p4pd_table_properties_t *tbl;

    tbl = _p4plus_txdma_tbls + tableid;
    tbl->base_mem_pa = pa;
    tbl->base_mem_va = va;
    tbl->cache = cache;
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
p4pluspd_txdma_table_properties_get (uint32_t tableid,
                                     p4pd_table_properties_t *tbl_ctx)
{
    if (tableid >= p4pd_txdma_tableid_max_get() || !_p4plus_txdma_tbls) {
        return P4PD_FAIL;
    }

    // Until json parsing is fixed comment out following line.
    memcpy(tbl_ctx, _p4plus_txdma_tbls + tableid, sizeof(p4pd_table_properties_t));
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
 *  OUT : p4pd_table_ctx_t **table_ctx : Returns a ptr to structure of data
 *                                       that contains table properties.
 * Return Value:
 *  P4PD_SUCCESS                       : Table properties copied into tbl_ctx
 *                                       Memory for tbl_ctx is provided by
 *                                       API caller.
 *
 *  P4PD_FAIL                          : If tableid is not valid
 */
p4pd_error_t
p4pluspd_txdma_table_properties_optimal_get (uint32_t tableid,
                                            p4pd_table_properties_t **tbl_ctx)
{
    if (tableid >= p4pd_txdma_tableid_max_get() || !_p4plus_txdma_tbls) {
        return P4PD_FAIL;
    }

    *tbl_ctx = (p4pd_table_properties_t *) (_p4plus_txdma_tbls + tableid);
    
    return P4PD_SUCCESS;
}
