#include <hal_state_pd.hpp>
#include <tenant_pd.hpp>
#include <nwsec_pd.hpp>
#include <l2seg_pd.hpp>
#include <lif_pd.hpp>
#include <endpoint_pd.hpp>
#include <uplinkif_pd.hpp>
#include <uplinkpc_pd.hpp>
#include <enicif_pd.hpp>
#include <session_pd.hpp>
#include <buf_pool_pd.hpp>
#include <queue_pd.hpp>
#include <policer_pd.hpp>
#include <acl_pd.hpp>
#include <pd.hpp>
#include <p4pd_api.hpp>
#include <p4pd.h>
#include <hal_pd.hpp>
#include <asic_rw.hpp>
#include <tlscb_pd.hpp>
#include <tcpcb_pd.hpp>
#include <wring_pd.hpp>
#include <proxy.hpp>

namespace hal {
namespace pd {

class hal_state_pd *g_hal_state_pd;

//------------------------------------------------------------------------------
// init() function to instantiate all the slabs
//------------------------------------------------------------------------------
bool
hal_state_pd::init(void)
{
    uint32_t p, n;

    // initialize tenant related data structures
    tenant_slab_ = slab::factory("Tenant PD", HAL_SLAB_LIF_PD,
                                 sizeof(hal::pd::pd_tenant_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((tenant_slab_ != NULL), false);

    tenant_hwid_idxr_ = new hal::utils::indexer(HAL_MAX_HW_VRFS);
    HAL_ASSERT_RETURN((tenant_hwid_idxr_ != NULL), false);

    tenant_hwid_ht_ = ht::factory(HAL_MAX_HW_VRFS,
                                  hal::pd::tenant_pd_get_hw_key_func,
                                  hal::pd::tenant_pd_compute_hw_hash_func,
                                  hal::pd::tenant_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((tenant_hwid_ht_ != NULL), false);

    // initialize security related data structures
    nwsec_profile_hwid_idxr_ = new hal::utils::indexer(HAL_MAX_HW_NWSEC_PROFILES);
    HAL_ASSERT_RETURN((nwsec_profile_hwid_idxr_ != NULL), false);

    // initialize l2seg related data structures
    l2seg_slab_ = slab::factory("L2Segment PD", HAL_SLAB_LIF_PD,
                                 sizeof(hal::pd::pd_l2seg_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((l2seg_slab_ != NULL), false);

    l2seg_hwid_idxr_ = new hal::utils::indexer(HAL_MAX_HW_L2SEGMENTS);
    HAL_ASSERT_RETURN((l2seg_hwid_idxr_ != NULL), false);

    l2seg_hwid_ht_ = ht::factory(HAL_MAX_HW_L2SEGMENTS,
                                 hal::pd::l2seg_pd_get_hw_key_func,
                                 hal::pd::l2seg_pd_compute_hw_hash_func,
                                 hal::pd::l2seg_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((l2seg_hwid_ht_ != NULL), false);

    // initialize LIF PD related data structures
    lif_pd_slab_ = slab::factory("LIF_PD", HAL_SLAB_LIF_PD,
                                 sizeof(hal::pd::pd_lif_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((lif_pd_slab_ != NULL), false);

    lif_hwid_idxr_ = new hal::utils::indexer(HAL_MAX_HW_LIFS);
    HAL_ASSERT_RETURN((lif_hwid_idxr_ != NULL), false);

    // initialize Uplink If PD related data structures
    uplinkif_pd_slab_ = slab::factory("UPLINKIF_PD", HAL_SLAB_UPLINKIF_PD,
                                 sizeof(hal::pd::pd_uplinkif_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((uplinkif_pd_slab_ != NULL), false);

    // initialize Uplink PC PD related data structures
    uplinkpc_pd_slab_ = slab::factory("UPLINKPC_PD", HAL_SLAB_UPLINKPC_PD,
                                 sizeof(hal::pd::pd_uplinkpc_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((uplinkpc_pd_slab_ != NULL), false);

    uplinkifpc_idxr_ = new hal::utils::indexer(HAL_MAX_UPLINK_IF_PCS);
    HAL_ASSERT_RETURN((uplinkifpc_idxr_ != NULL), false);

    // initialize ENIC If PD related data structures
    enicif_pd_slab_ = slab::factory("ENICIF_PD", HAL_SLAB_ENICIF_PD,
                                 sizeof(hal::pd::pd_enicif_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((enicif_pd_slab_ != NULL), false);

    // initialize EP PD related data structures
    ep_pd_slab_ = slab::factory("EP_PD", HAL_SLAB_EP_PD,
                                 sizeof(hal::pd::pd_ep_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((ep_pd_slab_ != NULL), false);

    // initialize EP PD l3 entry related data structures
    ep_pd_ip_entry_slab_ = slab::factory("EP_PD_IP_ENTRY", HAL_SLAB_EP_IP_ENTRY_PD,
                                 sizeof(hal::pd::pd_ep_ip_entry_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((ep_pd_ip_entry_slab_ != NULL), false);

    rw_table_idxr_ = new hal::utils::indexer(HAL_RW_TABLE_SIZE);
    HAL_ASSERT_RETURN((rw_table_idxr_ != NULL), false);

    // initialize nwsec PD related data structures
    nwsec_pd_slab_ = slab::factory("NWSEC_PD", HAL_SLAB_SECURITY_PROFILE_PD,
                                 sizeof(hal::pd::pd_nwsec_profile_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((nwsec_pd_slab_ != NULL), false);

    // initialize session related data structures
    session_slab_ = slab::factory("Session PD", HAL_SLAB_SESSION_PD,
                                  sizeof(hal::pd::pd_session_t), 128,
                                  true, true, true, true);
    HAL_ASSERT_RETURN((session_slab_ != NULL), false);

    // initialize Buf-Pool PD related data structures
    buf_pool_pd_slab_ = slab::factory("BUF_POOL_PD", HAL_SLAB_BUF_POOL_PD,
                                      sizeof(hal::pd::pd_buf_pool_t), 8,
                                      false, true, true, true);
    HAL_ASSERT_RETURN((buf_pool_pd_slab_ != NULL), false);

    for (p = 0; p < HAL_MAX_TM_PORTS; p++) {
        buf_pool_hwid_idxr_[p] = new hal::utils::indexer(HAL_MAX_HW_BUF_POOLS_PER_PORT);
        HAL_ASSERT_RETURN((buf_pool_hwid_idxr_[p] != NULL), false);
    }

    // initialize Queue PD related data structures
    queue_pd_slab_ = slab::factory("QUEUE_PD", HAL_SLAB_QUEUE_PD,
                                   sizeof(hal::pd::pd_queue_t), 8,
                                   false, true, true, true);
    HAL_ASSERT_RETURN((queue_pd_slab_ != NULL), false);

    for (p = 0; p < HAL_MAX_TM_PORTS; p++) {
        for (n = 0; n < HAL_HW_OQUEUE_NODE_TYPES; n++) {
            queue_hwid_idxr_[p][n] = 
                new hal::utils::indexer(queue_count_by_node_type(p, (queue_node_type_e)n));
            HAL_ASSERT_RETURN((queue_hwid_idxr_[p][n] != NULL), false);
        }
    }

    // initialize Policer PD related data structures
    policer_pd_slab_ = slab::factory("POLICER_PD", HAL_SLAB_POLICER_PD,
                                     sizeof(hal::pd::pd_policer_t), 8,
                                     false, true, true, true);
    HAL_ASSERT_RETURN((policer_pd_slab_ != NULL), false);

    ingress_policer_hwid_idxr_ = new hal::utils::indexer(HAL_MAX_HW_INGRESS_POLICERS);
    HAL_ASSERT_RETURN((ingress_policer_hwid_idxr_ != NULL), false);

    egress_policer_hwid_idxr_ = new hal::utils::indexer(HAL_MAX_HW_EGRESS_POLICERS);
    HAL_ASSERT_RETURN((egress_policer_hwid_idxr_ != NULL), false);
    
    // initialize TLSCB related data structures
    tlscb_slab_ = slab::factory("TLSCB PD", HAL_SLAB_TLSCB_PD,
                                sizeof(hal::pd::pd_tlscb_t), 128,
                                true, true, true, true);
    HAL_ASSERT_RETURN((tlscb_slab_ != NULL), false);

    tlscb_hwid_ht_ = ht::factory(HAL_MAX_HW_TLSCBS,
                                 hal::pd::tlscb_pd_get_hw_key_func,
                                 hal::pd::tlscb_pd_compute_hw_hash_func,
                                 hal::pd::tlscb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((tlscb_hwid_ht_ != NULL), false);

    // initialize TCPCB related data structures
    tcpcb_slab_ = slab::factory("TCPCB PD", HAL_SLAB_TCPCB_PD,
                                 sizeof(hal::pd::pd_tcpcb_t), 128,
                                 true, true, true, true);
    HAL_ASSERT_RETURN((tcpcb_slab_ != NULL), false);

    tcpcb_hwid_ht_ = ht::factory(HAL_MAX_HW_TCPCBS,
                                 hal::pd::tcpcb_pd_get_hw_key_func,
                                 hal::pd::tcpcb_pd_compute_hw_hash_func,
                                 hal::pd::tcpcb_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((tcpcb_hwid_ht_ != NULL), false);

    // initialize Acl PD related data structures
    acl_pd_slab_ = slab::factory("ACL_PD", HAL_SLAB_ACL_PD,
                                 sizeof(hal::pd::pd_acl_t), 8,
                                 false, true, true, true);
    HAL_ASSERT_RETURN((acl_pd_slab_ != NULL), false);

    // initialize WRING related data structures
    wring_slab_ = slab::factory("WRING PD", HAL_SLAB_WRING_PD,
                                 sizeof(hal::pd::pd_wring_t), 128,
                                 true, true, true, true);
    HAL_ASSERT_RETURN((wring_slab_ != NULL), false);

    wring_hwid_ht_ = ht::factory(HAL_MAX_HW_WRING,
                                 hal::pd::wring_pd_get_hw_key_func,
                                 hal::pd::wring_pd_compute_hw_hash_func,
                                 hal::pd::wring_pd_compare_hw_key_func);
    HAL_ASSERT_RETURN((wring_hwid_ht_ != NULL), false);


    dm_tables_ = NULL;
    hash_tcam_tables_ = NULL;
    tcam_tables_ = NULL;
    flow_table_ = NULL;
    met_table_ = NULL;
    acl_table_ = NULL;

    return true;
}

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
hal_state_pd::hal_state_pd()
{
    uint32_t p, n;

    tenant_slab_ = NULL;
    tenant_hwid_idxr_ = NULL;
    tenant_hwid_ht_ = NULL;

    nwsec_profile_hwid_idxr_ = NULL;

    l2seg_slab_ = NULL;
    l2seg_hwid_idxr_ = NULL;
    l2seg_hwid_ht_ = NULL;

    lif_pd_slab_ = NULL;
    lif_hwid_idxr_ = NULL;

    uplinkif_pd_slab_ = NULL;
    uplinkpc_pd_slab_ = NULL;
    uplinkifpc_idxr_ = NULL;

    enicif_pd_slab_ = NULL;

    ep_pd_slab_ = NULL;
    ep_pd_ip_entry_slab_ = NULL;

    nwsec_pd_slab_ = NULL;

    session_slab_ = NULL;

    buf_pool_pd_slab_ = NULL;

    for (p = 0; p < HAL_MAX_TM_PORTS; p++) {
        buf_pool_hwid_idxr_[p] = NULL;
    }

    queue_pd_slab_ = NULL;
    
    for (p = 0; p < HAL_MAX_TM_PORTS; p++) {
        for (n = 0; n < HAL_HW_OQUEUE_NODE_TYPES; n++) {
            queue_hwid_idxr_[p][n] = NULL;
        }
    }

    policer_pd_slab_ = NULL;
    ingress_policer_hwid_idxr_ = NULL;
    egress_policer_hwid_idxr_ = NULL;

    acl_pd_slab_ = NULL;
    
    tlscb_slab_ = NULL;
    tlscb_hwid_ht_ = NULL;
    
    tcpcb_slab_ = NULL;
    tcpcb_hwid_ht_ = NULL;
    
    wring_slab_ = NULL;
    wring_hwid_ht_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_state_pd::~hal_state_pd()
{
    uint32_t    tid;
    uint32_t    p, n;

    tenant_slab_ ? delete tenant_slab_ : HAL_NOP;
    tenant_hwid_idxr_ ? delete tenant_hwid_idxr_ : HAL_NOP;
    tenant_hwid_ht_ ? delete tenant_hwid_ht_ : HAL_NOP;

    nwsec_profile_hwid_idxr_ ? delete nwsec_profile_hwid_idxr_ : HAL_NOP;

    l2seg_slab_ ? delete l2seg_slab_ : HAL_NOP;
    l2seg_hwid_idxr_ ? delete l2seg_hwid_idxr_ : HAL_NOP;
    l2seg_hwid_ht_ ? delete l2seg_hwid_ht_ : HAL_NOP;

    lif_pd_slab_ ? delete lif_pd_slab_ : HAL_NOP;
    lif_hwid_idxr_ ? delete lif_hwid_idxr_ : HAL_NOP;

    uplinkif_pd_slab_ ? delete uplinkif_pd_slab_ : HAL_NOP;
    uplinkpc_pd_slab_ ? delete uplinkpc_pd_slab_ : HAL_NOP;
    uplinkifpc_idxr_ ? delete uplinkifpc_idxr_ : HAL_NOP;

    nwsec_pd_slab_ ? delete nwsec_pd_slab_ : HAL_NOP;

    session_slab_ ? delete session_slab_ : HAL_NOP;
    
    tlscb_slab_ ? delete tlscb_slab_ : HAL_NOP;
    tlscb_hwid_ht_ ? delete tlscb_hwid_ht_ : HAL_NOP;
    
    tcpcb_slab_ ? delete tcpcb_slab_ : HAL_NOP;
    tcpcb_hwid_ht_ ? delete tcpcb_hwid_ht_ : HAL_NOP;

    buf_pool_pd_slab_ ? delete buf_pool_pd_slab_ : HAL_NOP;
    for (p = 0; p < HAL_MAX_TM_PORTS; p++) {
        buf_pool_hwid_idxr_[p] ? delete buf_pool_hwid_idxr_[p] : HAL_NOP;
    }

    queue_pd_slab_ ? delete queue_pd_slab_ : HAL_NOP;
    for (p = 0; p < HAL_MAX_TM_PORTS; p++) {
        for (n = 0; n < HAL_HW_OQUEUE_NODE_TYPES; n++) {
            queue_hwid_idxr_[p][n] ? delete queue_hwid_idxr_[p][n] : HAL_NOP;
        }
    }

    policer_pd_slab_ ? delete policer_pd_slab_ : HAL_NOP;
    ingress_policer_hwid_idxr_ ? delete ingress_policer_hwid_idxr_ : HAL_NOP;
    egress_policer_hwid_idxr_ ? delete egress_policer_hwid_idxr_ : HAL_NOP;

    acl_pd_slab_ ? delete acl_pd_slab_ : HAL_NOP;

    wring_slab_ ? delete wring_slab_ : HAL_NOP;
    wring_hwid_ht_ ? delete wring_hwid_ht_ : HAL_NOP;


    if (dm_tables_) {
        for (tid = P4TBL_ID_INDEX_MIN; tid < P4TBL_ID_INDEX_MAX; tid++) {
            if (dm_tables_[tid]) {
                delete dm_tables_[tid];
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, dm_tables_);
    }

    if (hash_tcam_tables_) {
        for (tid = P4TBL_ID_HASH_OTCAM_MIN;
             tid < P4TBL_ID_HASH_OTCAM_MAX; tid++) {
            if (hash_tcam_tables_[tid]) {
                delete hash_tcam_tables_[tid];
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, hash_tcam_tables_);
    }

    if (tcam_tables_) {
        for (tid = P4TBL_ID_TCAM_MIN; tid < P4TBL_ID_TCAM_MIN; tid++) {
            if (tcam_tables_[tid]) {
                delete tcam_tables_[tid];
            }
        }
        HAL_FREE(HAL_MEM_ALLOC_PD, tcam_tables_);
    }

    if (flow_table_) {
        delete flow_table_;
    }

    if (met_table_) {
        delete met_table_;
    }

    if (acl_table_) {
        delete acl_table_;
    }
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
hal_state_pd *
hal_state_pd::factory(void)
{
    hal_state_pd *state;

    state = new hal_state_pd();
    HAL_ASSERT_RETURN((state != NULL), NULL);
    if (state->init() == false) {
        delete state;
        return NULL;
    }
    return state;
}

//------------------------------------------------------------------------------
// convert P4 table type to corresponding string
//------------------------------------------------------------------------------
static const char *
p4pd_table_type2str (p4pd_table_type_en table_type)
{
    switch (table_type) {
    case P4_TBL_TYPE_HASH:
        return "EXACT_HASH";
        break;
    case P4_TBL_TYPE_HASHTCAM:
        return "EXACT_HASH_OTCAM";
        break;

    case P4_TBL_TYPE_TCAM:
        return "TCAM";
        break;

    case P4_TBL_TYPE_INDEX:
        return "EXACT_IDX";
        break;

    case P4_TBL_TYPE_MPU:
        return "MPU";
        break;

    default:
        return "Invalid";
    }
}

#define P4PD_NUM_INGRESS_STAGES        6
#define P4PD_NUM_EGRESS_STAGES         6

//------------------------------------------------------------------------------
// dump all P4 PD table detailed information
//------------------------------------------------------------------------------
static void
p4pd_table_info_dump_ (void)
{
    uint32_t                   tid, stage = 0;
    p4pd_table_properties_t    tinfo;

    // print the header
    printf("======================================================================\n");
    printf("%-25s\tId\t%-18sIn/Egress\tStage\tStage Tbl Id\tSize\n",
           "Table", "Type");
    printf("======================================================================\n");

    // dump ingress tables stage-by-stage first
    while (stage < P4PD_NUM_INGRESS_STAGES) {
        for (tid = P4TBL_ID_TBLMIN; tid < P4TBL_ID_TBLMAX; tid++) {
            p4pd_table_properties_get(tid, &tinfo);
            if ((tinfo.gress != P4_GRESS_INGRESS) || (tinfo.stage != stage)) {
                continue;
            }
            printf("%-25s\t%u\t%-18s%s\t\t%u\t%u\t\t%u\n",
                   tinfo.tablename, tid, p4pd_table_type2str(tinfo.table_type),
                   (tinfo.gress == P4_GRESS_INGRESS) ? "ING" : "EGR",
                   tinfo.stage,
                   tinfo.stage_tableid, tinfo.tabledepth);
        }
        stage++;
    }

    // dump engress tables stage-by-stage now
    stage = 0;
    while (stage < P4PD_NUM_EGRESS_STAGES) {
        for (tid = P4TBL_ID_TBLMIN; tid < P4TBL_ID_TBLMAX; tid++) {
            p4pd_table_properties_get(tid, &tinfo);
            if ((tinfo.gress != P4_GRESS_EGRESS) || (tinfo.stage != stage)) {
                continue;
            }
            printf("%-25s\t%u\t%-18s%s\t\t%u\t%u\t\t%u\n",
                   tinfo.tablename, tid, p4pd_table_type2str(tinfo.table_type),
                   (tinfo.gress == P4_GRESS_INGRESS) ? "ING" : "EGR",
                   tinfo.stage,
                   tinfo.stage_tableid, tinfo.tabledepth);
        }
        stage++;
    }
    printf("======================================================================\n");
}

//------------------------------------------------------------------------------
// initializing tables
//------------------------------------------------------------------------------i
hal_ret_t
hal_state_pd::init_tables(void)
{
    uint32_t                   tid;
    hal_ret_t                  ret = HAL_RET_OK;
    p4pd_table_properties_t    tinfo, ctinfo;

    memset(&tinfo, 0, sizeof(tinfo));
    memset(&ctinfo, 0, sizeof(ctinfo));

    // parse the NCC generated table info file
    p4pd_init();

    // start instantiating tables based on the parsed information
    dm_tables_ =
        (DirectMap **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                                 sizeof(DirectMap *) *
                                 (P4TBL_ID_INDEX_MAX - P4TBL_ID_INDEX_MIN + 1));
    HAL_ASSERT(dm_tables_ != NULL);

    // make sure there is one flow table only
    HAL_ASSERT((P4TBL_ID_HASH_MAX - P4TBL_ID_HASH_MIN + 1) == 1);

    hash_tcam_tables_ =
        (Hash **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                            sizeof(Hash *) *
                            (P4TBL_ID_HASH_OTCAM_MAX - P4TBL_ID_HASH_OTCAM_MIN + 1));
    HAL_ASSERT(hash_tcam_tables_ != NULL);

    tcam_tables_ =
        (Tcam **)HAL_CALLOC(HAL_MEM_ALLOC_PD,
                            sizeof(Tcam *) *
                            (P4TBL_ID_TCAM_MAX - P4TBL_ID_TCAM_MIN + 1));
    HAL_ASSERT(tcam_tables_ != NULL);

    // for debugging
    p4pd_table_info_dump_();

    // TODO:
    // 1. take care of instantiating flow_table_, acl_table_ and met_table_
    // 2. When tables are instantiated proper names are not passed today,
    // waiting for an API from Mahesh that gives table name given table id

    for (tid = P4TBL_ID_TBLMIN; tid < P4TBL_ID_TBLMAX; tid++) {
        p4pd_table_properties_get(tid, &tinfo);
        switch (tinfo.table_type) {
        case P4_TBL_TYPE_HASHTCAM:
            if (tinfo.has_oflow_table) {
                p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
            }
            hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN] =
                new Hash(tinfo.tablename, tid,
                         tinfo.oflow_table_id,
                         tinfo.tabledepth,
                         tinfo.has_oflow_table ? ctinfo.tabledepth : 0,
                         tinfo.key_struct_size,
                         tinfo.actiondata_struct_size,
                         static_cast<Hash::HashPoly>(tinfo.hash_type));
            HAL_ASSERT(hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN] != NULL);
            break;

        case P4_TBL_TYPE_TCAM:
            if (tid == P4TBL_ID_NACL) {
                acl_table_ = acl_tcam::factory(tinfo.tablename, tid, tinfo.tabledepth,
                                               tinfo.key_struct_size, 
                                               tinfo.actiondata_struct_size, true);
                HAL_ASSERT(acl_table_ != NULL);
            } else {
                if (!tinfo.is_oflow_table) {
                    tcam_tables_[tid - P4TBL_ID_TCAM_MIN] =
                        new Tcam(tinfo.tablename, tid, tinfo.tabledepth,
                                 tinfo.key_struct_size, tinfo.actiondata_struct_size, false);
                    HAL_ASSERT(tcam_tables_[tid - P4TBL_ID_TCAM_MIN] != NULL);
                }
            }

            break;

        case P4_TBL_TYPE_INDEX:
            dm_tables_[tid - P4TBL_ID_INDEX_MIN] =
                new DirectMap(tinfo.tablename, tid, tinfo.tabledepth);
            HAL_ASSERT(dm_tables_[tid - P4TBL_ID_INDEX_MIN] != NULL);
            break;

        case P4_TBL_TYPE_HASH:
            HAL_ASSERT(tid == P4TBL_ID_FLOW_HASH);
            if (tinfo.has_oflow_table) {
                p4pd_table_properties_get(tinfo.oflow_table_id, &ctinfo);
            }
            flow_table_ =
                new Flow(tinfo.tablename, tid, ctinfo.oflow_table_id,
                         tinfo.tabledepth, ctinfo.tabledepth,
                         tinfo.key_struct_size,
                         sizeof(p4pd_flow_hash_data_t), 6,    // no. of hints
                         static_cast<Flow::HashPoly>(tinfo.hash_type));
            HAL_ASSERT(flow_table_ != NULL);
            break;

        case P4_TBL_TYPE_MPU:
        default:
            break;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// one time memory related initialization for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_pd_mem_init (void)
{
    hal_ret_t   ret = HAL_RET_OK;

    g_hal_state_pd = hal_state_pd::factory();
    HAL_ASSERT_RETURN((g_hal_state_pd != NULL), HAL_RET_ERR);

    HAL_TRACE_DEBUG("Initializing asic lib tables ...");
    ret = g_hal_state_pd->init_tables();
    if (ret != HAL_RET_OK) {
        delete g_hal_state_pd;
    }

    return ret;
}

//------------------------------------------------------------------------------
// program default entries in tables
//------------------------------------------------------------------------------
hal_ret_t
hal_pd_pgm_def_entries (void)
{
    hal_ret_t   ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Programming table default entries ...");
    ret = p4pd_table_defaults_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program default entries, err: {}", ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// program default entries for P4Plus tables
//------------------------------------------------------------------------------
hal_ret_t
hal_pd_pgm_def_p4plus_entries (void)
{
    hal_ret_t   ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Programming p4plus default entries ...");
    ret = wring_pd_init_global_rings();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program default rings, err: {}", ret);
    }
    return ret;
}


//------------------------------------------------------------------------------
// free an element back to given PD slab specified by its id
//------------------------------------------------------------------------------
hal_ret_t
free_to_slab (hal_slab_t slab_id, void *elem)
{
    switch (slab_id) {
    case HAL_SLAB_TENANT_PD:
        g_hal_state_pd->tenant_slab()->free_(elem);
        break;

    case HAL_SLAB_L2SEG_PD:
        g_hal_state_pd->l2seg_slab()->free_(elem);
        break;

    case HAL_SLAB_LIF_PD:
        g_hal_state_pd->lif_pd_slab()->free_(elem);
        break;

    case HAL_SLAB_UPLINKIF_PD:
        g_hal_state_pd->uplinkif_pd_slab()->free_(elem);
        break;

    case HAL_SLAB_UPLINKPC_PD:
        g_hal_state_pd->uplinkpc_pd_slab()->free_(elem);
        break;

    case HAL_SLAB_EP_PD:
        g_hal_state_pd->ep_pd_slab()->free_(elem);
        break;

    case HAL_SLAB_SESSION_PD:
        g_hal_state_pd->session_slab()->free_(elem);
        break;
 
    case HAL_SLAB_TLSCB_PD:
        g_hal_state_pd->tlscb_slab()->free_(elem);
        break;

    case HAL_SLAB_TCPCB_PD:
        g_hal_state_pd->tcpcb_slab()->free_(elem);
        break;

    case HAL_SLAB_BUF_POOL_PD:
        g_hal_state_pd->buf_pool_pd_slab()->free_(elem);
        break;

    case HAL_SLAB_QUEUE_PD:
        g_hal_state_pd->queue_pd_slab()->free_(elem);
        break;

    case HAL_SLAB_POLICER_PD:
        g_hal_state_pd->policer_pd_slab()->free_(elem);
        break;

    case HAL_SLAB_ACL_PD:
        g_hal_state_pd->acl_pd_slab()->free_(elem);
        break;

    case HAL_SLAB_WRING_PD:
        g_hal_state_pd->wring_slab()->free_(elem);
        break;

    default:
        HAL_TRACE_ERR("Unknown slab id {}", slab_id);
        HAL_ASSERT(FALSE);
        return HAL_RET_INVALID_ARG;
        break;
    }

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal

