//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/vmotion/vmotion.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/core/core.hpp"
#include "nic/hal/plugins/cfg/nw/ep_vmotion.hpp"

namespace hal {

//-----------------------------------------------------------------------------
// spawn vmotion thread
//-----------------------------------------------------------------------------
hal_vmotion *
hal_vmotion::factory(uint32_t max_servers, uint32_t max_clients, 
                     uint32_t port)
{
    hal_ret_t ret;
    void *mem;
    hal_vmotion *vm;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_VMOTION, sizeof(hal_vmotion));
    if (!mem) {
        HAL_TRACE_ERR("OOM: failed to allocate memory for vmotion");
        return NULL;
    }

    vm = new (mem) hal_vmotion();
    ret = vm->init_(max_servers, max_clients, port);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to init vmotion");
        vm->~hal_vmotion();
        HAL_FREE(HAL_MEM_ALLOC_VMOTION, vm);
        return NULL;
    }

    return vm;
}

hal_ret_t
hal_vmotion::init_(uint32_t max_servers, uint32_t max_clients,
                   uint32_t port)
{
    hal_ret_t ret = HAL_RET_OK;

    vmotion_.port              = port;
    vmotion_.max_servers       = max_servers;
    vmotion_.max_clients       = max_clients;
    vmotion_.server_thread_ids = indexer::factory(max_servers, false, false);
    vmotion_.client_thread_ids = indexer::factory(max_clients, false, false);

    // spawn master server thread
    ret = spawn_master_();

    return ret;
}

hal_ret_t
hal_vmotion::alloc_thread_id_(bool is_server, uint32_t *tid)
{
    hal_ret_t ret = HAL_RET_OK;
    indexer *tid_indexer;
    indexer::status st;
    uint32_t base;

    VMOTION_WLOCK

    tid_indexer = vmotion_.server_thread_ids;
    base = HAL_THREAD_ID_VMOTION_SERVER_MIN;
    if (!is_server) {
        tid_indexer = vmotion_.client_thread_ids;
        base = HAL_THREAD_ID_VMOTION_CLIENT_MIN;
    } 

    // allocated tid
    st = tid_indexer->alloc(tid);
    if (st != indexer::SUCCESS) {
        HAL_TRACE_ERR("unable to allocated thread id");
        ret = HAL_RET_ERR;
        goto end;
    }

    *tid += base;

end:
    VMOTION_WUNLOCK
    return ret;
}

hal_ret_t
hal_vmotion::release_thread_id_(bool is_server, uint32_t tid)
{
    hal_ret_t ret = HAL_RET_OK;
    indexer *tid_indexer;
    indexer::status st;

    VMOTION_WLOCK

    tid_indexer = vmotion_.server_thread_ids;
    if (!is_server) {
        tid_indexer = vmotion_.client_thread_ids;
    } 

    // free tid
    st = tid_indexer->free(tid);
    if (st != indexer::SUCCESS) {
        HAL_TRACE_ERR("unable to free thread id");
        ret = HAL_RET_ERR;
        goto end;
    }

end:
    VMOTION_WUNLOCK
    return ret;
}

hal_ret_t
hal_vmotion::spawn_master_(void)
{
    uint32_t tid;
    const char* thread_name;

    tid = HAL_THREAD_ID_VMOTION;
    thread_name = "vmn-srv-mas";

    vmotion_.vmotion_master = 
        hal_thread_create(thread_name, tid, sdk::lib::THREAD_ROLE_CONTROL,
                          0x0, // use all control cores
                          hal_vmotion::master_thread_cb_,
                          thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL), 
                          thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                          this);
    hal_thread_start(tid, hal_thread_get(tid));

    return HAL_RET_OK;
}

