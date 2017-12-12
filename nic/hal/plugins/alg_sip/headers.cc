/*
 * headers.cc
 *
 * SIP headers functionality
 */

#include "core.hpp"

namespace hal {
namespace plugins {
namespace alg_sip {

/*
 * SIP header types
 */
typedef enum sip_header_types {
    SIP_HDR_CSEQ,
    SIP_HDR_FROM,
    SIP_HDR_TO,
    SIP_HDR_CONTACT,
    SIP_HDR_VIA_UDP,
    SIP_HDR_VIA_TCP,
    SIP_HDR_EXPIRES,
    SIP_HDR_CONTENT_LENGTH,
    SIP_HDR_CALL_ID,
} sip_header_types_t;

#define SIP_HDR(__name, __cname, __search, __match)         \
    __SIP_HDR(__name, __cname, __search, __match)

/*
 * SIP header parsing: SIP headers are located at the beginning of a
 * line, but may span several lines, in which case the continuation
 * lines begin with a whitespace character. RFC 2543 allows lines to
 * be terminated with CR, LF or CRLF, RFC 3261 allows only CRLF, we
 * support both.
 * 
 * Headers are followed by (optionally) whitespace, a colon, again
 * (optionally) whitespace and the values. Whitespace in this context
 * means any amount of tabs, spaces and continuation lines, which are
 * treated as a single whitespace character.
 *
 * Some headers may appear multiple times. A comma separated list of
 * values is equivalent to multiple headers.
 */
static const sip_header_t sip_hdrs[] = {
    [SIP_HDR_CSEQ]          = SIP_HDR("CSeq", NULL, NULL,
                                      sip_digits_len),
    [SIP_HDR_FROM]          = SIP_HDR("From", "f", "sip:",
                                      sip_skp_epaddr_len),
    [SIP_HDR_TO]            = SIP_HDR("To", "t", "sip:",
                                      sip_skp_epaddr_len),
    [SIP_HDR_CONTACT]       = SIP_HDR("Contact", "m", "sip:",
                                      sip_skp_epaddr_len),
    [SIP_HDR_VIA_UDP]       = SIP_HDR("Via", "v", "UDP ",
                                      sip_epaddr_len),
    [SIP_HDR_VIA_TCP]       = SIP_HDR("Via", "v", "TCP ",
                                      sip_epaddr_len),
    [SIP_HDR_EXPIRES]       = SIP_HDR("Expires", NULL, NULL,
                                      sip_digits_len),
    [SIP_HDR_CONTENT_LENGTH]= SIP_HDR("Content-Length", "l", NULL,
                                      sip_digits_len),
    [SIP_HDR_CALL_ID]       = SIP_HDR("Call-Id", "i", NULL,
                                      sip_callid_len),
};

/*
 * sip_header_search
 *
 * Search within a SIP header value, dealing with continuation lines
 */ 
static const char * sip_header_search(const char *buf, const char *limit,
                                      const char *needle, uint32_t len)
{
    for (limit -= len; buf < limit; buf++) {
        if (*buf == '\r' || *buf == '\n') {
            buf = sip_follow_continuation(buf, limit);
            if (buf == NULL)
                break;
            continue;
        }

        if (strncasecmp(buf, needle, len) == 0)
            return buf;
   }
   return NULL;
}

/*
 * sip_get_header
 */
static int sip_get_header(fte::ctx_t &ctx, const char *buf,
                          uint32_t dataoff, uint32_t datalen,
                          enum sip_header_types type,
                          uint32_t *matchoff, uint32_t *matchlen)
{
    const sip_header_t *hdr = &sip_hdrs[type];
    const char *start = buf, *limit = buf + datalen;
    int shift = 0;

    for (buf += dataoff; buf < limit; buf++) {
        /* Find beginning of line */
        if (*buf != '\r' && *buf != '\n')
            continue;
        if (++buf >= limit)
            break;
        if (*(buf - 1) == '\r' && *buf == '\n') {
            if (++buf >= limit)
                break;
        }

        /* Skip continuation lines */
        if (*buf == ' ' || *buf == '\t')
            continue;

        /*
         * Find header. Compact headers must be followed by a
         * non-alphabetic character to avoid mismatches.
         */ 
        if (limit - buf >= hdr->len &&
            strncasecmp(buf, hdr->name, hdr->len) == 0)
            buf += hdr->len;
        else if (hdr->cname && limit - buf >= hdr->clen + 1 &&
                 strncasecmp(buf, hdr->cname, hdr->clen) == 0 &&
                 !isalpha(*(buf + hdr->clen)))
            buf += hdr->clen;
        else
            continue;

        /* Find and skip colon */
        buf = sip_skip_whitespace(buf, limit);
        if (buf == NULL)
            break;
        if (*buf != ':' || ++buf >= limit)
            break;

        /* Skip whitespace after colon */
        buf = sip_skip_whitespace(buf, limit);
        if (buf == NULL)
            break;

        *matchoff = buf - start;
        if (hdr->search) {
            buf = sip_header_search(buf, limit, hdr->search, hdr->slen);
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
 * sip_next_header
 * 
 * Get next header field in a list of comma separated values
 */
static int sip_next_header(fte::ctx_t &ctx, const char *buf,
                           uint32_t dataoff, uint32_t datalen,
                           enum sip_header_types type,
                           uint32_t *matchoff, uint32_t *matchlen)
{
    const sip_header_t *hdr = &sip_hdrs[type];
    const char *start = buf, *limit = buf + datalen;
    int shift = 0;

    buf += dataoff;

    buf = sip_header_search(buf, limit, ",", strlen(","));
    if (!buf)
        return 0;

    buf = sip_header_search(buf, limit, hdr->search, hdr->slen);
    if (!buf)
        return 0;
    buf += hdr->slen;

    *matchoff = buf - start;
    *matchlen = hdr->match_len(ctx, buf, limit, &shift);
    if (!*matchlen)
        return -1;
    *matchoff += shift;
    return 1;
}

/* 
 * sip_walk_headers
 *
 * Walk through headers until a parsable one is found or no header of the
 * given type is left
 */ 
static int sip_walk_headers(fte::ctx_t &ctx, const char *buf,
                            uint32_t dataoff, uint32_t datalen,
                            enum sip_header_types type, int *in_header,
                            uint32_t *matchoff, uint32_t *matchlen)
{
    int ret;

    if (in_header && *in_header) {
        while (1) {
            ret = sip_next_header(ctx, buf, dataoff, datalen,
                                     type, matchoff, matchlen);
            if (ret > 0)
                return ret;
            if (ret == 0)
                break;
            dataoff += *matchoff;
        }
        *in_header = 0;
    }

    while (1) {
        ret = sip_get_header(ctx, buf, dataoff, datalen,
                                type, matchoff, matchlen);
        if (ret > 0)
            break;
        if (ret == 0)
            return ret;
        dataoff += *matchoff;
    }

    if (in_header)
        *in_header = 1;
    return 1;
}

/*
 * sip_parse_header_uri
 *
 * Locate a SIP header, parse the URI and return the offset and length
 * of the address as well as the address and port themselves. A stream
 * of headers can be parsed by handing in a non-NULL datalen and in_header
 * pointer.
 */
int sip_parse_header_uri(fte::ctx_t &ctx, const char *buf,
                         uint32_t *dataoff, uint32_t datalen,
                         enum sip_header_types type, int *in_header,
                         uint32_t *matchoff, uint32_t *matchlen,
                         ip_addr_t *addr, uint16_t *port)
{
    const char *c, *limit = buf + datalen;
    uint32_t p;
    int ret;

    ret = sip_walk_headers(ctx, buf, dataoff ? *dataoff : 0, datalen,
                           type, in_header, matchoff, matchlen);
    if (ret == 0)
        return ret;

    if (!sip_parse_addr(ctx, buf + *matchoff, &c, addr, limit, true))
        return -1;
    if (*c == ':') {
        c++;
        p = strtoul(c, (char **)&c, 10);
        if (p < 1024 || p > 65535)
            return -1;
        *port = htons(p);
    } else
        *port = htons(SIP_PORT);

    if (dataoff)
        *dataoff = c - buf;
    return 1;
}

/*
 * sip_parse_param
 */
static int sip_parse_param(fte::ctx_t &ctx, const char *buf,
                           uint32_t dataoff, uint32_t datalen,
                           const char *name,
                           uint32_t *matchoff, uint32_t *matchlen)
{
    const char *limit = buf + datalen;
    const char *start;
    const char *end;

    limit = sip_header_search(buf + dataoff, limit, ",", strlen(","));
    if (!limit)
        limit = buf + datalen;

    start = sip_header_search(buf + dataoff, limit, name, strlen(name));
    if (!start)
        return 0;
    start += strlen(name);

    end = sip_header_search(start, limit, ";", strlen(";"));
    if (!end)
        end = limit;

    *matchoff = start - buf;
    *matchlen = end - start;
    return 1;
}

/* 
 * sip_parse_address_param
 *
 * Parse address from header parameter and return address, offset and length
 */ 
int sip_parse_address_param(fte::ctx_t &ctx, const char *buf,
                            uint32_t dataoff, uint32_t datalen,
                            const char *name,
                            uint32_t *matchoff, uint32_t *matchlen,
                            ip_addr_t *addr, bool delim)
{
    const char *limit = buf + datalen;
    const char *start, *end;

    limit = sip_header_search(buf + dataoff, limit, ",", strlen(","));
    if (!limit)
        limit = buf + datalen;

    start = sip_header_search(buf + dataoff, limit, name, strlen(name));
    if (!start)
        return 0;

    start += strlen(name);
    if (!sip_parse_addr(ctx, start, &end, addr, limit, delim))
        return 0;
    *matchoff = start - buf;
    *matchlen = end - start;
    return 1;
}

/* 
 * sip_parse_numberical param
 *
 * Parse numerical header parameter and return value, offset and length
 */
int sip_parse_numerical_param(fte::ctx_t &ctx, const char *buf,
                              uint32_t dataoff, uint32_t datalen,
                              const char *name,
                              uint32_t *matchoff, uint32_t *matchlen,
                              uint32_t *val)
{
    const char *limit = buf + datalen;
    const char *start;
    char *end;

    limit = sip_header_search(buf + dataoff, limit, ",", strlen(","));
    if (!limit)
        limit = buf + datalen;

    start = sip_header_search(buf + dataoff, limit, name, strlen(name));
    if (!start)
        return 0;

    start += strlen(name);
    *val = strtoul(start, &end, 0);
    if (start == end)
        return 0;
    if (matchoff && matchlen) {
        *matchoff = start - buf;
        *matchlen = end - start;
    }
    return 1;
}

/*
 * sip_parse_transport
 */
int sip_parse_transport(fte::ctx_t &ctx, const char *buf,
                        uint32_t dataoff, uint32_t datalen,
                        uint8_t *proto)
{
    uint32_t matchoff, matchlen;

    if (sip_parse_param(ctx, buf, dataoff, datalen, "transport=",
                        &matchoff, &matchlen)) {
        if (!strncasecmp(buf + matchoff, "TCP", strlen("TCP")))
            *proto = IPPROTO_TCP;
        else if (!strncasecmp(buf + matchoff, "UDP", strlen("UDP")))
            *proto = IPPROTO_UDP;
        else
            return 0;

        //KB -- need proto from packet
        //if (*proto != proto-from-packet)
        //    return 0;
    }   // else
        //*proto = proto-from-packet(ct);

    return 1;
}


}  // namespace alg_sip
}  // namespace plugins
}  // namespace hal
