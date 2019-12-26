//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// tag database handling
///
//----------------------------------------------------------------------------

#ifndef __TAG_STATE_HPP__
#define __TAG_STATE_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/tag.hpp"

namespace api {

/// \defgroup PDS_TAG__STATE - tag state/db functionality
/// \ingroup PDS_TAG
/// \@{

/// \brief    state maintained for tag entries
class tag_state : public state_base {
public:
    /// \brief constructor
    tag_state();

    /// \brief destructor
    ~tag_state();

    /// \brief  allocate memory required for a tag entry
    /// \return pointer to the allocated tag entry, NULL if no memory
    tag_entry *alloc(void);

    /// \brief    insert given tag entry into the tag db
    /// \param[in] tag tag entry to be added to the db
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(tag_entry *tag);

    /// \brief     remove the given tag entry from db
    /// \param[in] tag tag entry to be deleted from the db
    /// \return    pointer to the removed tag entry or NULL, if not found
    tag_entry *remove(tag_entry *tag);

    /// \brief    remove current object from the databse(s) and swap it with the
    ///           new instance of the obj (with same key)
    /// \param[in] curr_tag    current instance of the tag entry
    /// \param[in] new_tag     new instance of the tag entry
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(tag_entry *curr_tag, tag_entry *new_tag);

    /// \brief      free tag entry back to slab
    /// \param[in]  tag pointer to the allocated tag entry
    void free(tag_entry *tag);

    /// \brief     lookup a tag entry in database given the key
    /// \param[in] key tag entry key
    tag_entry *find(pds_tag_key_t *key) const;

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    ht *tag_ht(void) const { return tag_ht_; }
    slab *tag_slab(void) const { return tag_slab_; }
    friend class tag_entry;   // tag_entry is friend of tag_state

private:
    ht      *tag_ht_;      // tag database
    slab    *tag_slab_;    // slab to allocate tag entry
};

static inline tag_entry *
tag_find (void)
{
    return (tag_entry *)api_base::find_obj(OBJ_ID_TAG, NULL);
}

/// \@}    // end of PDS_TAG_STATE

}    // namespace api

using api::tag_state;

#endif    // __TAG_STATE_HPP__
