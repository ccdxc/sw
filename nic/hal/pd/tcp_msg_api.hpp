#pragma once

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/cpupkt_headers.hpp"

namespace hal {
namespace pd {

#define TCP_ACTL_MSG_TYPE_PKT          0
#define TCP_ACTL_MSG_TYPE_CB_CLEANUP   1

#define TCP_MSGS_MAX_BATCH_SIZE 64
typedef struct tcp_msg_info_s {
    uint8_t msg_type;
    union {
        struct {
            p4_to_p4plus_cpu_pkt_t *cpu_rxhdr;
            uint8_t                *pkt;
            size_t                 pkt_len;
            bool                   copied_pkt;
        } pkt;
        // Add struct for other types of msgs here
   } u;
} __PACK__ tcp_msg_info_t;

typedef struct tcp_msg_batch_s {
   tcp_msg_info_t   msg_info[TCP_MSGS_MAX_BATCH_SIZE];
   uint16_t         msg_cnt;
} __PACK__ tcp_msg_batch_t;


}
}
