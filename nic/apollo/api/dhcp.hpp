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
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"

namespace api {

// forward declaration
class dhcp_state;

/// \defgroup PDS_DHCP_POLICY - DHCP policy functionality
/// \ingroup PDS_DHCP
/// @{

/// \brief    DHCP policy entry
class dhcp_policy : public api_base {
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

    /// \brief     allocate h/w resources for this object
    /// \param[in] orig_obj    old version of the unmodified object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

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

    /// \brief    program all h/w tables relevant to this object except stage 0
    ///           table(s), if any
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief    cleanup all h/w tables relevant to this object except stage 0
    ///           table(s), if any, by updating packed entries with latest
    ///           epoch#
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///           bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief        add all objects that may be affected if this object is
    ///               updated to framework's object dependency list
    /// \param[in]    obj_ctxt    transient state associated with this API
    ///                           processing
    /// \return       SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override {
        // no other objects are effected if DHCP config is modified
        return SDK_RET_OK;
    }

    /// \brief    update all h/w tables relevant to this object except stage 0
    ///           table(s), if any, by updating packed entries with latest
    ///           epoch#
    /// \param[in] orig_obj    old version of the unmodified object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                     api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     activate the epoch in the dataplane by programming
    ///            stage 0 tables, if any
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
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
        return "dhcp-" + std::string(key_.str());
    }

    /// \brief        helper function to get key given DHCP policy entry
    /// \param[in]    entry    pointer to DHCP policy entry instance
    /// \return       pointer to the DHCP policy entry instance's key
    static void *dhcp_policy_key_func_get(void *entry) {
        dhcp_policy *policy = (dhcp_policy *)entry;
        return (void *)&(policy->key_);
    }

    /// \brief     return the key/id of this DHCP policy
    /// \return    key/id of the DHCP policy object
    pds_obj_key_t key(void) const { return key_; }

    /// \brief     return the type of this DHCP policy
    /// \return    type of the DHCP policy object
    pds_dhcp_policy_type_t type(void) const { return type_; }

    /// \brief     return server IP of the DHCP policy object
    /// \return    server IP 
    const ip_addr_t server_ip(void) const { return server_ip_; }

    /// \brief     return mtu of the DHCP policy object
    /// \return    mtu
    uint32_t mtu(void) const { return mtu_; }

    /// \brief     return gateway IP of the DHCP policy object
    /// \return    gateway IP 
    const ip_addr_t gateway_ip(void) const { return gateway_ip_; }

    /// \brief     return DNS server IP of the DHCP policy object
    /// \return    DNS server IP 
    const ip_addr_t dns_server_ip(void) const { return dns_server_ip_; }

    /// \brief     return NTP server IP of the DHCP policy object
    /// \return    NTP server IP 
    const ip_addr_t ntp_server_ip(void) const { return ntp_server_ip_; }

    /// \brief     return ptr to the domain name of the DHCP policy object
    /// \return    ptr to domain name
    const char *domain_name(void) const { return domain_name_; }
    
    /// \brief     return ptr to the boot filename of the DHCP policy object
    /// \return    ptr to boot filename 
    const char *boot_filename(void) const { return boot_filename_; }

    /// \brief     return the lease timeout  of the DHCP policy object
    /// \return    lease timeout
    uint32_t lease_timeout(void) const { return lease_timeout_; }

    /// \brief     return impl instance of this DHCP policy object
    /// \return    impl instance of the DHCP policy object
    impl_base *impl(void) { return impl_; }

private:
    /// \brief constructor
    dhcp_policy();

    /// \brief destructor
    ~dhcp_policy();

    /// \brief    fill the DHCP policy entry sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_dhcp_policy_spec_t *spec);

    /// \brief    free h/w resources used by this object, if any
    ///           (this API is invoked during object deletes)
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    /// DHCP policy entry key
    pds_obj_key_t key_;

    /// DHCP policy type
    pds_dhcp_policy_type_t type_;

    /// DHCP proxy configuration
    union {
        struct {
            /// DHCP server IP
            ip_addr_t server_ip_;

            /// MTU specified to clients
            uint32_t mtu_;

            /// gateway IP
            ip_addr_t gateway_ip_;

            /// DNS server IP
            ip_addr_t dns_server_ip_;

            /// NTP server IP
            ip_addr_t ntp_server_ip_;

            /// domain_name
            char domain_name_[PDS_MAX_DOMAIN_NAME_LEN + 1];

            /// boot_filename
            char boot_filename_[PDS_MAX_BOOT_FILENAME_LEN + 1];
            
            /// DHCP lease timeout
            uint32_t lease_timeout_;
            
        };
    };

    /// hash table context
    ht_ctxt_t ht_ctxt_;

    ///< impl object instance
    impl_base     *impl_;

    /// dhcp_state is friend of dhcp_policy
    friend class dhcp_state;
} __PACK__;

/// \@}    // end of PDS_DHCP

}    // namespace api

using api::dhcp_policy;

#endif    // __API_DHCP_HPP__
