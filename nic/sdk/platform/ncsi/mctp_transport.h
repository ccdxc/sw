/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __MCTP_TRANSPORT_H__
#define __MCTP_TRANSPORT_H__

#include <string>

namespace sdk {
namespace platform {
namespace ncsi {

class mctp_transport : public transport {

public:
    int Init() { printf("In MCTO transport Init() API\n"); return 0; };
    ssize_t SendPkt(const void *buf, size_t len) { return 0; };
    ssize_t RecvPkt(void *buf, size_t len) { return 0; };
};

} // namespace ncsi
} // namespace platform
} // namespace sdk

#endif //__MCTP_TRANSPORT_H__

