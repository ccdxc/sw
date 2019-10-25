//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Wrapper class for common pipeline APIs
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_PIPELINE_IMPL_BASE_HPP__
#define __FRAMEWORK_PIPELINE_IMPL_BASE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/asic.hpp"
#include "nic/apollo/framework/obj_base.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"

#define PDS_IMPL_FILL_TABLE_API_PARAMS(api_params, key_, mask_,              \
                                       data, action, hdl)                    \
{                                                                            \
    memset((api_params), 0, sizeof(*(api_params)));                          \
    (api_params)->key = (key_);                                              \
    (api_params)->mask = (mask_);                                            \
    (api_params)->appdata = (data);                                          \
    (api_params)->action_id = (action);                                      \
    (api_params)->handle = (hdl);                                            \
}

#define MEM_ADDR_TO_P4_MEM_ADDR(p4_mem_addr, mem_addr, p4_addr_size)      \
    for (uint32_t i = 0; i < (p4_addr_size); i++) {                       \
        p4_mem_addr[i] = ((mem_addr) >> (i * 8)) & 0xFF;                  \
    }

#define PDS_IMPL_FILL_TABLE_API_ACTION_PARAMS(tparams, idx, actiondata_,     \
                                              actiondata_mask_)              \
{                                                                            \
    memset((tparams), 0, sizeof(*(tparams)));                                \
    (tparams)->handle.pindex(idx);                                           \
    (tparams)->actiondata = actiondata_;                                     \
    (tparams)->actiondata_mask = actiondata_mask_;                           \
}

namespace api {
namespace impl {

/// \defgroup PDS_PIPELINE_IMPL pipeline wrapper implementation
/// @{

/// \brief pipeline configuration
typedef struct pipeline_cfg_s {
    std::string    name;    ///< name of the pipeline
} pipeline_cfg_t;

/// \brief helper class to sort p4/p4+ programs to maximize performance
class sort_mpu_programs_compare {
public:
    bool operator() (std::string p1, std::string p2) {
        std::map <std::string, p4pd_table_properties_t>::iterator it1, it2;

        it1 = tbl_map_.find(p1);
        it2 = tbl_map_.find(p2);
        if ((it1 == tbl_map_.end()) || (it2 == tbl_map_.end())) {
            return (p1 < p2);
        }
        p4pd_table_properties_t tbl_ctx1 = it1->second;
        p4pd_table_properties_t tbl_ctx2 = it2->second;
        if (tbl_ctx1.gress != tbl_ctx2.gress) {
            return (tbl_ctx1.gress < tbl_ctx2.gress);
        }
        if (tbl_ctx1.stage != tbl_ctx2.stage) {
            return (tbl_ctx1.stage < tbl_ctx2.stage);
        }
        return (tbl_ctx1.stage_tableid < tbl_ctx2.stage_tableid);
    }

    void add_table(std::string tbl_name, p4pd_table_properties_t tbl_ctx) {
        std::pair <std::string, p4pd_table_properties_t> key_value;
        key_value = std::make_pair(tbl_name.append(".bin"), tbl_ctx);
        tbl_map_.insert(key_value);
    }

private:
    std::map <std::string, p4pd_table_properties_t> tbl_map_;
};

/// \brief pipeline implementation
class pipeline_impl_base : public obj_base {
public:
    /// \brief Factory method to instantiate pipeline impl instance
    ///
    /// \param[in] pipeline_cfg Pipeline configuration information
    /// \return new instance of pipeline impl or NULL, in case of error
    static pipeline_impl_base *factory(pipeline_cfg_t *pipeline_cfg);

    /// \brief Destory method to free pipeline impl instance
    ///
    /// \param[in] impl pipeline impl instance
    static void destroy(pipeline_impl_base *impl);

    /// \brief Initialize program configuration
    ///
    /// \param[in] init_params Initialization time parameters passed by app
    /// \param[in] asic_cfg ASIC configuration to be populated with program info
    virtual void program_config_init(pds_init_params_t *init_params,
                                     asic_cfg_t *asic_cfg) { }

