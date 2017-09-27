// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <iostream>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nic/hal/hal.hpp"
#include "nic/include/hal_pd.hpp"
#include "nic/hal/periodic/periodic.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/hal/src/rdma.hpp"
#include "nic/hal/src/oif_list_mgr.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/tcpcb.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/include/fte.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/hal/plugins/plugins.hpp"


extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/tcpcb.hpp"
#include "nic/hal/src/proxy.hpp"

namespace hal {

// process globals
thread    *g_hal_threads[HAL_THREAD_ID_MAX];
uint64_t  g_hal_handle = 1;
bool      gl_super_user = false;

// TODO_CLEANUP: THIS DOESN'T BELONG HERE !!
LIFManager *g_lif_manager = nullptr;

// thread local variables
thread_local thread *t_curr_thread;
thread_local cfg_db_ctxt_t t_cfg_db_ctxt;

using boost::property_tree::ptree;

//------------------------------------------------------------------------------
// TODO - dummy for now !!
//------------------------------------------------------------------------------
static void *
fte_pkt_loop (void *ctxt)
{
    t_curr_thread = (thread *)ctxt;
    HAL_TRACE_DEBUG("Thread {} initializing ...", t_curr_thread->name());
    HAL_THREAD_INIT();

    fte::pkt_loop(HAL_THREAD_ID_FTE_MIN - t_curr_thread->thread_id());

    return NULL;
}

//------------------------------------------------------------------------------
// return current thread pointer, for gRPC threads t_curr_thread is not set,
// however, they are considered as cfg threads
//------------------------------------------------------------------------------
thread *
hal_get_current_thread (void)
{
    return t_curr_thread ? t_curr_thread : g_hal_threads[HAL_THREAD_ID_CFG];
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
hal_handle::hal_handle()
{
}

//------------------------------------------------------------------------------
// initialize hal handle
//------------------------------------------------------------------------------
bool
hal_handle::init(hal_obj_id_t obj_id)
{
#if 0
    HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    for (uint32_t i = 0; i < k_max_objs_; i++) {
         objs_[i].valid = FALSE;
         objs_[i].ver = HAL_CFG_VER_NONE;
         objs_[i].obj = NULL;
    }
    return true;
#endif
    if ((obj_id <= HAL_OBJ_ID_NONE) || (obj_id >= HAL_OBJ_ID_MAX)) {
        return false;
    }
    handle_id_ = HAL_HANDLE_INVALID;
    obj_id_ = obj_id;
    obj_ = NULL;

    return true;
}

//------------------------------------------------------------------------------
// factory method to allocate a handle
//------------------------------------------------------------------------------
hal_handle *
hal_handle::factory(hal_obj_id_t obj_id)
{
    void          *mem;
    hal_handle    *handle;

    // allocate from the handle slab
    mem = g_hal_state->hal_handle_slab()->alloc();
    HAL_ASSERT_RETURN((mem != NULL), NULL);

    handle = new(mem) hal_handle();
    // initialize the handle instance
    if (handle->init(obj_id) == false) {
        handle->~hal_handle();
        g_hal_state->hal_handle_slab()->free(handle);
        return NULL;
    }

    return handle;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
hal_handle::~hal_handle()
{
    //HAL_SPINLOCK_DESTROY(&slock_);
}

#if 0
//------------------------------------------------------------------------------
// add an object to this handle
// NOTE: this is written assuming multiple writers a given handle, otherwise
//       we don't need the per handle lock assuming 'valid' bit is
//       validated/invalidated only at the end of the operation
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::add_obj(void *obj)
{
    uint32_t     i;
    int          free_slot = -1;
    hal_ret_t    ret = HAL_RET_OK;

    HAL_ASSERT_RETURN((t_cfg_db_ctxt.cfg_op_ == CFG_OP_WRITE), HAL_RET_ERR);
    HAL_SPINLOCK_LOCK(&slock_);
    for (i = 0; i < k_max_objs_; i++) {
        if (objs_[i].valid) {
            if (objs_[i].ver > t_cfg_db_ctxt.wversion_) {
                // someone committed a version greater than this version, in
                // single-writer model this shouldn't happen at all !!!
                HAL_TRACE_ERR("Failed to add object to handle, "
                              "obj with ver {} >= {} exists already",
                              objs_[i].ver, t_cfg_db_ctxt.wversion_);
                ret = HAL_RET_ERR;
                HAL_ASSERT_GOTO((ret == HAL_RET_OK), end);
            } else if (objs_[i].obj != NULL) {
                // someone is using cfg db version >= this object's version,
                // we can't add new version as that will overwrite all the
                // pi, pd and hw state
                // TODO: this can happen in back to back add-del-add case
                //       while the previously added version is still in use.
                HAL_TRACE_ERR("Failed to add object, found existing "
                              "ver {} ({}) of this obj in handle, rsvd "
                              "write version {}", objs_[i].ver,
                              (g_hal_state->cfg_db()->is_cfg_ver_in_use(objs_[i].ver) ?
                                   "in-use" : "not-in-use"),
                              t_cfg_db_ctxt.wversion_);
                ret = HAL_RET_RETRY;
                HAL_ASSERT_GOTO(FALSE, end);
            }
        } else {
            // in single-writer scheme we can "break" here but to support
            // multiple writer case, we are checking to see if anyone else has
            // updated config db, by scanning all entries and looking for higher
            // numbered versions of this object
            free_slot = i;
        }
    }

    // we know that this version is the latest, try to add this object to
    // the handle
    if (free_slot < 0) {
        HAL_TRACE_ERR("Failed to add object, no free slot found");
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }
    objs_[free_slot].ver = t_cfg_db_ctxt.wversion_;
    objs_[free_slot].obj = obj;
    objs_[free_slot].valid = TRUE;

end:

    HAL_SPINLOCK_UNLOCK(&slock_);
    return ret;
}

//------------------------------------------------------------------------------
// delete an object from this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::del_obj(void *obj, hal_cfg_del_cb_t del_cb)
{
    uint32_t     i;
    int          free_slot = -1;
    hal_ret_t    ret = HAL_RET_OK;

    HAL_ASSERT_RETURN((t_cfg_db_ctxt.cfg_op_ == CFG_OP_WRITE), HAL_RET_ERR);
    HAL_SPINLOCK_LOCK(&slock_);
    for (i = 0; i < k_max_objs_; i++) {
        if (objs_[i].valid) {
            if (objs_[i].ver > t_cfg_db_ctxt.wversion_) {
                // someone committed a version greater than this version, in
                // single-writer model this shouldn't happen at all !!!
                HAL_TRACE_ERR("Failed to del  object from handle, "
                              "obj with ver {} >= {} exists already",
                              objs_[i].ver, t_cfg_db_ctxt.wversion_);
                ret = HAL_RET_ERR;
                HAL_ASSERT_GOTO((ret == HAL_RET_OK), end);
            } else if ((objs_[i].obj != NULL) &&
                       (objs_[i].ver == t_cfg_db_ctxt.wversion_)) {
                // we just added this object and trying to delete from the
                // db for some reason, which is allowed as long as
                // _db_close() is not done yet
                HAL_TRACE_DEBUG("Deleting object thats just added ?");
                objs_[i].valid = FALSE;
                objs_[i].ver = HAL_CFG_VER_NONE;
                objs_[i].obj = NULL;
                // we still need to free the handle eventually
                goto end;
            }
        } else {
            // in single-writer scheme we can "break" here but to support
            // multiple writer case, we are checking to see if anyone else has
            // updated config db, by scanning all entries and looking for higher
            // numbered versions of this object
            free_slot = i;
        }
    }

    // we know that this version is the latest, add NULL object to the handle
    if (free_slot < 0) {
        ret = HAL_RET_NO_RESOURCE;
    } else {
        objs_[free_slot].ver = t_cfg_db_ctxt.wversion_;
        objs_[free_slot].obj = NULL;
        objs_[free_slot].valid = TRUE;
    }

end:

    HAL_SPINLOCK_UNLOCK(&slock_);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // add an entry into cfg db's delete object cache
    g_hal_state->cfg_db()->add_obj_to_del_cache(this, obj, del_cb);
    return ret;
}

//------------------------------------------------------------------------------
// get an object that has the highest version that is <= read-version
// acquired by this thread
//------------------------------------------------------------------------------
void *
hal_handle::get_obj(void)
{
    void             *obj = NULL;
    cfg_version_t    max_ver = HAL_CFG_VER_NONE;
    uint32_t         i, loc;

    HAL_SPINLOCK_LOCK(&slock_);
    for (i = 0; i < k_max_objs_; i++) {
        if (objs_[i].valid && (objs_[i].ver <= t_cfg_db_ctxt.rversion_)) {
            if (objs_[i].ver > max_ver) {
                max_ver = objs_[i].ver;
                loc = i;
            }
        }
    }

    // check if a valid returnable object is found and return that
    if (max_ver) {
        obj = objs_[loc].obj;
    }

    HAL_SPINLOCK_UNLOCK(&slock_);
    return obj;
}

//------------------------------------------------------------------------------
// get any valid object that is non-NULL from this handle (note that there could
// be a valid entry but obj is NULL for objects that are deleted), this API
// won't lock the handle with the assumption that the caller has locked the
// handle already (for whatever reason)
//------------------------------------------------------------------------------
void *
hal_handle::get_any_obj(void)
{
    void             *obj = NULL;
    uint32_t         i;

    for (i = 0; i < k_max_objs_; i++) {
        if (objs_[i].valid && (objs_[i].obj != NULL)) {
            obj = objs_[i].obj;
            break;
        }
    }
    return obj;
}

//------------------------------------------------------------------------------
// get any valid object that is non-NULL from this handle (note that there could
// be a valid entry but obj is NULL for objects that are deleted), this API will
// ensure that there won't be any changes to the handle while lookup is being
// done
//------------------------------------------------------------------------------
void *
hal_handle::get_any_obj_safe(void)
{
    void             *obj = NULL;
    uint32_t         i;

    HAL_SPINLOCK_LOCK(&slock_);
    for (i = 0; i < k_max_objs_; i++) {
        if (objs_[i].valid && (objs_[i].obj != NULL)) {
            obj = objs_[i].obj;
            break;
        }
    }
    HAL_SPINLOCK_UNLOCK(&slock_);
    return obj;
}
#endif

//------------------------------------------------------------------------------
// private helper function that calls operation specific callback and then calls
// either commit or abort callback based on whether operation succeeded or
// failed
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::act_on_obj_(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t cb,
                        hal_cfg_commit_cb_t commit_cb,
                        hal_cfg_abort_cb_t abort_cb)
{
    hal_ret_t    ret;

    // invoke modify callback
    ret = cb(ctxt);
    if (ret == HAL_RET_OK) {
        // call commit callback if successful
        commit_cb(ctxt);
    } else {
        // call abort callback in case of failure
        abort_cb(ctxt);
    }

    return ret;
}

//------------------------------------------------------------------------------
// add an object to this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::add_obj(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t add_cb,
                    hal_cfg_commit_cb_t commit_cb, hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_ret_t      ret;
    dhl_entry_t    *entry;

    // release the read lock
    g_hal_state->cfg_db()->runlock();

    // acquire the write lock
    g_hal_state->cfg_db()->wlock();

    // invoke the callbacks passed
    ret = act_on_obj_(ctxt, add_cb, commit_cb, abort_cb);
    if (ret == HAL_RET_OK) {
        // if commit was successful, populate the handle (HACK ??)
        entry = dllist_entry(ctxt->dhl.next,
                             dhl_entry_t, dllist_ctxt);
        this->obj_ = entry->obj;
    }

    // call cleanup callback to either free the software state of original
    // object of cloned object (based on whether modify was successful or not)
    cleanup_cb(ctxt);

    // release the write lock
    g_hal_state->cfg_db()->wunlock();

    // reacquire the read lock again
    g_hal_state->cfg_db()->rlock();

    return ret;
}

//------------------------------------------------------------------------------
// helper function to overwrite current instance of the object with cloned obj,
// cloned obj is supposed to have all updated changes by now
//------------------------------------------------------------------------------
void
hal_handle::replace_(void *obj, void *cloned_obj)
{
    if (cloned_obj) {
        this->obj_ = cloned_obj;
    }
}

//------------------------------------------------------------------------------
// modify an object in this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::upd_obj(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t upd_cb,
                    hal_cfg_commit_cb_t commit_cb, hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_ret_t        ret;
    dllist_ctxt_t    *curr, *next;
    dhl_entry_t      *entry;

    // release the read lock
    g_hal_state->cfg_db()->runlock();

    // acquire the write lock
    g_hal_state->cfg_db()->wlock();

    // invoke the callbacks passed
    ret = act_on_obj_(ctxt, upd_cb, commit_cb, abort_cb);

    // if commit was successful, copy over old object with cloned object's
    // contents
    if (ret == HAL_RET_OK) {
        dllist_for_each_safe(curr, next, &ctxt->dhl) {
            entry = dllist_entry(curr, dhl_entry_t, dllist_ctxt);
            this->replace_(entry->obj, entry->cloned_obj);
        }
    }

    // call cleanup callback to either free the software state of original
    // object of cloned object (based on whether modify was successful or not)
    cleanup_cb(ctxt);

    // release the write lock
    g_hal_state->cfg_db()->wunlock();

    // reacquire the read lock again
    g_hal_state->cfg_db()->rlock();

    return ret;
}

//------------------------------------------------------------------------------
// delete object from this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle::del_obj(cfg_op_ctxt_t *ctxt, hal_cfg_op_cb_t del_cb,
                    hal_cfg_commit_cb_t commit_cb, hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    // release the read lock
    g_hal_state->cfg_db()->runlock();

