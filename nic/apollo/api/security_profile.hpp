//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file handles security profile configuration
///
//----------------------------------------------------------------------------

#ifndef __API_SECURITY_PROFILE_HPP__
#define __API_SECURITY_PROFILE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_stooge.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"

namespace api {

// forward declaration
class policy_state;

/// \defgroup PDS_SECURITY_PROFILE - security profile functionality
/// \ingroup PDS_SECURITY_PROFILE
/// @{

/// \brief    security profile entry
class security_profile : public api_stooge {
public:
    /// \brief          factory method to allocate & initialize a security profile
    /// \param[in]      spec    security profile information
    /// \return         new instance of security profile or NULL, in case of error
    static security_profile *factory(pds_security_profile_spec_t *spec);

    /// \brief          release all the s/w state associate with the given
    ///                 security profile, if any, and free the memory
    /// \param[in] profile   security profile to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(security_profile *profile);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] profile    security profile to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(security_profile *profile);

    /// \brief          initialize security profile entry with the given config
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
    /// \param[out]     info    pointer to the info object
    /// \return         SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_security_profile_info_t *info);

    /// \brief          add given security profile to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given security profile from the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief          this method is called on new object that needs to
    ///                 replace the old version of the object in the DBs
    /// \param[in]      orig_obj    old version of the object being swapped out
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          initiate delay deletion of this object
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief          return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "security-profile-" + std::string(key_.str());
    }

    /// \brief          helper function to get key given security profile
    /// \param[in]      entry    pointer to security profile instance
    /// \return         pointer to the security profile instance's key
    static void *security_profile_key_func_get(void *entry) {
        security_profile *profile = (security_profile *)entry;
        return (void *)&(profile->key_);
    }

    /// \brief     return the global default firewall policy action
    /// \return    global default firewall policy action
    fw_action_t default_fw_action(void) const {
        return default_fw_action_;
    }

private:
    /// \brief constructor
    security_profile();

    /// \brief destructor
    ~security_profile();

    /// \brief      fill the security profile sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_security_profile_spec_t *spec);

private:
    /// security profile key
    pds_obj_key_t key_;

    /// default firewall policy action
    fw_action_t default_fw_action_;

    /// hash table context
    ht_ctxt_t ht_ctxt_;

    /// policy_state is friend of security_profile
    friend class policy_state;
} __PACK__;

/// \@}    // end of PDS_SECURITY_PROFILE

}    // namespace api

using api::security_profile;

#endif    // __API_SECURITY_PROFILE_HPP__