    /// \brief initialize asm configuration
    ///
    /// \param[in] init_params initialization time parameters passed by app
    /// \param[in] asic_cfg asic configuration to be populated with asm info
    virtual void asm_config_init(pds_init_params_t *init_params,
                                 asic_cfg_t *asic_cfg) { }

    /// \brief initialize ring configuration
    ///
    /// \param[in] asic_cfg asic configuration to be populated with rings info
    virtual void ring_config_init(asic_cfg_t *asic_cfg) { }

    ///
    /// \brief Init routine to initialize the pipeline
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t pipeline_init(void) { return sdk::SDK_RET_ERR; }

    /// \brief Generic API to write to rxdma tables
    ///
    /// \param[in] addr Memory address to write the data to
    /// \param[in] tableid Table id
    /// \param[in] action_id Action id to write
    /// \param[in] action_data Action data to write
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t write_to_rxdma_table(mem_addr_t addr, uint32_t tableid,
                                           uint8_t action_id,
                                           void *actiondata) {
        return SDK_RET_ERR;
    }

    /// \brief Generic API to write to txdma tables
    ///
    /// \param[in] addr Memory address to write the data to
    /// \param[in] tableid Table id
    /// \param[in] action_id Action id to write
    /// \param[in] action_data Action data to write
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t write_to_txdma_table(mem_addr_t addr, uint32_t tableid,
                                           uint8_t action_id,
                                           void *actiondata) {
        return SDK_RET_ERR;
    }

    /// \brief API to begin transaction over all table mgmt library instances
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t table_transaction_begin(void) {
        return SDK_RET_ERR;
    }

    /// \brief API to end transaction over all table mgmt library instances
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t table_transaction_end(void) {
        return SDK_RET_ERR;
    }

    /// \brief API to retrieve table stats
    /// \param[in]  cb    callback to be called on stats
    ///             ctxt    opaque ctxt passed to the callback
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
        return SDK_RET_ERR;
    }

    /// \brief API to handle CLI calls
    /// \param[in]  ctxt    CLI command context
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t handle_cmd(cmd_ctxt_t *ctxt) {
        return SDK_RET_ERR;
    }

    /// \brief      API to get session stats
    /// \param[in]  cb      callback to be called on stats
    /// \param[in]  lowidx  Low Index for stats to be read
    /// \param[in]  highidx High Index for stats to be read
    /// \param[in]  ctxt    Opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t session_stats(debug::session_stats_get_cb_t cb,
                                    uint32_t lowidx, uint32_t highidx,
                                    void *ctxt) {
        return SDK_RET_ERR;
    }

    /// \brief      Meter Stats Get
    /// \param[in]  cb      Callback
    /// \param[in]  lowidx  Low Index for stats to be read
    /// \param[in]  highidx High Index for stats to be read
    /// \param[in]  ctxt    Opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t meter_stats(debug::meter_stats_get_cb_t cb,
                                  uint32_t lowidx, uint32_t highidx,
                                  void *ctxt) {
        return SDK_RET_ERR;
    }

    /// \brief      API to get session
    /// \param[in]  cb      callback to be called on session
    /// \param[in]  ctxt    Opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t session(debug::session_get_cb_t cb, void *ctxt) {
        return SDK_RET_ERR;
    }

    /// \brief      API to get flow
    /// \param[in]  cb      callback to be called on flow
    /// \param[in]  ctxt    Opaque context to be passed to callback
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t flow(debug::flow_get_cb_t cb, void *ctxt) {
        return SDK_RET_ERR;
    }

    /// \brief      API to clear session
    /// \param[in]  idx  Index for session to be cleared
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t session_clear(uint32_t idx) {
        return SDK_RET_ERR;
    }

    /// \brief      API to clear flow
    /// \param[in]  idx  Index for flow to be cleared
    /// \return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t flow_clear(uint32_t idx) {
        return SDK_RET_ERR;
    }

private:
    pipeline_cfg_t pipeline_cfg_;
};

/// \@}

}    // namespace impl
}    // namespace api

using api::impl::pipeline_cfg_t;
using api::impl::pipeline_impl_base;

#endif    // __FRAMEWORK_PIPELINE_IMPL_BASE_HPP__
