#pragma once

#include "nic/include/base.h"
#include "nic/gen/proto/hal/types.pb.h"

namespace hal {
namespace tls {

extern bool proxy_tls_bypass_mode;

hal_ret_t tls_api_init();
hal_ret_t tls_api_init_flow(uint32_t enc_qid, uint32_t dec_qid);
hal_ret_t tls_api_start_handshake(uint32_t enc_qid, uint32_t dec_qid, bool is_v4_flow);
hal_ret_t tls_api_data_receive(uint32_t qid, uint8_t* data, size_t len);

} // namespace tls
} // namespace hal
