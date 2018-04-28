// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_MSG_SOCK_UTILS_H_
#define _DELPHI_MSG_SOCK_UTILS_H_

// socket utility functions

#include <map>
#include <ev++.h>

#include "transport.hpp"

namespace delphi {
namespace messanger {

// TCP utility functions implemented in tcp_utils.cc
error sendMsg(int sock, MessagePtr msg);
int readHdr(char *buf, google::protobuf::uint32 *size);
MessagePtr readBody(int csock, int haederSize, google::protobuf::uint32 siz);

} // namespace messanger
} // namespace delphi

#endif // _DELPHI_MSG_SOCK_UTILS_H_
