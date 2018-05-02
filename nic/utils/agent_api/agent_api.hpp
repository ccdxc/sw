#include "nic/include/fte_ctx.hpp"
#include "nic/utils/agent_api/ipc.hpp"

// ipc_logger supports reporting fw events to the agent.
// Multiple instances can be created using the factory method after init.
// Each instance needs to be called from a single thread.
class ipc_logger {
public:
    // static methods for init-deinit and instantiate-destroy
    static int init(void);
    static void deinit(void);
    static ipc_logger *factory(void);
    // destroy is not supported. deinit can be used to free all instances.

    // per instance member functions
    void firew_log(fte::ctx_t*, int);

private:
    // ipc_logger private state
    ipc_logger(void){};
    ipc *pipe_;
};
