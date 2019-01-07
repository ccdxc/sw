/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_engine.cc
 *
 * @brief   API engine functionality
 */

#include "nic/hal/apollo/framework/api_engine.hpp"
#include "nic/hal/apollo/core/mem.hpp"

namespace api {

/**< API engine (singleton) instance */
api_engine    g_api_engine;

/**
 * @defgroup OCI_API_ENGINE - framework for processing APIs
 * @ingroup OCI_VCN
 * @{
 */

#if 0
/**
 * @brief    add/update h/w entries, based on the configuration without
 *           activating the epoch
 */
sdk_ret_t
api_engine::update_tables(void) {
    api_ctxt_t    api_ctxt;
    sdk_ret_t     ret;

    /**
     * walk over all the dirty objects and call program_config() on each object
     */
    for (vector<api_ctxt_t>::iterator it = batch_ctxt_.apis.begin();
         it != batch_ctxt_.apis.end(); ++it) {
        api_ctxt = *it;
        switch (api_ctxt.api_op) {
        case API_OP_CREATE:
            /**
             * call program_config() callback on the new object, note that the
             * object should have been in the s/w db already by this time but
             * marked as dirty
             */
            ret = api_ctxt.new_obj->program_config(&api_ctxt);
            SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
            break;

        case API_OP_DELETE:
            /**
             * call cleanup_config() callback on existing object and mark it for
             * deletion
             * NOTE: delete is same as updating the h/w entries with latest
             *       epoch, which will be activated later in the commit()
             *       stage (by programming tables in stage 0, if needed)
             * TODO: when do we free up this entry from table mgmt. libs ?
             *       we can do delay delete for these (with the caveat that
             *       until then the h/w entries can't be used)
             */
            ret = api_ctxt.curr_obj->cleanup_config(&api_ctxt);
            SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
            break;

        case API_OP_UPDATE:
            /**
             * call update_config() callback on the cloned object so new config
             * can be programmed in the h/w everywhere except stage0, which will
             * be programmed during commit() stage later
             * NOTE: during commit() stage, for update case, we will swap new
             *       obj with old/current one in the all the dbs as well before
             *       activating epoch is activated in h/w stage 0 (and old
             *       object should be freed back)
             */
            ret = api_ctxt.new_obj->update_config(&api_ctxt);
            SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
            break;

        default:
            return sdk::SDK_RET_INVALID_OP;
        }
    }
    return SDK_RET_OK;
}

/**
 * @brief    any objects having tables in stage 0 of datapath can act in this
 *           epoch activation stage to switch to new epoch
 *           NOTE: no failures must happen in this stage
 */
sdk_ret_t
api_engine::activate_config(void) {
    api_ctxt_t    api_ctxt;
    sdk_ret_t     ret;

    /**
     * walk over all the dirty objects and call program_config() on each object
     */
    for (vector<api_ctxt_t>::iterator it = batch_ctxt_.apis.begin();
         it != batch_ctxt_.apis.end(); ++it) {
        api_ctxt = *it;
        switch (api_ctxt.api_op) {
        case API_OP_CREATE:
            /**
             * object should be in the database by now, just trigger stage0
             * programming and clear the dirty flag on the object
             */
            ret = api_ctxt.new_obj->activate_config(API_OP_CREATE, &api_ctxt);
            SDK_ASSERT(ret == SDK_RET_OK);
            api_ctxt.new_obj->clear_dirty();
            break;

        case API_OP_DELETE:
            /**
             * we should have programmed hw entries with latest epoch and bit
             * indicating the entry is invalid already by now (except stage 0),
             * so reflect the object deletion in stage 0 now, remove the object
             * from all s/w dbs and enqueue for delay deletion (until then table
             * indices won't be freed, because there could be packets
             * circulating in the pipeline that picked older epoch still
             * (note that s/w can't access this obj anymore from this point
             *  onwards by doing lookups)
             */
            ret = api_ctxt.curr_obj->activate_config(API_OP_DELETE, &api_ctxt);
            SDK_ASSERT(ret == SDK_RET_OK);
            api_ctxt.curr_obj->del_from_db();
            api_ctxt.curr_obj->delay_delete();
            break;

        case API_OP_UPDATE:
            /**
             * other than stage 0 of datapath pipeline, all stages are updated
             * with this epoch, so update stage 0 now; but before doing that we
             * need to switch the latest epcoh in s/w, otherwise packets can
             * come to FTEs with new epoch even before s/w swich is done
             */
            ret = api_ctxt.new_obj->update_db(api_ctxt.curr_obj, &api_ctxt);
            SDK_ASSERT(ret == SDK_RET_OK);
            ret = api_ctxt.new_obj->activate_config(API_OP_UPDATE, &api_ctxt);
            SDK_ASSERT(ret == SDK_RET_OK);
            /**< enqueue the current (i.e., old) object for delay deletion */
            api_ctxt.curr_obj->delay_delete();
            break;

        default:
            return sdk::SDK_RET_INVALID_OP;
        }
    }
    return SDK_RET_OK;
}
#endif

/**
 * @brief    return a pre-populated de-duped API operation
 */
api_op_t
api_engine::api_op_(api_op_t old_op, api_op_t new_op) {
    return dedup_api_op_[old_op][new_op];
}

/**
 * @brief    process an API and form effected list of objs
 * @param[in] api_ctxt    transient state associated with this API
 */
sdk_ret_t
api_engine::pre_process_api_(api_ctxt_t *api_ctxt) {
    sdk_ret_t       ret = SDK_RET_OK;
    obj_ctxt_t      obj_ctxt;
    api_base        *api_obj;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_PRE_PROCESS),
                      sdk::SDK_RET_INVALID_OP);
    switch (api_ctxt->api_op) {
    case API_OP_CREATE:
        api_obj = api_base::find_obj(api_ctxt);
        if (api_obj == NULL) {
            /**< instantiate a new object */
            api_obj = api_base::factory(api_ctxt);
            if (unlikely(api_obj == NULL)) {
                batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
                return sdk::SDK_RET_ERR;
            }
            /**< add it to dirty object list */
            obj_ctxt.api_op = API_OP_CREATE;
            obj_ctxt.api_params = api_ctxt->api_params;
            add_to_dirty_list_(api_obj, obj_ctxt);
            /**< initialize the object with the given config */
            ret = api_obj->init_config(api_ctxt);
            SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
            api_obj->add_to_db();
        } else {
            /**
             * this could be XXX-DEL-ADD/ADD-DEL-XXX-ADD kind of scenario in
             * same batch, as we don't expect to see ADD-XXX-ADD (unless we want
             * to support idempotency)
             */
            SDK_ASSERT(api_obj->is_in_dirty_list() == true);
            obj_ctxt_t& octxt = batch_ctxt_.dirty_obj_map.find(api_obj)->second;
            SDK_ASSERT((octxt.api_op == API_OP_NONE) ||
                       (octxt.api_op == API_OP_DELETE));
            octxt.api_op = api_op_(octxt.api_op, API_OP_CREATE);
            SDK_ASSERT(octxt.api_op != API_OP_INVALID);

            /**< update the config, by cloning the object, if needed */
            if (octxt.cloned_obj == NULL) {
                /**
                 * XXX-DEL-ADD or ADD-XXX-DEL-XXX-ADD scenarios, we need to
                 * differentiate between these two
                 */
                if (octxt.api_op == API_OP_UPDATE) {
                    /**< XXX-DEL-ADD scenario, clone & re-init cfg */
                    octxt.api_params = api_ctxt->api_params;
                    octxt.cloned_obj = api_obj->clone();
                    ret = octxt.cloned_obj->init_config(api_ctxt);
                    SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
                } else {
                    /**< ADD-XXX-DEL-XXX-ADD scenario, re-init same object */
                    SDK_ASSERT(octxt.api_op == API_OP_CREATE);
                    octxt.api_params = api_ctxt->api_params;
                    ret = api_obj->init_config(api_ctxt);
                    SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
                }
            } else {
                /**< UPD-XXX-DEL-XXX-ADD scenario, re-init cloned obj's cfg */
                octxt.api_params = api_ctxt->api_params;
                ret = octxt.cloned_obj->init_config(api_ctxt);
                SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
            }
        }
        break;

    case API_OP_DELETE:
        /**
         * find non-dirty object since we don't support add and delete in same
         * batch as it doesn't make sense (may be it does from external
         * controller point of view, if its batching periodically
         */
        api_obj = api_base::find_obj(api_ctxt);
        if (api_obj) {
            if (api_obj->is_in_dirty_list()) {
                /**
                 * note that we could have cloned_obj as non-NULL in this case
                 * (e.g., UPD-XXX-DEL), but that doesn't matter here
                 */
                batch_ctxt_.dirty_obj_map[api_obj].api_op =
                    api_op_(batch_ctxt_.dirty_obj_map[api_obj].api_op,
                            API_OP_DELETE);
                SDK_ASSERT(batch_ctxt_.dirty_obj_map[api_obj].api_op !=
                           API_OP_INVALID);
            } else {
                /**< add the object to dirty list */
                obj_ctxt.api_op = API_OP_DELETE;
                add_to_dirty_list_(api_obj, obj_ctxt);
            }
        } else {
            ret = sdk::SDK_RET_ENTRY_NOT_FOUND;
            goto error;
        }
        break;

    case API_OP_UPDATE:
        api_obj = api_base::find_obj(api_ctxt);
        if (api_obj) {
            if (api_obj->is_in_dirty_list()) {
                obj_ctxt_t& octxt =
                    batch_ctxt_.dirty_obj_map.find(api_obj)->second;
                octxt.api_op = api_op_(octxt.api_op, API_OP_UPDATE);
                SDK_ASSERT(octxt.api_op != API_OP_INVALID);
                if (octxt.cloned_obj == NULL) {
                    /**
                     * XXX-ADD-XXX-UPD in same batch, no need to clone yet, just
                     * re-init the object with new config
                     */
                    octxt.api_params = api_ctxt->api_params;
                    ret = api_obj->init_config(api_ctxt);
                    SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
                } else {
                    /**< XXX-UPD-XXX-UPD scenario, update the cloned obj */
                    octxt.api_params = api_ctxt->api_params;
                    ret = octxt.cloned_obj->init_config(api_ctxt);
                    SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
                }
            } else {
                obj_ctxt.api_op = API_OP_UPDATE;
                obj_ctxt.cloned_obj = api_obj->clone();
                obj_ctxt.api_params = api_ctxt->api_params;
                add_to_dirty_list_(api_obj, obj_ctxt);
                ret = obj_ctxt.cloned_obj->init_config(api_ctxt);
                SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
            }
        } else {
            ret = sdk::SDK_RET_ENTRY_NOT_FOUND;
            goto error;
        }
        break;

    default:
        ret = sdk::SDK_RET_INVALID_OP;
        goto error;
    }

    return SDK_RET_OK;

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

