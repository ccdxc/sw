/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping.hpp
 *
 * @brief   mapping entry handling
 */

#ifndef __MAPPING_HPP__
#define __MAPPING_HPP__

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"

// mapping internal specification
typedef struct pds_mapping_spec_s {
    pds_mapping_key_t key;                // mapping key
    pds_subnet_key_t subnet;              // subnet this IP is part of
    pds_encap_t fabric_encap;             // fabric encap for this mapping
    mac_addr_t overlay_mac;               // MAC for this IP
    pds_nh_type_t nh_type;                // nexthop type information for
                                          // the mapping
    bool is_local;
    union {
        // information specific to remote mappings
        union {
            pds_tep_key_t tep;                // TEP address for this mapping
                                              // 1. device IP for local vnic
                                              // 2. remote TEP for remote vnic if
                                              //    provider IP is not valid
            pds_nexthop_group_key_t nh_group; // nexthop group mapping is behind
            pds_nexthop_key_t nexthop;        // nexthop (used in case traffic is
                                              // going out natively without encap)
        };
        // information specific to local IP mappings
        struct {
            pds_vnic_key_t vnic;              // vnic for local IP
            bool public_ip_valid;             // true if public IP is valid
            ip_addr_t public_ip;              // public IP address
            bool provider_ip_valid;           // true if provider IP is valid
            ip_addr_t provider_ip;            // provider IP address
        };
    };
    // number of tags/labels/security groups of the mapping
    uint32_t num_tags;
    // tags/labels/security groups for the mapping
    uint32_t tags[PDS_MAX_TAGS_PER_MAPPING];
} __PACK__ pds_mapping_spec_t;

/// \brief internal mapping information
typedef struct pds_mapping_info_s {
    pds_mapping_spec_t spec;        ///< specification
    pds_mapping_status_t status;    ///< status
    pds_mapping_stats_t stats;      ///< statistics
} __PACK__ pds_mapping_info_t;

namespace api {

/**
 * @defgroup PDS_MAPPING_ENTRY - mapping functionality
 * @ingroup PDS_MAPPING
 * @{
 */

/**
 * @brief    mapping entry
 */
class mapping_entry : public api_base {
public:
    /**
     * @brief    factory method to allocate and initialize a mapping entry
     * @param[in] pds_mapping    mapping information
     * @return    new instance of mapping or NULL, in case of error
     */
    static mapping_entry *factory(pds_mapping_spec_t *pds_mapping);

    /**
     * @brief    release all the s/w state associate with the given mapping,
     *           if any, and free the memory
     * @param[in] mapping     mapping to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(mapping_entry *mapping);

    /**
     * @brief    build object given its key from the (sw and/or hw state we
     *           have) and return an instance of the object (this is useful for
     *           stateless objects to be operated on by framework during DELETE
     *           or UPDATE operations)
     * @param[in] key    key of object instance of interest
     */
    static mapping_entry *build(pds_mapping_key_t *key);

    /**
     * @brief    free a stateless entry's temporary s/w only resources like
     *           memory etc., for a stateless entry calling destroy() will
     *           remove resources from h/w, which can't be done during ADD/UPD
     *           etc. operations esp. when object is constructed on the fly
     *  @param[in] mapping     mapping to be freed
     */
    static void soft_delete(mapping_entry *mapping);

    /**
     * @brief     initialize mapping entry with the given config
     * @param[in] api_ctxt API context carrying the configuration
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /**
     * @brief    allocate h/w resources for this object
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief     release h/w resources reserved for this object, if any
     *            (this API is invoked during the rollback stage)
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(void) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_config(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    activate the epoch in the dataplane by programming stage 0
     *           tables, if any
     * @param[in] epoch       epoch being activated
     * @param[in] api_op      api operation
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      obj_ctxt_t *obj_ctxt) override;

     /**
     * @brief     add given mapping to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief     delete given mapping from the database
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
                                obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    initiate delay deletion of this object
     */
    virtual sdk_ret_t delay_delete(void) override;

    /**< @brief    return stringified key of the object (for debugging) */
    virtual string key2str(void) const override {
        /**<
         * right now, we don't store even the key, we can do that in future if
         * its needed for debugging; as this is a stateless object which will be
         * destroyed as soon as API processing is done, having a key in this
         * class will only increase temporary usage of memory (and some cycles
         * to copy key bytes into this class)
         */
        return "mapping-";
    }

    /**
     * @brief read config
     * @param[in]  key Pointer to the key object
     * @param[out] info Pointer to the info object
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t read(pds_mapping_key_t *key, pds_mapping_info_t *info);

    /**
     * @brief set object is local or remote
     * @param[in] local ture if it is local false otherwise
     */
    void set_local(bool local) { local_ = local; }
    bool is_local() { return local_; }

    /**
     * @brief     return impl instance of this vnic object
     * @return    impl instance of the vnic object
     */
    impl_base *impl(void) { return impl_; }

private:
    /**< @brief    constructor */
    mapping_entry();

    /**< @brief    destructor */
    ~mapping_entry();

    /**
     * @brief     free h/w resources used by this object, if any
     *            (this API is invoked during object deletes)
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t nuke_resources_(void);

private:
    impl_base        *impl_;      /**< impl object instance */
    bool             local_;      /**< is it local or remote object */
} __PACK__;


/** @} */    // end of PDS_MAPPING_ENTRY

}    // namespace api

using api::mapping_entry;

#endif    /** __MAPPING_HPP__ */
