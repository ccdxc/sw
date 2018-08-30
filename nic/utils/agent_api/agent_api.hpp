#ifndef __AGENT_API_HPP__
#define __AGENT_API_HPP__

#include "nic/utils/agent_api/ipc.hpp"

// ipc_logger supports reporting fw events to the agent.
// Multiple instances can be created using the factory method after init.
// Each instance needs to be called from a single thread.
class ipc_logger {
public:
    // static methods for init-deinit and instantiate-destroy
    static void set_ipc_buf_sz(void);
    static void set_ipc_instances(int);
    static int init(void);
    static void deinit(void);
    static ipc_logger *factory(void);
    // destroy is not supported. deinit can be used to free all instances.
    uint8_t *get_buffer (int size);
    int write_buffer (uint8_t *buf, int size);

private:
    // ipc_logger private state
    ipc_logger (void) {};
    ipc *pipe_;
};

#endif
