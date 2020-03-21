//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/vmotion/vmotion.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/core/core.hpp"

namespace hal {

vmotion_dst_host_fsm_def* vmotion::dst_host_fsm_def_ = vmotion_dst_host_fsm_def::factory();
vmotion_src_host_fsm_def* vmotion::src_host_fsm_def_ = vmotion_src_host_fsm_def::factory();

hal_ret_t
vmotion_init (int vmotion_port)
{
    HAL_TRACE_DEBUG("vmotion init port:{}", vmotion_port);
    vmotion *vm =
        vmotion::factory(VMOTION_MAX_THREADS, (vmotion_port ? vmotion_port : VMOTION_PORT));

    g_hal_state->set_vmotion(vm);

    return HAL_RET_OK;
}
//-----------------------------------------------------------------------------
// spawn vmotion thread
//-----------------------------------------------------------------------------
vmotion *
vmotion::factory(uint32_t max_threads, uint32_t port)
{
    vmotion     *vmn;
    void        *mem = HAL_CALLOC(HAL_MEM_ALLOC_VMOTION, sizeof(vmotion));

    if (!mem) {
        HAL_TRACE_ERR("OOM failed to allocate memory for vmotion");
        return NULL;
    }

    vmn = new (mem) vmotion();
    vmn->init(max_threads, port);

    return vmn;
}

hal_ret_t
vmotion::init(uint32_t max_threads, uint32_t vmotion_port)
{
    vmotion_.max_threads     = max_threads;
    vmotion_.port            = vmotion_port;

    vmotion_.threads_idxr    = rte_indexer::factory(max_threads, false, false);
    SDK_ASSERT(vmotion_.threads_idxr != NULL);

    // spawn master server thread
    vmotion_.vmotion_master =
        sdk::event_thread::event_thread::factory("vmn-mas", HAL_THREAD_ID_VMOTION,
                                                 sdk::lib::THREAD_ROLE_CONTROL,
                                                 0x0,    // use all control cores
                                                 master_thread_init,  // Thread Init Fn
                                                 NULL,  // Thread Exit Fn
                                                 NULL,  // Thread Event CB
                                                 sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                                                 sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                                                 NULL);

    // Start the master thread
    vmotion_.vmotion_master->start(this);

    return HAL_RET_OK;
}

hal_ret_t
vmotion::alloc_thread_id(uint32_t *tid)
{
    hal_ret_t ret = HAL_RET_OK;

    // allocate tid
    VMOTION_WLOCK
    if (vmotion_.threads_idxr->alloc(tid) != SDK_RET_OK) {
        HAL_TRACE_ERR("unable to allocate thread id");
        ret = HAL_RET_ERR;
    }
    VMOTION_WUNLOCK

    (*tid) += HAL_THREAD_ID_VMOTION_THREADS_MIN;

    return ret;
}

hal_ret_t
vmotion::release_thread_id(uint32_t tid)
{
    hal_ret_t ret = HAL_RET_OK;

    tid -= HAL_THREAD_ID_VMOTION_THREADS_MIN;

    // free tid
    VMOTION_WLOCK
    if (vmotion_.threads_idxr->free(tid) != SDK_RET_OK) {
        HAL_TRACE_ERR("unable to free thread id: {}", tid);
        ret = HAL_RET_ERR;
    }
    VMOTION_WUNLOCK

    return ret;
}

void
vmotion::master_thread_init(void *ctxt)
{
    vmotion       *vmn = (vmotion *)ctxt;
    struct sockaddr_in address;
    int                master_socket;
    int                opt = true;

    // create master socket
    master_socket = socket(AF_INET , SOCK_STREAM , 0);

    // set master socket to allow multiple connections
    setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    // configure address settings
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(vmn->get_vmotion_port());

    // bind socket to localhost:port
    bind(master_socket, (struct sockaddr *) &address, sizeof(address));

    // listen
    if (listen(master_socket, vmn->get_max_threads()) != 0) {
        // close socket
        HAL_TRACE_ERR("vmotion: Master thread listen failed");
        close(master_socket);
        return;
    }

    vmn->get_master_thread_io()->ctx = ctxt;

    // initialize and start a watcher to accept client requests
    sdk::event_thread::io_init(vmn->get_master_thread_io(), master_thread_cb,
                               master_socket, EVENT_READ);
    sdk::event_thread::io_start(vmn->get_master_thread_io());
}

// call back when a new client is trying to connect.
// - create a new thread.
// - pass the thread object and fd to the new thread.
void
vmotion::master_thread_cb(sdk::event_thread::io_t *io, int sock_fd, int events)
{
    vmotion  *vmn = (vmotion *)io->ctx;

    if (EV_ERROR & events) {
        HAL_TRACE_ERR("invalid event.");
        return;
    }
    vmn->spawn_src_host_thread(sock_fd);
}

void
vmotion::run_vmotion(ep_t *ep, vmotion_thread_evt_t event)
{
    vmotion_ep *vmn_ep = get_vmotion_ep(ep);

    if ((!vmn_ep) && (event == VMOTION_EVT_EP_MV_ABORT)) {
        return;
    }

    HAL_TRACE_INFO("vMotion: vmn_ep:{:p} event:{}", (void *)vmn_ep, event);

    if (event == VMOTION_EVT_EP_MV_COLD) {
        // vMotion is happening from a non-pensando device to locally here.
        // We do - Disable TCP Normalization for the EP that migrated
        endpoint_migration_normalization_cfg(ep, true);
        return;
    }

    if (event == VMOTION_EVT_EP_MV_START) {
        if (!vmn_ep) {
            vmn_ep = create_vmotion_ep(ep, VMOTION_TYPE_MIGRATE_IN);
        } else {
            HAL_TRACE_ERR("vmotion ep already exists");
        }

        // In the destination host, when vMotion is started, delete the "host" entry of the 
        // EP in the Input Properties MAC VLAN table. This entry is used to avoid
        // double count of a packet for statistics, when the packet is arriving back in the
        // U-Turn traffic. skip_flow_update will be set in this entry.
        //  For vMotion, this can create problem for scenario where a Local to Remote session
        //  is getting converted to Local to Local session. So in the middle of the vMotion,
        //  it is a valid scenario, traffic (with Src MAC as EP MAC) (for the migrating EP)
        //  could be received in the uplink, this Input Mac Vlan table entry will do skip_flow_update
        //  for that traffic, and eventually that traffic could get aged out.
        //
        //  So, in the start of the vMotion, temporarily delete that entry and when vMotion is over
        //  add this entry back. 
        endpoint_migration_inp_mac_vlan_pgm(ep, false);
        if (vmn_ep) {
            VMOTION_FLAG_SET_INP_MAC_REMOVED(vmn_ep);
        }
    }

    if (vmn_ep) {
        vmotion_thread_evt_t *evt =
            (vmotion_thread_evt_t *)HAL_CALLOC(HAL_MEM_ALLOC_VMOTION, sizeof(vmotion_thread_evt_t));
        *evt = event;

        sdk::event_thread::message_send(vmn_ep->get_thread_id(), (void *)evt);
    }
}

vmotion_ep *
vmotion::get_vmotion_ep(ep_t *ep)
{
    auto it = std::find_if(vmn_eps_.begin(), vmn_eps_.end(), [ep](vmotion_ep *e) {
            return (e->get_ep() == ep); });
    return (it == vmn_eps_.end() ? NULL : *it);
}

vmotion_ep *
vmotion::create_vmotion_ep(ep_t *ep, ep_vmotion_type_t type)
{
    auto vmn_ep = vmotion_ep::factory(ep, type);

    vmn_eps_.push_back(vmn_ep);

    return vmn_ep;
}

hal_ret_t
vmotion::delete_vmotion_ep(vmotion_ep *vmn_ep)
{
    auto it = std::find(vmn_eps_.begin(), vmn_eps_.end(), vmn_ep);
    if (it == vmn_eps_.end()) {
        HAL_TRACE_ERR("vmotion: EP Missing in EP list");
        return HAL_RET_ERR;
    }
    // Delete the from the EP List
    vmn_eps_.erase(it);
    // Destroy EP
    vmotion_ep::destroy(vmn_ep);

    return HAL_RET_OK;
}

hal_ret_t
vmotion::vmotion_handle_ep_del(ep_t *ep)
{
    if (!VMOTION_IS_ENABLED()) {
        return HAL_RET_OK;
    }

    if (ep->vmotion_state == MigrationState::IN_PROGRESS) {
        // Loop the sessions, and start aging timer
        endpoint_migration_session_age_reset(ep);
    }

    run_vmotion(ep, VMOTION_EVT_EP_MV_ABORT);

    return HAL_RET_OK;
}

vmotion_ep *
vmotion_ep::factory(ep_t *ep, ep_vmotion_type_t type)
{
    vmotion_ep *mem = (vmotion_ep *)g_hal_state->vmotion_ep_slab()->alloc();
    if (!mem) {
        HAL_TRACE_ERR("OOM failed to allocate memory for vmotion ep");
        return NULL;
    }
    vmotion_ep* vmn_ep = new (mem) vmotion_ep();

    if (vmn_ep->init(ep, type) != HAL_RET_OK) {
        hal::delay_delete_to_slab(HAL_SLAB_VMOTION_EP, mem);
        return NULL;
    }
    return vmn_ep;
}

void
vmotion_ep::destroy(vmotion_ep *vmn_ep)
{
    // Free the state machine
    delete vmn_ep->get_sm();
    // Free the memory
    hal::delay_delete_to_slab(HAL_SLAB_VMOTION_EP, vmn_ep);
}

hal_ret_t
vmotion_ep::init(ep_t *ep, ep_vmotion_type_t type)
{
    if ((type != VMOTION_TYPE_MIGRATE_IN) && (type != VMOTION_TYPE_MIGRATE_OUT)) {
        return HAL_RET_ERR;
    }

    vmotion_ptr_        = g_hal_state->get_vmotion();
    vmotion_type_       = type;
    ep_hdl_             = ep->hal_handle;
    old_homing_host_ip_ = ep->old_homing_host_ip;

    ep->vmotion_type = type;

    if (type == VMOTION_TYPE_MIGRATE_IN) {
        sm_ = new fsm_state_machine_t(get_vmotion()->get_dst_host_fsm_def_func,
                                      STATE_DST_HOST_INIT,
                                      STATE_DST_HOST_END,
                                      STATE_DST_HOST_END, // TODO STATE_DST_HOST_TIMEOUT,
                                      STATE_DST_HOST_END, // TODO STATE_DST_HOST_REMOVE,
                                      (fsm_state_ctx)this,
                                      NULL);  // TODO get_timer_func);

        spawn_dst_host_thread();
    } else {
        sm_ = new fsm_state_machine_t(get_vmotion()->get_src_host_fsm_def_func,
                                      STATE_SRC_HOST_INIT,
                                      STATE_SRC_HOST_END,
                                      STATE_SRC_HOST_END, // TODO STATE_DST_HOST_TIMEOUT,
                                      STATE_SRC_HOST_END, // TODO STATE_DST_HOST_REMOVE,
                                      (fsm_state_ctx)this,
                                      NULL);  // TODO get_timer_func);
    }
    return HAL_RET_OK;
}

// This function returns TRUE, if packet will be handled by vMotion.
bool
vmotion::process_rarp(mac_addr_t mac)
{
    if (!VMOTION_IS_ENABLED()) {
        HAL_TRACE_VERBOSE("Ignore vMotion event. vMotion not enabled");
        return false;
    }

    auto  ep = find_ep_by_mac(mac);
    if ((!ep) || (ep->vmotion_state != MigrationState::IN_PROGRESS)) {
        HAL_TRACE_VERBOSE("Ignoring RARP Packet. EP {} ptr {:p} vmotion state {}",
                          macaddr2str(mac), (void *)ep, (ep ? ep->vmotion_state : 0));
        return false;
    }

    run_vmotion(ep, VMOTION_EVT_RARP_RCVD);

    return true;
}

void
vmotion_ep::set_last_sync_time() {
    timespec_t  ctime;

    // get current time
    clock_gettime(CLOCK_MONOTONIC, &ctime);

    // Set the time
    sdk::timestamp_to_nsecs(&ctime, &last_sync_time_);
}

} // namespace hal
