//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// apulu pipeline implementation
///
//----------------------------------------------------------------------------

#ifndef __APULU_IMPL_HPP__
#define __APULU_IMPL_HPP__

#include <vector>
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/p4/loader/loader.hpp"
#include "nic/sdk/include/sdk/qos.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"
#include "gen/p4gen/apulu/include/p4pd.h"

// system wide blackhole nexthop
#define PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID    0

// reserved VNIC h/w id
#define PDS_IMPL_RSVD_VNIC_HW_ID               0

// reserved BD h/w id
#define PDS_IMPL_RSVD_BD_HW_ID                 0

// reserved VPC h/w id
#define PDS_IMPL_RSVD_VPC_HW_ID                0

// reserved NAT h/w id
#define PDS_IMPL_RSVD_NAT_HW_ID                0

// reserved index in IP_MAC_BINDING table
#define PDS_IMPL_RSVD_IP_MAC_BINDING_HW_ID     0

// reserved DHCP relay NACL index
// NOTE:
// this index must be between PDS_IMPL_NACL_BLOCK_HIGH_PRIO_DYNAMIC_MIN and
// PDS_IMPL_NACL_BLOCK_HIGH_PRIO_DYNAMIC_MAX
#define PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX1     0
#define PDS_IMPL_RSVD_DHCP_RELAY_NACL_IDX2     1

// policer refresh interval in micro seconds
#define PDS_POLICER_DEFAULT_REFRESH_INTERVAL   2000

// max policer token per interval
#define PDS_POLICER_MAX_TOKENS_PER_INTERVAL    ((1ull << 39) -1 )

// various lif types in the datapath
#define P4_LIF_TYPE_HOST                        0  // host data (i.e non-mgmt) lifs
#define P4_LIF_TYPE_ARM_DPDK                    1  // dpdk s/w datapath lifs
#define P4_LIF_TYPE_ARM_LEARN                   2  // dpdk learn lif
#define P4_LIF_TYPE_ARM_MGMT                    3  // inband & oob lifs on ARM
#define P4_LIF_TYPE_UPLINK                      4  // lifs corresponding to uplink
#define P4_LIF_TYPE_HOST_MGMT                   5  // host<->arm mgmt. lifs

// NACL table partitioning
#define PDS_IMPL_NACL_BLOCK_HIGH_PRIO_DYNAMIC_MIN    0
#define PDS_IMPL_NACL_BLOCK_HIGH_PRIO_DYNAMIC_MAX    7
#define PDS_IMPL_NACL_BLOCK_GLOBAL_MIN               8
#define PDS_IMPL_NACL_BLOCK_GLOBAL_MAX               15
#define PDS_IMPL_NACL_BLOCK_LEARN_MIN                16
#define PDS_IMPL_NACL_BLOCK_LEARN_MAX                23
#define PDS_IMPL_NACL_BLOCK_GENERIC_MIN              24

namespace api {
namespace impl {

#define bd_info action_u.bd_bd_info

#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(key, vpc_hw_id, ip)             \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->key_metadata_local_mapping_lkp_id = vpc_hw_id;                    \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        (key)->key_metadata_local_mapping_lkp_type = KEY_TYPE_IPV6;          \
        sdk::lib::memrev((key)->key_metadata_local_mapping_lkp_addr,         \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        (key)->key_metadata_local_mapping_lkp_type = KEY_TYPE_IPV4;          \
        memcpy((key)->key_metadata_local_mapping_lkp_addr,                   \
               &(ip)->addr.v4_addr, IP4_ADDR8_LEN);                          \
    }                                                                        \
}

#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(data, vnic_hw_id, xlate_idx,  \
                                               binding_checks_en,            \
                                               binding_idx1, binding_idx2,   \
                                               allow_tagged)                 \
{                                                                            \
    memset(data, 0, sizeof(*(data)));                                        \
    (data)->vnic_id = (vnic_hw_id);                                          \
    (data)->xlate_id = (xlate_idx);                                          \
    (data)->binding_check_enabled = (binding_checks_en);                     \
    (data)->binding_id1 = (binding_idx1);                                    \
    (data)->binding_id2 = (binding_idx1);                                    \
    (data)->allow_tagged_pkts = (allow_tagged);                              \
}

