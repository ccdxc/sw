//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file handles DHCP related CRUD APIs
///
//----------------------------------------------------------------------------

#ifndef __API_DHCP_HPP__
#define __API_DHCP_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_stooge.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"

namespace api {

// forward declaration
class dhcp_state;

/// \defgroup PDS_DHCP_RELAY - DHCP relay functionality
/// \ingroup PDS_DHCP
/// @{

/// \brief    DHCP relay entry
class dhcp_relay : public api_stooge {
public:
    /// \brief          factory method to allocate & initialize DHCP relay entry
    /// \param[in]      spec  DHCP relay entry information
    /// \return         new instance of DHCP relay entry or NULL
    static dhcp_relay *factory(pds_dhcp_relay_spec_t *spec);

    /// \brief          release all the s/w state associate with the given
    ///                 DHCP relay entry, if any, and free the memory
    /// \param[in]      relay DHCP relay entry to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(dhcp_relay *relay);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] relay    DHCP relay entry to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(dhcp_relay *relay);

    /// \brief    build object given its key from the (sw and/or hw state we
    ///           have) and return an instance of the object (this is useful for
    ///           stateless objects to be operated on by framework during DELETE
    ///           or UPDATE operations)
    /// \param[in] key    key of object instance of interest
    /// \return    DHCP relay object instance corresponding to the key
    static dhcp_relay *build(pds_dhcp_relay_key_t *key);

    /// \brief          initialize DHCP relay entry entry with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief populate the IPC msg with object specific information
    ///        so it can be sent to other components
    /// \param[in] msg         IPC message to be filled in
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t populate_msg(pds_msg_t *msg,
                                   api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          read config
    /// \param[out]     info pointer to the info object
    /// \return         SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_dhcp_relay_info_t *info);

    /// \brief          add given DHCP relay entry to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given DHCP relay entry from the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief          this method is called on new object that needs to
    ///                 replace the old version of the object in the DBs
    /// \param[in]      orig_obj    old version of the object being swapped out
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     initiate delay deletion of this object
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief    return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "dhcp-relay-" + std::to_string(key_.id);
    }

    /// \brief        helper function to get key given DHCP relay entry
    /// \param[in]    entry    pointer to DHCP relay entry instance
    /// \return       pointer to the DHCP relay entry instance's key
    static void *dhcp_relay_key_func_get(void *entry) {
        dhcp_relay *relay = (dhcp_relay *)entry;
        return (void *)&(relay->key_);
    }

private:
    /// \brief constructor
    dhcp_relay();

    /// \brief destructor
    ~dhcp_relay();

    /// \brief    fill the DHCP relay entry sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_dhcp_relay_spec_t *spec);

private:
    /// DHCP relay entry key
    pds_dhcp_relay_key_t key_;

    /// hash table context
    ht_ctxt_t ht_ctxt_;

    /// dhcp_state is friend of dhcp_relay
    friend class dhcp_state;
} __PACK__;

/// \defgroup PDS_DHCP_POLICY - DHCP policy functionality
/// \ingroup PDS_DHCP
/// @{

/// \brief    DHCP policy entry
class dhcp_policy : public api_stooge {
public:
    /// \brief          factory method to allocate & initialize DHCP policy
    /// \param[in]      policy    DHCP policy entry information
    /// \return         new instance of DHCP policy entry or NULL
    static dhcp_policy *factory(pds_dhcp_policy_spec_t *policy);

    /// \brief          release all the s/w state associate with the given
    ///                 DHCP policy entry, if any, and free the memory
    /// \param[in]      policy DHCP policy entry to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(dhcp_policy *policy);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] policy    DHCP policy entry to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(dhcp_policy *policy);

    /// \brief    build object given its key from the (sw and/or hw state we
    ///           have) and return an instance of the object (this is useful for
    ///           stateless objects to be operated on by framework during DELETE
    ///           or UPDATE operations)
    /// \param[in] key    key of object instance of interest
    /// \return    DHCP policy object instance corresponding to the key
    static dhcp_policy *build(pds_dhcp_policy_key_t *key);

    /// \brief          initialize DHCP policy entry entry with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief populate the IPC msg with object specific information
    ///        so it can be sent to other components
    /// \param[in] msg         IPC message to be filled in
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t populate_msg(pds_msg_t *msg,
                                   api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          read config
    /// \param[out]     info pointer to the info object
    /// \return         SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_dhcp_policy_info_t *info);

    /// \brief          add given DHCP policy entry to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given DHCP policy entry from the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief          this method is called on new object that needs to
    ///                 replace the old version of the object in the DBs
    /// \param[in]      orig_obj    old version of the object being swapped out
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     initiate delay deletion of this object
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief    return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "dhcp-relay-" + std::to_string(key_.id);
    }

    /// \brief        helper function to get key given DHCP policy entry
    /// \param[in]    entry    pointer to DHCP policy entry instance
    /// \return       pointer to the DHCP policy entry instance's key
    static void *dhcp_policy_key_func_get(void *entry) {
        dhcp_policy *policy = (dhcp_policy *)entry;
        return (void *)&(policy->key_);
    }

private:
    /// \brief constructor
    dhcp_policy();

    /// \brief destructor
    ~dhcp_policy();

    /// \brief    fill the DHCP policy entry sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_dhcp_policy_spec_t *spec);

private:
    /// DHCP policy entry key
    pds_dhcp_policy_key_t key_;

    /// hash table context
    ht_ctxt_t ht_ctxt_;

    /// dhcp_state is friend of dhcp_relay
    friend class dhcp_state;
} __PACK__;

/// \@}    // end of PDS_DHCP

}    // namespace api

using api::dhcp_relay;
using api::dhcp_policy;

#endif    // __API_DHCP_HPP__
