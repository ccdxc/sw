//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// API processing framework/engine functionality
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_ENGINE_HPP__
#define __FRAMEWORK_API_ENGINE_HPP__

#include <vector>
#include <unordered_map>
#include <list>
#include <utility>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"

using std::vector;
using std::unordered_map;
using std::list;

namespace api {

/// \defgroup PDS_API_ENGINE Framework for processing APIs
/// @{

/// \brief Processing stage of the APIs in a given batch
typedef enum api_batch_stage_e {
    API_BATCH_STAGE_NONE,                 ///< Invalid stage
    API_BATCH_STAGE_INIT,                 ///< Initialization stage
    API_BATCH_STAGE_PRE_PROCESS,          ///< Pre-processing stage
    API_BATCH_STAGE_OBJ_DEPENDENCY,       ///< Dependency resolution stage
    API_BATCH_STAGE_RESERVE_RESOURCES,    ///< Reserve resources, if any
    API_BATCH_STAGE_PROGRAM_CONFIG,       ///< Table programming stage
    API_BATCH_STAGE_CONFIG_ACTIVATE,      ///< Epoch activation stage
    API_BATCH_STAGE_ABORT,                ///< Abort stage
} api_batch_stage_t;

/// \brief Per API object context
/// transient information maintained while a batch of APIs are being processed
///
/// \remark
///   api_params is not owned by this structure, so don't free it ... it
///   is owned by api_ctxt_t and hence when api_ctxt_t is being destroyed
///   we should return the api_params_t memory back to slab
typedef struct obj_ctxt_s obj_ctxt_t;
struct obj_ctxt_s {
    api_op_t        api_op;         ///< de-duped/compressed API opcode
    obj_id_t        obj_id;         ///< object identifier
    api_params_t    *api_params;    ///< API specific parameters
    api_base        *cloned_obj;    ///< cloned object, for UPD processing

    ///< object handlers can save arbitrary state across callbacks here and it
    ///< is opaque to the api engine
    struct {
        void        *cb_ctxt;
        uint64_t    upd_bmap;
    };
    uint8_t rsvd_rscs:1;          ///< true if resource reservation stage is done
    uint8_t hw_dirty:1;           ///< true if hw entries are updated,
                                  ///< but not yet activated

    obj_ctxt_s() {
        api_op = API_OP_INVALID;
        obj_id = OBJ_ID_NONE;
        api_params = NULL;
        cloned_obj = NULL;
        cb_ctxt = NULL;
        upd_bmap = 0;
    }

    bool operator==(const obj_ctxt_s& rhs) const {
        if ((this->api_op == rhs.api_op) &&
            (this->obj_id == rhs.obj_id) &&
            (this->api_params == rhs.api_params) &&
            (this->cloned_obj == rhs.cloned_obj) &&
            (this->cb_ctxt == rhs.cb_ctxt)) {
            return true;
        }
        return false;
    }
    /// \brief    add the given api object to dependency list, if it is not
    ///           already present in the list and recursively add the objects
    ///           that get effected if this object is updated
    sdk_ret_t add_deps(api_base *obj, api_op_t api_op);
};

// objects on which add/del/upd API calls are issued are put in a dirty
// list/map by API framework to de-dup potentially multiple API operations
// issued by caller in same batch ... these are called master objects
typedef unordered_map<api_base *, obj_ctxt_t> dirty_obj_map_t;
typedef list<api_base *> dirty_obj_list_t;

// objects affected (i.e. dirtied) by add/del/upd operations on other objects
// (aka. master objects) are called puppet/dependent objects and are maintained
// separately; normally these objects need to be either reprogrammed or deleted
typedef unordered_map<api_base *, api_op_t> dep_obj_map_t;
typedef list<api_base *> dep_obj_list_t;

/// \brief Batch context, which is a list of all API contexts
typedef struct api_batch_ctxt_s {
    pds_epoch_t             epoch;           ///< epoch in progress, passed in
                                             ///< pds_batch_begin()
    api_batch_stage_t       stage;           ///< phase of the batch processing
    vector<api_ctxt_t *>    *api_ctxts;      ///< API contexts per batch

    // dirty object map is needed because in the same batch we could have
    // multiple modifications of same object, like security rules change and
    // route changes can happen for same vnic in two different API calls but in
    // same batch and we need to activate them in one write (not one after
    // another)
    dirty_obj_map_t         dirty_obj_map;   ///< dirty object map
    dirty_obj_list_t        dirty_obj_list;  ///< dirty object list
    dep_obj_map_t           dep_obj_map;     ///< dependent object map
    dep_obj_list_t          dep_obj_list;    ///< dependent object list
} api_batch_ctxt_t;

/// \brief API counters maintained by API engine
typedef struct api_counters_s {
    struct {
        struct {
            uint64_t ok;
            uint64_t oom_err;
            uint64_t init_cfg_err;
            uint64_t obj_exists_err;
            uint64_t invalid_op_err;
        } create;
        struct {
            uint64_t ok;
            uint64_t obj_build_err;
            uint64_t not_found_err;
        } del;
        struct {
            uint64_t ok;
            uint64_t obj_build_err;
            uint64_t init_cfg_err;
            uint64_t not_found_err;
        } upd;
    } preprocess;
} api_counters_t;

/// \brief Encapsulation for all API processing framework
class api_engine {
public:
    /// \brief Constructor
    api_engine() {}

