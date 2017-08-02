#include <init.hpp>
#include <pkt.hpp>

static act_ret_t
svc_l3_pre_process (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

static act_ret_t
svc_l3_lookup (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

int
l3_init (void)
{
    return svc_register(SVC_ID_L3, svc_l3_pre_process, svc_l3_lookup);
}