/**
 * @brief    walk over the API contexts collected in this batch and form a
 *           "dirty" list of objects that are effected by this batch commit
 */
sdk_ret_t
api_engine::pre_process_stage_(void) {
    sdk_ret_t     ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_INIT),
                      sdk::SDK_RET_INVALID_ARG);
    batch_ctxt_.stage = API_BATCH_STAGE_PRE_PROCESS;
    for (auto it = batch_ctxt_.api_ctxts.begin();
         it != batch_ctxt_.api_ctxts.end(); ++it) {
        ret = pre_process_api_(&*it);
        SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
    }
    return SDK_RET_OK;

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

/**
 * @brief    process given object from the dirty list by doing add/update of
 *           corresponding h/w entries, based on the accumulated configuration
 *           without activating the epoch
 * @param[in] api_obj    API object being processed
 * @param[in] obj_ctxt   transient information maintained to process the API
 */
sdk_ret_t
api_engine::program_config_(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t     ret;

    switch (obj_ctxt->api_op) {
    case API_OP_NONE:
        /**< no-op, but during ACTIVATE_EPOCH/ABORT stage, free the object */
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        return SDK_RET_OK;
        break;

    case API_OP_CREATE:
        /**
         * call program_config() callback on the object, note that the object
         * should have been in the s/w db already by this time marked as dirty
         */
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        ret = api_obj->program_config(obj_ctxt);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
        break;

    case API_OP_DELETE:
        /**
         * call cleanup_config() callback on existing object and mark it for
         * deletion (cloned_obj, if one exists, doesn't matter and needs to be
         * freed eventually)
         * NOTE: delete is same as updating the h/w entries with latest
         *       epoch, which will be activated later in the commit()
         *       stage (by programming tables in stage 0, if needed)
         */
        ret = api_obj->cleanup_config(obj_ctxt);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
        break;

    case API_OP_UPDATE:
        /**
         * call update_config() callback on the cloned object so new config
         * can be programmed in the h/w everywhere except stage0, which will
         * be programmed during commit() stage later
         * NOTE: during commit() stage, for update case, we will swap new
         *       obj with old/current one in the all the dbs as well before
         *       activating epoch is activated in h/w stage 0 (and old
         *       object should be freed back)
         */
        SDK_ASSERT(obj_ctxt->cloned_obj != NULL);
        ret = obj_ctxt->cloned_obj->update_config(api_obj, obj_ctxt);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
        break;

    default:
        return sdk::SDK_RET_INVALID_OP;
        break;
    }

    /**
     * remember that hw is dirtied at this time, so we can rollback if needed
     * additionally, if there is a case where hw entry needs to be restored to
     * its original state, in the event of rollback, object implementation of
     * the above callbacks must read current state of respective entries from
     * hw, stash them in the obj_ctxt_t, so the same state can be rewritten back
     * to hw
     */
    api_obj->set_hw_dirty();

    return SDK_RET_OK;
}

