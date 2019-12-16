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
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
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
    API_BATCH_STAGE_RESERVE_RESOURCES,    ///< Reserve resources, if any
    API_BATCH_STAGE_OBJ_DEPENDENCY,       ///< Dependency resolution stage
    API_BATCH_STAGE_PROGRAM_CONFIG,       ///< Table programming stage
    API_BATCH_STAGE_CONFIG_ACTIVATE,      ///< Epoch activation stage
    API_BATCH_STAGE_ABORT,                ///< Abort stage
} api_batch_stage_t;

/// \brief    per API object context
/// transient information maintained while a batch of APIs are being processed
///
/// \remark
///   api_params is not owned by this structure, so don't free it ... it
///   is owned by api_ctxt_t and hence when api_ctxt_t is being destroyed
///   we should return the api_params_t memory back to slab
typedef struct api_obj_ctxt_s api_obj_ctxt_t;
struct api_obj_ctxt_s {
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

    api_obj_ctxt_s() {
        api_op = API_OP_INVALID;
        obj_id = OBJ_ID_NONE;
        api_params = NULL;
        cloned_obj = NULL;
        cb_ctxt = NULL;
        upd_bmap = 0;
        rsvd_rscs = 0;
        hw_dirty = 0;
    }

    bool operator==(const api_obj_ctxt_s& rhs) const {
        if ((this->api_op == rhs.api_op) &&
            (this->obj_id == rhs.obj_id) &&
            (this->api_params == rhs.api_params) &&
            (this->cloned_obj == rhs.cloned_obj) &&
            (this->cb_ctxt == rhs.cb_ctxt)) {
            return true;
        }
        return false;
    }
};

// objects on which add/del/upd API calls are issued are put in a dirty
// list/map by API framework to de-dup potentially multiple API operations
// issued by caller in same batch
typedef unordered_map<api_base *, api_obj_ctxt_t *> dirty_obj_map_t;
typedef list<api_base *> dirty_obj_list_t;

// objects affected (i.e. dirtied) by add/del/upd operations on other objects
// are called affected/dependent objects and are maintained separately;
// normally these objects need to be either reprogrammed or deleted
typedef unordered_map<api_base *, api_obj_ctxt_t *> dep_obj_map_t;
typedef list<api_base *> dep_obj_list_t;

/// \brief    batch context, which is a list of all API contexts
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
    dirty_obj_map_t         dom;    ///< dirty object map
    dirty_obj_list_t        dol;    ///< dirty object list
    dep_obj_map_t           aom;    ///< affected/dependent object map
    dep_obj_list_t          aol;    ///< affected/dependent object list
} api_batch_ctxt_t;

/// \brief API counters maintained by API engine
typedef struct api_counters_s {
    // pre-process stage specific counters
    struct {
        // common counters
        uint32_t invalid_op_err;
        // CREATE specific counters
        struct {
            uint32_t ok;
            uint32_t oom_err;
            uint32_t init_cfg_err;
            uint32_t obj_clone_err;
            uint32_t obj_exists_err;
            uint32_t invalid_op_err;
            uint32_t invalid_upd_err;
        } create;
        // DELETE specific counters
        struct {
            uint32_t ok;
            uint32_t obj_build_err;
            uint32_t not_found_err;
        } del;
        // UPDATE specific counters
        struct {
            uint32_t ok;
            uint32_t obj_build_err;
            uint32_t init_cfg_err;
            uint32_t obj_clone_err;
            uint32_t not_found_err;
            uint32_t invalid_op_err;
            uint32_t invalid_upd_err;
        } upd;
    } preprocess;
    // resource reservation stage specific counters
    struct {
        // CREATE specific counters
        struct {
            uint32_t ok;
            uint32_t err;
        } create;
        // DELETE specific counters
        struct {
            uint32_t ok;
            uint32_t err;
        } del;
        // UPDATE specific counters
        struct {
            uint32_t ok;
            uint32_t err;
        } upd;
    } rsv_rsc;
    // object dependency stage specific counters
    struct {
        struct {
            uint32_t ok;
            uint32_t err;
        } upd;
    } obj_dep;
    // program config stage specific counters
    struct {
        // CREATE specific counters
        struct {
            uint32_t ok;
            uint32_t err;
        } create;
        // DELETE specific counters
        struct {
            uint32_t ok;
            uint32_t err;
        } del;
        // UPDATE specific counters
        struct {
            uint32_t ok;
            uint32_t err;
        } upd;
    } pgm_cfg;
    // activate config stage specific counters
    struct {
        // CREATE specific counters
        struct {
            uint32_t ok;
            uint32_t err;
        } create;
        // DELETE specific counters
        struct {
            uint32_t ok;
            uint32_t err;
        } del;
        // UPDATE specific counters
        struct {
            uint32_t ok;
            uint32_t err;
        } upd;
    } act_cfg;
    // re-program config stage specific counters
    struct {
        struct {
            uint32_t ok;
            uint32_t err;
        } upd;
    } re_pgm_cfg;
    // re-activate config stage specific counters
    struct {
        struct {
            uint32_t ok;
            uint32_t err;
        } upd;
    } re_act_cfg;
} api_counters_t;