    // acquire the write lock
    g_hal_state->cfg_db()->wlock();

    // invoke the callbacks passed
    act_on_obj_(ctxt, del_cb, commit_cb, abort_cb);

    // call cleanup callback to either free the software state of original
    // object of cloned object (based on whether modify was successful or not)
    cleanup_cb(ctxt);

    // release the write lock
    g_hal_state->cfg_db()->wunlock();

    // reacquire the read lock again
    g_hal_state->cfg_db()->rlock();

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// allocate a handle for an object instance
//------------------------------------------------------------------------------
hal_handle_t
hal_handle_alloc (hal_obj_id_t obj_id)
{
    hal_ret_t                ret;
    hal_handle_t             handle_id;
    hal_handle               *handle;
    hal_handle_ht_entry_t    *entry;

    // allocate an entry to create mapping from handle-id to its handle obj
    entry =
        (hal_handle_ht_entry_t *)g_hal_state->hal_handle_ht_entry_slab()->alloc();
    if (entry == NULL) {
        HAL_TRACE_ERR("Failed to allocate hal handle ht entry");
        return HAL_HANDLE_INVALID;
    }

    // allocate hal handle object itself
    handle = hal_handle::factory(obj_id);
    if (handle == NULL) {
        HAL_TRACE_ERR("Failed to allocate handle");
        g_hal_state->hal_handle_ht_entry_slab()->free(entry);
        return HAL_HANDLE_INVALID;
    }
    // allocate unique handle id
    handle_id = HAL_ATOMIC_INC_UINT32(&g_hal_handle, 1);
    handle->set_handle_id(handle_id);

    // prepare the entry to be inserted
    entry->handle = handle;
    entry->ht_ctxt.reset();
    HAL_TRACE_DEBUG("Added handle id {} to handle obj mapping", handle_id);
    ret = g_hal_state->hal_handle_id_ht()->insert_with_key(&handle_id,
                                                           entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add handle id {} to handle obj", handle_id);
        g_hal_state->hal_handle_ht_entry_slab()->free(entry);
        handle->~hal_handle();
        g_hal_state->hal_handle_slab()->free(handle);
        return HAL_HANDLE_INVALID;
    }

    return handle_id;
}

//------------------------------------------------------------------------------
// return a hal handle back so it can be reallocated for another object
//------------------------------------------------------------------------------
void
hal_handle_free (hal_handle_t handle_id)
{
    hal_handle_ht_entry_t    *entry;

    entry = (hal_handle_ht_entry_t *)
                g_hal_state->hal_handle_id_ht()->remove(&handle_id);
    if (entry == NULL) {
        return;
    }
    if (entry->handle) {
        entry->handle->~hal_handle();
        g_hal_state->hal_handle_slab()->free(entry->handle);
    }
    g_hal_state->hal_handle_ht_entry_slab()->free(entry);

    return;
}

//------------------------------------------------------------------------------
// given a handle id, return the corresponding handle object
//------------------------------------------------------------------------------
hal_handle *
hal_handle_get_from_handle_id (hal_handle_t handle_id)
{
    hal_handle_ht_entry_t    *entry;

    // lookup by handle id and get the handle class
    entry =
        (hal_handle_ht_entry_t *)g_hal_state->hal_handle_id_ht()->lookup(&handle_id);
    if (entry) {
        return entry->handle;
    }
    HAL_TRACE_ERR("Failed to get handle obj from handle id {}", handle_id);
    return NULL;
}

//------------------------------------------------------------------------------
// given a handle id, return the corresponding config object in the db
//------------------------------------------------------------------------------
void *
hal_handle_get_obj (hal_handle_t handle_id)
{
    hal_handle_ht_entry_t    *entry;

    // lookup by handle id and get the handle class
    entry =
        (hal_handle_ht_entry_t *)g_hal_state->hal_handle_id_ht()->lookup(&handle_id);
    if (entry && entry->handle) {
        return entry->handle->get_obj();
    }
    return NULL;
}

//------------------------------------------------------------------------------
// given a handle id, add the object to the cfg db with this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle_add_obj (hal_handle_t handle_id, cfg_op_ctxt_t *ctxt,
                    hal_cfg_op_cb_t add_cb,
                    hal_cfg_commit_cb_t commit_cb,
                    hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_handle    *handle;