#define PDS_IMPL_FILL_IP_MAPPING_SWKEY(key, vpc_hw_id, ip)                   \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->p4e_i2e_mapping_lkp_id = (vpc_hw_id);                             \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        (key)->p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV6;                     \
        sdk::lib::memrev((key)->p4e_i2e_mapping_lkp_addr,                    \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        (key)->p4e_i2e_mapping_lkp_type = KEY_TYPE_IPV4;                     \
        memcpy((key)->p4e_i2e_mapping_lkp_addr,                              \
               &(ip)->addr.v4_addr, IP4_ADDR8_LEN);                          \
    }                                                                        \
}

#define PDS_IMPL_FILL_L2_MAPPING_SWKEY(key, bd_hw_id, mac)                   \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->p4e_i2e_mapping_lkp_id = (bd_hw_id);                              \
    (key)->p4e_i2e_mapping_lkp_type = KEY_TYPE_MAC;                          \
    sdk::lib::memrev((key)->p4e_i2e_mapping_lkp_addr, (mac), ETH_ADDR_LEN);  \
}

#define nat_action action_u.nat_nat_rewrite
#define PDS_IMPL_FILL_NAT_DATA(data, xlate_ip, xlate_port)                   \
{                                                                            \
    memset((data), 0, sizeof(*(data)));                                      \
    if ((xlate_ip)->af == IP_AF_IPV6) {                                      \
        sdk::lib::memrev((data)->ip,                                         \
                         (xlate_ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);     \
    } else {                                                                 \
        memcpy((data)->ip, &(xlate_ip)->addr.v4_addr,                        \
               IP4_ADDR8_LEN);                                               \
    }                                                                        \
    (data)->port = (xlate_port);                                             \
}

static inline void
nexthop_type_to_string (std::string &nexthop_str, uint8_t nexthop_type)
{
    switch (nexthop_type) {
    case NEXTHOP_TYPE_VPC:
        nexthop_str = "vpc";
        break;
    case NEXTHOP_TYPE_ECMP:
        nexthop_str = "ecmp";
        break;
    case NEXTHOP_TYPE_TUNNEL:
        nexthop_str = "tunnel";
        break;
    case NEXTHOP_TYPE_NEXTHOP:
        nexthop_str = "nexthop";
        break;
    default:
        nexthop_str = "-";
        break;
    }
}

/// \defgroup PDS_PIPELINE_IMPL - pipeline wrapper implementation
/// \ingroup PDS_PIPELINE
/// @{

/// \brief pipeline implementation
class apulu_impl : public pipeline_impl_base {
public:
    /// \brief     factory method to pipeline impl instance
    /// \param[in] pipeline_cfg pipeline configuration information
    /// \return    new instance of apulu pipeline impl or NULL,
    ///            in case of error
    static apulu_impl *factory(pipeline_cfg_t *pipeline_cfg);

    /// \brief     destroy method to pipeline impl instance
    /// \param[in] impl pointer to the allocated instance
    static void destroy(apulu_impl *impl);

    /// \brief     initialize program configuration
    /// \param[in] init_params initialization time parameters passed by app
    /// \param[in] asic_cfg    asic configuration to be populated with program
    ///                        information
    virtual void program_config_init(pds_init_params_t *init_params,
                                     asic_cfg_t *asic_cfg) override;

    /// \brief     initialize asm configuration
    /// \param[in] init_params initialization time parameters passed by app
    /// \param[in] asic_cfg    asic configuration to be populated with asm
    ///            information
    virtual void asm_config_init(pds_init_params_t *init_params,
                                 asic_cfg_t *asic_cfg) override;

    /// \brief     initialize ring configuration
    /// \param[in] asic_cfg    asic configuration to be populated with ring
    ///            information
    virtual void ring_config_init(asic_cfg_t *asic_cfg) override;

    /// \brief  init routine to initialize the pipeline
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t pipeline_init(void) override;

    /// \brief  routine to backup the states during the pipeline upgrade
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t upg_backup(void) override;

    /// \brief  routine to switchover during the pipeline upgrade
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t upg_switchover(void) override;

    /// \brief     generic API to write to rxdma tables
    /// \param[in] addr        memory address to write the data to
    /// \param[in] tableid     table id
    /// \param[in] action_id   action id to write
    /// \param[in] action_data action data to write
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t write_to_rxdma_table(mem_addr_t addr, uint32_t tableid,
                                   uint8_t action_id,
                                   void *actiondata) override;

    /// \brief     generic API to write to txdma tables
    /// \param[in] addr        memory address to write the data to
    /// \param[in] tableid     table id
    /// \param[in] action_id   action id to write
    /// \param[in] action_data action data to write
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t write_to_txdma_table(mem_addr_t addr, uint32_t tableid,
                                           uint8_t action_id,
                                           void *actiondata) override;

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t transaction_begin(void) override;

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t transaction_end(void) override;

    /// \brief     API to get table stats
    /// \param[in] cb   callback to be called on stats
    ///            ctxt opaque ctxt passed to the callback
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t table_stats(debug::table_stats_get_cb_t cb, void *ctxt)
            override;

    /// \brief      API to get session stats
    /// \param[in]  cb      callback to be called on stats
    /// \param[in]  lowidx  Low Index for stats to be read
    /// \param[in]  highidx High Index for stats to be read
    /// \param[in]  ctxt    Opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t session_stats(debug::session_stats_get_cb_t cb,
                                    uint32_t lowidx, uint32_t highidx,
                                    void *ctxt) override;

    /// \brief      API to get session
    /// \param[in]  cb      callback to be called on session
    /// \param[in]  ctxt    Opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t session(debug::session_get_cb_t cb, void *ctxt) override;

    /// \brief      API to get flow
    /// \param[in]  cb      callback to be called on flow
    /// \param[in]  ctxt    Opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t flow(debug::flow_get_cb_t cb, void *ctxt) override;

    /// \brief      API to clear session
    /// \param[in]  idx     Index for session to be cleared
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t session_clear(uint32_t idx) override;

    /// \brief API to handle CLI calls
    /// \param[in]  ctxt    CLI command context
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t handle_cmd(cmd_ctxt_t *ctxt) override;

    /// \brief      API to walk impl state slab
    /// \param[in]  walk_cb callback to be called on slab
    /// \param[in]  ctxt    opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t impl_state_slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

private:
    /// \brief constructor
    apulu_impl() {}

    /// \brief destructor
    ~apulu_impl() {}

    /// \brief     initialize an instance of apulu impl class
    /// \param[in] pipeline_cfg pipeline information
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t init_(pipeline_cfg_t *pipeline_cfg);

    /// \brief  init routine to initialize inter pipe table
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t inter_pipe_init_(void);

    /// \brief  initialize ingress drop stats table
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t ingress_drop_stats_init_(void);

    /// \brief  initialize egress drop stats table
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t egress_drop_stats_init_(void);

    /// \brief  initialize all the stats tables, where needed
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t stats_init_(void);

    /// \brief  initialize any global ACLs during init time
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t nacl_init_(void);

    /// \brief  initialize checksum table
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t checksum_init_(void);

    /// \brief  program all p4/p4+ tables that require one time initialization
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_init_(void);

    /// \brief     apulu specific mpu program sort function
    /// \param[in] program information
    static void sort_mpu_programs_(std::vector<std::string>& programs);

    /// \brief     apulu specific rxdma symbols init function
    /// \param[in] program information
    static uint32_t rxdma_symbols_init_(void **p4plus_symbols,
                                        platform_type_t platform_type);

    /// \brief     apulu specific txdma symbols init function
    /// \param[in] program information
    static uint32_t txdma_symbols_init_(void **p4plus_symbols,
                                        platform_type_t platform_type);

    /// \brief  init routine to initialize p4plus tables
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t p4plus_table_init_(void);

    /// \brief  routine to save the table engine config
    /// \param[in] pipe pipeline
    void table_engine_cfg_backup_(p4pd_pipeline_t pipe);

private:
    pipeline_cfg_t      pipeline_cfg_;
};

