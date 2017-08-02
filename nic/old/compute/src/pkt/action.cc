#include <pkt.hpp>

action_list_t g_pkt_actions;

//------------------------------------------------------------------------------
// action init function called when a compute core comes up with global actions
//------------------------------------------------------------------------------
int
action_init (action_func flow_lookup, action_func flow_setup,
             action_func fast_path)
{
    g_pkt_actions.flow_lookup_act = flow_lookup;
    g_pkt_actions.flow_setup_act = flow_setup;
    g_pkt_actions.fast_path_act = fast_path;

    return 0;
}

//------------------------------------------------------------------------------
// top level packet processing function that processes all packets coming to
// this compute NPU
//
// this function polls for packets queued to this NPU, and for each packet that
// it receives, it goes through the aciton functions in the order
//------------------------------------------------------------------------------
int
process_packet (uint8_t *pkt)
{
    return 0;
}

