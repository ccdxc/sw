//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __VMOTION_HPP__
#define __VMOTION_HPP__

#include "nic/include/base.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/include/fte.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/vmotion/vmotion_tls.hpp"
#include "nic/hal/vmotion/vmotion_src_host.hpp"
#include "nic/hal/vmotion/vmotion_dst_host.hpp"
#include "ev.h"

namespace hal {

#define VMOTION_PORT                 50055

#define VMOTION_MAX_SESS_PER_MSG     1000
#define VMOTION_AGE_DELAY            300000000000 // 5 Minutes 
#define VMOTION_SESS_NORMALIZATION   300000       // (in Milliseconds) - 5 Minutes
#define VMOTION_CONNECT_RETRY_TIME   30           // in Seconds
#define VMOTION_TIMEOUT              900000       // (in Milliseconds) - 15 Minutes
#define VMOTION_THR_DELAY_DEL_TIME   1000         // (in Milliseconds) - 1 Sec

#define VMOTION_WLOCK   vmotion_.rwlock.wlock();
#define VMOTION_WUNLOCK vmotion_.rwlock.wunlock();

#define VMOTION_MAX_THREADS \
        (HAL_THREAD_ID_VMOTION_THREADS_MAX - HAL_THREAD_ID_VMOTION_THREADS_MIN + 1)

#define VMOTION_IS_ENABLED()  (g_hal_state->get_vmotion() ? true : false)

class vmotion;

enum vmotion_flag_t {
    VMOTION_FLAG_POS_RARP_RCVD   = 0,
    VMOTION_FLAG_POS_EP_MOV_DONE_RCVD = 1,
    VMOTION_FLAG_EP_QUIESCE_ADDED = 2,
    VMOTION_FLAG_THREAD_EXITED = 3,
    VMOTION_FLAG_INP_MAC_REMOVED = 4,
};

enum vmotion_thread_evt_t {
    VMOTION_EVT_RARP_RCVD    = 1,
    VMOTION_EVT_EP_MV_START  = 2,
    VMOTION_EVT_EP_MV_DONE   = 3,
    VMOTION_EVT_EP_MV_ABORT  = 4,
    VMOTION_EVT_EP_MV_COLD   = 5,
    VMOTION_EVT_TIMEOUT      = 6
};

#define VMOTION_SET_BIT(flags, bit_pos)     ((flags) |=  (1 << (bit_pos)))
#define VMOTION_RESET_BIT(flags, bit_pos)   ((flags) &= (~(1 << (bit_pos))))
#define VMOTION_IS_BIT_SET(flags, bit_pos)  ((flags) & (1 << (bit_pos)))

#define VMOTION_FLAG_SET_RARP_RCVD(vmn_ep) \
    (vmn_ep->set_flags(VMOTION_SET_BIT(*(vmn_ep->get_flags()), VMOTION_FLAG_POS_RARP_RCVD)))
#define VMOTION_FLAG_SET_EP_MOV_DONE_RCVD(vmn_ep) \
    (vmn_ep->set_flags(VMOTION_SET_BIT(*(vmn_ep->get_flags()), VMOTION_FLAG_POS_EP_MOV_DONE_RCVD)))
#define VMOTION_FLAG_SET_EP_QUIESCE_ADDED(vmn_ep) \
    (vmn_ep->set_flags(VMOTION_SET_BIT(*(vmn_ep->get_flags()), VMOTION_FLAG_EP_QUIESCE_ADDED)))
#define VMOTION_FLAG_SET_THREAD_EXITED(vmn_ep) \
    (vmn_ep->set_flags(VMOTION_SET_BIT(*(vmn_ep->get_flags()), VMOTION_FLAG_THREAD_EXITED)))
#define VMOTION_FLAG_SET_INP_MAC_REMOVED(vmn_ep) \
    (vmn_ep->set_flags(VMOTION_SET_BIT(*(vmn_ep->get_flags()), VMOTION_FLAG_INP_MAC_REMOVED)))

#define VMOTION_FLAG_RESET_EP_QUIESCE_ADDED(vmn_ep) \
    (vmn_ep->set_flags(VMOTION_RESET_BIT(*(vmn_ep->get_flags()), VMOTION_FLAG_EP_QUIESCE_ADDED)))
#define VMOTION_FLAG_RESET_INP_MAC_REMOVED(vmn_ep) \
    (vmn_ep->set_flags(VMOTION_RESET_BIT(*(vmn_ep->get_flags()), VMOTION_FLAG_INP_MAC_REMOVED)))

#define VMOTION_FLAG_IS_RARP_SET(vmn_ep) \
    (VMOTION_IS_BIT_SET(*(vmn_ep->get_flags()), VMOTION_FLAG_POS_RARP_RCVD))
#define VMOTION_FLAG_IS_EP_MOV_DONE_SET(vmn_ep) \
    (VMOTION_IS_BIT_SET(*(vmn_ep->get_flags()), VMOTION_FLAG_POS_EP_MOV_DONE_RCVD))
#define VMOTION_FLAG_IS_EP_QUIESCE_ADDED(vmn_ep) \
    (VMOTION_IS_BIT_SET(*(vmn_ep->get_flags()), VMOTION_FLAG_EP_QUIESCE_ADDED))
#define VMOTION_FLAG_IS_THREAD_EXITED(vmn_ep) \
    (VMOTION_IS_BIT_SET(*(vmn_ep->get_flags()), VMOTION_FLAG_THREAD_EXITED))
#define VMOTION_FLAG_IS_INP_MAC_REMOVED(vmn_ep) \
    (VMOTION_IS_BIT_SET(*(vmn_ep->get_flags()), VMOTION_FLAG_INP_MAC_REMOVED))

typedef struct vmotion_stats_s {
    uint32_t    total_vmotion;
    uint32_t    mig_in_vmotion;
    uint32_t    mig_out_vmotion;
    uint32_t    mig_success;
    uint32_t    mig_failed;
    uint32_t    mig_aborted;
    uint32_t    mig_timeout;
    uint32_t    mig_cold;
} vmotion_stats_t;

class vmotion_ep {
public:
    // Factory methods
    static vmotion_ep*               factory(ep_t *ep, ep_vmotion_type_t type);
    static void                      destroy(vmotion_ep* vmn_ep);
    hal_ret_t                        init(ep_t *ep, ep_vmotion_type_t type);

