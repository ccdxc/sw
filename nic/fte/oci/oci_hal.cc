#include "nic/fte/fte.hpp"
#include "nic/fte/fte_impl.hpp"
#include "nic/p4/iris/include/defines.h"
#include "nic/include/pd_api.hpp"
#include "nic/include/hal.hpp"
#include "nic/include/hal_cfg.hpp"

namespace fte {
namespace impl {

void cfg_db_open()
{
}

void cfg_db_close()
{
}

hal::pd::cpupkt_ctxt_t *
cpupkt_ctxt_alloc_init(uint32_t qid)
{
    return NULL;
}

hal_ret_t cpupkt_poll_receive(hal::pd::cpupkt_ctxt_t *ctx,
                              cpu_rxhdr_t **cpu_rxhdr,
                              uint8_t **pkt, size_t *pkt_len)
{
    return HAL_RET_RETRY;
}


void process_pending_queues()
{
    
}

hal_ret_t cpupkt_send(hal::pd::cpupkt_ctxt_t *ctx,
                   uint32_t qid,
                   hal::pd::cpu_to_p4plus_header_t* cpu_header,
                   hal::pd::p4plus_to_p4_header_t* p4plus_header,
                   uint8_t* pkt, size_t pkt_len)
{
    return HAL_RET_ERR;
}


}
}

namespace hal {
namespace pd {

hal_ret_t hal_pd_call(pd_func_id_t, pd_func_args_t *) {
    return HAL_RET_ERR;
}
}
}