void *
hal_vmotion::master_thread_cb_(void *ctxt)
{
    thread *curr_thread = (thread *)ctxt;
    hal_vmotion *vm = (hal_vmotion *)curr_thread->data();
    struct ev_loop *loop = ev_loop_new(EVBACKEND_POLL | EVBACKEND_SELECT | EVFLAG_NOENV);
    struct ev_io w_accept;
    struct sockaddr_in address;
    int master_socket;
    int opt = true;

    SDK_THREAD_INIT(ctxt);

    // create master socket
    master_socket = socket(AF_INET , SOCK_STREAM , 0);
    
    // set master socket to allow multiple connections
    setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
               sizeof(opt));

    // configure address settings
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(vm->port());

    // bind socket to localhost:port
    bind(master_socket, (struct sockaddr *) &address, sizeof(address));

    // listen
    if (listen(master_socket, vm->max_servers()) != 0) {
        // close socket
        close(master_socket);
        return NULL;
    }

    // initialize and start a watcher to accept client requests
    ev_io_init(&w_accept, new_server_slave_cb_, master_socket, EV_READ);
    ev_io_start(loop, &w_accept);

    // start event loop
    ev_run(loop, 0);

    return NULL;
}

/*
 * call back when a new client is trying to connect.
 * - create a new thread. 
 * - pass the thread object and fd to the new thread.
 */
void 
hal_vmotion::new_server_slave_cb_(struct ev_loop *loop, struct ev_io *watcher, 
                                  int revents)
{
    hal_ret_t               ret = HAL_RET_OK;
    uint32_t                tid;
    struct sockaddr_in      client_addr;
    socklen_t               client_len = sizeof(client_addr);
    vmotion_server_slave_t  *ss_info;
    int                     server_slave_socket;
    char                    thread_name[1024];
    thread                  *curr_thread;
    hal_vmotion             *vm;

    if (EV_ERROR & revents) {
        HAL_TRACE_ERR("invalid event.");
        return;
    }

    // accept the client request
    server_slave_socket = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);
    if (server_slave_socket < 0) {
        HAL_TRACE_ERR("accept error");
        return;
    }

    // start a thread for server slave
    curr_thread = sdk::lib::thread::current_thread();
    vm = (hal_vmotion *)curr_thread->data();
    ret = vm->alloc_thread_id_(true, &tid);
    if (ret != HAL_RET_OK) {
        return;
    }
    HAL_TRACE_DEBUG("server slave active for client: {}:{} with thread_id: {}",
                    inet_ntoa(client_addr.sin_addr),
                    (int) ntohs(client_addr.sin_port),
                    tid);
    snprintf(thread_name, sizeof(thread_name), "vmn-ssl-%d", tid);
    ss_info = (vmotion_server_slave_t * )HAL_CALLOC(HAL_MEM_ALLOC_VMOTION_SERVER_SLAVE_DATA,
                                                    sizeof(vmotion_server_slave_t));
    ss_info->slave_sd = server_slave_socket;
    ss_info->th = 
        hal_thread_create(thread_name, tid, sdk::lib::THREAD_ROLE_CONTROL,
                          0x0, // use all control cores
                          hal_vmotion::server_slave_thread_cb_,
                          thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL), 
                          thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                          ss_info);
    hal_thread_start(tid, hal_thread_get(tid));
}

void
hal_vmotion::server_slave_thread_cleanup_(void *ctxt)
{
    thread *curr_thread = (thread *)ctxt;
    vmotion_server_slave_t *ss_info = (vmotion_server_slave_t *)curr_thread->data();
    int server_slave_sd = ss_info->slave_sd;

    HAL_TRACE_DEBUG("server slave thread cleanup");

    // close socket descriptor
    close(server_slave_sd);
}

void *
hal_vmotion::server_slave_thread_cb_(void *ctxt)
{
    thread *curr_thread = (thread *)ctxt;
    vmotion_server_slave_t *ss_info = (vmotion_server_slave_t *)curr_thread->data();
    struct ev_loop *loop = ev_loop_new(EVBACKEND_POLL | EVBACKEND_SELECT | EVFLAG_NOENV);
    struct ev_io w_read;
    int server_slave_sd = ss_info->slave_sd;

    SDK_THREAD_INIT(ctxt);

    pthread_cleanup_push(server_slave_thread_cleanup_, ctxt);

    // initialize and start a watcher to accept client messages
    ev_io_init(&w_read, server_slave_cb_, server_slave_sd, EV_READ);
    ev_io_start(loop, &w_read);

    // start event loop
    ev_run(loop, 0);

    pthread_cleanup_pop(1);

    return NULL;
}

