#include <assert.h>
#include <init.hpp>
#include <pkt.hpp>

static act_ret_t
svc_l4_pre_process (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

static act_ret_t
svc_ddos_lookup (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

static act_ret_t
svc_sfw_lookup (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

static act_ret_t
svc_nat_lookup (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

static act_ret_t
svc_l4lb_lookup (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

int
l4_init (void)
{
    int    rv;

    rv = svc_register(SVC_ID_DDOS, svc_l4_pre_process, svc_ddos_lookup);
    assert(rv == 0);
    rv = svc_register(SVC_ID_SFW, NULL, svc_sfw_lookup);
    assert(rv == 0);
    rv = svc_register(SVC_ID_NAT, NULL, svc_nat_lookup);
    assert(rv == 0);
    rv = svc_register(SVC_ID_L4LB, NULL, svc_l4lb_lookup);

    return rv;
}
