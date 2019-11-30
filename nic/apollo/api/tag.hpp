//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// tag entry handling
///
//----------------------------------------------------------------------------

#ifndef __API_TAG_HPP__
#define __API_TAG_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"

namespace api {

// forward declaration
class tag_state;

/// \defgroup PDS_TAG_ENTRY - tag entry functionality
/// \ingroup PDS_TAG
/// @{

/// \brief    tag entry
class tag_entry : public api_base {
public:
    /// \brief          factory method to allocate and initialize a tag entry
    /// \param[in]      spec    tag information
    /// \return         new instance of tag or NULL, in case of error
    static tag_entry *factory(pds_tag_spec_t *spec);

    /// \brief          release all the s/w state associate with the given tag,
    ///                 if any, and free the memory
    /// \param[in]      tag     tag to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(tag_entry *tag);

    /// \brief          initialize tag entry with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief          allocate h/w resources for this object
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief          free h/w resources used by this object, if any
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief          program all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(obj_ctxt_t *obj_ctxt) override;

    /// \brief          cleanup all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override {
        // there is no need cleanup in either the rollback case or
        // route table delete case, we simply have to free the resources in
        // either case
        return SDK_RET_OK;
    }

    /// \brief          update all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /// \brief          activate the epoch in the dataplane by programming
    ///                 stage 0 tables, if any
    /// \param[in]      epoch       epoch being activated
    /// \param[in]      api_op      api operation
    /// \param[in]      orig_obj old/original version of the unmodified object
    /// \param          obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      obj_ctxt_t *obj_ctxt) override;

    /// \brief          add given tag to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given tag from the database
    ///
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief          this method is called on new object that needs to
    ///                 replace theold version of the object in the DBs
    /// \param[in]      orig_obj    old version of the object being swapped out
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                obj_ctxt_t *obj_ctxt) override;

    /// \brief          initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief    compute all the objects depending on this object and add to
    ///           framework's dependency list
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(obj_ctxt_t *obj_ctxt) override;

    /// \brief          return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "tag-" + std::to_string(key_.id);
    }

    /// \brief          helper function to get key given tag entry
    /// \param          entry    pointer to tag instance
    /// \return         pointer to the tag instance's key
    static void *tag_key_func_get(void *entry) {
        tag_entry *tag = (tag_entry *)entry;
        return (void *)&(tag->key_);
    }

    /// \brief   helper function to get size of key
    /// \return  size of key
    static uint32_t key_size(void) {
        return sizeof(pds_tag_key_t);
    }

    /// \brief          return the tag entry's key/id
    /// \return         key/id of the tag entry
    pds_tag_key_t key(void) const { return key_; }

    /// \brief return address family of this tag entry
    /// \return IP_AF_IPV4, if tag entry is IPv4 or else IP_AF_IPV6
    uint8_t af(void) const { return af_; }

    /// \brief     return impl instance of this tag entry object
    /// \return    impl instance of the tag entry object
    impl_base *impl(void) { return impl_; }

private:
    /// \brief constructor
    tag_entry();

    /// \brief destructor
    ~tag_entry();

    /// \brief    free h/w resources used by this object, if any
    ///           (this API is invoked during object deletes)
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_tag_key_t key_;        ///< tag key
    uint8_t af_;               ///< IP_AF_IPV4 or IP_AF_IPV6
    ht_ctxt_t ht_ctxt_;        ///< hash table context
    impl_base *impl_;          ///< impl object instance

    friend class tag_state;    ///< tag_state is friend of tag_entry
} __PACK__;

/// \@}    // end of PDS_TAG_ENTRY

}    // namespace api

using api::tag_entry;

#endif    // __API_TAG_HPP__