/// \brief helper function to program LIF table
/// \param[in] lif_hw_id    hw id of the lif to be programmed
/// \param[in] lif_type     type of the lif
/// \param[in] vpc_hw_id    hw id of the vpc for this lif
/// \param[in] bd_hw_id     hw id of the BD for this lif
/// \param[in] vnic_hw_id   hw id of the vnic for this lif
/// \param[in] learn_en     enable or disable learning on this lif
/// \return SDK_RET_OK on success, failure status code on error
sdk_ret_t program_lif_table(uint16_t lif_hw_id, uint8_t lif_type,
                            uint16_t vpc_hw_id, uint16_t bd_hw_id,
                            uint16_t vnic_hw_id, bool learn_en);

#define POLICER_WRITE_HW_ENTRY(hw, val)       \
do {                                          \
    uint64_t tmpval = (val);                  \
    tmpval >>= 32;                            \
    (hw)[0] = (val) & 0xff;                   \
    (hw)[1] = ((val) >> 8) & 0xff;            \
    (hw)[2] = ((val) >> 16) & 0xff;           \
    (hw)[3] = ((val) >> 24) & 0xff;           \
    (hw)[4] = (tmpval) & 0xff;                \
} while(0)

#define POLICER_READ_HW_ENTRY(hw, val)        \
do {                                          \
    (val) = (hw)[4];                          \
    (val) <<= 32;                             \
    (val) |= (hw)[0];                         \
    (val) |= (hw)[1] << 8;                    \
    (val) |= (hw)[2] << 16;                   \
    (val) |= (hw)[3] << 24;                   \
} while(0)

