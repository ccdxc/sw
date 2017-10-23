#include <unistd.h>
#include <string>
#include <sstream>
#include <ostream>
#include <iomanip>
#include "nic/hal/hal.hpp"
#include "nic/hal/lkl/lklshim.hpp"
#include "nic/hal/lkl/lklshim_tls.hpp"
#include "nic/include/cpupkt_headers.hpp"

extern "C" {
#include "nic/third-party/lkl/export/include/lkl.h"
#include "nic/third-party/lkl/export/include/lkl_host.h"
}


namespace hal {
// byte array to hex string for logging
std::string hex_dump(const uint8_t *buf, size_t sz)
{
    std::ostringstream result;

    for(size_t i = 0; i < sz; i+=8) {
        result << " 0x";
        for (size_t j = i ; j < sz && j < i+8; j++) {
            result << std::setw(2) << std::setfill('0') << std::hex << (int)buf[j];
        }
    }

    return result.str();
}


bool 
lklshim_release_client_syn(uint16_t qid) 
{
    lklshim_flow_t *flow = lklshim_flow_by_qid[qid];

    
    if (flow == NULL) {
      HAL_TRACE_ERR("lklshim: flow does't exist to release client syn for qid = {}", qid);
      return false;
    }
    
    HAL_TRACE_DEBUG("lklshim: trying to release client syn for qid = {}", qid);

    if (flow->hostns.skbuff != NULL) {
        void *pkt_skb = flow->hostns.skbuff;
        ether_header_t *eth = (ether_header_t*)lkl_get_mac_start(pkt_skb);
        ipv4_header_t *ip = (ipv4_header_t*)lkl_get_network_start(pkt_skb);
        tcp_header_t *tcp = (tcp_header_t*)lkl_get_transport_start(pkt_skb);

        HAL_TRACE_DEBUG("lklshim: flow miss rx eth={}", hex_dump((uint8_t*)eth, 18));
        HAL_TRACE_DEBUG("lklshim: flow miss rx ip={}", hex_dump((uint8_t*)ip, sizeof(ipv4_header_t)));
        HAL_TRACE_DEBUG("lklshim: flow miss rx tcp={}", hex_dump((uint8_t*)tcp, sizeof(tcp_header_t)));
        
        lkl_tcp_v4_rcv(pkt_skb);
    }


    return true;
}

} // namespace hal


