#include <pkt.hpp>

//------------------------------------------------------------------------------
// init function for service infra
//------------------------------------------------------------------------------
int
svc_init (void)
{
    action_init(svc_flow_lookup_act, svc_flow_setup_act, svc_fast_path_act);
    return 0;
}

//------------------------------------------------------------------------------
// register a given service with provided action functions
//------------------------------------------------------------------------------
int
svc_register (svc_id_t svc_id, action_func pre_process, action_func policy_lookup)
{
    if (svc_id >= SVC_ID_MAX) {
        return -1;
    }

    g_pkt_actions.pre_process_act[svc_id] = pre_process;
    g_pkt_actions.policy_lookup_act[svc_id] = policy_lookup;
    return 0;
}
