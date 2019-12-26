/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    service_state.hpp
 *
 * @brief   service mapping database maintenance
 */

#ifndef __SVC_MAPPING_STATE_HPP__
#define __SVC_MAPPING_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/service.hpp"

namespace api {

/**
 * @defgroup PDS_SVC_MAPPING_STATE - service mapping state handling
 * @ingroup PDS_SVC_MAPPING
 * @{
 */

/**
 * @brief    state maintained for service mappings
 */
class svc_mapping_state : public state_base {
public:
    /**
     * @brief    constructor
     */
    svc_mapping_state();

    /**
     * @brief    destructor
     */
    ~svc_mapping_state();

    /**
     * @brief    allocate memory required for a service mapping
     * @return pointer to the allocated service mapping, NULL if no memory
     */
    svc_mapping *alloc(void);

    /// \brief     insert given svc mapping instance into the svc mapping db
    /// \param[in] mapping svc mapping entry to be added to the db
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(svc_mapping *mapping);

    /// \brief     remove the given instance of svc mapping object from db
    /// \param[in] mapping svc mapping entry to be deleted from the db
    /// \return    pointer to the removed svc mapping instance or NULL, if not found
    svc_mapping *remove(svc_mapping *mapping);

    /**
     * @brief      free service mapping instance back to slab
     * @param[in]  mapping   pointer to the allocated service mapping
     */
    void free(svc_mapping *mapping);

    /// \brief     lookup a svc mapping in database given the key
    /// \param[in] key key for the svc mapping object
    /// \return    pointer to the svc mapping instance found or NULL
    svc_mapping *find(pds_svc_mapping_key_t *key) const;

    /// \brief API to walk all the db elements
    /// \param[in] walk_cb    callback to be invoked for every node
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt) override;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *svc_mapping_ht(void) const { return svc_mapping_ht_; }
    slab *svc_mapping_slab(void) const { return svc_mapping_slab_; }

private:
    ht *svc_mapping_ht_;        /**< mapping databse */
    slab *svc_mapping_slab_;    /**< slab to allocate service mapping entry */
};

static inline svc_mapping *
svc_mapping_find (void)
{
    return (svc_mapping *)api_base::find_obj(OBJ_ID_SVC_MAPPING, NULL);
}

/** @} */    // end of PDS_SVC_MAPPING_STATE

}    // namespace api

using api::svc_mapping_state;

#endif    // __SVC_MAPPING_STATE_HPP__
