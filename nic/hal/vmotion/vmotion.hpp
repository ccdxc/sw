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
#include "nic/hal/vmotion/vmotion_src_host.hpp"
#include "nic/hal/vmotion/vmotion_dst_host.hpp"
#include "ev.h"

namespace hal {

#define VMOTION_WLOCK   vmotion_.rwlock.wlock();
#define VMOTION_WUNLOCK vmotion_.rwlock.wunlock();

class vmotion;

enum vmotion_type_t {
    VMOTION_TYPE_MIGRATE_IN,
    VMOTION_TYPE_MIGRATE_OUT
};

class vmotion_ep {
public:
    // Factory methods
    static vmotion_ep*               factory(ep_t *ep, vmotion_type_t type);
    static void                      destroy(vmotion_ep* vmn_ep);
    hal_ret_t                        init(ep_t *ep, vmotion_type_t type);

    // Get methods
    fsm_state_machine_t*             get_sm(void) { return sm_; }
    vmotion_type_t                   get_vmotion_type(void) const { return vmotion_type_; }
    vmotion*                         get_vmotion(void) { return vmotion_ptr_; }
    ep_t*                            get_ep(void) { return ep_; }
    mac_addr_t*                      get_ep_mac(void) { return &mac_; }
    int                              get_socket_fd(void) { return sock_fd_; }
    string                           get_old_host_ip(void) const { return old_host_ip_; }
    sdk::event_thread::io_t*         get_event_io(void) { return &evt_io_; }
    sdk::event_thread::event_thread* get_event_thread(void) { return evt_thread_; }

    // Set methods
    void                set_socket_fd(int fd) { sock_fd_ = fd; }
    void                set_event_thread(sdk::event_thread::event_thread *th) { evt_thread_ = th; }

    // Methods
    hal_ret_t           spawn_dst_host_thread(void);
    hal_ret_t           dst_host_init(void);
    hal_ret_t           dst_host_exit(void);
    hal_ret_t           ep_quiesce_rule_add(void);

    // FSM Methods
    void  process_event(uint32_t event, fsm_event_data data) { sm_->process_event(event, data); }
    void throw_event(uint32_t event, fsm_event_data data) { sm_->throw_event(event, data); }

private:
    vmotion                         *vmotion_ptr_;
    sdk::event_thread::event_thread *evt_thread_;
    int                              sock_fd_;
    uint32_t                         thread_id_;
    uint32_t                         flags_;
    sdk::event_thread::io_t          evt_io_;
    mac_addr_t                       mac_;
    fsm_state_machine_t             *sm_;
    ep_t                            *ep_; 
    vmotion_type_t                   vmotion_type_;
    string                           old_host_ip_ = "127.0.0.1"; // TEMP
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
    vmotion_ep*   create_vmotion_ep(ep_t *ep, vmotion_type_t type);
    hal_ret_t     delete_vmotion_ep(vmotion_ep *vmn_ep);
    void          run_vmotion(ep_t *ep, vmotion_type_t type);
    hal_ret_t     alloc_thread_id(uint32_t *tid);
    hal_ret_t     release_thread_id(uint32_t tid);
    hal_ret_t     spawn_src_host_thread(int sock_fd);
    hal_ret_t     process_rarp(mac_addr_t mac);

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
};

// data passed to the destination/source host thread.
typedef struct vmotion_thread_ctx_s {
    vmotion_ep                      *vmn_ep;
    int                              fd;
    sdk::event_thread::event_thread *th;
    uint32_t                         tid;
    sdk::event_thread::io_t          io;
} vmotion_thread_ctx_t;


hal_ret_t vmotion_init(hal_cfg_t *hal_cfg);

} // namespace hal

#endif    // __VMOTION_HPP__

