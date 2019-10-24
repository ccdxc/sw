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

    /**
     * @brief      free service mapping instance back to slab
     * @param[in]  mapping   pointer to the allocated service mapping
     */
    void free(svc_mapping *mapping);

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    slab *svc_mapping_slab(void) const { return svc_mapping_slab_; }

private:
    slab *svc_mapping_slab_;    /**< slab to allocate service mapping entry */
};

/** @} */    // end of PDS_SVC_MAPPING_STATE

}    // namespace api

using api::svc_mapping_state;

#endif    // __SVC_MAPPING_STATE_HPP__
