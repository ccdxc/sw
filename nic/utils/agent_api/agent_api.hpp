#include "nic/utils/agent_api/ipc.hpp"
#include "nic/gen/proto/hal/fwlog.pb.h"

enum ipc_log_type_t {
    IPC_LOG_TYPE_FW  = 0,
    IPC_LOG_TYPE_MAX = 1,
}; 

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

    // per instance member functions
    void fw_log(fwlog::FWEvent ev);

private:
    // ipc_logger private state
    ipc_logger (void) {};
    ipc *pipe_;
};