/**
 * @brief    allocate/free resources and program p4 tables, with the exception
 *           of stage 0 tables
 */
sdk_ret_t
api_engine::program_config_stage_(void) {
    sdk_ret_t    ret;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_PRE_PROCESS),
                      sdk::SDK_RET_INVALID_ARG);

    /**< walk over all the dirty objects and program hw, if any */
    batch_ctxt_.stage = API_BATCH_STAGE_TABLE_UPDATE;
    for (auto it = batch_ctxt_.dirty_obj_list.begin();
         it != batch_ctxt_.dirty_obj_list.end(); ++it) {
        ret = program_config_(it->first, &it->second);
        SDK_ASSERT_GOTO((ret == SDK_RET_OK), error);
    }

error:

    batch_ctxt_.stage = API_BATCH_STAGE_ABORT;
    return ret;
}

/**
 * @brief    if the object has effected any stage 0 datapath table, switch to
 *           new epoch in this stage
 *           NOTE: NO failures MUST happen in this stage
 * @param[in] api_obj    API object being processed
 * @param[in] obj_ctxt   transient information maintained to process the API
 */
sdk_ret_t
api_engine::activate_config_(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t    ret;

    switch (obj_ctxt->api_op) {
    case API_OP_NONE:
        /**
         * only case where a dirty obj ends up with this opcode is when new
         * object is created and (eventually) deleted in the same batch
         */
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        api_obj->del_from_db();
        api_obj->delay_delete();
        del_from_dirty_list_(api_obj);
        break;

    case API_OP_CREATE:
        /**
         * object is already in the database by now, just trigger stage0
         * programming and clear the dirty flag on the object
         */
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        ret = api_obj->activate_config(batch_ctxt_.epoch, API_OP_CREATE,
                                       obj_ctxt);
        SDK_ASSERT(ret == SDK_RET_OK);
        del_from_dirty_list_(api_obj);
        break;

    case API_OP_DELETE:
        /**
         * we should have programmed hw entries with latest epoch and bit
         * indicating the entry is invalid already by now (except stage 0),
         * so reflect the object deletion in stage 0 now, remove the object
         * from all s/w dbs and enqueue for delay deletion (until then table
         * indices won't be freed, because there could be packets
         * circulating in the pipeline that picked older epoch still
         * (note that s/w can't access this obj anymore from this point
         *  onwards by doing lookups)
         */
        ret = api_obj->activate_config(batch_ctxt_.epoch, API_OP_DELETE,
                                       obj_ctxt);
        SDK_ASSERT(ret == SDK_RET_OK);
        api_obj->del_from_db();
        api_obj->delay_delete();
        if (obj_ctxt->cloned_obj) {
            obj_ctxt->cloned_obj->delay_delete();
        }
        del_from_dirty_list_(api_obj);
        break;

    case API_OP_UPDATE:
        /**
         * other than stage 0 of datapath pipeline, all stages are updated
         * with this epoch, so update stage 0 now; but before doing that we
         * need to switch the latest epcoh in s/w, otherwise packets can
         * come to FTEs with new epoch even before s/w swich is done
         */
        ret = obj_ctxt->cloned_obj->update_db(api_obj, obj_ctxt);
        SDK_ASSERT(ret == SDK_RET_OK);
        ret = obj_ctxt->cloned_obj->activate_config(batch_ctxt_.epoch,
                                                    API_OP_UPDATE, obj_ctxt);
        SDK_ASSERT(ret == SDK_RET_OK);
        del_from_dirty_list_(api_obj);
        /**
         * enqueue the current (i.e., old) object for delay deletion, note that
         * the current obj is already deleted from the s/w db and swapped with
         * cloned_obj when update_db() was called on cloned_obj above
         */
        api_obj->delay_delete();
        break;

    default:
        return sdk::SDK_RET_INVALID_OP;
    }

    return SDK_RET_OK;
}

