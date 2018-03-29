#include "rtsp_parse.hpp"
#include "core.hpp"
#include "nic/hal/plugins/alg_utils/mime.hpp"

/*
 * RTSP (Real-Time Streaming Protocol) is an Application Layer protocol for controlling
 * the delivery of data with real-time properties. It is similar in syntax and operation
 * to HTTP/1.1. Unlike SIP and H.323, the purpose of RTSP is to access existing media
 * files over the network and to control the replay of the media. The typical
 * communication is between a client (running RealPlayer for example) and a streaming
 * media server. Commands include the ability to pause and play media files from the
 * remote server.
 *
 * RTSP is a control channel protocol between the media client and media server.
 * The data channel uses a different protocol, usually Real-Time Transport Protocol
 * (RTP) or RTP Control Protocol (RTCP).
 *
 * In RTSP standard mode, the client sets up three network channels with the RTSP
 * server when media data is delivered using RTP over UDP. RTSP runs over TCP.
 * RTP and RTCP run over UDP. The ports for RTP and RTCP packets
 * are dynamically negotiated by the client and server using RTSP.
 *
 * In RTSP interleave mode, media data can be made into packets using RTP or
 * RDT over TCP. In this scenario , a single full-duplex TCP connection is used for
 * both control and for media data delivery from the RTSP server to the client.
 * The data stream is interleaved with the RTSP control stream.
 *
 * v1  https://tools.ietf.org/html/rfc2326 
 * v2  https://tools.ietf.org/html/rfc7826 
 */


