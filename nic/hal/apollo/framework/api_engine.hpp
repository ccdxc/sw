/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_engine.hpp
 *
 * @brief   API processing framework/engine functionality
 */

#if !defined (__API_ENGINE_HPP__)
#define __API_ENGINE_HPP__

#include <vector>
#include <unordered_map>

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"
#include "nic/hal/apollo/framework/api_ctxt.hpp"
#include "nic/hal/apollo/include/api/oci_batch.hpp"
#include "nic/hal/apollo/api/vcn.hpp"
#include "nic/hal/apollo/api/subnet.hpp"
#include "nic/hal/apollo/api/vnic.hpp"

using std::vector;
using std::unordered_map;

namespace api {

/**
 * @brief   processing stage of the APIs in a given batch
 */
typedef enum api_batch_stage_e {
    API_BATCH_STAGE_NONE,              /**< invalid stage */
    API_BATCH_STAGE_INIT,              /**< initialization stage */
    API_BATCH_STAGE_PRE_PROCESS,       /**< pre-processing stage */
    API_BATCH_STAGE_TABLE_UPDATE,      /**< table programming stage */
    API_BATCH_STAGE_ACTIVATE_EPOCH,    /**< epoch activation stage */
    API_BATCH_STAGE_ABORT,             /**< abort stage */
} api_batch_stage_t;

/**
 * @brief    per api object context, which is transient information maintained
 *           while a batch of APIs are being processed
 *
 * NOTE:     api_params is not owned by this structure, so don't free it ... it
 *           is owned by api_ctxt_t and hence when api_ctxt_t is being destroyed
 *           we should return the api_params_t memory back to slab
 */
typedef struct obj_ctxt_s obj_ctxt_t;
struct obj_ctxt_s {
    api_op_t      api_op;         /**< de-duped/compressed API opcode */
    api_params_t  *api_params;    /**< API specific parameters */
    api_base      *cloned_obj;    /**< cloned object, for UPD processing */
    void          *cb_ctxt;       /**< object handlers can save & free state
                                       across callbacks here and is opaque to
                                       the api engine */
    obj_ctxt_s() {
        api_op = API_OP_INVALID;
        api_params = NULL;
        cloned_obj = NULL;
        cb_ctxt = NULL;
    }
};

/**
 * @brief    per batch context which is a list of all API contexts
 */
typedef struct api_batch_ctxt_s {
    oci_epoch_t                     epoch;        /**< epoch in progress, passed in
                                                       oci_batch_begin() */
    api_batch_stage_t               stage;        /**< phase of the batch processing */
    vector<api_ctxt_t>              api_ctxts;    /**< API contexts per batch */
    /**
     * dirty object map is needed because in the same batch we could have
     * multiple modifications of same object, like security rules change and
     * route changes can happen for same vnic in two different API calls but in
     * same batch and we need to activate them in one write (not one after
     * another)
     */
    // TODO: in addition to map, we should keep list of objects to preserve
    // order of processing APIs (currently order is decided by the map itself,
    // which might be incorrect !!!)
    unordered_map<api_base *, obj_ctxt_t> dirty_objs;  /**< dirty object map */
} api_batch_ctxt_t;

/**
 * @brief    encapsulation for all API processing framework
 */
class api_engine {
public:
    /**
     * @brief    constructor
     */
    api_engine();

    /**
     * @brief    destructor
     */
    ~api_engine();

    /**
     * @brief    handle batch begin by setting up per API batch context
     */
    sdk_ret_t batch_begin(oci_batch_params_t *batch_params);

    /**
     * @brief    commit all the APIs in this batch, release any temporary
     *           state or resources like memory, per API context info etc.
     */
    sdk_ret_t batch_commit(void);

    /**
     * @brief    abort all the APIs in this batch, release any temporary
     *           state or resources like memory, per API context info etc.
     */
    sdk_ret_t batch_abort(void);

    /**
     * @brief    wrapper function for processing all API calls
     */
    sdk_ret_t process_api(api_ctxt_t *api_ctxt);

    slab *api_params_slab(void) { return api_params_slab_; }

private:

    /**
     * @brief    de-dup given API operation based on the currently computed
     *           operation and new API operation seen on the object
     * @param[in] curr_op    current outstanding API operation on the object
     * @param[in] new_op     newly encountered API operation on the object
     * @return   de-duped/compressed API operation
     */
    api_op_t api_op_(api_op_t curr_op, api_op_t new_op);

    /**
     * @brief    process an API and form effected list of objs
     * @param[in] api_ctxt    transient state associated with this API
     */
    sdk_ret_t pre_process_api_(api_ctxt_t *api_ctxt);

    /**
     * @brief    process given object from the dirty list by doing add/update of
     *           corresponding h/w entries, based on accumulated configuration
     *           without activating the epoch
     * @param[in] api_obj    API object being processed
     * @param[in] obj_ctxt   transient information maintained to process the API
     */
    sdk_ret_t program_config_(api_base *api_obj, obj_ctxt_t *obj_ctxt);

    /**
     * @brief    if object has effected any stage 0 datapath table(s), switch to
     *           new epoch in this stage
     *           NOTE: NO failures must happen in this stage
     * @param[in] api_obj    API object being processed
     * @param[in] obj_ctxt   transient information maintained to process the API
     */
    sdk_ret_t activate_config_(api_base *api_obj, obj_ctxt_t *obj_ctxt);

    /**
     * @brief    abort all changes made to an object, rollback to its previous
     *           state
     * NOTE:     this is not expected to fail and also epoch is not activated
     *           if we are here
     * @param[in] api_obj    API object being processed
     * @param[in] obj_ctxt   transient information maintained to process the API
     */
    sdk_ret_t rollback_config_(api_base *api_obj, obj_ctxt_t *obj_ctxt);

    /**
     * @brief    pre process all API calls in a given batch and form dirty
     *           list of effected objects
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t pre_process_stage_(void);

    /**
     * @brief    datapath table update stage
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t program_config_stage_(void);

    /**
     * @brief    final epoch activation stage
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t activate_config_stage_(void);

    /**
     * @brief    add given api object to dirty list of the API batch
     * @param[in] api_obj    API object being processed
     * @param[in] obj_ctxt   transient information maintained to process the API
     */
    void add_to_dirty_list_(api_base *api_obj, obj_ctxt_t obj_ctxt) {
        api_obj->set_in_dirty_list();
        batch_ctxt_.dirty_objs[api_obj] = obj_ctxt;
    }

    /**
     * @brief    del given api object from dirty list of the API batch
     * @param[in] api_obj    API object being processed
     */
    void del_from_dirty_list_(api_base *api_obj) {
        batch_ctxt_.dirty_objs.erase(api_obj);
        api_obj->clear_in_dirty_list();
    }

private:
    /**< API operation de-dup matrix */
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
};

/**< API engine (singleton) instance */
extern api_engine    g_api_engine;

static inline slab *
api_params_slab (void)
{
    return g_api_engine.api_params_slab();
}

}    // namespace api

#endif    /** __API_ENGINE_HPP__ */
