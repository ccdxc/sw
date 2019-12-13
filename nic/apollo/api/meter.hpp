//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// meter entry handling
///
//----------------------------------------------------------------------------

#ifndef __API_METER_HPP__
#define __API_METER_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"

namespace api {

// forward declaration
class meter_state;

/// \defgroup PDS_METER_ENTRY - meter entry functionality
/// \ingroup PDS_METER
/// @{

/// \brief    meter entry
class meter_entry : public api_base {
public:
    /// \brief          factory method to allocate and initialize a meter entry
    /// \param[in]      spec    meter information
    /// \return         new instance of meter or NULL, in case of error
    static meter_entry *factory(pds_meter_spec_t *spec);

    /// \brief          release all the s/w state associate with the given
    ///                 meter, if any, and free the memory
    /// \param[in]      meter     meter to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(meter_entry *meter);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] meter    meter entry to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(meter_entry *meter);

    /// \brief          initialize meter entry with the given config
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
        // meter delete case, we simply have to free the resources in
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

    /// \brief          add given meter to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given meter from the database
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
        return "meter-" + std::to_string(key_.id);
    }

    /// \brief          helper function to get key given meter entry
    /// \param          entry    pointer to meter instance
    /// \return         pointer to the meter instance's key
    static void *meter_key_func_get(void *entry) {
        meter_entry *meter = (meter_entry *)entry;
        return (void *)&(meter->key_);
    }

    /// \brief          return the meter entry's key/id
    /// \return         key/id of the meter entry
    pds_meter_key_t key(void) const { return key_; }

    /// \brief return address family of this meter entry
    /// \return IP_AF_IPV4, if meter entry is IPv4 or else IP_AF_IPV6
    uint8_t af(void) const { return af_; }

    /// \brief     return impl instance of this meter entry object
    /// \return    impl instance of the meter entry object
    impl_base *impl(void) { return impl_; }

    /// \brief      read meter entry from hardware
    /// \param[out] info pointer to the info object
    /// \return     #SDK_RET_OK on success, error code in case of failure
    sdk_ret_t read(pds_meter_info_t *info);

private:
    /// \brief constructor
    meter_entry();

    /// \brief destructor
    ~meter_entry();

    /// \brief      fill the meter sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_meter_spec_t *spec);

    /// \brief    free h/w resources used by this object, if any
    ///           (this API is invoked during object deletes)
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_meter_key_t key_;        ///< meter key
    uint8_t af_;                 ///< IP_AF_IPV4 or IP_AF_IPV6
    ht_ctxt_t ht_ctxt_;          ///< hash table context
    impl_base *impl_;            ///< impl object instance

    friend class meter_state;    ///< meter_state is friend of meter_entry
} __PACK__;

/// \@}    // end of PDS_METER_ENTRY

}    // namespace api

using api::meter_entry;

#endif    // __API_METER_HPP__