    handle = hal_handle_get_from_handle_id(handle_id);
    return handle->add_obj(ctxt, add_cb, commit_cb, abort_cb, cleanup_cb);
}

//------------------------------------------------------------------------------
// given a handle id, delete the object to the cfg db with this handle
//------------------------------------------------------------------------------
hal_ret_t
hal_handle_del_obj (hal_handle_t handle_id, cfg_op_ctxt_t *ctxt,
                    hal_cfg_op_cb_t del_cb,
                    hal_cfg_commit_cb_t commit_cb,
                    hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_handle    *handle;

    handle = hal_handle_get_from_handle_id(handle_id);
    return handle->del_obj(ctxt, del_cb, commit_cb, abort_cb, cleanup_cb);
}

//------------------------------------------------------------------------------
// given a handle id, update the corresponding config object in the db
//------------------------------------------------------------------------------
hal_ret_t
hal_handle_upd_obj (hal_handle_t handle_id, cfg_op_ctxt_t *ctxt,
                    hal_cfg_op_cb_t upd_cb,
                    hal_cfg_commit_cb_t commit_cb,
                    hal_cfg_abort_cb_t abort_cb,
                    hal_cfg_cleanup_cb_t cleanup_cb)
{
    hal_handle    *handle;

    handle = hal_handle_get_from_handle_id(handle_id);
    return handle->del_obj(ctxt, upd_cb, commit_cb, abort_cb, cleanup_cb);
}

