//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// device entry handling
///
///----------------------------------------------------------------------------

#if !defined (__DEVICE_HPP__)
#define __DEVICE_HPP__

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_device.hpp"

namespace api {


/// \defgroup PDS_DEVICE_ENTRY - device entry functionality
/// \ingroup PDS_DEVICE
/// \@{

/// \brief device entry
class device_entry : public api_base {
public:
     /// \brief     factory method to allocate and initialize device entry
     /// \param[in] pds_device device information
     /// \return    new instance of device or NULL, in case of error
    static device_entry *factory(pds_device_spec_t *pds_device);

    /// \brief     release all the s/w state associate with the given device,
    ///            if any, and free the memory
    /// \param[in] device device to be freed
    /// NOTE:      h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this
    static void destroy(device_entry *device);

    /// \brief      fill the device sw status
    /// \param[out] status specification
    static void fill_status(pds_device_status_t *status);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] device    device object being freed
    /// \return   SDK_RET_OK or error code
    static sdk_ret_t free(device_entry *device);

    /// \brief     allocate/reserve h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override {
        // this object results in register programming only, hence no resources
        // need to be reserved for this object
        return SDK_RET_OK;
    }

    /// \brief  release h/w resources reserved for this object, if any
    ///         (this API is invoked during the rollback stage)
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override {
        // no resources are resreved for this object and until
        // activate stage, no hw programming done as well, hence
        // this is a no-op
        return SDK_RET_OK;
    }

    /// \brief     initialize device entry with the given config
    /// \param[in] api_ctxt API context carrying the configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief     program all h/w tables relevant to this object except stage 0
    ///            table(s), if any
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override {
        // all configuration is programmed only during activate stage
        // for this object, hence this is a no-op
        return SDK_RET_OK;
    }

    /// \brief     cleanup all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///             epoch# and setting invalid bit (if any) in the h/w entries
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override {
        // as program_create() is no-op, cleanup_config() is no-op as well
        return SDK_RET_OK;
    }

    /// \brief populate the IPC msg with object specific information
    ///        so it can be sent to other components
    /// \param[in] msg         IPC message to be filled in
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t populate_msg(pds_msg_t *msg,
                                   api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///           bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///            epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                    api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     activate the epoch in the dataplane by programming stage 0
    ///            tables, if any
    /// \param[in] epoch epoch/version of new config
    /// \param[in] api_op   api operation
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          add all objects that may be affected if this object is
    ///                 updated to framework's object dependency list
    /// \param[in]      obj_ctxt    transient state associated with this API
    ///                             processing
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt)  {
        return SDK_RET_OK;
    }

    /// \brief     this method is called on new object that needs to replace the
    ///            old version of the object in the DBs
    /// \param[in] orig_obj old version of the object being swapped out
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief  add device object to the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief  del device object from the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "device-cfg";
    }

    /// \brief      read config
    /// \param[out] info Pointer to the info object
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_device_info_t *info);

    /// \brief  return the device's IP address
    /// \return IP address of the device
    ip_addr_t ip_addr(void) const { return ip_addr_; }

    /// \brief  return the device's MAC addresss
    /// \return MAC address of the device
    mac_addr_t& mac(void) { return mac_addr_; }

    /// \brief  return the device's gateway IP address
    /// \return gateway IP address of the device
    ip_addr_t gw_ip_addr(void) const { return gw_ip_addr_; }

    /// \brief  return the device's operational mode
    /// \return operational mode of the device
    pds_device_oper_mode_t oper_mode(void) const { return oper_mode_; }

    /// \brief return true if L2 bridging is enabled or else false
    /// \return    true or false based on whether bridging is enabled or not
    bool bridging_enabled(void) const { return bridging_en_; }

    /// \brief return true if learning is enabled or else false
    /// \return    true or false based on whether learning is enabled or not
    bool learning_enabled(void) const { return learning_en_; }

    /// \brief return the aging timeout for MAC or IP when learning is enabled
    /// \return    aging timeout value in seconds
    uint32_t learn_age_timeout(void) const { return learn_age_timeout_; }

    /// \brief return true if control plane stack is enabled or else false
    /// \return  true or false based on whether control plane stack is enabled or not
    bool overlay_routing_enabled(void) const { return overlay_routing_en_; }

private:
    /// \brief constructor
    device_entry() {
        impl_ = NULL;
    }

    /// \brief destructor
    ~device_entry() {}

    /// \brief      fill the device sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_device_spec_t *spec);


    /// \brief  free h/w resources used by this object, if any
    ///         (this API is invoked during object deletes)
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void) {
        // no resources are used by this object, hence this is a no-op
        return SDK_RET_OK;
    }

private:
    ///< physical IP (aka. MyTEP IP) in underlay
    ip_addr_t ip_addr_;
    ///< MyTEP mac address
    mac_addr_t mac_addr_;
    ///< IPv4 addres of default gw in underlay
    ip_addr_t gw_ip_addr_;
    ///< operational mode of the device
    pds_device_oper_mode_t oper_mode_;
    ///< true if L2 bridging is enabled
    bool bridging_en_;
    ///< true if learning is enabled
    bool learning_en_;
    ///< aging timeout in seconds for learnt MAC or IP
    uint32_t learn_age_timeout_;
    ///< true if control plane stack is enabled
    bool overlay_routing_en_;
    ///< impl object instance
    impl_base *impl_;
} __PACK__;

/// \@}

}    // namespace api

using api::device_entry;

#endif     // __DEVICE_HPP__
