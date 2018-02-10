// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/gen/include/p4pd_table.h"
#include "nic/gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "nic/gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/p4/nw/include/defines.h"

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

static p4pd_table_properties_t *_p4tbls;
namespace pt = boost::property_tree;

/* START: Common capri inits - applicable to IRIS & GFT */
/* TODO: These functions need to be moved to asic-pd common layer */
/* Store base address for the table. */
static uint64_t capri_table_asm_err_offset[P4TBL_ID_TBLMAX];
static uint64_t capri_table_asm_base[P4TBL_ID_TBLMAX];
hal_ret_t
p4pd_capri_program_table_mpu_pc (void)
{
    p4pd_table_properties_t       tbl_ctx;
    for (int i = P4TBL_ID_TBLMIN; i < P4TBL_ID_TBLMAX; i++) {
        p4pd_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.is_oflow_table &&
            tbl_ctx.table_type == P4_TBL_TYPE_TCAM) {
            // OTCAM and hash table share the same table id
            // so mpu_pc shouldn't be overwritten
            continue;
        }
        capri_program_table_mpu_pc(tbl_ctx.tableid,
                                   (tbl_ctx.gress == P4_GRESS_INGRESS),
                                   tbl_ctx.stage,
                                   tbl_ctx.stage_tableid,
                                   capri_table_asm_err_offset[i],
                                   capri_table_asm_base[i]);
    }
    return HAL_RET_OK;
}

#define P4ACTION_NAME_MAX_LEN (100)
static hal_ret_t
p4pd_capri_table_mpu_base_init (p4pd_cfg_t *p4pd_cfg)
{
    char action_name[P4ACTION_NAME_MAX_LEN] = {0};
    char progname[P4ACTION_NAME_MAX_LEN] = {0};
    uint64_t capri_action_asm_base;
    //p4pd_table_properties_t tbl_ctx;

    HAL_TRACE_DEBUG("In p4pd_capri_table_mpu_base_init\n");
    for (int i = P4TBL_ID_TBLMIN; i < P4TBL_ID_TBLMAX; i++) {
        snprintf(progname, P4ACTION_NAME_MAX_LEN, "%s%s", p4pd_tbl_names[i], ".bin");
        capri_program_to_base_addr(p4pd_cfg->p4pd_pgm_name, progname,
                                   &capri_table_asm_base[i]);
        for (int j = 0; j < p4pd_get_max_action_id(i); j++) {
            p4pd_get_action_name(i, j, action_name);
            capri_action_asm_base = 0;
            capri_program_label_to_offset(p4pd_cfg->p4pd_pgm_name, progname,
                                          action_name, &capri_action_asm_base);
            /* Action base is in byte and 64B aligned... */
            HAL_ASSERT((capri_action_asm_base & 0x3f) == 0);
            capri_action_asm_base >>= 6;
            HAL_TRACE_DEBUG("Program-Name {}, Action-Name {}, Action-Pc {:#x}",
                            progname, action_name, capri_action_asm_base);
            capri_set_action_asm_base(i, j, capri_action_asm_base);
        }

        /* compute error program offset for each table */
        snprintf(action_name, P4ACTION_NAME_MAX_LEN, "%s_error",
                 p4pd_tbl_names[i]);
        capri_program_label_to_offset(p4pd_cfg->p4pd_pgm_name, progname,
                                      action_name,
                                      &capri_table_asm_err_offset[i]);
        HAL_ASSERT((capri_table_asm_err_offset[i] & 0x3f) == 0);
        capri_table_asm_err_offset[i] >>= 6;
        HAL_TRACE_DEBUG("Program-Name {}, Action-Name {}, Action-Pc {:#x}",
                        progname, action_name, capri_table_asm_err_offset[i]);
    }
    return HAL_RET_OK;
}

static hal_ret_t
p4pd_capri_deparser_init (void)
{
    capri_deparser_init(TM_PORT_INGRESS, TM_PORT_EGRESS);
    return HAL_RET_OK;    
}