    /// \brief Destructor
    ~api_engine() {}

    /// \brief Handle batch begin by setting up per API batch context
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t batch_begin(pds_batch_params_t *batch_params);

    /// \brief Commit all the APIs in this batch
    /// Release any temporary state or resources like memory, per API context
    /// info etc.
    /// param[in] batch    batch of APIs to process
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t batch_commit(batch_info_t *batch);

private:

    /// \brief De-dup given API operation
    /// This is based on the currently computed operation and new API operation
    /// seen on the object
    ///
    /// \param[in] curr_op Current outstanding API operation on the object
    /// \param[in] new_op Newly encountered API operation on the object
    /// \return De-duped/compressed API operation
    api_op_t api_op_(api_op_t curr_op, api_op_t new_op);

    /// \brief Pre-process create operation and form effected list of objs
    ///
    /// \param[in] api_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t pre_process_create_(api_ctxt_t *api_ctxt);

    /// \brief Pre-process delete operation and form effected list of objs
    ///
    /// \param[in] api_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t pre_process_delete_(api_ctxt_t *api_ctxt);

    /// \brief Pre-process update operation and form effected list of objs
    ///
    /// \param[in] api_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t pre_process_update_(api_ctxt_t *api_ctxt);

    /// \brief Process an API and form effected list of objs
    ///
    /// \param[in] api_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t pre_process_api_(api_ctxt_t *api_ctxt);

    /// \brief Allocate any sw & hw resources for the given object and operation
    ///
    /// \param[in] api_obj API object being processed
    /// \param[in] obj_ctxt Transient information maintained to process the API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t reserve_resources_(api_base *api_obj, obj_ctxt_t *obj_ctxt);

    /// \brief Process given object from the dirty list
    /// This is done by doing add/update of corresponding h/w entries, based
    /// on accumulated configuration without activating the epoch
    ///
    /// \param[in] api_obj API object being processed
    /// \param[in] obj_ctxt Transient information maintained to process the API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t program_config_(api_base *api_obj, obj_ctxt_t *obj_ctxt);

    /// \brief Add objects that are dependent on given object to dependent
    ///        object list
    /// \param[in] api_obj API object being processed
    /// \param[in] obj_ctxt Transient information maintained to process the API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_deps_(api_base *api_obj, obj_ctxt_t *obj_ctxt);

    /// \brief Activate configuration by switching to new epoch
    /// If object has effected any stage 0 datapath table(s), switch to new
    /// epoch in this stage NOTE: NO failures must happen in this stage
    ///
    /// \param[in] api_obj API object being processed
    /// \param[in] it    iterator position of api obj to be deleted
    /// \param[in] obj_ctxt    transient information maintained to process API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_config_(dirty_obj_list_t::iterator it,
                               api_base *api_obj, obj_ctxt_t *obj_ctxt);