/**
 * @brief    program stage0 tables and activate the epoch in s/w & h/w
 * NOTE:     this is not expected to fail, so its a point of no return
 */
sdk_ret_t
api_engine::activate_config_stage_(void) {
    sdk_ret_t                     ret;
    dirty_obj_list_t::iterator    next_it;

    batch_ctxt_.stage = API_BATCH_STAGE_ACTIVATE_EPOCH;
    for (auto it = batch_ctxt_.dirty_obj_list.begin(), next_it = it;
             it != batch_ctxt_.dirty_obj_list.end(); it = next_it) {
        next_it++;
        ret = activate_config_(it->first, &it->second);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return SDK_RET_OK;
}

/**
 * @brief    abort all changes made to an object, rollback to its previous state
 * NOTE:     this is not expected to fail and also epoch is not activated if we
 *           are here
 * @param[in] api_obj    API object being processed
 * @param[in] obj_ctxt   transient information maintained to process the API
 */
sdk_ret_t
api_engine::rollback_config_(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t    ret = SDK_RET_OK;

    switch (obj_ctxt->api_op) {
    case API_OP_NONE:
        /**
         * only case where a dirty obj ends up with this opcode is when new
         * object is created and (eventually) deleted in the same batch. so far
         * we haven't done any h/w programming, so we just delete the obj from
         * db(s) and delay delete the slab object
         */
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        del_from_dirty_list_(api_obj);
        api_obj->del_from_db();
        api_obj->delay_delete();
        break;

    case API_OP_CREATE:
        /**
         * so far we did the following for the API_OP_CREATE:
         * 1. instantiated the object
         * 2. initialized config (in s/w)
         * 3. set the dirty bit
         * 4. added the object to the db(s)
         * 5. programmed the h/w (if program_config_stage_() got to this object
         *    in dirty list) before failing
         * so, now we have to undo these actions
         */
        SDK_ASSERT(obj_ctxt->cloned_obj == NULL);
        del_from_dirty_list_(api_obj);
        if (api_obj->is_hw_dirty()) {
            api_obj->cleanup_config(obj_ctxt);
            api_obj->clear_hw_dirty();
        }
        api_obj->del_from_db();
        api_obj->delay_delete();
        break;

    case API_OP_DELETE:
        /**
         * so far we did the following for the API_OP_DELETE:
         * 1. added the object to the dirty list
         * 2. potentially cloned the original object (UPD-XXX-DEL case)
         * 3. potentially called cleanup_config() to clear entries in hw and
         * 4. called set_hw_dirty()
         * so, now we have to undo these actions
         */
        del_from_dirty_list_(api_obj);
        if (api_obj->is_hw_dirty()) {
            api_obj->program_config(obj_ctxt);  // TODO: don't see a need for this !!
            api_obj->clear_hw_dirty();
        }
        if (obj_ctxt->cloned_obj) {
            obj_ctxt->cloned_obj->delay_delete();
        }
        break;

    case API_OP_UPDATE:
        /**
         * so far we did the following for the API_OP_UPDATE:
         * 1. cloned the original object
         * 2. added original object to dirty list
         * 3. updated cloned obj's s/w cfg to cfg specified in latest update
         * 4. called update_config() on cloned obj
         * 5. called set_hw_dirty() on original object
         * so, now we have to undo these actions
         */
        del_from_dirty_list_(api_obj);
        if (api_obj->is_hw_dirty()) {
            obj_ctxt->cloned_obj->cleanup_config(obj_ctxt);
            // api_obj->program_config(obj_ctxt);
            api_obj->clear_hw_dirty();
        }
        obj_ctxt->cloned_obj->delay_delete();
        break;

    default:
        ret = sdk::SDK_RET_INVALID_OP;
        break;
    }

    return ret;
}

/**
 * @brief    wrapper function for processing all API calls
 * @param[in] api_ctxt    API context carrying the config information
 */
sdk_ret_t
api_engine::process_api(api_ctxt_t *api_ctxt) {
    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_INIT),
                      sdk::SDK_RET_INVALID_OP);
    /**< stash this API so we can process it later */
    batch_ctxt_.api_ctxts.push_back(*api_ctxt);
    return SDK_RET_OK;
}