#define PROGRAM_POLICER_TABLE_ENTRY(policer, tbl, tid, aid, idx, upd)          \
{                                                                              \
    sdk_ret_t ret;                                                             \
    p4pd_error_t p4pd_ret;                                                     \
    uint64_t rate_tokens = 0;                                                  \
    uint64_t burst_tokens = 0;                                                 \
    tbl ## _actiondata_t tbl ## _data = { 0 };                                 \
    tbl ## _actiondata_t tbl ## _data_mask;                                    \
                                                                               \
    tbl ## _data.action_id = aid;                                              \
    if (policer->rate) {                                                       \
        tbl ## _data.tbl ## _info.entry_valid = 1;                             \
        if (policer->type == sdk::POLICER_TYPE_PPS) {                          \
            tbl ## _data.tbl ## _info.pkt_rate = 1;                            \
        }                                                                      \
        ret = policer_to_token_rate(policer,                                   \
                                    PDS_POLICER_DEFAULT_REFRESH_INTERVAL,      \
                                    PDS_POLICER_MAX_TOKENS_PER_INTERVAL,       \
                                    &rate_tokens, &burst_tokens);              \
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);                           \
        PDS_TRACE_DEBUG("burst %lu, rate %lu, tbkt %u", burst_tokens,          \
                        rate_tokens, burst_tokens);                            \
        memcpy(tbl ## _data.tbl ## _info.burst, &burst_tokens,                 \
                std::min(sizeof(tbl ## _data.tbl ## _info.burst),              \
                                sizeof(burst_tokens)));                        \
        memcpy(tbl ## _data.tbl ## _info.rate, &rate_tokens,                   \
               std::min(sizeof(tbl ## _data.tbl ## _info.rate),                \
                        sizeof(rate_tokens)));                                 \
        memcpy(tbl ## _data.tbl ## _info.tbkt, &burst_tokens,                  \
               std::min(sizeof(tbl ## _data.tbl ## _info.tbkt),                \
                        sizeof(burst_tokens)));                                \
    }                                                                          \
    if (upd) {                                                                 \
        memset(&tbl ## _data_mask.tbl ## _info, 0xFF,                          \
               sizeof(tbl ## _data_mask.tbl ## _info));                        \
        tbl ## _data_mask.tbl ## _info.rsvd = 0;                               \
        tbl ## _data_mask.tbl ## _info.axi_wr_pend = 0;                        \
        memset(tbl ## _data_mask.tbl ## _info.tbkt, 0,                         \
               sizeof(tbl ## _data_mask.tbl ## _info.tbkt));                   \
        p4pd_ret = p4pd_global_entry_write_with_datamask(tid, idx, NULL, NULL, \
                                                         &tbl ## _data,        \
                                                         &tbl ## _data_mask);  \
    } else {                                                                   \
        p4pd_ret = p4pd_global_entry_write(tid, idx, NULL, NULL,               \
                                           &tbl ## _data);                     \
    }                                                                          \
    if (p4pd_ret != P4PD_SUCCESS) {                                            \
        PDS_TRACE_ERR("Failed to write to tbl %u table at idx %u", tid, idx);  \
        return sdk::SDK_RET_HW_PROGRAM_ERR;                                    \
    }                                                                          \
    return SDK_RET_OK;                                                         \
}

#define copp_info    action_u.copp_copp
static inline sdk_ret_t
program_copp_entry_ (sdk::policer_t *policer, uint16_t idx, bool upd)
{
    // skip policer programming in non-h/w platforms as the token refresh logic
    // is not activated in those platforms
    if (g_pds_state.platform_type() == platform_type_t::PLATFORM_TYPE_HW) {
        PROGRAM_POLICER_TABLE_ENTRY(policer, copp, P4TBL_ID_COPP,
                                    COPP_COPP_ID, idx, upd);
    }
    return SDK_RET_OK;
}
#undef copp_info

/// \@}

}    // namespace impl
}    // namespace api

#endif    //__APULU_IMPL_HPP__