    /// \brief Abort all changes made to an object, rollback to its prev state
    /// NOTE: this is not expected to fail and also epoch is not activated if
    /// we are here
    ///
    /// \param[in] api_obj API object being processed
    /// \param[in] it    iterator position of api obj to be deleted
    /// \param[in] obj_ctxt    transient information maintained to process API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t rollback_config_(dirty_obj_list_t::iterator it,
                               api_base *api_obj, obj_ctxt_t *obj_ctxt);

    /// \brief Pre-process all API calls in a given batch
    /// Form a dirty list of effected obejcts as a result
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t pre_process_stage_(void);

    /// \brief compute list of affected objects that need to reprogrammed
    ///        because of API operations on dirty list objects
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t obj_dependency_computation_stage_(void);

    /// \brief reserve all needed resources for programming the config
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t resource_reservation_stage_(void);

    /// \brief Datapath table update stage
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t program_config_stage_(void);

    /// \brief Final epoch activation stage
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_config_stage_(void);

    /// \brief Abort all the APIs in this batch
    /// Release any temporary state or resources like memory, per API context
    /// info etc.
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t batch_abort_(void);

    /// \brief Add given api object to dirty list of the API batch
    /// \param[in] api_obj API object being processed
    /// \param[in] obj_ctxt Transient information maintained to process the API
    void add_to_dirty_list_(api_base *api_obj, obj_ctxt_t obj_ctxt) {
        api_obj->set_in_dirty_list();
        batch_ctxt_.dirty_obj_map[api_obj] = obj_ctxt;
        batch_ctxt_.dirty_obj_list.push_back(api_obj);
    }

    /// \brief Delete given api object from dirty list of the API batch
    /// \param[in] it iterator position of api obj to be deleted
    /// \param[in] api_obj API object being processed
    void del_from_dirty_list_(dirty_obj_list_t::iterator it,
                              api_base *api_obj) {
        batch_ctxt_.dirty_obj_list.erase(it);
        batch_ctxt_.dirty_obj_map.erase(api_obj);
        api_obj->clear_in_dirty_list();
    }

    /// \brief Add given api object to dependent/puppet object list if its not
    //         in the dirty object list and dependent object list already
    /// \param[in] api_obj API object being processed
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_to_deps_list_(api_base *api_obj, api_op_t api_op) {
        if (api_obj->in_dirty_list()) {
            return SDK_RET_ENTRY_EXISTS;
        }
        if (api_obj->in_deps_list()) {
            return SDK_RET_ENTRY_EXISTS;
        }
        api_obj->set_in_deps_list();
        batch_ctxt_.dep_obj_map[api_obj] = api_op;
        batch_ctxt_.dep_obj_list.push_back(api_obj);
        return SDK_RET_OK;
    }

    /// \brief Delete given api object from dependent/puppet object list
    /// \param[in] it iterator position of api obj to be deleted
    /// \param[in] api_obj API object being processed
    void del_from_deps_list_(dirty_obj_list_t::iterator it,
                             api_base *api_obj) {
        batch_ctxt_.dep_obj_list.erase(it);
        batch_ctxt_.dep_obj_map.erase(api_obj);
        api_obj->clear_in_deps_list();
    }

private:
    friend obj_ctxt_t;

    /// \brief API operation de-dup matrix
    api_op_t dedup_api_op_[API_OP_INVALID][API_OP_INVALID] = {
        // API_OP_NONE
        {API_OP_INVALID, API_OP_CREATE, API_OP_INVALID, API_OP_INVALID },
        // API_OP_CREATE
        {API_OP_INVALID, API_OP_INVALID, API_OP_NONE, API_OP_CREATE },
        // API_OP_DELETE
        {API_OP_INVALID, API_OP_UPDATE, API_OP_DELETE, API_OP_INVALID },
        // API_OP_UPDATE
        {API_OP_INVALID, API_OP_INVALID, API_OP_DELETE, API_OP_UPDATE },
    };
    api_batch_ctxt_t    batch_ctxt_;
    slab                *api_params_slab_;
    api_counters_t      counters_;
};

/// \brief    initialize the API engine
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t api_engine_init(void);

/// \brief    return the API engine instance
/// \return    pointer to the (singleton) API engine instance
api_engine *api_engine_get(void);

/// \@}

}    // namespace api

using api::obj_ctxt_t;

#endif    // __FRAMEWORK_API_ENGINE_HPP__
