//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// artemis pipeline implementation
///
//----------------------------------------------------------------------------

#if !defined (__ARTEMIS_IMPL_HPP__)
#define __ARTEMIS_IMPL_HPP__

#include <vector>
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/p4/loader/loader.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/apollo/p4/include/artemis_defines.h"

#define PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID           0
// vpc hw id for Internet traffic
#define PDS_IMPL_PUBLIC_VPC_HW_ID                    0
// vpc hw id for provider (infra/underlay) vpc
#define PDS_IMPL_PROVIDER_VPC_HW_ID                  0
// vpc hw id (unused) for remote 4to6 tunnel
#define PDS_IMPL_SERVICE_TUNNEL_VPC_HW_ID            0
// reserve vnic hw id
#define PDS_IMPL_RSVD_VNIC_HW_ID                     0

// reserved tag id
#define PDS_IMPL_RESERVED_TAG_ID                     0
// reserved meter id
#define PDS_IMPL_RESERVED_METER_HW_ID                0
#define PDS_IMPL_NAT_TBL_RSVD_ENTRY_IDX              0
// nexthop types
#define PDS_IMPL_NH_TYPE_PEER_VPC_MASK               ROUTE_RESULT_TYPE_PEER_VPC_MASK
#define PDS_IMPL_NH_TYPE_SVC_TUNNEL_MASK             ROUTE_RESULT_TYPE_SVC_TUNNEL_MASK
#define PDS_IMPL_NH_TYPE_REMOTE_SVC_TUNNEL_MASK      ROUTE_RESULT_TYPE_REM_SVC_TUNNEL_MASK
// NAT table entry reserved for no translation
#define PDS_IMPL_NAT_TBL_RSVD_ENTRY_IDX              0
#define PDS_IMPL_LOCAL_46_MAPPING_RSVD_ENTRY_IDX     0

#define nat_action action_u.nat_nat_rewrite
#define PDS_IMPL_FILL_NAT_DATA(data, ip)                                     \
{                                                                            \
    memset((data), 0, sizeof(*(data)));                                      \
    (data)->action_id = NAT_NAT_REWRITE_ID;                                  \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        sdk::lib::memrev((data)->nat_action.nat_ip,                          \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        /* key is initialized to zero by the caller */                       \
        memcpy((data)->nat_action.nat_ip, &(ip)->addr.v4_addr,               \
               IP4_ADDR8_LEN);                                               \
    }                                                                        \
}

namespace api {
namespace impl {

/// \defgroup PDS_PIPELINE_IMPL pipeline wrapper implementation
/// \ingroup PDS_PIPELINE
/// @{

/// \brief  pipeline implementation
class artemis_impl : public pipeline_impl_base {
public:
    /// \brief      factory method to pipeline impl instance
    /// \param[in]  pipeline_cfg pipeline configuration information
    /// \return     new instance of artemis pipeline impl or NULL, in case
    ///             of error
    static artemis_impl *factory(pipeline_cfg_t *pipeline_cfg);

    /// \brief      desroy method to pipeline impl instance
    /// \param[in]  impl pointer to the allocated instance
    static void destroy(artemis_impl *impl);

    /// \brief      initialize program configuration
    /// \param[in]  init_params initialization time parameters passed by app
    /// \param[in]  asic_cfg    asic configuration to be populated with program
    ///                         information
    virtual void program_config_init(pds_init_params_t *init_params,
                                     asic_cfg_t *asic_cfg) override;

    /// \brief      initialize asm configuration
    /// \param[in]  init_params initialization time parameters passed by app
    /// \param[in]  asic_cfg    asic configuration to be populated with asm
    ///                         information
    virtual void asm_config_init(pds_init_params_t *init_params,
                                 asic_cfg_t *asic_cfg) override;

    /// \brief      initialize ring configuration
    /// \param[in]  asic_cfg    asic configuration to be populated with ring
    ///                         information
    virtual void ring_config_init(asic_cfg_t *asic_cfg) override;

