#include "nic/utils/agent_api/agent_api.hpp"
#include "nic/utils/agent_api/shared_constants.h"
#include "nic/gen/proto/hal/fwlog.pb.h"
#define IPv4_LEN 4
#define IPv6_LEN 16
#define L4PORT_LEN 2
#define FW_LOG_SIZE 48

// init sets up shared memory for use by the ipc logger
int ipc_logger::init(void)
{
    return ipc::setup_shm(IPC_MEM_SIZE, IPC_INSTANCES);
}

// deinit frees up the shared memory
void ipc_logger::deinit(void)
{
    ipc::tear_down_shm();
}

// factory creates a new instance of ipc_logger
ipc_logger *ipc_logger::factory(void)
{
    ipc *ipc_inst;

    ipc_inst = ipc::factory();
    if (ipc_inst == NULL) {
        return NULL;
    }

    ipc_logger *il = new(ipc_logger);
    il->pipe_ = ipc_inst;

    return il;
}

// firew_log reports a firewall event to the agent
void ipc_logger::firew_log(fte::ctx_t *ctx, int action)
{
    uint8_t *buf = pipe_->get_buffer(FW_LOG_SIZE);
    if (buf == NULL) {
        return;
    }

    hal::flow_key_t fk = ctx->key();
    if (fk.flow_type != hal::FLOW_TYPE_V4) {
        return; // only ipv4 for now.
    }

    fwlog::FWEvent ev;

    ev.set_sipv4(fk.sip.v4_addr);
    ev.set_dipv4(fk.dip.v4_addr);
    ev.set_sport(fk.sport);
    ev.set_dport(fk.dport);
    ev.set_ipprot(fk.proto);
    ev.set_direction(fk.dir);
    ev.set_action(action);
    if (!ev.SerializeToArray(buf, FW_LOG_SIZE)) {
        return;
    }

    int size = ev.ByteSizeLong();
    pipe_->put_buffer(buf, size);
}
