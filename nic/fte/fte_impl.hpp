
#include "nic/include/cpupkt_api.hpp"

namespace fte {
namespace impl {

void cfg_db_open();
void cfg_db_close();

hal::pd::cpupkt_ctxt_t *
cpupkt_ctxt_alloc_init(uint32_t qid);

hal_ret_t cpupkt_poll_receive(hal::pd::cpupkt_ctxt_t *ctx,
                              fte::cpu_rxhdr_t **cpu_rxhdr,
                              uint8_t **pkt, size_t *pkt_len);


hal_ret_t cpupkt_send(hal::pd::cpupkt_ctxt_t *ctx,
                      uint32_t qid,
                      hal::pd::cpu_to_p4plus_header_t* cpu_header,
                      hal::pd::p4plus_to_p4_header_t* p4plus_header,
                      uint8_t* pkt, size_t pkt_len);

void process_pending_queues();

}

}

