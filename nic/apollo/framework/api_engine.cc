//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// API engine functionality
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"

#define PDS_API_PREPROCESS_CREATE_COUNTER_INC(cntr_, val)    \
            counters_.preprocess.create.cntr_ += (val)
#define PDS_API_PREPROCESS_DELETE_COUNTER_INC(cntr_, val)    \
            counters_.preprocess.del.cntr_ += (val)
#define PDS_API_PREPROCESS_UPDATE_COUNTER_INC(cntr_, val)    \
            counters_.preprocess.upd.cntr_ += (val)

// TODO: is there a need to clone a stateless obj ? can we reuse built obj ?

namespace api {

static slab *g_api_ctxt_slab_ = NULL;
static slab *g_api_msg_slab_ = NULL;

/// \defgroup PDS_API_ENGINE Framework for processing APIs
/// @{

static pds_epoch_t    g_current_epoch_ = PDS_EPOCH_INVALID;
static thread_local api_engine    g_api_engine;

api_engine *
api_engine_get (void) {
    return &g_api_engine;
}

slab *
api_ctxt_slab (void)
{
    return g_api_ctxt_slab_;
}

slab *
api_msg_slab (void)
{
    return g_api_msg_slab_;
}

pds_epoch_t
get_current_epoch (void)
{
    return g_current_epoch_;
}

sdk_ret_t
obj_ctxt_t::add_deps(api_base *api_obj, api_op_t api_op) {
    sdk_ret_t ret;

    ret = g_api_engine.add_to_deps_list_(api_obj, api_op);
    if (ret == SDK_RET_OK) {
        api_obj->add_deps(this);
    }
    return SDK_RET_OK;
}

api_op_t
api_engine::api_op_(api_op_t old_op, api_op_t new_op) {
    return dedup_api_op_[old_op][new_op];
}

sdk_ret_t
api_engine::pre_process_create_(api_ctxt_t *api_ctxt) {
    sdk_ret_t     ret;
    obj_ctxt_t    obj_ctxt;
    api_base      *api_obj;

    api_obj = api_base::find_obj(api_ctxt);
    if (api_obj == NULL) {
        // instantiate a new object
        api_obj = api_base::factory(api_ctxt);
        if (unlikely(api_obj == NULL)) {
            batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
            PDS_API_PREPROCESS_CREATE_COUNTER_INC(oom_err, 1);
            return SDK_RET_ERR;
        }
        PDS_TRACE_VERBOSE("Allocated api obj %p, api op %u, obj id %u",
                          api_obj, api_ctxt->api_op, api_ctxt->obj_id);
        // add it to dirty object list
        obj_ctxt.api_op = API_OP_CREATE;
        obj_ctxt.obj_id = api_ctxt->obj_id;
        obj_ctxt.api_params = api_ctxt->api_params;
        add_to_dirty_list_(api_obj, obj_ctxt);
        // initialize the object with the given config
        ret = api_obj->init_config(api_ctxt);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_API_PREPROCESS_CREATE_COUNTER_INC(init_cfg_err, 1);
            PDS_TRACE_ERR("Config initialization failed for obj id %u, "
                          "api op %u, err %u", api_ctxt->obj_id,
                          api_ctxt->api_op, ret);
            return ret;
        }
        ret = api_obj->add_to_db();
        SDK_ASSERT(ret == SDK_RET_OK);
    } else {
        // this could be XXX-DEL-ADD/ADD-XXX-DEL-XXX-ADD kind of scenario in
        // same batch, as we don't expect to see ADD-XXX-ADD (unless we want
        // to support idempotency)
        if (unlikely(api_obj->in_dirty_list() == false)) {
            // attemping a CREATE on already created object
            PDS_TRACE_ERR("Creating an obj that exists already, obj %s",
                          api_obj->key2str().c_str());
            PDS_API_PREPROCESS_CREATE_COUNTER_INC(obj_exists_err, 1);
            return SDK_RET_ENTRY_EXISTS;
        }
        obj_ctxt_t& octxt = batch_ctxt_.dirty_obj_map.find(api_obj)->second;
        if (unlikely((octxt.api_op != API_OP_NONE) &&
                     (octxt.api_op != API_OP_DELETE))) {
            // only API_OP_NONE and API_OP_DELETE are expected in current state
            // in the dirty obj map
            PDS_TRACE_ERR("Invalid CREATE operation on obj %s, id %u with "
                          "outstanding api op %u", api_obj->key2str().c_str(),
                          api_ctxt->obj_id, octxt.api_op);
            PDS_API_PREPROCESS_CREATE_COUNTER_INC(invalid_op_err, 1);
            return SDK_RET_INVALID_OP;
        }
        octxt.api_op = api_op_(octxt.api_op, API_OP_CREATE);
        if (unlikely(octxt.api_op == API_OP_INVALID)) {
            PDS_TRACE_ERR("Invalid resultant api op on obj %s id %u with "
                          "API_OP_CREATE", api_obj->key2str().c_str(),
                          api_ctxt->obj_id);
            PDS_API_PREPROCESS_CREATE_COUNTER_INC(invalid_op_err, 1);
            return SDK_RET_INVALID_OP;
        }

        // update the config, by cloning the object, if needed
        if (octxt.cloned_obj == NULL) {
            // XXX-DEL-ADD or ADD-XXX-DEL-XXX-ADD scenarios, we need to
            // differentiate between these two
            if (octxt.api_op == API_OP_UPDATE) {
                // XXX-DEL-ADD scenario, clone & re-init cfg
                octxt.api_params = api_ctxt->api_params;
                octxt.cloned_obj = api_obj->clone(api_ctxt);
                ret = octxt.cloned_obj->init_config(api_ctxt);
                if (unlikely(ret != SDK_RET_OK)) {
                    PDS_TRACE_ERR("Config initialization failed for obj %s, "
                                  "id %u, api op %u, err %u",
                                  api_obj->key2str().c_str(), api_ctxt->obj_id,
                                  octxt.api_op, ret);
                    PDS_API_PREPROCESS_CREATE_COUNTER_INC(init_cfg_err, 1);
                    return ret;
                }
            } else {
                // ADD-XXX-DEL-XXX-ADD scenario, re-init same object
                if (unlikely(octxt.api_op != API_OP_CREATE)) {
                    PDS_TRACE_ERR("Unexpected api op %u, obj %s, id %u, "
                                  "expected CREATE", octxt.api_op,
                                  api_obj->key2str().c_str(), api_ctxt->obj_id);
                    PDS_API_PREPROCESS_CREATE_COUNTER_INC(invalid_op_err, 1);
                    return SDK_RET_INVALID_OP;
                }
                octxt.api_params = api_ctxt->api_params;
                ret = api_obj->init_config(api_ctxt);
                if (unlikely(ret != SDK_RET_OK)) {
                    PDS_TRACE_ERR("Config initialization failed for obj %s, "
                                  "id %u, api op %u, err %u",
                                  api_obj->key2str().c_str(), api_ctxt->obj_id,
                                  octxt.api_op, ret);
                    PDS_API_PREPROCESS_CREATE_COUNTER_INC(init_cfg_err, 1);
                    return ret;
                }
            }
        } else {
            // UPD-XXX-DEL-XXX-ADD scenario, re-init cloned obj's cfg (if
            // we update original obj, we may not be able to abort later)
            if (unlikely(octxt.api_op != API_OP_UPDATE)) {
                PDS_TRACE_ERR("Unexpected api op %u, obj %s, id %u, "
                              "expected CREATE", octxt.api_op,
                              api_obj->key2str().c_str(), api_ctxt->obj_id);
                PDS_API_PREPROCESS_CREATE_COUNTER_INC(invalid_op_err, 1);
                return SDK_RET_INVALID_OP;
            }
            octxt.api_params = api_ctxt->api_params;
            ret = octxt.cloned_obj->init_config(api_ctxt);
            if (unlikely(ret != SDK_RET_OK)) {
                PDS_TRACE_ERR("Config initialization failed for obj %s, "
                              "id %u, api op %u, err %u",
                              api_obj->key2str().c_str(), api_ctxt->obj_id,
                              octxt.api_op, ret);
                PDS_API_PREPROCESS_CREATE_COUNTER_INC(init_cfg_err, 1);
                return ret;
            }
        }
    }
    PDS_API_PREPROCESS_CREATE_COUNTER_INC(ok, 1);
    return ret;
}

