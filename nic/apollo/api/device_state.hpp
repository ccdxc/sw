//
// Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// \file
/// device database handling
///
//----------------------------------------------------------------------------

#ifndef __DEVICE_STATE_HPP__
#define __DEVICE_STATE_HPP__

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/device.hpp"

namespace api {

/// \defgroup PDS_DEVICE_STATE - device functionality
/// \ingroup PDS_DEVICE
/// \@{

/// \brief state maintained for devices
class device_state : public state_base {
public:
    /// \brief constructor
    device_state() {
        device_cfg_ = NULL;
    }

    /// \brief destructor
    ~device_state() {}

    /// \brief  allocate memory required for device object
    /// \return pointer to the allocated device, NULL if no memory
    device_entry *alloc(void);

    /// \brief  insert given device instance into the device db
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(device_entry *device);

    /// \brief  remove the (singleton) instance of device object from db
    /// \return pointer to the removed device instance or NULL, if not found
    device_entry *remove(void);

    /// \brief     free device instance back to slab
    /// \param[in] device pointer to the allocated device
    void free(device_entry *device);

    /// \@brief    lookup a device in database given the key
    /// \param[in] device_key device key
    /// \return    pointer to the instance of device config or NULL,
    ///            if not found
    device_entry *find(void) {
        return device_cfg_;
    }

    /// \brief     API to walk all the db elements
    /// \param[in] walk_cb callback to be invoked for every node
    /// \param[in] ctxt    opaque context passed back to the callback
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt) override;

private:
    device_entry *device_cfg_;    ///< user provided config
};

/// \@}

}    // namespace api

using api::device_state;

#endif    // __DEVICE_STATE_HPP__