/// \brief Encapsulation for all API processing framework
class api_engine {
public:
    /// \brief Constructor
    api_engine() {
        api_obj_ctxt_slab_ = NULL;
    }

    /// \brief Destructor
    ~api_engine() {}

    // API engine initialization function
    sdk_ret_t init(void) {
        api_obj_ctxt_slab_ =
            slab::factory("api-obj-ctxt", PDS_SLAB_ID_API_OBJ_CTXT,
                          sizeof(api_obj_ctxt_t), 512, true, true, true, NULL);
        if (api_obj_ctxt_slab_) {
            return SDK_RET_OK;
        }
        return SDK_RET_OOM;
    }

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

    /// \brief    given an object, return its cloned object, if the object is
    ///           found in the dirty object map and cloned
    /// param[in] api_obj    API object found in the db
    /// \return cloned object of the given object, if found, or else NULL
    api_base *cloned_obj(api_base *api_obj) {
        if (api_obj->in_dirty_list()) {
            api_obj_ctxt_t *octxt = batch_ctxt_.dom.find(api_obj)->second;
            if (octxt->cloned_obj) {
                return octxt->cloned_obj;
            }
        }
        return NULL;
    }

    /// \brief    given an object, return its cloned object, if the object is
    ///           found in the dirty object map and cloned or else return the
    ///           same object - essentially return the object that framework is
    ///           operating on in the current batch
    /// NOTE: this object may not be in the dirty list at all, in which case
    ///       its not part of current batch of API objects being processed
    ///       (i.e., it is a committed object)
    /// param[in] api_obj    API object found in the db
    /// \return framework object of the given object (i.e., either same or
    ///         cloned object)
    api_base *framework_obj(api_base *api_obj) {
        api_base *clone;

        clone = cloned_obj(api_obj);
        return clone ? clone : api_obj;
    }

    /// \brief Add given api object to dependent object list if its not
    //         in the dirty object list and dependent object list already
    /// \param[in] obj_id    object id identifying the API object
    /// \param[in] api_op    API operation due to which this call is triggered
    /// \param[in] api_obj   API object being added to the dependent list
    /// \param[in] upd_bmap  bitmap indicating which attributes of the API
    ///                      object are being updated
    /// \return pointer to the API object context containing cumulative update
    ///         bitmap if there is any new update or else NULL
    api_obj_ctxt_t *add_to_deps_list(obj_id_t obj_id, api_op_t api_op,
                                 api_base *api_obj, uint64_t upd_bmap) {
        api_obj_ctxt_t *octxt;

        if (api_obj->in_dirty_list()) {
            // if needed, update the update bitmap to indicate that more udpates
            // are seen on an object thats being updated in this batch
            if (batch_ctxt_.dom[api_obj]->cloned_obj) {
                if ((batch_ctxt_.dom[api_obj]->upd_bmap & upd_bmap) !=
                        upd_bmap) {
                    batch_ctxt_.dom[api_obj]->upd_bmap |= upd_bmap;
                    PDS_TRACE_DEBUG("%s already in DoL, updated upd bmap to "
                                    "0x%lx", api_obj->key2str(),
                                    batch_ctxt_.dom[api_obj]->upd_bmap);
                    return batch_ctxt_.dom[api_obj];
                }
                // entry exists and the update was already noted
                PDS_TRACE_DEBUG("%s already in DoL", api_obj->key2str());
                return NULL;
            }
#if 0
            // NOTE: re-ordering objects here will break functionality
            // if the object is in dirty list already, move it to the end
            // as this update can trigger other updates
            batch_ctxt_.dol.remove(api_obj);
            batch_ctxt_.dol.push_back(api_obj);
#endif
            // fall thru
        }
        if (api_obj->in_deps_list()) {
            // if needed, update the update bitmap
            if ((batch_ctxt_.aom[api_obj]->upd_bmap & upd_bmap) != upd_bmap) {
                batch_ctxt_.aom[api_obj]->upd_bmap |= upd_bmap;
                PDS_TRACE_DEBUG("%s already in AoL, update upd bmap to 0x%lx",
                                api_obj->key2str(),
                                batch_ctxt_.aom[api_obj]->upd_bmap);
                return batch_ctxt_.aom[api_obj];
            }
            // if the object is in deps list already, push it to the end
            // NOTE: this reshuffling doesn't help as the whole dependent
            //       list is processed after the dirty object list with updated
            //       specs anyway
            batch_ctxt_.aol.remove(api_obj);
            batch_ctxt_.aol.push_back(api_obj);
            // entry exists and the update was already noted
            PDS_TRACE_DEBUG("%s already in AoL", api_obj->key2str());
            return NULL;
        }

        // object not in dirty list or dependent list, add it now to
        // affected/dependent object list/map
        octxt = api_obj_ctxt_alloc_();
        octxt->api_op = api_op;
        octxt->obj_id = obj_id;
        octxt->upd_bmap = upd_bmap;
        api_obj->set_in_deps_list();
        batch_ctxt_.aom[api_obj] = octxt;
        batch_ctxt_.aol.push_back(api_obj);
        PDS_TRACE_DEBUG("Added %s to AoL, update bitmap 0x%lx",
                        api_obj->key2str(), upd_bmap);
        return octxt;
    }

private:

    /// \brief    allocate and return an api object context entry
    /// \return    allocated API object context entry or NULL
    api_obj_ctxt_t *api_obj_ctxt_alloc_(void) {
        api_obj_ctxt_t *octxt;

        octxt = (api_obj_ctxt_t *)api_obj_ctxt_slab_->alloc();
        if (octxt) {
            new (octxt) api_obj_ctxt_t();
        }
        return octxt;
    }

    /// \brief    free given object context entry back to its slab
    /// \param[in]    API object context pointer to be freed
    void api_obj_ctxt_free_(api_obj_ctxt_t *octxt) {
        if (octxt) {
            octxt->~api_obj_ctxt_t();
            api_obj_ctxt_slab_->free(octxt);
        }
    }

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
    sdk_ret_t reserve_resources_(api_base *api_obj, api_obj_ctxt_t *obj_ctxt);

    /// \brief Process given object from the dirty list
    /// This is done by doing add/update of corresponding h/w entries, based
    /// on accumulated configuration without activating the epoch
    ///
    /// \param[in] api_obj API object being processed
    /// \param[in] obj_ctxt Transient information maintained to process the API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t program_config_(api_base *api_obj, api_obj_ctxt_t *obj_ctxt);

    /// \brief Add objects that are dependent on given object to dependent
    ///        object list
    /// \param[in] api_obj API object being processed
    /// \param[in] obj_ctxt Transient information maintained to process the API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t add_deps_(api_base *api_obj, api_obj_ctxt_t *obj_ctxt);

    /// \brief Activate configuration by switching to new epoch
    /// If object has effected any stage 0 datapath table(s), switch to new
    /// epoch in this stage NOTE: NO failures must happen in this stage
    ///
    /// \param[in] api_obj API object being processed
    /// \param[in] it    iterator position of api obj to be deleted
    /// \param[in] obj_ctxt    transient information maintained to process API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t activate_config_(dirty_obj_list_t::iterator it,
                               api_base *api_obj, api_obj_ctxt_t *obj_ctxt);

    /// \brief Abort all changes made to an object, rollback to its prev state
    /// NOTE: this is not expected to fail and also epoch is not activated if
    /// we are here
    ///
    /// \param[in] api_obj API object being processed
    /// \param[in] it    iterator position of api obj to be deleted
    /// \param[in] obj_ctxt    transient information maintained to process API
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t rollback_config_(dirty_obj_list_t::iterator it,
                               api_base *api_obj, api_obj_ctxt_t *obj_ctxt);

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
    void add_to_dirty_list_(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
        api_obj->set_in_dirty_list();
        batch_ctxt_.dom[api_obj] = obj_ctxt;
        batch_ctxt_.dol.push_back(api_obj);
    }

    /// \brief Delete given api object from dirty list of the API batch
    /// \param[in] it iterator position of api obj to be deleted
    /// \param[in] api_obj API object being processed
    void del_from_dirty_list_(dirty_obj_list_t::iterator it,
                              api_base *api_obj) {
        batch_ctxt_.dol.erase(it);
        batch_ctxt_.dom.erase(api_obj);
        api_obj->clear_in_dirty_list();
    }

    /// \brief Delete given api object from dependent object list
    /// \param[in] it iterator position of api obj to be deleted
    /// \param[in] api_obj API object being processed
    void del_from_deps_list_(dep_obj_list_t::iterator it,
                             api_base *api_obj) {
        batch_ctxt_.aol.erase(it);
        batch_ctxt_.aom.erase(api_obj);
        api_obj->clear_in_deps_list();
    }

private:
    friend api_obj_ctxt_t;

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
    api_counters_t      counters_;
    slab                *api_obj_ctxt_slab_;
};

/// \brief    initialize the API engine
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t api_engine_init(void);

/// \brief    return the API engine instance
/// \return    pointer to the (singleton) API engine instance
api_engine *api_engine_get(void);

/// \brief    given an api object, add it to the dependent object list and
///           recursively add any other objects that may needed to be updated
/// \param[in] obj_id    object id identifying the API object
/// \param[in] api_op    API operation due to which this call is triggered
/// \param[in] api_obj   API object being added to the dependent list
/// \param[in] upd_bmap  bitmap indicating which attributes of the API object
///                      are being updated
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t api_obj_add_to_deps(obj_id_t obj_id, api_op_t api_op,
                              api_base *api_obj, uint64_t upd_bmap);

/// \@}

}    // namespace api

using api::api_obj_ctxt_t;

#endif    // __FRAMEWORK_API_ENGINE_HPP__