sdk_ret_t
api_engine::pre_process_delete_(api_ctxt_t *api_ctxt) {
    obj_ctxt_t    obj_ctxt;
    api_base      *api_obj;

    // look for the object in the cfg db
    api_obj = api_base::find_obj(api_ctxt);
    if (api_obj == nullptr) {
        if (api_base::stateless(api_ctxt->obj_id)) {
            // build stateless objects on the fly
            api_obj = api_base::build(api_ctxt);
            if (api_obj) {
                // and temporarily add to the db (these must be removed during
                // commit/rollback operation)
                api_obj->add_to_db();
            } else {
                PDS_TRACE_ERR("Failed to find/build stateless obj %u",
                              api_ctxt->obj_id);
                PDS_API_PREPROCESS_DELETE_COUNTER_INC(obj_build_err, 1);
                return sdk::SDK_RET_ENTRY_NOT_FOUND;
            }
        } else {
            PDS_TRACE_ERR("Failed to preprocess delete on obj %u",
                          api_ctxt->obj_id);
            PDS_API_PREPROCESS_DELETE_COUNTER_INC(not_found_err, 1);
            return sdk::SDK_RET_ENTRY_NOT_FOUND;
        }
    }

    // by now either we built the object (and added to db) or found the object
    // that is in the db
    if (api_obj->in_dirty_list()) {
        // NOTE.
        // 1. for stateful objects, this probably is a cloned obj
        // (e.g., UPD-XXX-DEL), but that doesn't matter here
        // 2. if this is a stateless obj, we probably built this object, either
        //    due to ADD/UPD before and added to the db
        batch_ctxt_.dirty_obj_map[api_obj].api_op =
            api_op_(batch_ctxt_.dirty_obj_map[api_obj].api_op,
                    API_OP_DELETE);
        SDK_ASSERT(batch_ctxt_.dirty_obj_map[api_obj].api_op !=
                   API_OP_INVALID);
    } else {
        // add the object to dirty list
        obj_ctxt.api_op = API_OP_DELETE;
        obj_ctxt.obj_id = api_ctxt->obj_id;
        add_to_dirty_list_(api_obj, obj_ctxt);
    }
    PDS_API_PREPROCESS_DELETE_COUNTER_INC(ok, 1);
    return SDK_RET_OK;
}

