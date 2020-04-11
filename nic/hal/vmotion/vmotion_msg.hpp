#ifndef __VMOTION_MSG_HPP__
#define __VMOTION_MSG_HPP__

#include "gen/proto/vmotion.pb.h"
#include "nic/include/base.hpp"
#include "nic/hal/vmotion/vmotion.hpp"
#include "nic/sdk/include/sdk/eth.hpp"

namespace hal {

using vmotion_msg::VmotionMessage;

hal_ret_t vmotion_send_msg(VmotionMessage& msg, SSL *ssl);
hal_ret_t vmotion_recv_msg(VmotionMessage& msg, SSL *ssl);

} // namespace hal

#endif    // __VMOTION_MSG_HPP__

