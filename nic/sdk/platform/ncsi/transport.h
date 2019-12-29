/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <string>

namespace sdk {
namespace platform {
namespace ncsi {


class transport {

public:
    virtual int Init() = 0;
    virtual ssize_t SendPkt(const void *buf, size_t len) = 0;
    virtual ssize_t RecvPkt(void *buf, size_t len) = 0;
};

} // namespace ncsi
} // namespace platform
} // namespace sdk

#endif //__TRANSPORT_H__

