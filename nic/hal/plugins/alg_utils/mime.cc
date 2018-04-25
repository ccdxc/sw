//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// MIME functions for ALG parsing.  This file provides implementations
// for basic MIME parsing.  MIME headers are used in many protocols, such as
// HTTP, RTSP, SIP, etc.
//-----------------------------------------------------------------------------

#include "mime.hpp"

namespace hal {
namespace plugins {
namespace alg_utils {

#define is_space(c) ((c) == ' ' || (c) == '\t')
#define is_ws(c) (is_space(c) || (c) == '\r' || (c) == '\n')
//------------------------------------------------------------------------
// Skip white space
//------------------------------------------------------------------------
void
alg_mime_skipws(const char *buf, uint32_t len, uint32_t *offset)
{
    if (*offset >= len) {
        return;
    }

    while (is_ws(buf[*offset])) {
        *offset += 1;
        if (*offset >= len) {
            return;
        }
    }
}

//------------------------------------------------------------------------
// Given a buffer and length, advance to the next line and mark the current
// line.  If the current line is empty, *plinelen will be set to zero.  If
// not, it will be set to the actual line length (including CRLF).
//
// if lws_cont is set then 'line'  means logical line (includes LWS continuations).
// Returns true on success, false on failure.
//------------------------------------------------------------------------
bool
alg_mime_nextline(const char* p, uint32_t len, uint32_t* poff,
             uint32_t* plineoff, uint32_t* plinelen, bool lws_cont)
{
    uint32_t    off = *poff;
    uint32_t    physlen = 0;

    if (off >= len) {
        return false;
    }

    do {
        while (p[off] != '\n') {
            if (len-off <= 1) {
                return false;
            }

            physlen++;
            off++;
        }

        /* if we saw a crlf, physlen needs adjusted */
        if (physlen > 0 && p[off] == '\n' && p[off-1] == '\r') {
            physlen--;
        }

        /* advance past the newline */
        off++;

        /* check for an empty line */
        if (physlen == 0) {
            break;
        }
    } while (lws_cont && is_space(p[off]));

    *plineoff = *poff;
    *plinelen = (physlen == 0) ? 0 : (off - *poff);
    *poff = off;

    return true;
}



//------------------------------------------------------------------------
// Given a buffer, offset and length, compare the token with buf and
// advance offset to the next token in case of match.
//
// token - token to match
// sep   - separator after the token
// space - allow space before the sep
// ignore_case - non case sensitice cmp
//
// Returns true on success, false on failure.
//------------------------------------------------------------------------
bool
alg_mime_token_cmp(const char *buf, uint32_t len, uint32_t *offset,
                   const char *token, char sep, bool space, bool ignore_case)
{
    uint32_t off;

    // compare the token
    for (off = *offset; *token; off++, token++) {
        if (off >= len) {
            return false;
        }

        if (ignore_case) {
            if (tolower(buf[off]) != tolower(*token)) {
                return false;
            }
        } else {
            if (buf[off] != *token) {
                return false;
            }
        }
    }

    if (off >= len) {
        return false;
    }


    if (sep == ' ') {
        if (!is_ws(buf[off])) {
            return false;
        }
    } else if (sep) {
        // skip white space before the sep
        if (space) {
            while (is_space(buf[off])) {
                off++;
                if (off >= len) {
                    return false;
                }
            }
        }
        if (buf[off] != sep) {
            return false;
        }
        off++; // advance once for sep
    }


    // skip trailing white space (including crlf)
    alg_mime_skipws(buf, len, &off);

    *offset = off;
    return true;
}

//------------------------------------------------------------------------
// Parse a string containing a 16-bit unsigned integer.
//------------------------------------------------------------------------
bool
alg_mime_strtou16(const char* buf, uint32_t len, uint32_t *poff, uint16_t* pval)
{
    uint32_t off = *poff;

    *pval = 0;
    while (off < len && isdigit(buf[off])) {
        *pval = (*pval * 10) + (buf[off] - '0');
        off++;
    }

    if (off == *poff) {
        return false;
    }

    *poff = off;
    return true;
}

//------------------------------------------------------------------------
// Parse a string containing a 32-bit unsigned integer.
//------------------------------------------------------------------------
bool
alg_mime_strtou32(const char* buf, uint32_t len, uint32_t *poff, uint32_t* pval)
{
    uint32_t off = *poff;

    *pval = 0;
    while (off < len && isdigit(buf[off])) {
        *pval = (*pval * 10) + (buf[off] - '0');
        off++;
    }

    if (off == *poff) {
        return false;
    }

    *poff = off;
    return true;
}

//------------------------------------------------------------------------
// Parse a string containing a ipv4/ipv6 address
// Returns true on success and false if ip address format is not correct
//------------------------------------------------------------------------
bool
alg_mime_strtoip(const char* buf, uint32_t len, uint32_t *poff, ip_addr_t* pval )
{
    uint32_t off = *poff;

    char str[INET6_ADDRSTRLEN];
    int ret;

    if (len-off >= INET6_ADDRSTRLEN) {
        return false;
    }

    strncpy(str, buf+off, len-off);
    str[len-off] = '\0';

    ret = inet_pton(AF_INET, str, &pval->addr.v4_addr);
    if (ret > 0) {
        pval->af = IP_AF_IPV4;
        pval->addr.v4_addr = ntohl(pval->addr.v4_addr);
        *poff = len;
        return true;
    }

    ret = inet_pton(AF_INET6, str, pval->addr.v6_addr.addr8);
    if (ret > 0) {
        pval->af = IP_AF_IPV6;
        *poff = len;
        return true;
    }

    *pval = {};
    return false;
}

} // namepsace alg_utils
} // namepsace plugins
} // namepsace hal
