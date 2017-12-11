/*
 * core.hpp
 */
#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace plugins {
namespace alg_sip {

/* 
 * Defines
 */
#define SIP_PORT    5060

/*
 * Constants
 */
const std::string FTE_FEATURE_ALG_SIP("pensando.io/alg_sip:alg_sip");

/*
 * SIP header handlers
 */
typedef struct sip_header_s {
    const char  *name;      // Name of the header
    uint32_t    len;        // Length of the header name
    const char  *cname;     // Compact form of header name
    uint32_t    clen;       // Length of compact header name
    const char  *search;    // Search string for start
    uint32_t    slen;       // Length of search string
    int         (*match_len)(fte::ctx_t &ctx, const char *buf,
                             const char *limit, int *shift);
                            // Match length callback for each header
} sip_header_t;

#define __SIP_HDR(__name, __cname, __search, __match)       \
{                                                           \
    .name       = (__name),                                 \
    .len        = sizeof(__name) - 1,                       \
    .cname      = (__cname),                                \
    .clen       = (__cname) ? sizeof(__cname) - 1 : 0,      \
    .search     = (__search),                               \
    .slen       = (__search) ? sizeof(__search) - 1 : 0,    \
    .match_len  = (__match),                                \
}

/*
 * Function prototypes
 */

// plugin.cc
fte::pipeline_action_t alg_sip_exec(fte::ctx_t &ctx);

// utils.cc
int sip_in4_pton(const char *src, int srclen, uint8_t *dst,
                 int delim, const char **end);
int sip_in6_pton(const char *src, int srclen, uint8_t *dst,
                 int delim, const char **end);
int sip_digits_len(fte::ctx_t &ctx, const char *buf, const char *limit,
                   int *shift);
int sip_callid_len(fte::ctx_t &ctx, const char *buf, const char *limit,
                   int *shift);
int sip_media_len(fte::ctx_t &ctx, const char *buf, const char *limit,
                  int *shift);
int sip_epaddr_len(fte::ctx_t &ctx, const char *buf, const char *limit,
                   int *shift);
int sip_skp_epaddr_len(fte::ctx_t &ctx, const char *buf, const char *limit,
                       int *shift);
int sip_parse_addr(fte::ctx_t &ctx, const char *cp,
                   const char **endp, ip_addr_t *addr,
                   const char *limit, bool delim);
const char * sip_skip_whitespace(const char *buf, const char *limit);
const char* sip_follow_continuation(const char *buf, const char *limit);

// headers.cc
int sip_parse_address_param(fte::ctx_t &ctx, const char *buf,
                            uint32_t dataoff, uint32_t datalen,
                            const char *name,
                            uint32_t *matchoff, uint32_t *matchlen,
                            ip_addr_t *addr, bool delim);
int sip_parse_numerical_param(fte::ctx_t &ctx, const char *buf,
                              uint32_t dataoff, uint32_t datalen,
                              const char *name,
                              uint32_t *matchoff, uint32_t *matchlen,
                              uint32_t *val);
int sip_parse_transport(fte::ctx_t &ctx, const char *buf,
                        uint32_t dataoff, uint32_t datalen,
                        uint8_t *proto);

// sdp.cc
int process_sdp(fte::ctx_t &ctx, uint32_t protoff, uint32_t dataoff,
                const char **buf, uint32_t *datalen, uint32_t cseq);

}  // namespace alg_sip 
}  // namespace plugins
}  // namespace hal
