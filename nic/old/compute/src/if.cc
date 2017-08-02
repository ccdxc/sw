#include <init.hpp>
#include <pkt.hpp>

static act_ret_t
svc_if_pre_process (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

static act_ret_t
svc_if_lookup (pkt_meta_t *pkt_meta, uint8_t *pkt)
{
    return ACT_RET_OK;
}

int
if_init (void)
{
    return svc_register(SVC_ID_IF, svc_if_pre_process, svc_if_lookup);
}