sdk_ret_t
api_engine::pre_process_update_(api_ctxt_t *api_ctxt) {
    sdk_ret_t     ret;
    obj_ctxt_t    obj_ctxt;
    api_base      *api_obj;

    api_obj = api_base::find_obj(api_ctxt);
    if (api_obj == nullptr) {
        if (api_base::stateless(api_ctxt->obj_id)) {
            // build stateless objects on the fly
            api_obj = api_base::build(api_ctxt);
            if (api_obj) {
                // and temporarily add to the db (these must be removed during
                // commit/rollback operation)
                api_obj->add_to_db();
            } else {
                PDS_TRACE_ERR("Failed to find/build stateless obj %u",
                              api_ctxt->obj_id);
                PDS_API_PREPROCESS_UPDATE_COUNTER_INC(obj_build_err, 1);
                return sdk::SDK_RET_ENTRY_NOT_FOUND;
            }
        } else {
            PDS_API_PREPROCESS_UPDATE_COUNTER_INC(not_found_err, 1);
            return sdk::SDK_RET_ENTRY_NOT_FOUND;
        }
    }

    if (api_obj->in_dirty_list()) {
        obj_ctxt_t& octxt =
            batch_ctxt_.dirty_obj_map.find(api_obj)->second;
        octxt.api_op = api_op_(octxt.api_op, API_OP_UPDATE);
        SDK_ASSERT(octxt.api_op != API_OP_INVALID);
        if (octxt.cloned_obj == NULL) {
            // XXX-ADD-XXX-UPD in same batch, no need to clone yet, just
            // re-init the object with new config and continue with de-duped
            // operation as ADD
            octxt.api_params = api_ctxt->api_params;
            ret = api_obj->init_config(api_ctxt);
            if (ret != SDK_RET_OK) {
                PDS_API_PREPROCESS_UPDATE_COUNTER_INC(init_cfg_err, 1);
                return ret;
            }
        } else {
            // XXX-UPD-XXX-UPD scenario, update the cloned obj
            octxt.api_params = api_ctxt->api_params;
            ret = octxt.cloned_obj->init_config(api_ctxt);
            if (ret != SDK_RET_OK) {
                PDS_API_PREPROCESS_UPDATE_COUNTER_INC(init_cfg_err, 1);
                return ret;
            }
        }
    } else {
        // UPD-XXX scenario, update operation is seen 1st time on this
        // object in this batch
        obj_ctxt.api_op = API_OP_UPDATE;
        obj_ctxt.obj_id = api_ctxt->obj_id;
        obj_ctxt.cloned_obj = api_obj->clone(api_ctxt);
        obj_ctxt.api_params = api_ctxt->api_params;
        add_to_dirty_list_(api_obj, obj_ctxt);
        ret = obj_ctxt.cloned_obj->init_config(api_ctxt);
        if (ret != SDK_RET_OK) {
            PDS_API_PREPROCESS_UPDATE_COUNTER_INC(init_cfg_err, 1);
            return ret;
        }
    }
    PDS_API_PREPROCESS_UPDATE_COUNTER_INC(ok, 1);
    return SDK_RET_OK;
}