/**
 * @brief    handle batch begin by setting up per API batch context
 * @param[in] params batch specific parameters
 */
sdk_ret_t
api_engine::batch_begin(oci_batch_params_t *params) {
    SDK_ASSERT_RETURN((params != NULL), sdk::SDK_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((params->epoch != OCI_EPOCH_INVALID),
                      sdk::SDK_RET_INVALID_ARG);
    batch_ctxt_.epoch = params->epoch;
    batch_ctxt_.stage = API_BATCH_STAGE_INIT;
    batch_ctxt_.api_ctxts.reserve(16);
    return SDK_RET_OK;
};

/**
 * @brief    commit all the APIs in this batch, release any temporary
 *           state or resources like memory, per API context info etc.
 */
sdk_ret_t
api_engine::batch_commit(void) {
    sdk_ret_t     ret;

    /**
     * pre process the APIs by walking over the stashed API contexts to form
     * dirty object list
     */
    ret = pre_process_stage_();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /**
     * walk over the dirty object list, performe the de-duped operation on
     * each object including allocating resources and h/w programming (with the
     * exception of stage 0 programming
     */
    ret = program_config_stage_();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /**< activate the epoch in h/w & s/w by programming stage0 tables, if any */
    ret = activate_config_stage_();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /**< clear all batch related info */
    batch_ctxt_.epoch = OCI_EPOCH_INVALID;
    batch_ctxt_.stage = API_BATCH_STAGE_NONE;
    for (auto it = batch_ctxt_.api_ctxts.begin();
         it != batch_ctxt_.api_ctxts.end(); ++it) {
        if ((*it).api_params) {
            api_params_slab()->free((*it).api_params);
        }
    }
    batch_ctxt_.api_ctxts.clear();
    batch_ctxt_.dirty_obj_map.clear();
    batch_ctxt_.dirty_obj_list.clear();
    return SDK_RET_OK;
}

/**
 * @brief    abort all the APIs in this batch, release any temporary
 *           state or resources like memory, per API context info etc.
 */
sdk_ret_t
api_engine::batch_abort(void) {
    sdk_ret_t                     ret;
    dirty_obj_list_t::iterator    next_it;

    SDK_ASSERT_RETURN((batch_ctxt_.stage == API_BATCH_STAGE_ABORT),
                      sdk::SDK_RET_INVALID_ARG);

    for (auto it = batch_ctxt_.dirty_obj_map.begin(), next_it = it;
         it != batch_ctxt_.dirty_obj_map.end(); it = next_it) {
        next_it++;
        ret = rollback_config_(it->first, &it->second);
        SDK_ASSERT(ret == SDK_RET_OK);
    }

    // clear all batch related info
    batch_ctxt_.epoch = OCI_EPOCH_INVALID;
    batch_ctxt_.stage = API_BATCH_STAGE_NONE;
    for (auto it = batch_ctxt_.api_ctxts.begin();
         it != batch_ctxt_.api_ctxts.end(); ++it) {
        if ((*it).api_params) {
            api_params_slab()->free((*it).api_params);
        }
    }
    batch_ctxt_.api_ctxts.clear();
    batch_ctxt_.dirty_obj_map.clear();
    batch_ctxt_.dirty_obj_list.clear();
    return SDK_RET_OK;
}

/**< @brief    constructor */
api_engine::api_engine() {
    api_params_slab_ =
        slab::factory("api params", OCI_SLAB_ID_API_PARAMS,
                      sizeof(api_params_t), 128, true, true, true, NULL);
}

/**< @brief    destructor */
api_engine::~api_engine() {
    if (api_params_slab_) {
        slab::destroy(api_params_slab_);
    }
}

/** @} */    // end of OCI_API_ENGINE

}    // namespace api
