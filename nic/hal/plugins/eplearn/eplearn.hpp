#ifndef HAL_PLUGINS_EPLEARN_EPLEARN_HPP_
#define HAL_PLUGINS_EPLEARN_EPLEARN_HPP_
#include "common/trans.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_trans.hpp"
#include "nic/hal/plugins/eplearn/arp/arp_trans.hpp"

namespace hal {
namespace eplearn {

#define EPLEARN_TRANS_CTX_MAX 2

const std::string FTE_FEATURE_EP_LEARN("pensando.io/eplearn:eplearn");

typedef struct eplearn_trans_ctx_s {
    trans_t       *trans; /* Transaction that has to be processed */
    union {
        dhcp_event_data  dhcp_data;
        arp_event_data_t arp_data;
    };
} eplearn_trans_ctx_t;

typedef struct eplearn_info_s {
    eplearn_trans_ctx_t trans_ctx[EPLEARN_TRANS_CTX_MAX];
    uint32_t            trans_ctx_cnt;
} eplearn_info_t;


}
}


#endif /* HAL_PLUGINS_EPLEARN_EPLEARN_HPP_ */