sdk_ret_t
api_engine::pre_process_api_(api_ctxt_t *api_ctxt) {
    sdk_ret_t     ret = SDK_RET_OK;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_PRE_PROCESS),
                       sdk::SDK_RET_INVALID_OP);
    switch (api_ctxt->api_op) {
    case API_OP_CREATE:
        ret = pre_process_create_(api_ctxt);
        break;

    case API_OP_DELETE:
        ret = pre_process_delete_(api_ctxt);
        break;

    case API_OP_UPDATE:
        ret = pre_process_update_(api_ctxt);
        break;

    default:
        ret = sdk::SDK_RET_INVALID_OP;
        break;
    }

    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failure in pre-process stage, api op %u, obj id %u, "
                      "err %u", api_ctxt->api_op, api_ctxt->obj_id, ret);
        batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    }
    return ret;
}

sdk_ret_t
api_engine::pre_process_stage_(void) {
    sdk_ret_t     ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_INIT),
                      sdk::SDK_RET_INVALID_ARG);
    batch_ctxt_.stage = API_BATCH_STAGE_PRE_PROCESS;
    for (auto it = batch_ctxt_.api_ctxts->begin();
         it != batch_ctxt_.api_ctxts->end(); ++it) {
        ret = pre_process_api_(*it);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

sdk_ret_t
api_engine::reserve_resources_(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t     ret;

    switch (obj_ctxt->api_op) {
    case API_OP_NONE:
        return SDK_RET_OK;

    case API_OP_CREATE:
        ret = api_obj->reserve_resources(api_obj, obj_ctxt);
        obj_ctxt->rsvd_rscs = 1;   // TODO: why do we need this ?
        break;

    case API_OP_DELETE:
        // no additional resources needed for delete operation
        return sdk::SDK_RET_OK;

    case API_OP_UPDATE:
        ret = obj_ctxt->cloned_obj->reserve_resources(api_obj, obj_ctxt);
        obj_ctxt->rsvd_rscs = 1;
        break;

    default:
        return sdk::SDK_RET_INVALID_OP;
        break;
    }

    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failure in resource reservation stage, "
                      "obj %s, op %u, err %u", api_obj->key2str().c_str(),
                      obj_ctxt->api_op, ret);
    }
    return ret;
}

sdk_ret_t
api_engine::program_config_(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t     ret;

    switch (obj_ctxt->api_op) {
    case API_OP_NONE:
        // no-op, but during ACTIVATE_EPOCH/ABORT stage, free the object
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        return SDK_RET_OK;
        break;

    case API_OP_CREATE:
        // call program_config() callback on the object, note that the object
        // should have been in the s/w db already by this time marked as dirty
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        ret = api_obj->program_config(obj_ctxt);
        break;

    case API_OP_DELETE:
        // call cleanup_config() callback on existing object and mark it for
        // deletion (cloned_obj, if one exists, doesn't matter and needs to be
        // freed eventually)
        // NOTE: delete is same as updating the h/w entries with latest
        //       epoch, which will be activated later in the commit()
        //       stage (by programming tables in stage 0, if needed)
        // TODO: this doesn't sound correct --> for delete we should
        // program stage0 1st and then update non-stage0 tables, so
        // activate_config() is the right thing here and then we should
        // call cleanup_config() in the next stage of API processing
        ret = api_obj->cleanup_config(obj_ctxt);
        break;

    case API_OP_UPDATE:
        // call update_config() callback on the cloned object so new config
        // can be programmed in the h/w everywhere except stage0, which will
        // be programmed during commit() stage later
        // NOTE: during commit() stage, for update case, we will swap new
        //       obj with old/current one in the all the dbs as well before
        //       activating epoch is activated in h/w stage 0 (and old
        //       object should be freed back)
        SDK_ASSERT(obj_ctxt->cloned_obj != NULL);
        ret = obj_ctxt->cloned_obj->update_config(api_obj, obj_ctxt);
        break;

    default:
        return sdk::SDK_RET_INVALID_OP;
        break;
    }

    // remember that hw is dirtied at this time, so we can rollback if needed
    // additionally, if there is a case where hw entry needs to be restored to
    // its original state, in the event of rollback, object implementation of
    // the above callbacks must read current state of respective entries from
    // hw, stash them in the obj_ctxt_t, so the same state can be rewritten back
    // to hw
    obj_ctxt->hw_dirty = 1;

    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failure in program config stage, "
                      "obj %s, op %u, err %u", api_obj->key2str().c_str(),
                      obj_ctxt->api_op, ret);
    }
    return ret;
}