static hal_ret_t
p4pd_capri_program_hbm_table_base_addr (void)
{
    p4pd_table_properties_t       tbl_ctx;
    /* Program table base address into capri TE */
    for (int i = P4TBL_ID_TBLMIN; i < P4TBL_ID_TBLMAX; i++) {
        p4pd_global_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.table_location != P4_TBL_LOCATION_HBM) {
            continue;
        }
        capri_program_hbm_table_base_addr(tbl_ctx.stage_tableid,
                    tbl_ctx.tablename, tbl_ctx.stage,
                    (tbl_ctx.gress == P4_GRESS_INGRESS));
    }
    return HAL_RET_OK;
}

hal_ret_t
p4pd_capri_toeplitz_init (void)
{
    p4pd_table_properties_t tbl_ctx;
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_INDIR,
                                     &tbl_ctx);
    capri_toeplitz_init(tbl_ctx.stage, tbl_ctx.stage_tableid);
    return HAL_RET_OK;
}

hal_ret_t
p4pd_capri_p4plus_table_init (void)
{
    p4pd_table_properties_t tbl_ctx_apphdr;
    p4pd_table_properties_t tbl_ctx_apphdr_off;
    p4pd_table_properties_t tbl_ctx_txdma_act;

    /* P4 plus table inits */
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE,
                                     &tbl_ctx_apphdr);
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_OFFSET_64,
                                     &tbl_ctx_apphdr_off);
    p4pd_global_table_properties_get(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_TABLE_S0_T0,
                                     &tbl_ctx_txdma_act);
    capri_p4plus_table_init(tbl_ctx_apphdr.stage,
                            tbl_ctx_apphdr.stage_tableid,
                            tbl_ctx_apphdr_off.stage,
                            tbl_ctx_apphdr_off.stage_tableid,
                            tbl_ctx_txdma_act.stage,
                            tbl_ctx_txdma_act.stage_tableid);
    return HAL_RET_OK;
}

hal_ret_t
p4pd_capri_p4plus_recirc_init (void)
{
    capri_p4plus_recirc_init();
    return HAL_RET_OK;
}

hal_ret_t
p4pd_capri_timer_init (void)
{
    capri_timer_init();
    return HAL_RET_OK;
}

/* END: Common capri inits - applicable to IRIS & GFT */

static uint16_t
p4pd_get_tableid_from_tablename (const char *tablename)
{
    for (int i = P4TBL_ID_TBLMIN; i < P4TBL_ID_TBLMAX; i++) {
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
    int                        tableid, num_tables = P4TBL_ID_TBLMAX;
    char                       *cfg_path;

    // TODO: man, this logic shouldn't be in this lib ...
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
         full_path =  std::string(cfg_path) + "/" + p4pd_cfg->table_map_cfg_file;
     } else {
         printf("Please specify HAL_CONFIG_PATH env. variable ... ");
         exit(0);
     }

    std::ifstream tbl_json(full_path.c_str());
    read_json(tbl_json, json_pt);
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

//-----------------------------------------------------------------------------
// P4PD asic common initialization
//-----------------------------------------------------------------------------
p4pd_error_t
p4pd_asic_init (p4pd_cfg_t *p4pd_cfg)
{
    // common capri inits - applicable to all pipelines (iris, gft etc.)
    /* TODO: These functions need to be moved to asic-pd common layer */
    HAL_ASSERT(p4pd_capri_table_mpu_base_init(p4pd_cfg) == HAL_RET_OK);
    HAL_ASSERT(p4pd_capri_program_table_mpu_pc() == HAL_RET_OK);
    HAL_ASSERT(p4pd_capri_deparser_init() == HAL_RET_OK);
    HAL_ASSERT(p4pd_capri_program_hbm_table_base_addr() == HAL_RET_OK);
    return P4PD_SUCCESS;
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
    if (tableid >= P4TBL_ID_TBLMAX || !_p4tbls) {
        return P4PD_FAIL;
    }
    memcpy(tbl_ctx, _p4tbls + tableid, sizeof(p4pd_table_properties_t));
    return P4PD_SUCCESS;
}

