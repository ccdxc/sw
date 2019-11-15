//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains policer class definition
///
//----------------------------------------------------------------------------

#ifndef __API_POLICER_HPP__
#define __API_POLICER_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"
#include "nic/apollo/framework/impl_base.hpp"

namespace api {

// forward declaration
class policer_state;

/// \defgroup PDS_POLICER_ENTRY - policer functionality
/// \ingroup PDS_POLICER
/// @{

/// \brief    policer
class policer : public api_base {
public:
    /// \brief          factory method to allocate and initialize a policer
    /// \param[in]      spec    policer information
    /// \return         new instance of policer or NULL, in case of error
    static policer *factory(pds_policer_spec_t *spec);

    /// \brief          release all the s/w state associate with the given
    ///                 policer, if any, and free the memory
    /// \param[in]      pol     policer to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(policer *pol);

    /// \brief          initialize policer with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief          allocate h/w resources for this object
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief          program all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) override;

    /// \brief          reprogram all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any
    /// \param[in] api_op    API operation
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reprogram_config(api_op_t api_op) override;

    /// \brief          free h/w resources used by this object, if any
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief          cleanup all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override;

    /// \brief          update all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_config(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /// \param[in]      epoch       epoch being activated
    /// \param[in]      api_op      api operation
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      obj_ctxt_t *obj_ctxt) override;

    /// \brief re-activate config in the hardware stage 0 tables relevant to
    ///        this object, if any, this reactivation must be based on existing
    ///        state and any of the state present in the dirty object list
    ///        (like clone objects etc.) only and not directly on db objects
    /// \param[in] api_op API operation
    /// \return #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reactivate_config(pds_epoch_t epoch,
                                        api_op_t api_op) override;

    /// \brief          add given policer to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given policer from the database
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
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief          read config
    /// \param[in]      key pointer to the key object
    /// \param[out]     info pointer to the info object
    /// \return         SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_policer_key_t *key, pds_policer_info_t *info);

    /// \brief          return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "policer-" + std::to_string(key_.id);
    }

    /// \brief          helper function to get key given policer
    /// \param[in]      entry    pointer to policer instance
    /// \return         pointer to the policer instance's key
    static void *policer_key_func_get(void *entry) {
        policer *pol = (policer *)entry;
        return (void *)&(pol->key_);
    }

    /// \brief   helper function to get size of key
    /// \return  size of key
    static uint32_t key_size(void) {
        return sizeof(pds_policer_key_t);
    }

    /// \brief          return the policer key/id
    /// \return         key/id of the policer
    pds_policer_key_t key(void) const { return key_; }

    /// \brief    return the traffic direction in which policer is applied
    /// \return   direction in which policer is enforced
    pds_policer_dir_t dir(void) const { return dir_; }

    /// \brief     return impl instance of this policer object
    /// \return    impl instance of the policer object
    impl_base *impl(void) { return impl_; }

private:
    /// \brief constructor
    policer();

    /// \brief destructor
    ~policer();

    /// \brief    free h/w resources used by this object, if any
    ///           (this API is invoked during object deletes)
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_policer_key_t key_;        ///< policer key
    pds_policer_dir_t dir_;       ///< policer enforcement direction
    ht_ctxt_t ht_ctxt_;            ///< hash table context

    // P4 datapath specific state
    impl_base *impl_;              ///< impl object instance
    friend class policer_state;    ///< policer_state is friend of policer
} __PACK__;

/// \@}    // end of PDS_POLICER_ENTRY

}    // namespace api

using api::policer;

#endif    // __API_POLICER_HPP__
