//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mirror sesision implementation state
///
//----------------------------------------------------------------------------

#ifndef __MIRROR_IMPL_STATE_HPP__
#define __MIRROR_IMPL_STATE_HPP__

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_MIRROR_IMPL_STATE - mirror session impl state functionality
/// \ingroup PDS_MIRROR
/// \@{

// forward declaration
class mirror_impl;

/// \brief state maintained for mirror sessions
class mirror_impl_state : public state_base {
public:
    /// \brief constructor
    mirror_impl_state(pds_state *state);

    /// \brief destructor
    ~mirror_impl_state();

    /// \brief  allocate memory required for a mirror session impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    mirror_impl *alloc(void);

    /// \brief     free mirror session impl instance back to slab
    /// \param[in] impl pointer to the allocated impl instance
    void free(mirror_impl *impl);

    /// \brief      allocate a free entry in the mirror table and return its
    ///             index
    /// \param[in]  key   pointer to mirror session key
    /// \param[out] hw_id pointer where allocated h/w id needs to be stored
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t alloc_hw_id(pds_obj_key_t *key, uint16_t *hw_id);

    /// \brief      free h/w mirror table entry given its index
    /// \param[in]  hw_id h/w entry index
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t free_hw_id(uint16_t hw_id);

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

private:
    friend class mirror_impl;    ///< friend of mirror_impl_state

private:
    uint8_t session_bmap_;
    slab    *mirror_impl_slab_;
};

///   \@}

}    // namespace impl
}    // namespace api

#endif    // __MIRROR_IMPL_STATE_HPP__