    // Get methods
    fsm_state_machine_t*             get_sm(void) { return sm_; }
    ep_vmotion_type_t                get_vmotion_type(void) const { return vmotion_type_; }
    vmotion*                         get_vmotion(void) { return vmotion_ptr_; }
    ep_t*                            get_ep(void) { return find_ep_by_handle(ep_hdl_); }
    hal_handle_t                     get_ep_handle(void) { return ep_hdl_; }
    const ip_addr_t&                 get_old_homing_host_ip(void) { return old_homing_host_ip_; }
    uint64_t                         get_last_sync_time(void) { return last_sync_time_; }
    mac_addr_t*                      get_ep_mac(void) { return &mac_; }
    sdk::event_thread::io_t*         get_event_io(void) { return &evt_io_; }
    sdk::event_thread::event_thread* get_event_thread(void) { return evt_thread_; }
    uint32_t*                        get_flags(void) { return &flags_;}
    uint32_t                         get_thread_id(void) { return thread_id_; }
    MigrationState                   get_migration_state(void) { return migration_state_; }
    SSL*                             get_ssl(void) { return tls_connection_->get_ssl(); }

    // Set methods
    void                set_socket_fd(int fd) { sock_fd_ = fd; }
    void                set_event_thread(sdk::event_thread::event_thread *th) { evt_thread_ = th; }
    void                set_last_sync_time(void);
    void                set_flags(uint32_t flags) { flags_ = flags;}
    void                set_thread_id(uint32_t thr_id) { thread_id_ = thr_id; }
    void                set_migration_state(MigrationState state) { migration_state_ = state; }
    void                set_expiry_timer(void *tmr) { expiry_timer_ = tmr; }
    void                set_tls_connection(TLSConnection *conn) { tls_connection_ = conn; }

    // Methods
    hal_ret_t           spawn_dst_host_thread(void);
    hal_ret_t           dst_host_init(void);
    hal_ret_t           dst_host_exit(void);
    void                populate_vmotion_ep_dump(internal::VmotionDebugEp *rsp); 

