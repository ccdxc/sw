//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// SDP functionality
//-----------------------------------------------------------------------------

#include "core.hpp"

namespace hal {
namespace plugins {
namespace alg_sip {

/*
 * sdp_parse_addr
 */
static int sdp_parse_addr(fte::ctx_t &ctx, const char *cp,
                          const char **endp, ip_addr_t *addr,
                          const char *limit)
{
    const char *end;
    int ret;
    int af = AF_INET; // KB -- Fix me, get info from packet

    memset(addr, 0, sizeof(*addr));
    switch (af) {
    case AF_INET:
        ret = sip_in4_pton(cp, limit-cp, (uint8_t *)&addr->addr.v4_addr,
                           -1, &end);
        break;
    case AF_INET6:
        ret = sip_in6_pton(cp, limit-cp, (uint8_t *)&addr->addr.v6_addr,
                           -1, &end);
        break;
    default:
        break;
    }

    if (ret == 0)
        return 0;

    if (endp)
        *endp = end;
    return 1;
}

/*
 * sdp_addr_len
 *
 * Skip ip address. returns its length
 */
static int sdp_addr_len(fte::ctx_t &ctx, const char *buf,
                        const char *limit, int *shift)
{
    ip_addr_t addr;
    const char *aux = buf;

    if (!sdp_parse_addr(ctx, buf, &buf, &addr, limit)) {
        return 0;
    }

    return buf - aux;
}

/*
 * SDP header types
 */
typedef enum sdp_header_types {
    SDP_HDR_UNSPEC,
    SDP_HDR_VERSION,
    SDP_HDR_OWNER,
    SDP_HDR_CONNECTION,
    SDP_HDR_MEDIA,
} sdp_header_types_t;

#define SDP_HDR(__name, __search, __match)      \
    __SIP_HDR(__name, NULL, __search, __match)

/*
 * SDP header parsing: a SDP session description contains an ordered
 * set of headers, starting with a section containing general session
 * parameters, optionally followed by multiple media descriptions.
 *
 * SDP headers always start at the beginning of a line. According to
 * RFC 2327: "The sequence CRLF (0x0d0a) is used to end a record,
 * although parsers should be tolerant and also accept records
 * terminated with a single newline character". We handle both cases.
 */
static const sip_header_t sdp_hdrs_v4[] = {
    [SDP_HDR_UNSPEC]        = SDP_HDR("unspec", NULL, NULL),
    [SDP_HDR_VERSION]       = SDP_HDR("v=", NULL, sip_digits_len),
    [SDP_HDR_OWNER]         = SDP_HDR("o=", "IN IP4 ", sdp_addr_len),
    [SDP_HDR_CONNECTION]    = SDP_HDR("c=", "IN IP4 ", sdp_addr_len),
    [SDP_HDR_MEDIA]         = SDP_HDR("m=", NULL, sip_media_len),
};

static const sip_header_t sdp_hdrs_v6[] = {
    [SDP_HDR_UNSPEC]        = SDP_HDR("unspec", NULL, NULL),
    [SDP_HDR_VERSION]       = SDP_HDR("v=", NULL, sip_digits_len),
    [SDP_HDR_OWNER]         = SDP_HDR("o=", "IN IP6 ", sdp_addr_len),
    [SDP_HDR_CONNECTION]    = SDP_HDR("c=", "IN IP6 ", sdp_addr_len),
    [SDP_HDR_MEDIA]         = SDP_HDR("m=", NULL, sip_media_len),
};

/*
 * sdp_header_search
 *
 * Linear string search within SDP header values
 */
static const char * sdp_header_search(const char *buf, const char *limit,
                                      const char *needle, uint32_t len)
{
    for (limit -= len; buf < limit; buf++) {
        if (*buf == '\r' || *buf == '\n')
            break;
        if (strncmp(buf, needle, len) == 0)
            return buf;
    }
    return NULL;
}

/*
 * sip_get_sdp_header
 *
 * Locate a SDP header (optionally a substring within the header
 * value), optionally stopping at the first occurrence of the term
 * header, parse it and return the offset and length of the data
 * we're interested in.
 */
int sip_get_sdp_header(fte::ctx_t &ctx, const char *buf,
                       uint32_t dataoff, uint32_t datalen,
                       enum sdp_header_types type,
                       enum sdp_header_types term,
                       uint32_t *matchoff, uint32_t *matchlen)
{
    const sip_header_t *hdrs, *hdr, *thdr;
    const char *start = buf, *limit = buf + datalen;
    int shift = 0;
    int af = AF_INET; // KB -- fix me from packet

    hdrs = af == AF_INET ? sdp_hdrs_v4 : sdp_hdrs_v6;
    hdr = &hdrs[type];
    thdr = &hdrs[term];

    for (buf += dataoff; buf < limit; buf++) {
        /* Find beginning of line */
        if (*buf != '\r' && *buf != '\n')
            continue;
        if (++buf >= limit)
            break;
        if (*(buf- 1) == '\r' && *buf == '\n') {
            if (++buf >= limit)
                break;
        }

        if (term != SDP_HDR_UNSPEC &&
            limit - buf >= thdr->len &&
            strncasecmp(buf, thdr->name, thdr->len) == 0)
                break;
        else if (limit - buf >= hdr->len &&
                 strncasecmp(buf, hdr->name, hdr->len) == 0)
            buf += hdr->len;
        else
            continue;

        *matchoff = buf - start;
        if (hdr->search) {
            buf = sdp_header_search(buf, limit, hdr->search,
                                    hdr->slen);
            if (!buf)
                return -1;
            buf += hdr->slen;
        }

        *matchlen = hdr->match_len(ctx, buf, limit, &shift);
        if (!*matchlen)
            return -1;
        *matchoff = buf - start + shift;
        return 1;
    }
    return 0;
}

/*
 * sip_parse_sdp_addr
 */
static int sip_parse_sdp_addr(fte::ctx_t &ctx, const char *buf,
                              uint32_t dataoff, uint32_t datalen,
                              enum sdp_header_types type,
                              enum sdp_header_types term,
                              uint32_t *matchoff, uint32_t *matchlen,
                              ip_addr_t *addr)
{
    int ret;

    ret = sip_get_sdp_header(ctx, buf, dataoff, datalen, type, term,
                             matchoff, matchlen);
    if (ret <= 0)
        return ret;

    if (!sdp_parse_addr(ctx, buf + *matchoff, NULL, addr,
                        buf + *matchoff + *matchlen))
        return -1;
    return 1;
}

typedef enum sip_expectation_classes {
    SIP_EXPECT_SIGNALLING,
    SIP_EXPECT_AUDIO,
    SIP_EXPECT_VIDEO,
    SIP_EXPECT_IMAGE,
    __SIP_EXPECT_MAX
} sip_expectation_classes_t;

#define SIP_EXPECT_MAX  (__SIP_EXPECT_MAX - 1)

typedef struct sdp_media_type {
    const char                  *name;
    uint32_t                    len;
    sip_expectation_classes_t   expect_class;
} sdp_media_type_t;

#define SDP_MEDIA_TYPE(__name, __expect_class)  \
{                                               \
    .name          = (__name),                  \
    .len           = sizeof(__name) - 1,        \
    .expect_class  = (__expect_class),          \
}

static const sdp_media_type_t sdp_media_types[] = {
    SDP_MEDIA_TYPE("audio ", SIP_EXPECT_AUDIO),
    SDP_MEDIA_TYPE("video ", SIP_EXPECT_VIDEO),
    SDP_MEDIA_TYPE("image ", SIP_EXPECT_IMAGE),
};

/*
 * sdp_media_type
 */
static const sdp_media_type_t * sdp_media_type(const char *buf,
                                               uint32_t matchoff,
                                               uint32_t matchlen)
{
    const sdp_media_type_t *media;
    unsigned int i;

    for (i = 0; i < sizeof(sdp_media_types)/sizeof(sdp_media_type_t); i++) {
        media = &sdp_media_types[i];
        if (matchlen < media->len ||
            strncmp(buf + matchoff, media->name, media->len))
            continue;
        return media;
    }
    return NULL;
}

/*
 * process_sdp
 *
 * Process SDP headers
 */
int process_sdp(fte::ctx_t &ctx, uint32_t protoff, uint32_t dataoff,
                const char **buf, uint32_t *datalen, uint32_t cseq)
{
    uint32_t matchoff, matchlen;
    uint32_t mediaoff, medialen;
    uint32_t sdpoff;
    uint32_t caddr_len, maddr_len;
    uint32_t i;
    ip_addr_t caddr, maddr, rtp_addr;
    unsigned int port;
    const sdp_media_type_t *media;
    int ret = HAL_RET_OK;

    /* Find beginning of session description */
    if (sip_get_sdp_header(ctx, *buf, 0, *datalen,
                           SDP_HDR_VERSION, SDP_HDR_UNSPEC,
                           &matchoff, &matchlen) <= 0)
        return HAL_RET_OK;

    sdpoff = matchoff;

    /*
     * The connection information is contained in the session
     * description and/or once per media description. The first
     * media description marks the end of the session description
     */
    caddr_len = maddr_len = 0;
    if (sip_parse_sdp_addr(ctx, *buf, sdpoff, *datalen,
                           SDP_HDR_CONNECTION, SDP_HDR_MEDIA,
                           &matchoff, &matchlen, &caddr) > 0)
        caddr_len = matchlen;

    mediaoff = sdpoff;
    for (i = 0; i < sizeof(sdp_media_types)/sizeof(sdp_media_type_t); ) {
        if (sip_get_sdp_header(ctx, *buf, mediaoff, *datalen,
                               SDP_HDR_MEDIA, SDP_HDR_UNSPEC,
                               &mediaoff, &medialen) <= 0)
            break;

        /*
         * Get media type and port number. A media port value of
         * zero indicates an inactive stream.
         */
        media = sdp_media_type(*buf, mediaoff, medialen);
        if (!media) {
            mediaoff += medialen;
            continue;
        }
        mediaoff += media->len;
        medialen -= media->len;

        port = strtoul(*buf+ mediaoff, NULL, 10);
        if (port == 0)
            continue;
        if (port < 1024 || port > 65535)
            return HAL_RET_ERR;

        /* The media description overrides the session description */
        maddr_len = 0;
        if (sip_parse_sdp_addr(ctx, *buf, mediaoff, *datalen,
                               SDP_HDR_CONNECTION, SDP_HDR_MEDIA,
                                &matchoff, &matchlen, &maddr) > 0) {
            maddr_len = matchlen;
            memcpy(&rtp_addr, &maddr, sizeof(rtp_addr));
        } else if (caddr_len)
            memcpy(&rtp_addr, &caddr, sizeof(rtp_addr));
        else {
            return HAL_RET_ERR;
        }

#if 0 // KB -- fix me after parsing rtp rtcp headers
        ret = set_expected_rtp_rtcp(ctx, protoff, dataoff,
                                    buf, datalen,
                                    &rtp_addr, htons(port), t->class,
                                    mediaoff, medialen);
        if (ret != HAL_RET_OK)
            return ret;
#endif

        i++;
    }

    return ret;
}


}  // namespace alg_sip
}  // namespace plugins
}  // namespace hal
