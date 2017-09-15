// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <iostream>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <hal.hpp>
#include <hal_pd.hpp>
#include <periodic/periodic.hpp>
#include <lif_manager.hpp>
#include <rdma.hpp>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <interface.hpp>
#include <tcpcb.hpp>
#include <proxy.hpp>
#include <fte.hpp>
#include <defines.h>
#include <cpupkt_api.hpp>
#include <plugins/plugins.hpp>

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

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

    uint8_t fte_id = HAL_THREAD_ID_FTE_MIN - t_curr_thread->thread_id();

    // only one FTE for now
    if (fte_id != 0) {
        return NULL;
    }

    hal::pd::cpupkt_ctxt_t* arm_ctx = hal::pd::cpupkt_ctxt_alloc_init();
    cpupkt_register_rx_queue(arm_ctx, types::WRING_TYPE_ARQRX);
    cpupkt_register_tx_queue(arm_ctx, types::WRING_TYPE_ASQ);

    fte::phv_t rx_phv;
    hal::pd::p4_to_p4plus_cpu_pkt_t *rx_hdr = NULL;
    uint8_t *rx_pkt = NULL;

    fte::arm_rx_t arm_rx = [&](fte::phv_t **phv, uint8_t **pkt, size_t *pkt_len) {
        hal_ret_t ret;
        if (rx_hdr != NULL) {
            cpupkt_free(rx_hdr, rx_pkt);
        }

        rx_hdr = NULL;
        rx_pkt = NULL;
        rx_phv = {};

        ret = cpupkt_poll_receive(arm_ctx, &rx_hdr, &rx_pkt, pkt_len);
        if (ret == HAL_RET_OK) {
            bool inner_valid = false; 
            rx_phv.src_lif = rx_hdr->src_lif;
            rx_phv.lif = 1;  // TODO
            rx_phv.qtype = 0;
            rx_phv.qid = 0; 
            rx_phv.lkp_type = rx_hdr->lkp_type;
            if (rx_hdr->flags & CPU_FLAGS_LKP_DIR) {
                rx_phv.lkp_dir = 1;
            }
            rx_phv.lkp_vrf = rx_hdr->lkp_vrf;

            // extract src/dst
            switch (rx_phv.lkp_type) {
            case FLOW_KEY_LOOKUP_TYPE_MAC:
                if (rx_hdr->flags & CPU_FLAGS_TUNNEL_TERMINATE) {
                    MAC_UINT64_TO_ADDR(rx_phv.lkp_src, rx_hdr->mac_sa_inner);
                    MAC_UINT64_TO_ADDR(rx_phv.lkp_dst, rx_hdr->mac_da_inner);
                } else {
                    MAC_UINT64_TO_ADDR(rx_phv.lkp_src, rx_hdr->mac_sa_outer);
                    MAC_UINT64_TO_ADDR(rx_phv.lkp_dst, rx_hdr->mac_da_outer);
                }
                // TODO(goli)
            case FLOW_KEY_LOOKUP_TYPE_IPV4:
                inner_valid  = (rx_hdr->flags & CPU_FLAGS_INNER_IPV4_VALID);
                if (inner_valid) {
                    memcpy(rx_phv.lkp_src, rx_hdr->ip_sa_inner+4, 4);
                    memcpy(rx_phv.lkp_dst, rx_hdr->ip_sa_inner+8, 4);
                } else {
                    memcpy(rx_phv.lkp_src, rx_hdr->ip_sa_outer+4, 4);
                    memcpy(rx_phv.lkp_dst, rx_hdr->ip_sa_outer+8, 4);
                }
                *(ipv4_addr_t*)rx_phv.lkp_src = ntohl(*(ipv4_addr_t*)rx_phv.lkp_src);
                *(ipv4_addr_t*)rx_phv.lkp_dst = ntohl(*(ipv4_addr_t*)rx_phv.lkp_dst);

                break;
            case FLOW_KEY_LOOKUP_TYPE_IPV6:
                inner_valid  = (rx_hdr->flags & CPU_FLAGS_INNER_IPV6_VALID);
                if (inner_valid) {
                    memcpy(rx_phv.lkp_src, rx_hdr->ip_sa_inner, sizeof(rx_phv.lkp_src));
                    memcpy(rx_phv.lkp_dst, rx_hdr->ip_da_inner, sizeof(rx_phv.lkp_dst));
                } else {
                    memcpy(rx_phv.lkp_src, rx_hdr->ip_sa_outer, sizeof(rx_phv.lkp_src));
                    memcpy(rx_phv.lkp_dst, rx_hdr->ip_da_outer, sizeof(rx_phv.lkp_dst));
                }
                break;
            }

            // extract proto
            rx_phv.lkp_proto = inner_valid ? rx_hdr->ip_proto_inner : rx_hdr->ip_proto_outer;

            // extract l4
            if (rx_phv.lkp_proto == IP_PROTO_UDP && !inner_valid) {
                rx_phv.lkp_sport = ntohs(rx_hdr->l4_sport_outer);
                rx_phv.lkp_dport = ntohs(rx_hdr->l4_dport_outer);
            } else {
                rx_phv.lkp_sport = ntohs(rx_hdr->l4_sport_inner);
                rx_phv.lkp_dport = ntohs(rx_hdr->l4_dport_inner);
            }

            *phv = &rx_phv;
            *pkt = rx_pkt;
        }
        return ret;
    };

    fte::arm_tx_t arm_tx = [&](const fte::phv_t *phv, const uint8_t *pkt, size_t pkt_len) {
        hal::pd::p4plus_to_p4_header_t header = {};
        HAL_TRACE_DEBUG("writing pkt to arq");
        return cpupkt_send(arm_ctx, &header, (uint8_t *)pkt, pkt_len);
    };

    fte::pkt_loop(arm_rx, arm_tx);

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
//------------------------------------------------------------------------------
bool
hal_handle::init()
{
    HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    for (uint32_t i = 0; i < k_max_objs_; i++) {
         objs_[i].valid = FALSE;
         objs_[i].ver = HAL_CFG_VER_NONE;
         objs_[i].obj = NULL;
    }
    return true;
}