    // FSM Methods
    void  process_event(uint32_t event, fsm_event_data data) { sm_->process_event(event, data); }
    void throw_event(uint32_t event, fsm_event_data data) { sm_->throw_event(event, data); }

private:
    vmotion                         *vmotion_ptr_;
    sdk::event_thread::event_thread *evt_thread_;
    int                              sock_fd_;
    uint32_t                         thread_id_;
    uint32_t                         flags_;
    uint64_t                         last_sync_time_;
    sdk::event_thread::io_t          evt_io_;
    mac_addr_t                       mac_;
    ip_addr_t                        old_homing_host_ip_;
    fsm_state_machine_t             *sm_;
    hal_handle_t                     ep_hdl_;
    ep_vmotion_type_t                vmotion_type_;
    void                            *expiry_timer_;
    MigrationState                   migration_state_;
    TLSConnection                   *tls_connection_; 
};

class vmotion {
public:
    // Factory methods
    static vmotion *factory(uint32_t max_threads, uint32_t vmotion_port);
    static void    destroy(vmotion *vmotion);

    // Get Methods
    uint32_t                 get_vmotion_port(void) { return vmotion_.port; }
    uint32_t                 get_max_threads(void) { return vmotion_.max_threads; }
    vmotion_ep*              get_vmotion_ep(ep_t *ep);
    sdk::event_thread::io_t* get_master_thread_io(void) { return &(vmotion_.master_thread_io); }

    // Methods
    vmotion_ep*   create_vmotion_ep(ep_t *ep, ep_vmotion_type_t type);
    hal_ret_t     delete_vmotion_ep(vmotion_ep *vmn_ep);
    hal_ret_t     vmotion_handle_ep_del(ep_t *ep);
    void          run_vmotion(ep_t *ep, vmotion_thread_evt_t event);
    hal_ret_t     alloc_thread_id(uint32_t *tid);
    hal_ret_t     release_thread_id(uint32_t tid);
    hal_ret_t     spawn_src_host_thread(int sock_fd);
    bool          process_rarp(mac_addr_t mac);
    void          populate_vmotion_dump(internal::VmotionDebugResponse *rsp);
    void          incr_stats(uint32_t vmotion_stats_t::* const p_field) { (stats_.*p_field)++; }
    void          incr_migration_state_stats(MigrationState state); 
    TLSContext*   get_tls_context(void) { return tls_context_; }
    static void   delay_delete_thread(sdk::event_thread::event_thread *thr);

    // FSM Related methods
    static vmotion_src_host_fsm_def* src_host_fsm_def_;
    static vmotion_dst_host_fsm_def* dst_host_fsm_def_;
    static fsm_state_machine_def_t*  get_src_host_fsm_def_func(void) {
        return src_host_fsm_def_->get_state_machine();
    }
    static fsm_state_machine_def_t*  get_dst_host_fsm_def_func(void) {
        return dst_host_fsm_def_->get_state_machine();
    }

private:
    hal_ret_t   init(uint32_t max_threads, uint32_t vmotion_port);
    static void master_thread_init(void *ctxt);
    static void master_thread_cb(sdk::event_thread::io_t *io, int sock_fd, int events);

    typedef struct vmotion_s {
        sdk::event_thread::event_thread *vmotion_master;
        sdk::event_thread::io_t          master_thread_io;
        rte_indexer                     *threads_idxr; 
        uint32_t                         max_threads;
        uint32_t                         port;
        sdk::wp_rwlock                   rwlock;
    } vmotion_t;

    vmotion_t                  vmotion_;
    std::vector<vmotion_ep *>  vmn_eps_;
    vmotion_stats_t            stats_; 
    TLSContext                *tls_context_; 
};

// data passed to the destination/source host thread.
typedef struct vmotion_thread_ctx_s {
    vmotion_ep                      *vmn_ep;
    int                              fd;
    sdk::event_thread::event_thread *th;
    uint32_t                         tid;
    sdk::event_thread::io_t          io;
    void                            *expiry_timer;
    TLSConnection                   *tls_connection;
} vmotion_thread_ctx_t;


hal_ret_t vmotion_init(int vmotion_port);

} // namespace hal

#endif    // __VMOTION_HPP__

