#ifndef __SVC_HPP__
#define __SVC_HPP__

#include <action.hpp>

//------------------------------------------------------------------------------
// list of all supported services
// NOTE: when a new service is introduced insert it in the right order in the
// following list
//------------------------------------------------------------------------------
typedef enum svc_id_e {
    SVC_ID_IF,
    SVC_ID_L2,
    SVC_ID_L3,
    SVC_ID_DDOS,
    SVC_ID_SFW,
    SVC_ID_NAT,
    SVC_ID_L4LB,
    SVC_ID_L7,      // TODO: expand this once L7 services are finalized
    SVC_ID_MAX,
} svc_id_t;

//------------------------------------------------------------------------------
// every service must register with the infra using the following API after it
// finished its initialization
//------------------------------------------------------------------------------
int svc_register(svc_id_t svc, action_func pre_process,
                 action_func policy_lookup);

//------------------------------------------------------------------------------
// init function for service infra
//------------------------------------------------------------------------------
extern int svc_init(void);
extern act_ret_t svc_flow_lookup_act(pkt_meta_t *pctxt, uint8_t *pkt);
extern act_ret_t svc_flow_setup_act(pkt_meta_t *pctxt, uint8_t *pkt);
extern act_ret_t svc_fast_path_act(pkt_meta_t *pctxt, uint8_t *pkt);

#endif    // __SVC_HPP__

