//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __VMOTION_HPP__
#define __VMOTION_HPP__

#include "nic/include/base.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/sdk/lib/indexer/indexer.hpp"
#include "ev.h"

namespace hal {

#define VMOTION_WLOCK vmotion_.rwlock_.wlock();
#define VMOTION_WUNLOCK vmotion_.rwlock_.wunlock();

using sdk::lib::thread;
class hal_vmotion;

/*
 * Note:
 *  - Make sure there are enough thread IDs to support the max number of
 *    destinations and sources.
 */
typedef struct vmotion_s {
    sdk::wp_rwlock  rwlock_;
    thread          *vmotion_master; 
    uint32_t        port;
    indexer         *server_thread_ids;
    indexer         *client_thread_ids;
    uint32_t        max_servers;       // max no: of vmotions with naples as source
    uint32_t        max_clients;       // max no: of vmotion with naples as destination
} vmotion_t;

/*
 * data passed to the server slave thread.
 */
typedef struct vmotion_server_slave_s {
    void    *ctxt;          // vmotioned ep info 
    int     slave_sd;
    thread  *th;
} vmotion_server_slave_t;

typedef struct vmotion_client_s {
    void        *ctxt;      // vmotioned ep info
    int         client_sd;
    thread      *th;
    hal_vmotion *vm;
} vmotion_client_t;

class hal_vmotion {
public:
    static hal_vmotion *factory(uint32_t max_servers,
                                uint32_t max_clients,
                                uint32_t vmotion_port);
    static void destroy(hal_vmotion *vmotion);
    uint32_t port(void) { return vmotion_.port; }
    uint32_t max_servers(void) { return vmotion_.max_servers; } 
    hal_ret_t vmotion_trigger_client(void *ctxt);

private:
    vmotion_t vmotion_;

private:
    hal_vmotion() {};
    ~hal_vmotion() {};

    hal_ret_t init_(uint32_t max_servers, uint32_t max_clients,
                    uint32_t port);
    hal_ret_t alloc_thread_id_(bool is_server, uint32_t *tid);
    hal_ret_t release_thread_id_(bool is_server, uint32_t tid);
    hal_ret_t spawn_master_(void);
    hal_ret_t spawn_client_(void *ctxt);
    static void *master_thread_cb_(void *ctxt);
    static void new_server_slave_cb_(struct ev_loop *loop, struct ev_io *watcher, 
                                     int revents);
    static void *server_slave_thread_cb_(void *ctxt);
    static void server_slave_thread_cleanup_(void *ctxt);
    static void server_slave_cb_(struct ev_loop *loop, struct ev_io *watcher, 
                                 int revents);
    static void *client_thread_cb_(void *ctxt);
};

hal_ret_t hal_vmotion_init(hal_cfg_t *hal_cfg);

} // namespace hal

#endif    // __VMOTION_HPP__