sdk_ret_t
api_engine::add_deps_(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    if (obj_ctxt->api_op != API_OP_UPDATE) {
        // currently we need to add object dependencies only when a object is
        // updated, however, it is possible in future that add or delete of
        // an object might impact other objects as well ... current assumption
        // is that it is handled in the agent when such a case arises
        return SDK_RET_OK;
    }
    // NOTE: we are invoking this method on the original unmodified object
    return api_obj->add_deps(obj_ctxt);
}

sdk_ret_t
api_engine::resource_reservation_stage_(void)
{
    sdk_ret_t    ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_PRE_PROCESS),
                      sdk::SDK_RET_INVALID_ARG);
    // walk over all the dirty objects and reserve resources, if needed
    batch_ctxt_.stage = API_BATCH_STAGE_RESERVE_RESOURCES;
    for (auto it = batch_ctxt_.dirty_obj_list.begin();
         it != batch_ctxt_.dirty_obj_list.end(); ++it) {
        ret = reserve_resources_(*it, &batch_ctxt_.dirty_obj_map[*it]);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

sdk_ret_t
api_engine::obj_dependency_computation_stage_(void)
{
    sdk_ret_t    ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_RESERVE_RESOURCES),
                      sdk::SDK_RET_INVALID_ARG);
    // walk over all the dirty objects and make a list of objects that are
    // effected because of each dirty object
    batch_ctxt_.stage = API_BATCH_STAGE_OBJ_DEPENDENCY;
    for (auto it = batch_ctxt_.dirty_obj_list.begin();
         it != batch_ctxt_.dirty_obj_list.end(); ++it) {
        ret = add_deps_(*it, &batch_ctxt_.dirty_obj_map[*it]);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

sdk_ret_t
api_engine::program_config_stage_(void) {
    sdk_ret_t    ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_OBJ_DEPENDENCY),
                      sdk::SDK_RET_INVALID_ARG);

    // walk over all the dirty objects and program hw, if any
    batch_ctxt_.stage = API_BATCH_STAGE_PROGRAM_CONFIG;
    for (auto it = batch_ctxt_.dirty_obj_list.begin();
         it != batch_ctxt_.dirty_obj_list.end(); ++it) {
        ret = program_config_(*it, &batch_ctxt_.dirty_obj_map[*it]);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }

    // walk over all the dependent objects and reprogram hw, if any
    for (auto it = batch_ctxt_.dep_obj_list.begin();
         it != batch_ctxt_.dep_obj_list.end(); ++it) {
        ret = (*it)->reprogram_config(batch_ctxt_.dep_obj_map[*it]);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

sdk_ret_t
api_engine::activate_config_(dirty_obj_list_t::iterator it,
                             api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t    ret;

    switch (obj_ctxt->api_op) {
    case API_OP_NONE:
        // only case where a dirty obj ends up with this opcode is when new
        // object is created and (eventually) deleted in the same batch
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        api_obj->del_from_db();
        del_from_dirty_list_(it, api_obj);
        api_obj->delay_delete();
        break;

    case API_OP_CREATE:
        // object is already in the database by now, just trigger stage0
        // programming and clear the dirty flag on the object
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        ret = api_obj->activate_config(batch_ctxt_.epoch, API_OP_CREATE,
                                       obj_ctxt);
        SDK_ASSERT(ret == SDK_RET_OK);
        obj_ctxt->hw_dirty = 0;
        del_from_dirty_list_(it, api_obj);
        if (api_obj->stateless()) {
            // destroy this object as it is not needed anymore
            PDS_TRACE_VERBOSE("Doing soft delete of stateless obj %s",
                              api_obj->key2str().c_str());
            api_base::soft_delete(obj_ctxt->obj_id, api_obj);
        }
        break;

    case API_OP_DELETE:
        // we should have programmed hw entries with latest epoch and bit
        // indicating the entry is invalid already by now (except stage 0),
        // so reflect the object deletion in stage 0 now, remove the object
        // from all s/w dbs and enqueue for delay deletion (until then table
        // indices won't be freed, because there could be packets
        // circulating in the pipeline that picked older epoch still
        // (note that s/w can't access this obj anymore from this point
        //  onwards by doing lookups)
        ret = api_obj->activate_config(batch_ctxt_.epoch, API_OP_DELETE,
                                       obj_ctxt);
        SDK_ASSERT(ret == SDK_RET_OK);
        api_obj->del_from_db();
        del_from_dirty_list_(it, api_obj);
        api_obj->delay_delete();
        if (obj_ctxt->cloned_obj) {
            obj_ctxt->cloned_obj->delay_delete();
        }
        break;

    case API_OP_UPDATE:
        // other than stage 0 of datapath pipeline, all stages are updated
        // with this epoch, so update stage 0 now; but before doing that we
        // need to switch the latest epcoh in s/w, otherwise packets can
        // come to FTEs with new epoch even before s/w swich is done
        ret = obj_ctxt->cloned_obj->update_db(api_obj, obj_ctxt);
        SDK_ASSERT(ret == SDK_RET_OK);
        ret = obj_ctxt->cloned_obj->activate_config(batch_ctxt_.epoch,
                                                    API_OP_UPDATE, obj_ctxt);
        SDK_ASSERT(ret == SDK_RET_OK);
        // enqueue the current (i.e., old) object for delay deletion, note that
        // the current obj is already deleted from the s/w db and swapped with
        // cloned_obj when update_db() was called on cloned_obj above
        del_from_dirty_list_(it, api_obj);
        if (api_obj->stateless()) {
            // destroy cloned object as it is not needed anymore
            if (obj_ctxt->cloned_obj->stateless()) {
                PDS_TRACE_VERBOSE("Doing soft delete of stateless obj %s",
                                  api_obj->key2str().c_str());
                api_base::soft_delete(obj_ctxt->obj_id, obj_ctxt->cloned_obj);
            }
        }
        api_base::soft_delete(obj_ctxt->obj_id, api_obj);
        break;

    default:
        return sdk::SDK_RET_INVALID_OP;
    }
    return SDK_RET_OK;
}

sdk_ret_t
api_engine::activate_config_stage_(void) {
    sdk_ret_t                     ret;
    obj_ctxt_t                    octxt;
    dirty_obj_list_t::iterator    next_it;

    // walk over all the dirty objects and activate their config in hw, if any
    batch_ctxt_.stage = API_BATCH_STAGE_CONFIG_ACTIVATE;
    for (auto it = batch_ctxt_.dirty_obj_list.begin(), next_it = it;
             it != batch_ctxt_.dirty_obj_list.end(); it = next_it) {
        next_it++;
        octxt = batch_ctxt_.dirty_obj_map[*it];
        ret = activate_config_(it, *it, &octxt);
        SDK_ASSERT(ret == SDK_RET_OK);
    }

    // walk over all the dependent objects & re-activate their config hw, if any
    for (auto it = batch_ctxt_.dep_obj_list.begin();
         it != batch_ctxt_.dep_obj_list.end(); ++it) {
        ret = (*it)->reactivate_config(batch_ctxt_.epoch,
                                       batch_ctxt_.dep_obj_map[*it]);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

sdk_ret_t
api_engine::rollback_config_(dirty_obj_list_t::iterator it, api_base *api_obj,
                             obj_ctxt_t *obj_ctxt) {
    sdk_ret_t    ret = SDK_RET_OK;

    switch (obj_ctxt->api_op) {
    case API_OP_NONE:
        // only case where a dirty obj ends up with this opcode is when new
        // object is created and (eventually) deleted in the same batch. so far
        // we haven't done any h/w programming, so we just delete the obj from
        // db(s) and delay delete the slab object
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        api_obj->del_from_db();
        del_from_dirty_list_(it, api_obj);
        api_obj->delay_delete();
        break;

    case API_OP_CREATE:
        // so far we did the following for the API_OP_CREATE:
        // 1. instantiated the object
        // 2. initialized config (in s/w)
        // 3. added the object to the db(s)
        // 4. programmed the h/w (if program_config_stage_() got to this object
        //    in dirty list) before failing (i.e., set hw_dirty bit to true)
        // so, now we have to undo these actions
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        if (obj_ctxt->hw_dirty) {
            api_obj->cleanup_config(obj_ctxt);
            obj_ctxt->hw_dirty = 0;
        }
        api_obj->del_from_db();
        del_from_dirty_list_(it, api_obj);
        api_obj->delay_delete();
        break;

    case API_OP_DELETE:
        // so far we did the following for the API_OP_DELETE:
        // 1. for stateless objs, we built the object and added to db
        // 2. added the object to the dirty list
        // 3. potentially cloned the original object (UPD-XXX-DEL case)
        // 4. potentially called cleanup_config() to clear entries in hw and
        // 5. called set_hw_dirty()
        // so, now we have to undo these actions
        if (obj_ctxt->hw_dirty) {
            //api_obj->program_config(obj_ctxt);  // TODO: don't see a need for this !!
            obj_ctxt->hw_dirty = 0;
        }
        if (obj_ctxt->cloned_obj) {
            obj_ctxt->cloned_obj->delay_delete();
        }
        del_from_dirty_list_(it, api_obj);
        if (api_obj->stateless()) {
            PDS_TRACE_VERBOSE("Doing soft delete of stateless obj %s",
                              api_obj->key2str().c_str());
            api_base::soft_delete(obj_ctxt->obj_id, api_obj);
        }
        break;

    case API_OP_UPDATE:
        // so far we did the following for the API_OP_UPDATE:
        // 1. for stateless objs, we built the object and added to db
        // 2. cloned the original object
        // 3. added original object to dirty list
        // 4. updated cloned obj's s/w cfg to cfg specified in latest update
        // 5. called update_config() on cloned obj
        // 6. called set_hw_dirty() on original object
        // so, now we have to undo these actions
        if (obj_ctxt->hw_dirty) {
            obj_ctxt->cloned_obj->cleanup_config(obj_ctxt);
            // api_obj->program_config(obj_ctxt);
            obj_ctxt->hw_dirty = 0;
        }
        obj_ctxt->cloned_obj->delay_delete();
        del_from_dirty_list_(it, api_obj);
        if (api_obj->stateless()) {
            PDS_TRACE_VERBOSE("Doing soft delete of stateless obj %s",
                              api_obj->key2str().c_str());
            api_base::soft_delete(obj_ctxt->obj_id, api_obj);
        }
        break;

    default:
        ret = sdk::SDK_RET_INVALID_OP;
        break;
    }

    return ret;
}

sdk_ret_t
api_engine::batch_abort_(void) {
    sdk_ret_t                     ret;
    dirty_obj_list_t::iterator    next_it;
    obj_ctxt_t                    octxt;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_ABORT),
                      sdk::SDK_RET_INVALID_ARG);

    PDS_TRACE_INFO("Starting rollback config stage");
    for (auto it = batch_ctxt_.dirty_obj_list.begin(), next_it = it;
         it != batch_ctxt_.dirty_obj_list.end(); it = next_it) {
        next_it++;
        octxt = batch_ctxt_.dirty_obj_map[*it];
        ret = rollback_config_(it, *it, &octxt);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    PDS_TRACE_INFO("Finished rollback config stage");

    // clear all batch related info
    batch_ctxt_.epoch = PDS_EPOCH_INVALID;
    batch_ctxt_.stage = API_BATCH_STAGE_NONE;

    // walk dirty object map/list & release all stateless object memory
    PDS_TRACE_INFO("Clearing dirty object map/list ...");
#if 0
    // TODO: handle stateless case inside rollback_config_() itself
    //       (similar to activate_config_())
    for (auto it = batch_ctxt_.dirty_obj_list.begin();
         it != batch_ctxt_.dirty_obj_list.end(); ) {
        api_obj = it->first;
        if (api_obj->stateless()) {
            // destroy this object as it is not needed anymore
            api_obj->delay_delete();
        }
        batch_ctxt_.dirty_obj_list.erase(it++);
    }
#endif
    batch_ctxt_.dirty_obj_map.clear();
    batch_ctxt_.dirty_obj_list.clear();
    PDS_TRACE_INFO("Finished clearing dirty object map/list ...");

    return SDK_RET_OK;
}

sdk_ret_t
api_engine::batch_commit(batch_info_t *batch) {
    sdk_ret_t    ret;

    batch_ctxt_.epoch = batch->epoch;
    batch_ctxt_.stage = API_BATCH_STAGE_INIT;
    batch_ctxt_.api_ctxts = &batch->apis;

    // pre process the APIs by walking over the stashed API contexts to form
    // dirty object list
    PDS_TRACE_INFO("Preprocess stage start, epoch %u, API count %u",
                   batch_ctxt_.epoch,  batch_ctxt_.api_ctxts->size());
    ret = pre_process_stage_();
    PDS_TRACE_INFO("Preprocess stage end, epoch %u, result %u",
                   batch_ctxt_.epoch, ret);
    if (ret != SDK_RET_OK) {
        goto error;
    }
    PDS_TRACE_INFO("Dirty object list size %u, Dirty object map size %u",
                   batch_ctxt_.dirty_obj_list.size(),
                   batch_ctxt_.dirty_obj_map.size());

    // start table mgmt. lib transaction
    impl_base::pipeline_impl()->table_transaction_begin();

    PDS_TRACE_INFO("Starting resource reservation phase");
    ret = resource_reservation_stage_();
    PDS_TRACE_INFO("Finished resource reservation phase");
    if (ret != SDK_RET_OK) {
        goto error;
    }

    // walk over the dirty object list and compute the (aka. puppet) objects
    // that could be effected because of dirty list objects
    PDS_TRACE_INFO("Starting object dependency computation stage for epoch %u",
                   batch_ctxt_.epoch);
    ret = obj_dependency_computation_stage_();
    PDS_TRACE_INFO("Finished object dependency computation stage for epoch %u",
                   batch_ctxt_.epoch);
    if (ret != SDK_RET_OK) {
        goto error;
    }
    PDS_TRACE_INFO("Dependency object list size %u, map size %u",
                   batch_ctxt_.dep_obj_list.size(),
                   batch_ctxt_.dep_obj_map.size());

    // walk over the dirty object list, performe the de-duped operation on
    // each object including allocating resources and h/w programming (with the
    // exception of stage 0 programming
    PDS_TRACE_INFO("Starting program config stage for epoch %u",
                   batch_ctxt_.epoch);
    ret = program_config_stage_();
    PDS_TRACE_INFO("Finished program config stage for epoch %u",
                   batch_ctxt_.epoch);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    // activate the epoch in h/w & s/w by programming stage0 tables, if any
    PDS_TRACE_INFO("Starting activate config stage for epoch %u",
                   batch_ctxt_.epoch);
    ret = activate_config_stage_();
    PDS_TRACE_INFO("Finished activate config stage for epoch %u",
                   batch_ctxt_.epoch);

    // end the table mgmt. lib transaction
    impl_base::pipeline_impl()->table_transaction_end();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Table transaction end API failure, err %u", ret);
        return ret;
    }

    // walk dirty object map/list & release all stateless object memory
    PDS_TRACE_INFO("Dirty object list size %u, Dirty object map size %u",
                   batch_ctxt_.dirty_obj_list.size(),
                   batch_ctxt_.dirty_obj_map.size());
    PDS_TRACE_INFO("Clearing dirty object map/list ...");
    batch_ctxt_.dirty_obj_map.clear();
    batch_ctxt_.dirty_obj_list.clear();
    PDS_TRACE_INFO("Finished clearing dirty object map/list ...");

    // update the epoch to current epoch
    PDS_TRACE_INFO("Advancing from epoch %u to epoch %u",
                   g_current_epoch_, batch_ctxt_.epoch);
    g_current_epoch_ = batch_ctxt_.epoch;
    batch_ctxt_.epoch = PDS_EPOCH_INVALID;
    batch_ctxt_.stage = API_BATCH_STAGE_NONE;
    return SDK_RET_OK;

error:

    batch_abort_();
    return ret;
}

sdk_ret_t
api_engine_init (void)
{
    api_params_init();
    g_api_ctxt_slab_ =
        slab::factory("api-ctxt", PDS_SLAB_ID_API_CTXT,
                      sizeof(api_ctxt_t), 512, true, true, true, NULL);
    g_api_msg_slab_ =
        slab::factory("api-msg", PDS_SLAB_ID_API_MSG,
                      sizeof(api_msg_t), 512, true, true, true, NULL);
    return SDK_RET_OK;
}

/// \@}

}    // namespace api
