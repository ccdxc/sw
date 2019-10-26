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
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"

// system wide blackhole nexthop
#define PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID    0

// nexthop types
#define PDS_IMPL_NH_TYPE_PEER_VPC_MASK               ROUTE_RESULT_TYPE_PEER_VPC_MASK

// reserved VNIC h/w id
#define PDS_IMPL_RSVD_VNIC_HW_ID               0

// reserved BD h/w id
#define PDS_IMPL_RSVD_BD_HW_ID                 0

// reserved VPC h/w id
#define PDS_IMPL_RSVD_VPC_HW_ID                0

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

#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(data, vnic_hw_id, xlate_idx)  \
{                                                                            \
    memset(data, 0, sizeof(*(data)));                                        \
    (data)->vnic_id = (vnic_hw_id);                                          \
}

#define PDS_IMPL_FILL_IP_MAPPING_SWKEY(key, vpc_hw_id, ip)                   \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->p4e_i2e_mapping_lkp_id = vpc_hw_id;                               \
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

#define nat_action action_u.nat_nat_rewrite
#define PDS_IMPL_FILL_NAT_DATA(data, xlate_ip, xlate_port)                   \
{                                                                            \
    memset((data), 0, sizeof(*(data)));                                      \
    (data)->action_id = NAT_NAT_REWRITE_ID;                                  \
    if ((xlate_ip)->af == IP_AF_IPV6) {                                      \
        sdk::lib::memrev((data)->nat_action.ip,                              \
                         (xlate_ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);     \
    } else {                                                                 \
        memcpy((data)->nat_action.ip, &(xlate_ip)->addr.v4_addr,             \
               IP4_ADDR8_LEN);                                               \
    }                                                                        \
    /* (data)->port = (xlate_port); */                                       \
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
    virtual sdk_ret_t table_transaction_begin(void) override;

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t table_transaction_end(void) override;

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

    /// \brief      API to clear flow
    /// \param[in]  idx     Index for flow to be cleared
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t flow_clear(uint32_t idx) override;

    /// \brief API to handle CLI calls
    /// \param[in]  ctxt    CLI command context
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t handle_cmd(cmd_ctxt_t *ctxt) override;

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

private:
    pipeline_cfg_t      pipeline_cfg_;
};

/// \@}

}    // namespace impl
}    // namespace api

#endif    //__APULU_IMPL_HPP__
