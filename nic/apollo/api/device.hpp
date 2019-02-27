/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    device.hpp
 *
 * @brief   switcthport entry handling
 */

#if !defined (__DEVICE_HPP__)
#define __DEVICE_HPP__

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/include/api/pds_device.hpp"

namespace api {

/**
 * @defgroup PDS_DEVICE_ENTRY - device entry functionality
 * @ingroup PDS_DEVICE
 * @{
 */

/**
 * @brief    device entry
 */
class device_entry : public api_base {
public:
    /**
     * @brief    factory method to allocate and initialize device entry
     * @param[in] pds_device device information
     * @return    new instance of device or NULL, in case of error
     */
    static device_entry *factory(pds_device_spec_t *pds_device);

    /**
     * @brief    release all the s/w state associate with the given device,
     *           if any, and free the memory
     * @param[in] device device to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(device_entry *device);

    /**
     * @brief     initialize device entry with the given config
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
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(void) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     *           and setting invalid bit (if any) in the h/w entries
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
     * @brief     add device object to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief    del device object from the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t del_from_db(void) override;

    static device_entry *find_in_db(void);

    /**
     * @brief    this method is called on new object that needs to replace the
     *           old version of the object in the DBs
     * @param[in] orig_obj    old version of the object being swapped out
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
        return "device-cfg";
    }

    /**
     * @brief     return impl instance of this tep object
     * @return    impl instance of the tep object
     */
    impl_base *impl(void) { return impl_; }

    ipv4_addr_t ip_addr(void) const { return ip_addr_; }
    mac_addr_t& mac_addr(void) { return mac_addr_; }
    ipv4_addr_t gw_ip_addr(void) const { return gw_ip_addr_; }

private:
    /**< @brief    constructor */
    device_entry() {
        impl_ = NULL;
    }

    /**< @brief    destructor */
    ~device_entry() {}

private:
    ipv4_addr_t    ip_addr_;       /**< physical IP (aka. MyTEP IP) in substrate */
    mac_addr_t     mac_addr_;      /**< MyTEP mac address */
    ipv4_addr_t    gw_ip_addr_;    /**< IP of default gw in the substrate */
    impl_base      *impl_;         /**< impl object instance */
} __PACK__;

/** @} */    // end of PDS_DEVICE_ENTRY

}    // namespace api

using api::device_entry;

#endif    /** __DEVICE_HPP__ */