    /// \brief  init routine to initialize the pipeline
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t pipeline_init(void) override;

    /// \brief      generic API to write to rxdma tables
    /// \param[in]  addr        memory address to write the data to
    /// \param[in]  tableid     table id
    /// \param[in]  action_id   action id to write
    /// \param[in]  action_data action data to write
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t write_to_rxdma_table(mem_addr_t addr, uint32_t tableid,
                                   uint8_t action_id,
                                   void *actiondata) override;

    /// \brief      generic API to write to txdma tables
    /// \param[in]  addr        memory address to write the data to
    /// \param[in]  tableid     table id
    /// \param[in]  action_id   action id to write
    /// \param[in]  action_data action data to write
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t write_to_txdma_table(mem_addr_t addr, uint32_t tableid,
                                           uint8_t action_id,
                                           void *actiondata) override;

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t table_transaction_begin(void) override;

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t table_transaction_end(void) override;

    /// \brief      API to get table stats
    /// \param[in]  cb   callback to be called on stats
    /// \param[in]  ctxt opaque ctxt passed to the callback
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t table_stats(debug::table_stats_get_cb_t cb,
                                  void *ctxt) override;

    /// \brief      Meter Stats Get
    /// \param[in]  cb      Callback
    /// \param[in]  lowidx  Low Index for stats to be read
    /// \param[in]  highidx High Index for stats to be read
    /// \param[in]  ctxt    Opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t meter_stats(debug::meter_stats_get_cb_t cb,
                                  uint32_t lowidx, uint32_t highidx,
                                  void *ctxt) override;

    /// \brief      API to get session stats
    /// \param[in]  cb      callback to be called on stats
    /// \param[in]  lowidx  Low Index for stats to be read
    /// \param[in]  highidx High Index for stats to be read
    /// \param[in]  ctxt    Opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t session_stats(debug::session_stats_get_cb_t cb, uint32_t lowidx,
                                    uint32_t highidx, void *ctxt) override;

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

private:
    /// constructor
    artemis_impl() {}

    /// destructor
    ~artemis_impl() {}

    /// \brief      initialize an instance of artemis impl class
    /// \param[in]  pipeline_cfg pipeline information
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t init_(pipeline_cfg_t *pipeline_cfg);

    /// \brief  init routine to initialize key native table
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t key_native_init_(void);

    /// \brief  init routine to initialize key tunnel table
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t key_tunneled_init_(void);

    /// \brief  init routine to initialize key tunnel2 table
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t key_tunneled2_init_(void);

    /// \brief  init routine to initialize ingress to rxdma table
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t ingress_to_rxdma_init_(void);

    /// \brief  init routine to initialize inter pipe table
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t inter_pipe_init_(void);

    /// \brief  initialize ingress drop stats table
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t ingress_drop_stats_init_(void);

    /// \brief  initialize egress drop stats table
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t egress_drop_stats_init_(void);

    /// \brief  initialize all the stats tables, where needed
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t stats_init_(void);

    /// \brief  initialize any global ACLs during init time
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t nacl_init_(void);

    /// \brief  initialize checksum table
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t checksum_init_(void);

    /// \brief  program all p4/p4+ tables that require one time initialization
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_init_(void);

    /// \brief      artemis specific mpu program sort function
    /// \param[in]  program programs information
    static void sort_mpu_programs_(std::vector<std::string>& programs);

    /// \brief      artemis specific rxdma symbols init function
    /// \param[in]  program information
    static uint32_t rxdma_symbols_init_(void **p4plus_symbols,
                                        platform_type_t platform_type);

    /// \brief      artemis specific txdma symbols init function
    /// \param[in]  program information
    static uint32_t txdma_symbols_init_(void **p4plus_symbols,
                                        platform_type_t platform_type);

    /// \brief  init routine to initialize p4plus tables
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t p4plus_table_init_(void);

private:
    pipeline_cfg_t      pipeline_cfg_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __ARTEMIS_IMPL_HPP__
