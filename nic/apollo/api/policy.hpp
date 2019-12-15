/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    policy.hpp
 *
 * @brief   policy handling
 */

#ifndef __POLICY_HPP__
#define __POLICY_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"

namespace api {

/**
 * @defgroup PDS_POLICY - security policy functionality
 */

/**
 * @brief   security policy
 */
class policy : public api_base {
public:
    /**
     * @brief    factory method to allocate & initialize a security policy
     *           instance
     * @param[in] spec    security policy spec
     * @return    new instance of security policy or NULL, in case of error
     */
    static policy *factory(pds_policy_spec_t *spec);

    /**
     * @brief    release all the s/w state associate with the given security
     *           policy, if any, and free the memory
     * @param[in] policy security policy to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(policy *policy);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] policy    policy to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(policy *policy);

    /**
     * @brief    allocate h/w resources for this object
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief     release h/w resources reserved for this object, if any
     *            (this API is invoked during the rollback stage)
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(void) override;

    /**
     * @brief     initialize security policy instance with the given config
     * @param[in] api_ctxt API context carrying the configuration
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /**
     * @brief    compute all the objects depending on this object and add to
     *           framework's dependency list
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///           bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(api_obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                    api_obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    activate the epoch in the dataplane by programming stage 0
     *           tables, if any
     * @param[in] epoch       epoch being activated
     * @param[in] api_op      api operation
     * @param[in] orig_obj    old/original version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      api_obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief     add given security policy to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief     delete security policy from the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t del_from_db(void) override;

    /**
     * @brief    this method is called on new object that needs to replace the
     *           old version of the object in the DBs
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    initiate delay deletion of this object
     */
    virtual sdk_ret_t delay_delete(void) override;

    /**< @brief    return stringified key of the object (for debugging) */
    virtual string key2str(void) const override {
        return "policy-"  + std::to_string(key_.id);
    }

    /**
     * @brief     helper function to get key given security policy
     * @param[in] entry    pointer to security policy instance
     * @return    pointer to the security policy instance's key
     */
    static void *policy_key_func_get(void *entry) {
        policy *table = (policy *)entry;
        return (void *)&(table->key_);
    }

    ///\brief      read config
    ///\param[out] info pointer to the info object
    ///\return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_policy_info_t *info);

    /// \brief     return the policy key/id
    /// \return    key/id of the policy
    pds_policy_key_t key(void) const { return key_; }

    /**
     * @brief     return IP address family for this policy
     * @return    IP_AF_IPV4 or IP_AF_IPV6 or IP_AF_NONE
     */
    uint8_t af(void) const { return af_; }

    /**
     * @brief     return the policy enforcement direction
     * @return    RULE_DIR_INGRESS or RULE_DIR_EGRESS
     */
    rule_dir_t dir(void) const { return dir_; }

    /**
     * @brief     return impl instance of this security policy object
     * @return    impl instance of the rout table object
     */
    impl_base *impl(void) { return impl_; }

private:
    /**< @brief    constructor */
    policy();

    /**< @brief    destructor */
    ~policy();

    /// \brief      fill the policy sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_policy_spec_t *spec);

    /**
     * @brief     free h/w resources used by this object, if any
     *            (this API is invoked during object deletes)
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t nuke_resources_(void);

private:
    pds_policy_key_t    key_;        /**< security policy key */
    uint8_t             af_;         /**< IP address family of this policy */
    rule_dir_t          dir_;        /**< traffic direction in which this policy is applied on */

    ht_ctxt_t           ht_ctxt_;    /**< hash table context */
    impl_base           *impl_;      /**< impl object instance */

    friend class policy_state;       // policy_state is friend of policy
} __PACK__;

/** @} */    // end of PDS_POLICY

}    // namespace api

using api::policy;

#endif    // __POLICY_HPP__