//------------------------------------------------------------------------------
// factory method to allocate a handle
//------------------------------------------------------------------------------
hal_handle *
hal_handle::factory(void)
{
    void          *mem;
    hal_handle    *handle;

    // allocate from the handle slab
    mem = g_hal_state->hal_handle_slab()->alloc();
    HAL_ASSERT_RETURN((mem != NULL), NULL);

    handle = new(mem) hal_handle();
    // initialize the handle instance
    if (handle->init() == false) {
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
    uint32_t    i;

    HAL_SPINLOCK_DESTROY(&slock_);
    for (i = 0; i < HAL_ARRAY_SIZE(objs_); i++) {
        if (objs_[i].valid) {
            HAL_TRACE_ERR("HAL handle destroy failied, valid object found at "
                          "idx {}, version {}, obj {}", i, objs_[i].ver,
                          objs_[i].obj);
            HAL_ABORT(FALSE);
        }
    }
}

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
                HAL_TRACE_ERR("Failed to add object le, found "
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
    HAL_ASSERT_GOTO((free_slot >= 0), end);
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
                HAL_ASSERT_GOTO((ret == HAL_RET_OK), error);
                goto error;
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
                goto done;
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

done:

    // add an entry into cfg db's delete object cache
    g_hal_state->cfg_db()->add_obj_to_del_cache(this, obj, del_cb);

error:

    HAL_SPINLOCK_UNLOCK(&slock_);
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
// be a valid entry but obj is NULL for objects that are deleted)
//------------------------------------------------------------------------------
void *
hal_handle::get_any_obj(void)
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
hal_handle_free (uint64_t handle)
{
    hal_handle    *hndl;

    hndl = reinterpret_cast<hal_handle *>(handle);
    hndl->~hal_handle();
    g_hal_state->hal_handle_slab()->free(hndl);
    return;
}

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

    // start fte threads
    for (int tid = HAL_THREAD_ID_FTE_MIN; tid <= HAL_THREAD_ID_FTE_MAX; tid++) {
        //g_hal_threads[tid]->start(g_hal_threads[tid]);
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
