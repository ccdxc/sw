//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// poseidon pipeline implementation
///
//----------------------------------------------------------------------------

#ifndef __POSEIDON_IMPL_HPP__
#define __POSEIDON_IMPL_HPP__

#include <vector>
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/p4/loader/loader.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/apollo/p4/include/defines.h"

#define PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID    0
#define PDS_IMPL_MYTEP_NEXTHOP_HW_ID          1
#define PDS_IMPL_MYTEP_HW_ID                  0
#define PDS_IMPL_TEP_INVALID_INDEX            0xFFFF

// nexthop types
#define PDS_IMPL_NH_TYPE_PEER_VPC_MASK        ROUTE_RESULT_TYPE_PEER_VPC_MASK

namespace api {
namespace impl {

/// \defgroup PDS_PIPELINE_IMPL - pipeline wrapper implementation
/// \ingroup PDS_PIPELINE
/// @{

/// \brief pipeline implementation
class poseidon_impl : public pipeline_impl_base {
public:
    /// \brief     factory method to pipeline impl instance
    /// \param[in] pipeline_cfg pipeline configuration information
    /// \return    new instance of poseidon pipeline impl or NULL, in case of error
    static poseidon_impl *factory(pipeline_cfg_t *pipeline_cfg);

    /// \brief     destroy method to pipeline impl instance
    /// \param[in] impl pointer to the allocated instance
    static void destroy(poseidon_impl *impl);

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

    /// \brief API to handle CLI calls
    /// \param[in]  ctxt    CLI command context
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t handle_cmd(cmd_ctxt_t *ctxt) override;

private:
    /// \brief constructor
    poseidon_impl() {}

    /// \brief destructor
    ~poseidon_impl() {}

    /// \brief     initialize an instance of poseidon impl class
    /// \param[in] pipeline_cfg pipeline information
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t init_(pipeline_cfg_t *pipeline_cfg);

    /// \brief  init routine to initialize key native table
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t key_native_init_(void);

    /// \brief  init routine to initialize key tunnel table
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t key_tunneled_init_(void);

    /// \brief  init routine to initialize ingress to rxdma table
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t ingress_to_rxdma_init_(void);

    /// \brief  initialize ingress drop stats table
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t ingress_drop_stats_init_(void);

    /// \brief  initialize egress drop stats table
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t egress_drop_stats_init_(void);

    /// \brief  initialize all the stats tables, where needed
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t stats_init_(void);

    /// \brief  program all p4/p4+ tables that require one time initialization
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_init_(void);

    /// \brief     poseidon specific mpu program sort function
    /// \param[in] program information
    static void sort_mpu_programs_(std::vector<std::string>& programs);

    /// \brief     poseidon specific rxdma symbols init function
    /// \param[in] program information
    static uint32_t rxdma_symbols_init_(void **p4plus_symbols,
                                        platform_type_t platform_type);

    /// \brief     poseidon specific txdma symbols init function
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

#endif    //__POSEIDON_IMPL_HPP__