void 
hal_vmotion::server_slave_cb_(struct ev_loop *loop, struct ev_io *watcher, 
                              int revents)
{
    hal_ret_t ret = HAL_RET_OK;
    int server_slave_sd = watcher->fd;
    // int n;
    // char buff[1024] = {0};

    HAL_TRACE_DEBUG("Received data from client");
    ret = ep_vmotion_server(server_slave_sd);
    if (ret != HAL_RET_OK) {
        // client closed the connection
        HAL_TRACE_DEBUG("client closed the connection");
        // stop watcher, break the loop, destroy the loop
        ev_io_stop(loop, watcher);
        ev_break(loop, EVBREAK_ALL);
        ev_loop_destroy(loop);
    }

#if 0
    HAL_TRACE_ERR("Received data from client");
    n = recv(server_slave_sd, buff, sizeof(buff), 0);
    if (n <= 0) {
        if (n == 0) {
            // client closed the connection
            HAL_TRACE_DEBUG("client closed the connection");
            // stop watcher, break the loop, destroy the loop
            ev_io_stop(loop, watcher);
            ev_break(loop, EVBREAK_ALL);
            ev_loop_destroy(loop);
        } else if (EAGAIN == errno) {
            // buffer full
            HAL_TRACE_ERR("receive timedout");
        } else {
            HAL_TRACE_ERR("receive error: n:{}, errno: {}", n, errno);
        }
    }
    HAL_TRACE_ERR("Message from client: {} of len: {}", buff, strlen(buff));
#endif
}

hal_ret_t 
hal_vmotion::vmotion_trigger_client(void *ctxt)
{
    // spawn a client thread
    return spawn_client_(ctxt);
}

hal_ret_t 
hal_vmotion::spawn_client_(void *ctxt)
{
    hal_ret_t ret = HAL_RET_OK;
    vmotion_client_t *vm_client;
    uint32_t tid;
    char thread_name[1024];

    HAL_TRACE_DEBUG("spawning client");

    ret = alloc_thread_id_(false, &tid);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    snprintf(thread_name, sizeof(thread_name), "vm-cl-%d", tid);
    vm_client = (vmotion_client_t *)HAL_CALLOC(HAL_MEM_ALLOC_VMOTION_CLIENT_DATA,
                                               sizeof(vmotion_client_t));
    vm_client->ctxt = ctxt;
    vm_client->th = 
        hal_thread_create(thread_name, tid, sdk::lib::THREAD_ROLE_CONTROL,
                          0x0, // use all control cores
                          hal_vmotion::client_thread_cb_,
                          thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL), 
                          thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                          vm_client);
    hal_thread_start(tid, hal_thread_get(tid));

    return ret;
}

void *
hal_vmotion::client_thread_cb_(void *ctxt)
{
    thread              *curr_thread = (thread *)ctxt;
    vmotion_client_t    *vm_client = (vmotion_client_t *)curr_thread->data();
    int                 client_sd;
    struct sockaddr_in  addr;
    int                 addr_len = sizeof(addr);
    string              server_ip_addr_str;
    uint32_t            server_port;

    SDK_THREAD_INIT(ctxt);

    // Create client socket
    if ((client_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        HAL_TRACE_ERR("client socket creation failed.");
        return NULL;
    }

    vm_client->client_sd = client_sd;

    // get server's IP and port
    ep_vmotion_get_server_info(vm_client->ctxt, &server_ip_addr_str, &server_port);


    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    if (ep_vmotion_is_platform_type_sim()) {
        addr.sin_port = htons(50055);
        addr.sin_addr.s_addr =  htonl(INADDR_ANY); // TODO: Have to get IP from ctxt which is EP info.
    } else {
        addr.sin_port = htons(server_port);
        addr.sin_addr.s_addr =  inet_addr(server_ip_addr_str.c_str());
    }

    HAL_TRACE_DEBUG("connecting to: {}:{}", addr.sin_addr.s_addr, addr.sin_port);

    // Connect to server socket
    if (connect(client_sd, (struct sockaddr *)&addr, addr_len) < 0) {
        HAL_TRACE_ERR("client connection to server failed.");
        return NULL;
    }

    ep_vmotion_client(vm_client);

    // client processing done ... close connection
    close(client_sd);

    return NULL;
}


} // namespace hal