// TODO: cleanup these two APIs once all objects move to new APIs
//------------------------------------------------------------------------------
// allocate a handle for an object instance
// TODO: if this can be called from FTE, we need atomic increments
//------------------------------------------------------------------------------
hal_handle_t
hal_alloc_handle (void)
{
    return g_hal_handle++;
}

//------------------------------------------------------------------------------
// return a hal handle back so it can be reallocated for another object
//------------------------------------------------------------------------------
void
hal_free_handle (uint64_t handle)
{
    return;
}

#if 0
//------------------------------------------------------------------------------
// allocate a handle for an object instance
//------------------------------------------------------------------------------
hal_handle_t
hal_handle_alloc (void)
{
    hal_handle    *handle;

    handle = hal_handle::factory();
    return reinterpret_cast<hal_handle_t>(handle);
}

//------------------------------------------------------------------------------
// return a hal handle back so it can be reallocated for another object
//------------------------------------------------------------------------------
void
hal_handle_free (hal_handle_t handle)
{
    hal_handle    *hndl;

    hndl = reinterpret_cast<hal_handle *>(handle);
    hndl->~hal_handle();
    g_hal_state->hal_handle_slab()->free(hndl);
    return;
}
#endif

//------------------------------------------------------------------------------
// initialize all the signal handlers
//------------------------------------------------------------------------------
static void
hal_sig_handler (int sig, siginfo_t *info, void *ptr)
{
    HAL_TRACE_DEBUG("HAL received signal {}", sig);

    switch (sig) {
    case SIGINT:
        HAL_GCOV_FLUSH();
        utils::hal_logger().flush();
        exit(0);
        break;

    case SIGUSR1:
    case SIGUSR2:
        HAL_GCOV_FLUSH();
        utils::hal_logger().flush();
        break;

    case SIGHUP:
    case SIGQUIT:
    case SIGCHLD:
    case SIGURG:
    case SIGTERM:
    default:
        utils::hal_logger().flush();
        break;
    }
}

