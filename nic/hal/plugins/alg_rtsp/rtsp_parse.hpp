//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/base.h"
#include "nic/include/ip.h"

namespace hal {
namespace plugins {
namespace alg_rtsp {

#define RTSP_MESSAGE_ENTRIES(ENTRY)              \
    ENTRY(RTSP_MSG_REQUEST, 0, "Request")        \
    ENTRY(RTSP_MSG_RESPONSE, 1, "Response")

DEFINE_ENUM(rtsp_msg_type_t, RTSP_MESSAGE_ENTRIES)
#undef RTSP_MESSAGE_ENTRIES

#define RTSP_VER_ENTRIES(ENTRY)         \
    ENTRY(RTSP_V1, 0, "RTSP/1.0")       \
    ENTRY(RTSP_V2, 1, "RTSP/2.0")

DEFINE_ENUM(rtsp_ver_t, RTSP_VER_ENTRIES)
#undef RTSP_VER_ENTRIES

#define RTSP_MEHOTD_ENTRIES(ENTRY)                            \
    ENTRY(RTSP_METHOD_NONE, 0, "None")                        \
    ENTRY(RTSP_METHOD_SETUP, 1, "SETUP")                      \
    ENTRY(RTSP_METHOD_TEARDOWN, 2, "TEARDOWN")                \
    ENTRY(RTSP_METHOD_REDIRECT, 3, "REDIRECT")

DEFINE_ENUM(rtsp_method_t, RTSP_MEHOTD_ENTRIES)
#undef RTSP_MEHOTD_ENTRIES

#define RTSP_HDR_ENTRIES(ENTRY)                         \
    ENTRY(RTSP_HDR_NONE, 0, "None")                     \
    ENTRY(RTSP_HDR_CSEQ, 1, "CSeq")                     \
    ENTRY(RTSP_HDR_TRANSPORT, 2, "Transport")           \
    ENTRY(RTSP_HDR_SESSION, 3, "Session")               \
    ENTRY(RTSP_HDR_CONTENT_LENGTH, 4, "Content-Length")

DEFINE_ENUM(rtsp_hdr_type_t, RTSP_HDR_ENTRIES)
#undef RTSP_HDR_ENTRIES

#define RTSP_STATUS_OK 200
#define DEFAULT_SESSION_TIMEOUT 60
#define MAX_TRANSPORT_SPECS 8
typedef char rtsp_session_id_t[129];

struct rtsp_transport_t {
    ip_addr_t client_ip;
    uint16_t  client_port_start;
    uint16_t  client_port_end;
    ip_addr_t server_ip;
    uint16_t  server_port_start;
    uint16_t  server_port_end;
    uint16_t  ip_proto;
};

struct rtsp_hdrs_t {
    struct {
        uint8_t cseq:1;
        uint8_t transport:1;
        uint8_t session:1;
        uint8_t content_length:1;
    } valid;

    uint32_t cseq;
    struct {
        rtsp_session_id_t  id;
        uint32_t      timeout; //seconds
    } session;
    struct {
        uint8_t nspecs;
        rtsp_transport_t specs[MAX_TRANSPORT_SPECS];
    } transport;
    uint32_t content_length;
};

struct rtsp_msg_t {
    rtsp_msg_type_t type;
    rtsp_ver_t      ver;
    union {
        struct {
            rtsp_method_t method;
        } req;

        struct {
            uint32_t  status_code;
        } rsp;
    };
    rtsp_hdrs_t hdrs;
};

// spdlog formatter for flow_key_t
std::ostream& operator<<(std::ostream& os, const rtsp_msg_t& msg);

bool rtsp_parse_msg(const char *buf, uint32_t len, uint32_t *offset, rtsp_msg_t*msg);

} // alg_rtsp
} // plugins
} // hal

