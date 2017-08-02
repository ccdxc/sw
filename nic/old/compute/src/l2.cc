#include <init.hpp>
#include <pkt.hpp>

static act_ret_t
svc_l2_pre_process (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

static act_ret_t
svc_l2_lookup (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

int
l2_init (void)
{
    return svc_register(SVC_ID_L2, svc_l2_pre_process, svc_l2_lookup);
}