//------------------------------------------------------------------------------
// initialize all the signal handlers
// TODO: save old handlers and restore when signal happened
//------------------------------------------------------------------------------
static hal_ret_t
hal_sig_init (void)
{
    struct sigaction    act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = hal_sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGURG, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
//  spawn and setup all the HAL threads - both config and packet loop threads
//------------------------------------------------------------------------------
static hal_ret_t
hal_thread_init (void)
{
    uint32_t              tid, core_id;
    int                   rv, thread_prio;
    char                  thread_name[16];
    struct sched_param    sched_param = { 0 };
    pthread_attr_t        attr;
    cpu_set_t             cpus;

    // spawn data core threads and pin them to their cores
    thread_prio = sched_get_priority_max(SCHED_FIFO);
    assert(thread_prio >= 0);
    for (tid = HAL_THREAD_ID_FTE_MIN, core_id = 1;
         tid <= HAL_THREAD_ID_FTE_MAX;       // TODO: fix the env !!
         tid++, core_id++) {
        HAL_TRACE_DEBUG("Spawning FTE thread {}", tid);
        snprintf(thread_name, sizeof(thread_name), "fte-core-%u", core_id);
        g_hal_threads[tid] =
            thread::factory(static_cast<const char *>(thread_name), tid,
                            core_id, fte_pkt_loop,
                            thread_prio, SCHED_FIFO, false);
        HAL_ABORT(g_hal_threads[tid] != NULL);
    }

    // spawn periodic thread that does background tasks
    g_hal_threads[HAL_THREAD_ID_PERIODIC] =
        thread::factory(std::string("periodic-thread").c_str(),
                        HAL_THREAD_ID_PERIODIC,
                        HAL_CONTROL_CORE_ID,
                        hal::periodic::periodic_thread_start,
                        thread_prio - 1, SCHED_RR, true);
    HAL_ABORT(g_hal_threads[HAL_THREAD_ID_PERIODIC] != NULL);
    g_hal_threads[HAL_THREAD_ID_PERIODIC]->start(g_hal_threads[HAL_THREAD_ID_PERIODIC]);

    // make the current thread, main hal config thread (also a real-time thread)
    rv = pthread_attr_init(&attr);
    if (rv != 0) {
        HAL_TRACE_ERR("pthread_attr_init failure, err : {}", rv);
        return HAL_RET_ERR;
    }

    // set core affinity
    CPU_ZERO(&cpus);
    CPU_SET(HAL_CONTROL_CORE_ID, &cpus);
    rv = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
    if (rv != 0) {
        HAL_TRACE_ERR("pthread_attr_setaffinity_np failure, err : {}", rv);
        return HAL_RET_ERR;
    }

    if (gl_super_user) {
        HAL_TRACE_DEBUG("Started by root, switching to real-time scheduling");
        sched_param.sched_priority = sched_get_priority_max(SCHED_RR);
        rv = sched_setscheduler(0, SCHED_RR, &sched_param);
        if (rv != 0) {
            HAL_TRACE_ERR("sched_setscheduler failure, err : {}", rv);
            return HAL_RET_ERR;
        }
    }

    // create a thread object for this main thread
    g_hal_threads[HAL_THREAD_ID_CFG] =
        thread::factory(std::string("cfg-thread").c_str(),
                        HAL_THREAD_ID_CFG,
                        HAL_CONTROL_CORE_ID,
                        thread::dummy_entry_func,
                        sched_param.sched_priority, SCHED_RR, true);
    g_hal_threads[HAL_THREAD_ID_CFG]->set_ctxt(g_hal_threads[HAL_THREAD_ID_CFG]);
    t_curr_thread = g_hal_threads[HAL_THREAD_ID_CFG];
    g_hal_threads[HAL_THREAD_ID_CFG]->set_pthread_id(pthread_self());
    g_hal_threads[HAL_THREAD_ID_CFG]->set_running(true);

    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
//  uninit all  the HAL threads - both config and packet loop threads.
//------------------------------------------------------------------------------
static hal_ret_t
hal_thread_destroy (void)
{
    HAL_ABORT(g_hal_threads[HAL_THREAD_ID_PERIODIC] != NULL);
    g_hal_threads[HAL_THREAD_ID_PERIODIC]->stop();
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// wait for all the HAL threads to be terminated and any other background
// activities
//------------------------------------------------------------------------------
hal_ret_t
hal_wait (void)
{
    int         rv;
    uint32_t    tid;

    for (tid = HAL_THREAD_ID_PERIODIC; tid < HAL_THREAD_ID_MAX; tid++) {
        if (g_hal_threads[tid]) {
            rv = pthread_join(g_hal_threads[tid]->pthread_id(), NULL);
            if (rv != 0) {
                HAL_TRACE_ERR("pthread_join failure, thread {}, err : {}",
                              g_hal_threads[tid]->name(), rv);
                return HAL_RET_ERR;
            }
        }
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// parse HAL configuration
//------------------------------------------------------------------------------
hal_ret_t
hal_parse_cfg (const char *cfgfile, hal_cfg_t *hal_cfg)
{
    ptree             pt;
    std::string       sparam;

    if (!cfgfile || !hal_cfg) {
        return HAL_RET_INVALID_ARG;
    }

    std::ifstream json_cfg(cfgfile);
    read_json(json_cfg, pt);
    try {
		std::string mode = pt.get<std::string>("mode");
        if (mode == "sim") {
            hal_cfg->sim = true;
        } else {
            hal_cfg->sim = false;
        }

        sparam = pt.get<std::string>("asic.name");
        strncpy(hal_cfg->asic_name, sparam.c_str(), HAL_MAX_NAME_STR);
        hal_cfg->grpc_port = pt.get<std::string>("sw.grpc_port");
        if (getenv("HAL_GRPC_PORT")) {
            hal_cfg->grpc_port = getenv("HAL_GRPC_PORT");
            HAL_TRACE_DEBUG("Overriding GRPC Port to : {}", hal_cfg->grpc_port);
        }
        sparam = pt.get<std::string>("sw.feature_set");
        strncpy(hal_cfg->feature_set, sparam.c_str(), HAL_MAX_NAME_STR);
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// init function for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_init (hal_cfg_t *hal_cfg)
{
    char    *user;

    HAL_TRACE_DEBUG("Initializing HAL ...");

    // check to see if HAL is running with root permissions
    user = getenv("USER");
    if (user && !strcmp(user, "root")) {
        gl_super_user = true;
    }

    // install signal handlers
    hal_sig_init();

    // do memory related initialization
    HAL_ABORT(hal_mem_init() == HAL_RET_OK);

    // init fte and hal plugins
    hal::init_plugins();

    // spawn all necessary PI threads
    HAL_ABORT(hal_thread_init() == HAL_RET_OK);
    HAL_TRACE_DEBUG("Spawned all HAL threads");

    // do platform dependent init
    HAL_ABORT(hal::pd::hal_pd_init(hal_cfg) == HAL_RET_OK);
    HAL_TRACE_DEBUG("Platform initialization done");

    // TODO_CLEANUP: this doesn't belong here, why is this outside
    // hal_state ??? how it this special compared to other global state ??
    g_lif_manager = new LIFManager();

    // do rdma init
    HAL_ABORT(rdma_hal_init() == HAL_RET_OK);

    if(!getenv("CAPRI_MOCK_MODE")) {
        // start fte threads
        for (int tid = HAL_THREAD_ID_FTE_MIN; tid <= HAL_THREAD_ID_FTE_MAX; tid++) {
            g_hal_threads[tid]->start(g_hal_threads[tid]);
            break; // TODO(goli) only one FTE thread until driver supports multiple ARQs
        }
    }
    
    hal_proxy_svc_init();
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// un init function for HAL
//------------------------------------------------------------------------------
hal_ret_t
hal_destroy (void)
{
    // cancel  all necessary PI threads
    HAL_ABORT(hal_thread_destroy() == HAL_RET_OK);
    HAL_TRACE_DEBUG("Cancelled  all HAL threads");

    return HAL_RET_OK;

}


}    // namespace hal