namespace hal {
namespace plugins {
namespace alg_rtsp {

//------------------------------------------------------------------------
// Given a buffer and length, advance poff to the next param and mark the
// current param.
//
// Returns true on success, false on failure.
//------------------------------------------------------------------------
inline bool
rtsp_next_param(const char* buf, uint32_t len, uint32_t* poff,
                char sep, uint32_t *paramoff, uint32_t* paramlen)
{
    uint32_t off = *poff;
    const char *paramend;

    if (off >= len) {
        return false;
    }
    paramend = (const char*)memchr(buf+off, sep, len-off);
    off = (paramend == NULL) ? len - 1 : paramend - buf;

    if (paramoff) {
        *paramoff = *poff;
    }
    if (paramlen) {
        *paramlen = off;        
    }

    *poff = off + 1;

    return true;
}

//------------------------------------------------------------------------
// Parse RTSP message start line
//
//   Request-Line = Method SP Request-URI SP RTSP-Version CRLF
//
//   Status-Line  = RTSP-Version SP Status-Code SP Reason-Phrase CRLF
//------------------------------------------------------------------------

inline bool
rtsp_parse_start_line(const char *buf, uint32_t len, rtsp_msg_t *msg)
{
    uint32_t off = 0;

    if (alg_utils::alg_mime_token_cmp(buf, len, &off, RTSP_V1_str(), ' ')) {
        msg->type = RTSP_MSG_RESPONSE;
        msg->ver = RTSP_V1;
        alg_utils::alg_mime_strtou32(buf, len, &off, &msg->rsp.status_code);
    } else if (alg_utils::alg_mime_token_cmp(buf, len, &off, RTSP_V2_str(), ' ')) {
        msg->type = RTSP_MSG_RESPONSE;
        msg->ver = RTSP_V2;
        alg_utils::alg_mime_strtou32(buf, len, &off, &msg->rsp.status_code);
    } else {
        msg->type =  RTSP_MSG_REQUEST;
        if (alg_utils::alg_mime_token_cmp(buf, len, &off, RTSP_METHOD_SETUP_str(), ' ')) {
            msg->req.method = RTSP_METHOD_SETUP;
        } else if (alg_utils::alg_mime_token_cmp(buf, len, &off, RTSP_METHOD_TEARDOWN_str(), ' ')) {
            msg->req.method = RTSP_METHOD_TEARDOWN;
        } else if (alg_utils::alg_mime_token_cmp(buf, len, &off, RTSP_METHOD_REDIRECT_str(), ' ')) {
            msg->req.method = RTSP_METHOD_REDIRECT;
        } else {
            return false;
        }

        // skip uri
        if (!rtsp_next_param(buf, len, &off, ' ', NULL, NULL)) {
            return false;
        }

        if (alg_utils::alg_mime_token_cmp(buf, len, &off, RTSP_V1_str(), ' ')) {
            msg->ver = RTSP_V1;
        } else if (alg_utils::alg_mime_token_cmp(buf, len, &off, RTSP_V2_str(), ' ')) {
            msg->ver = RTSP_V2;
        } else {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------
// Parse CSEQ header 
//
// CSEQ = "CSeq" ":" cseq-nr 
//------------------------------------------------------------------------
inline bool
rtsp_parse_cseq_header(const char *buf, uint32_t len, rtsp_msg_t*msg)
{
    uint32_t off = 0;

    if (!alg_utils::alg_mime_token_cmp(buf, len, &off, RTSP_HDR_CSEQ_str(), ':', true)) {
        return false;  
    }

    if(alg_utils::alg_mime_strtou32(buf, len, &off, &msg->hdrs.cseq) != 0) {
        msg->hdrs.valid.cseq = 1;
    }

    return true;
}

//------------------------------------------------------------------------
// Parse port range
// ------------------------------------------------------------------------
inline int
rtsp_parse_port_range(const char *buf, uint32_t len, uint32_t *poff,
                      uint16_t *loport, uint16_t *hiport)
{
    uint32_t off = *poff;

    if (!alg_utils::alg_mime_strtou16(buf, len, &off, loport))
        return false;

    if (buf[off] != '-') {
        *hiport = *loport;
    } else {
        off++;
        if (!alg_utils::alg_mime_strtou16(buf, len, &off, hiport))
            return false;

        // If we have a range, assume rtp:
        // loport must be even, hiport must be loport+1
        if ((*loport & 0x0001) != 0 || *hiport - *loport > 1) {
            *loport &= 0xfffe;
            *hiport = *loport+1;
        }
    }

    *poff = off;
    return true;
}

//------------------------------------------------------------------------
// Parse transport protocol
// transport-protocol/profile[/lower-transport]
//                            *parameter
//   transport-protocol  =    "RTP"
//   profile             =    "AVP"
//   lower-transport     =    "TCP" | "UDP"
//------------------------------------------------------------------------
inline bool
rtsp_parse_transport_proto(const char *buf, uint32_t len, uint32_t *poff, rtsp_transport_t* spec)
{
    uint32_t off = *poff;
 
    // skip proto
    if (!rtsp_next_param(buf, len, &off, '/', NULL, NULL)) {
        return false;
    }

    // skip profile
    if (!rtsp_next_param(buf, len, &off, '/', NULL, NULL)) {
        return false;
    }

    // lower-transport
    if (len-off >= 3 && strncasecmp(buf+off, "TCP", 3) == 0) {
        spec->ip_proto = IP_PROTO_TCP;
    } else {
        // default UDP
        spec->ip_proto = IP_PROTO_UDP;
    }

    *poff = off;
    return true;
}

//------------------------------------------------------------------------
// Parse host-port
//  host-port            = ( host [":" port] ) | ( ":" port )
//
// Example:
//    198.51.100.241:6256
//------------------------------------------------------------------------
inline bool
rtsp_parse_host_port(const char *buf, uint32_t len, uint32_t *poff, 
                     ip_addr_t *addr, uint16_t *port)
{
    uint32_t off = *poff;
    uint32_t paramoff;
    uint32_t paramlen;

    if (!rtsp_next_param(buf, len, &off, ':', &paramoff, &paramlen)) {
        return false;
    }

    // parse host
    if (paramlen > paramoff)
        alg_utils::alg_mime_strtoip(buf, paramlen, &paramoff, addr);  

    // parse port
    if (!alg_utils::alg_mime_strtou16(buf, len, &off, port))
        return false;

    *poff = off;
    return true;
}

//------------------------------------------------------------------------
// Parse quoted-address
//  quoted-addr          = DQUOTE (host-port) DQUOTE
//  host-port            = ( host [":" port] ) | ( ":" port )
//
// Example:
//    "198.51.100.241:6256"
//------------------------------------------------------------------------
inline bool
rtsp_parse_quoted_addr(const char *buf, uint32_t len, uint32_t *poff,
                       ip_addr_t *addr, uint16_t *port)
{
    uint32_t off = *poff;
    uint32_t paramoff;
    uint32_t paramlen;

    alg_utils::alg_mime_skipws(buf, len, &off);
    if (buf[off] != '"') {
        return false;
    }
    off++;

    if (!rtsp_next_param(buf, len, &off, '"', &paramoff, &paramlen)) {
        return false;
    }

    if(!rtsp_parse_host_port(buf, paramlen, &paramoff, addr, port)) {
        return false;
    }

    *poff = off;
    return true;
}

//------------------------------------------------------------------------
// Parse address list
//  addr-list            = quoted-addr *(SLASH quoted-addr)
//  quoted-addr          = DQUOTE (host-port) DQUOTE
//  host-port            = ( host [":" port] ) | ( ":" port )
//
// Example:
//    "198.51.100.241:6256"/"198.51.100.241:6257"
//------------------------------------------------------------------------
inline bool
rtsp_parse_addr_list(const char *buf, uint32_t len, uint32_t *poff,
                     ip_addr_t *addr, uint16_t *port1, uint16_t *port2)
{
    uint32_t off = *poff;


    if (!rtsp_parse_quoted_addr(buf, len, &off, addr, port1)) {
        return false;
    }

    alg_utils::alg_mime_skipws(buf, len, &off);

    if (buf[off] == '/') {
        off++;
        return rtsp_parse_quoted_addr(buf, len, &off, addr, port2);
    }

    return true;
}
// 
//------------------------------------------------------------------------
// Parse transport spec
//
//   transport-spec      =    transport-protocol/profile[/lower-transport]
//                            *parameter
//   transport-protocol  =    "RTP"
//   profile             =    "AVP"
//   lower-transport     =    "TCP" | "UDP"
//   parameter           =    ( "unicast" | "multicast" )
//                       |    ";" "destination" [ "=" address ]
//                       |    ";" "source" [ "=" address ]
//                       |    ";" "port" "=" port [ "-" port ]
//                       |    ";" "client_port" "=" port [ "-" port ]
//                       |    ";" "server_port" "=" port [ "-" port ]
//                       |    ";" "src_addr" "=" addr-list 
//                       |    ";" "dst_addr" "=" addr-list 
//  addr-list            = quoted-addr *(SLASH quoted-addr)
//  quoted-addr          = DQUOTE (host-port) DQUOTE
//  host-port            = ( host [":" port] ) | ( ":" port )
//
//   Example:
//     Transport: RTP/AVP;multicast;ttl=127;mode="PLAY",
//                RTP/AVP;unicast;client_port=3456-3457;mode="PLAY"
//
//------------------------------------------------------------------------
inline bool
rtsp_parse_transport_spec(const char *buf, uint32_t len, rtsp_transport_t* spec)
{
    uint32_t off = 0;

    *spec = {};

    uint32_t paramoff;
    uint32_t paramlen;

    while (rtsp_next_param(buf, len, &off, ';', &paramoff, &paramlen)) {
        if (paramoff == 0) {
            if (!rtsp_parse_transport_proto(buf, paramlen, &paramoff, spec)) {
                return false;
            }
        } else if (alg_utils::alg_mime_token_cmp(buf, paramlen, &paramoff, "source", '=')) {
            alg_utils::alg_mime_strtoip(buf, paramlen, &paramoff, &spec->server_ip);
        } else if (alg_utils::alg_mime_token_cmp(buf, paramlen, &paramoff, "destination", '=')) {
            alg_utils::alg_mime_strtoip(buf, paramlen, &paramoff, &spec->client_ip);
        } else if (alg_utils::alg_mime_token_cmp(buf, paramlen, &paramoff, "port", '=')) {
            rtsp_parse_port_range(buf, paramlen, &paramoff, &spec->client_port_start, &spec->client_port_end); 
        } else if (alg_utils::alg_mime_token_cmp(buf, paramlen, &paramoff, "client_port", '=')) {
            rtsp_parse_port_range(buf, paramlen, &paramoff, &spec->client_port_start, &spec->client_port_end); 
        } else if (alg_utils::alg_mime_token_cmp(buf, paramlen, &paramoff, "server_port", '=')) {
            rtsp_parse_port_range(buf, paramlen, &paramoff, &spec->server_port_start, &spec->server_port_end); 
        } else if (alg_utils::alg_mime_token_cmp(buf, paramlen, &paramoff, "dest_addr", '=')) {
            rtsp_parse_addr_list(buf, paramlen, &paramoff, &spec->client_ip,
                                 &spec->client_port_start, &spec->client_port_end);
        } else if (alg_utils::alg_mime_token_cmp(buf, paramlen, &paramoff, "src_addr", '=')) {
            rtsp_parse_addr_list(buf, paramlen, &paramoff, &spec->server_ip,
                                 &spec->server_port_start, &spec->server_port_end);
        } 

        alg_utils::alg_mime_skipws(buf, len, &off);
    }

    // return true if client or server port is specified
    return spec->client_port_start != 0 || spec->server_port_start != 0;
}

//------------------------------------------------------------------------
// Parse transport header
//
// Transport            =    "Transport" ":" transport-spec *("," transport-spec)
//
//   Example:
//     Transport: RTP/AVP;multicast;ttl=127;mode="PLAY",
//                RTP/AVP;unicast;client_port=3456-3457;mode="PLAY"
//
//------------------------------------------------------------------------
inline bool
rtsp_parse_transport_header(const char *buf, uint32_t len, rtsp_msg_t*msg)
{
    uint32_t off = 0;

    if (!alg_utils::alg_mime_token_cmp(buf, len, &off,
                                       RTSP_HDR_TRANSPORT_str(), ':', true)) {
        return false;  
    }

    while (off < len && msg->hdrs.transport.nspecs < MAX_TRANSPORT_SPECS) {
        const char *spec_end;
        uint32_t nextspec_off;

        spec_end = (const char*)memchr(buf+off, ',', len - off);
        nextspec_off = (spec_end == NULL) ? len : spec_end - buf + 1;
        if (rtsp_parse_transport_spec(buf+off, nextspec_off - off,
                                      &msg->hdrs.transport.specs[msg->hdrs.transport.nspecs])) {
            msg->hdrs.transport.nspecs++;
            msg->hdrs.valid.transport = 1;
        }

        off = nextspec_off;
        alg_utils::alg_mime_skipws(buf, len, &off);
    }

    return true;
}

//------------------------------------------------------------------------
// Parse session header 
//
// Session  = "Session" ":" session-id [ ";" "timeout" "=" delta-seconds ]
//------------------------------------------------------------------------
inline bool
rtsp_parse_session_header(const char *buf, uint32_t len, rtsp_msg_t*msg)
{
    uint32_t off = 0;
    uint32_t nbytes = 0;

    if (!alg_utils::alg_mime_token_cmp(buf, len, &off, RTSP_HDR_SESSION_str(), ':', true)) {
        return false;  
    }

    msg->hdrs.valid.session = 1;
    
    while(off < len) {
        if (buf[off] == ';' || buf[off] == '\r' ||  buf[off] == '\n') {
            break;
        }
        if (nbytes >= sizeof(msg->hdrs.session.id)) {
            return false;
        }

        msg->hdrs.session.id[nbytes] = buf[off];
        nbytes++;
        off++;
    }
    msg->hdrs.session.id[nbytes] = '\0';
    msg->hdrs.session.timeout = DEFAULT_SESSION_TIMEOUT;

    if (buf[off] != ';') {
        return true;
    }

    off++;
    if (alg_utils::alg_mime_token_cmp(buf, len, &off, "timeout", '=')) {
        alg_utils::alg_mime_strtou32(buf, len, &off, &msg->hdrs.session.timeout);
    }

    return true;
}

//------------------------------------------------------------------------
// Parse Content-Length header 
//
// Content-Length = "Content-Length" : length
//------------------------------------------------------------------------
inline bool
rtsp_parse_content_length_header(const char *buf, uint32_t len, rtsp_msg_t*msg)
{
    uint32_t off = 0;

    if (!alg_utils::alg_mime_token_cmp(buf, len, &off,
                                       RTSP_HDR_CONTENT_LENGTH_str(), ':', true)) {
        return false;  
    }

    if(alg_utils::alg_mime_strtou32(buf, len, &off, &msg->hdrs.content_length) != 0) {
        msg->hdrs.valid.content_length = 1;
    }

    return true;
}

//------------------------------------------------------------------------
// Parse an rtsp header field
//------------------------------------------------------------------------
inline bool
rtsp_parse_header(const char *buf, uint32_t len, rtsp_msg_t*msg)
{
    if (rtsp_parse_cseq_header(buf, len, msg)) {
        return true;
    }

    if (rtsp_parse_transport_header(buf, len, msg)) {
        return true;
    }

    if (rtsp_parse_session_header(buf, len, msg)) {
        return true;
    }

    if (rtsp_parse_content_length_header(buf, len, msg)) {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------
// rtsp_parse_msg
//
// RTSP messages are either requests from client to server or from
// server to client, and responses in the reverse direction.  Request
// and response messages use a format based on the generic message format
// of RFC 5322.  https://tools.ietf.org/html/rfc5322
// Both types of messages consist of a start-line, zero or more header fields
// (also known as "headers"), an empty line (i.e., a line with nothing preceding the
// CRLF) indicating the end of the headers, and possibly the data of the
// message body.  
//    generic-message = start-line
//        *(rtsp-header CRLF)
//        CRLF
//        [ message-body-data ]
//        start-line = Request-Line / Status-Line
//------------------------------------------------------------------------
bool
rtsp_parse_msg(const char *buf, uint32_t len, uint32_t *poff, rtsp_msg_t*msg)
{
    uint32_t lineoff;
    uint32_t linelen;
    bool valid_msg = false;

    while (!valid_msg && *poff < len) {
        
        *msg = {};

        // parse start line
        if (!alg_utils::alg_mime_nextline(buf, len, poff, &lineoff, &linelen, false)) {
            return false;
        }
        valid_msg = rtsp_parse_start_line(buf+lineoff, linelen, msg);

        // parse headers
        while (alg_utils::alg_mime_nextline(buf, len, poff, &lineoff, &linelen, true)) {
            if (linelen == 0) {
                break;
            }
            
            rtsp_parse_header(buf+lineoff, linelen, msg);
        }

        // skip the msg body
        if (msg->hdrs.valid.content_length) {
            *poff += msg->hdrs.valid.content_length;
            if (*poff > len) {
                *poff = len;
            }
        }

        if ((!msg->hdrs.valid.cseq) ||
            (msg->type == RTSP_MSG_RESPONSE && !msg->hdrs.valid.session)) {
            valid_msg = false;
        }
    }

    return valid_msg;
}

std::ostream& operator<<(std::ostream& os, const rtsp_msg_t& msg)
{
    char buf[400];
    fmt::ArrayWriter out(buf, sizeof(buf));

    out.write("{{type:{}, ver:{}", msg.type, msg.ver);
    if (msg.type == RTSP_MSG_REQUEST) {
        out.write(" method={}", msg.req.method);
    } else {
        out.write(", status:{}", msg.rsp.status_code);
    }

    if (msg.hdrs.valid.cseq) {
        out.write(", cseq:{}", msg.hdrs.cseq);
    }
    if (msg.hdrs.valid.transport) {
        out.write(", transport:[");
        for(int i =0 ; i < msg.hdrs.transport.nspecs; i++) {
            out.write("{{proto:{}, client:{}:{}-{}, server:{}:{}-{}}}",
                      msg.hdrs.transport.specs[i].ip_proto,
                      msg.hdrs.transport.specs[i].client_ip,
                      msg.hdrs.transport.specs[i].client_port_start,
                      msg.hdrs.transport.specs[i].client_port_end,
                      msg.hdrs.transport.specs[i].server_ip,
                      msg.hdrs.transport.specs[i].server_port_start,
                      msg.hdrs.transport.specs[i].server_port_end);
        }
        out.write("]");
    }
    if (msg.hdrs.valid.session) {
        out.write(", session:{{id:{}, timeout:{}}}", msg.hdrs.session.id, msg.hdrs.session.timeout);
    }
    if (msg.hdrs.valid.content_length) {
        out.write(", content-length:{}", msg.hdrs.content_length);
    }
    
    out.write("}}");


    buf[out.size()] = '\0';

    return os << buf;
}

} // alg_rtsp
} // plugins
} // hal
