/*
 * utils.cc
 *
 * SIP utility routines
 */

#include "core.hpp"

namespace hal {
namespace plugins {
namespace alg_sip {

#define IN6PTON_XDIGIT      0x00010000
#define IN6PTON_DIGIT       0x00020000
#define IN6PTON_COLON_MASK  0x00700000
#define IN6PTON_COLON_1     0x00100000  /* single : requested */
#define IN6PTON_COLON_2     0x00200000  /* second : requested */
#define IN6PTON_COLON_1_2   0x00400000  /* :: requested */
#define IN6PTON_DOT         0x00800000  /* . */
#define IN6PTON_DELIM       0x10000000
#define IN6PTON_NULL        0x20000000  /* first/tail */
#define IN6PTON_UNKNOWN     0x40000000

/*
 * sip_digit2bin
 */
static inline int sip_xdigit2bin(char c, int delim)
{
    if (c == delim || c == '\0')
        return IN6PTON_DELIM;
    if (c == ':')
        return IN6PTON_COLON_MASK;
    if (c == '.')
        return IN6PTON_DOT;
    if (c >= '0' && c <= '9')
        return (IN6PTON_XDIGIT | IN6PTON_DIGIT| (c - '0'));
    if (c >= 'a' && c <= 'f')
        return (IN6PTON_XDIGIT | (c - 'a' + 10));
    if (c >= 'A' && c <= 'F')
        return (IN6PTON_XDIGIT | (c - 'A' + 10));
    if (delim == -1)
        return IN6PTON_DELIM;
    return IN6PTON_UNKNOWN;
}

/*
 * sip_in4_pton
 *
 * Convert IPv4 printable address to binary form 
 */
static int sip_in4_pton(const char *src, int srclen, uint8_t *dst,
                        int delim, const char **end)
{
    const char *s;
    uint8_t *d;
    uint8_t dbuf[4];
    int ret = 0;
    int i;
    int w = 0;

    if (srclen < 0)
        srclen = strlen(src);
    s = src;
    d = dbuf;
    i = 0;
    while(1) {
        int c;
        c = sip_xdigit2bin(srclen > 0 ? *s : '\0', delim);
        if (!(c & (IN6PTON_DIGIT | IN6PTON_DOT |
                   IN6PTON_DELIM | IN6PTON_COLON_MASK))) {
            goto out;
        }
        if (c & (IN6PTON_DOT | IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
            if (w == 0)
                goto out;
            *d++ = w & 0xff;
            w = 0;
            i++;
            if (c & (IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
                if (i != 4)
                    goto out;
                break;
            }
            goto cont;
        }
        w = (w * 10) + c;
        if ((w & 0xffff) > 255) {
            goto out;
        }
cont:
        if (i >= 4)
            goto out;
        s++;
        srclen--;
    }
    ret = 1;
    memcpy(dst, dbuf, sizeof(dbuf));
out:
    if (end)
        *end = s;
    return ret;
}

/*
 * sip_in6_pton
 *
 * Convert IPv6 printable address to binary form 
 */
static int sip_in6_pton(const char *src, int srclen, uint8_t *dst,
                        int delim, const char **end)
{
    const char *s, *tok = NULL;
    uint8_t *d, *dc = NULL;
    uint8_t dbuf[16];
    int ret = 0;
    int i;
    int state = IN6PTON_COLON_1_2 | IN6PTON_XDIGIT | IN6PTON_NULL;
    int w = 0;

    memset(dbuf, 0, sizeof(dbuf));

    s = src;
    d = dbuf;
    if (srclen < 0)
        srclen = strlen(src);

    while (1) {
        int c;

        c = sip_xdigit2bin(srclen > 0 ? *s : '\0', delim);
        if (!(c & state))
            goto out;
        if (c & (IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
            /* process one 16-bit word */
            if (!(state & IN6PTON_NULL)) {
                *d++ = (w >> 8) & 0xff;
                *d++ = w & 0xff;
            }
            w = 0;
            if (c & IN6PTON_DELIM) {
                /* We've processed last word */
                break;
            }
            /*
             * COLON_1 => XDIGIT
             * COLON_2 => XDIGIT|DELIM
             * COLON_1_2 => COLON_2
             */
            switch (state & IN6PTON_COLON_MASK) {
            case IN6PTON_COLON_2:
                dc = d;
                state = IN6PTON_XDIGIT | IN6PTON_DELIM;
                if ((dc - dbuf) >= (int)sizeof(dbuf))
                    state |= IN6PTON_NULL;
                break;
            case IN6PTON_COLON_1|IN6PTON_COLON_1_2:
                state = IN6PTON_XDIGIT | IN6PTON_COLON_2;
                break;
            case IN6PTON_COLON_1:
                state = IN6PTON_XDIGIT;
                break;
            case IN6PTON_COLON_1_2:
                state = IN6PTON_COLON_2;
                break;
            default:
                state = 0;
            }
            tok = s + 1;
            goto cont;
        }

        if (c & IN6PTON_DOT) {
            ret = sip_in4_pton(tok ? tok : s, srclen + (int)(s - tok),
                               d, delim, &s);
            if (ret > 0) {
                d += 4;
                break;
            }
            goto out;
        }

        w = (w << 4) | (0xff & c);
        state = IN6PTON_COLON_1 | IN6PTON_DELIM;
        if (!(w & 0xf000)) {
            state |= IN6PTON_XDIGIT;
        }
        if (!dc && d + 2 < dbuf + sizeof(dbuf)) {
            state |= IN6PTON_COLON_1_2;
            state &= ~IN6PTON_DELIM;
        }
        if (d + 2 >= dbuf + sizeof(dbuf)) {
            state &= ~(IN6PTON_COLON_1|IN6PTON_COLON_1_2);
        }
cont:
        if ((dc && d + 4 < dbuf + sizeof(dbuf)) ||
            d + 4 == dbuf + sizeof(dbuf)) {
            state |= IN6PTON_DOT;
        }
        if (d >= dbuf + sizeof(dbuf)) {
            state &= ~(IN6PTON_XDIGIT|IN6PTON_COLON_MASK);
        }
        s++;
        srclen--;
    }

    i = 15; d--;

    if (dc) {
        while(d >= dc)
            dst[i--] = *d--;
        while(i >= dc - dbuf)
            dst[i--] = 0;
        while(i >= 0)
            dst[i--] = *d--;
    } else
        memcpy(dst, dbuf, sizeof(dbuf));

    ret = 1;
out:
    if (end)
        *end = s;
    return ret;
}

/*
 * sip_string_len
 *
 * Find the length of the (textual) string until it reaches limit
 */
static int sip_string_len(const char *buf, const char *limit, int *shift) {
    int len = 0;

    while (buf < limit && isalpha(*buf)) {
        buf++;
        len++;
    }
    return len;
}

/*
 * sip_digits_len
 *
 * Find the length of the (digital) string until it reaches limit 
 */
static int sip_digits_len(const char *buf, const char *limit, int *shift) {
    int len = 0;

    while (buf < limit && isdigit(*buf)) {
        buf++;
        len++;
    }
    return len;
}

/*
 * sip_iswordc
 */
static int sip_iswordc(const char c) {
    if (isalnum(c) || c == '!' || c == '"' || c == '%' ||
        (c >= '(' && c <= '+') || c == ':' || c == '<' || c == '>' ||
        c == '?' || (c >= '[' && c <= ']') || c == '_' || c == '`' ||
        c == '{' || c == '}' || c == '~' || (c >= '-' && c <= '/') ||
        c == '\'')
        return 1;
    return 0;
}

/*
 * sip_word_len
 *
 * Find the length of the word string until it reaches limit 
 */
static int sip_word_len(const char *buf, const char *limit) {
    int len = 0;

    while (buf < limit && sip_iswordc(*buf)) {
        buf++;
        len++;
    }
    return len;
}

/*
 * sip_callid_len
 *
 * Find the call-id length until the string reaches limit
 */
int sip_callid_len(const char *buf, const char *limit, int *shift) {
    int len, domain_len;

    len = sip_word_len(buf, limit);
    buf += len;
    if (!len || buf == limit || *buf != '@')
        return len;

    buf++;
    len++;

    domain_len = sip_word_len(buf, limit);
    if (!domain_len)
        return 0;

    return len + domain_len;
}

/*
 * sip_media_len
 *
 * Find the media length until the string reaches limit
 */
int sip_media_len(const char *buf, const char *limit, int *shift) {
    int len = sip_string_len(buf, limit, shift);

    buf += len;
    if (buf >= limit || *buf != ' ')
        return 0;
    len++;
    buf++;

    return len + sip_digits_len(buf, limit, shift);
}

/*
 * sip_follow_continuation
 */
static const char* sip_follow_continuation(const char *buf, const char *limit)
{
    /* Walk past newline */
    if (++buf >= limit)
        return NULL;

    /* Skip '\n' in CR LF */
    if (*(buf - 1) == '\r' && *buf == '\n') {
        if (++buf >= limit)
            return NULL;
    }

    /* Continuation line? */
    if (*buf != ' ' && *buf != '\t')
        return NULL;

    /* skip leading whitespace */
    for (; buf < limit; buf++) {
        if (*buf != ' ' && *buf != '\t')
            break;
    }
    return buf;
}

/*
 * sip_skip_whitespace
 */
const char * sip_skip_whitespace(const char *buf, const char *limit)
{
    for (; buf < limit; buf++) {
        if (*buf == ' ' || *buf == '\t')
            continue;
        if (*buf != '\r' && *buf != '\n')
            break;
        buf = sip_follow_continuation(buf, limit);
        break;
    }
    return buf;
}

/*
 * sip_parse_addr
 *
 * Parse address from buffer to IP structure
 */
static int sip_parse_addr(uint16_t af, const char *cp, const char **endp,
                          ip_addr_t *addr, const char *limit, bool delim)
{
    const char *end;
    int ret;

    memset(addr, 0, sizeof(*addr));
    switch (af) {
    case AF_INET:
        ret = sip_in4_pton(cp, limit-cp, (uint8_t *)&addr->addr.v4_addr, -1,
                           &end);
        if (ret == 0)
            return 0;
        break;
    case AF_INET6:
        if (cp < limit && *cp == '[')
            cp++;
        else if (delim)
            return 0;

        ret = sip_in6_pton(cp, limit-cp, (uint8_t *)&addr->addr.v6_addr, -1,
                           &end);
        if (ret == 0)
            return 0;

        if (end < limit && *end == ']')
            end++;
        else if (delim)
            return 0;
        break;
    default:
        return 0;
    }

    if (endp)
        *endp = end;
    return 1;
}

/* 
 * sip_epaddr_len
 *
 * Skip ip address. returns its length.
 */ 
static int sip_epaddr_len(uint16_t af, const char *buf, const char *limit,
                          int *shift)
{
    ip_addr_t addr;
    const char *aux = buf;

    if (!sip_parse_addr(af, buf, &buf, &addr, limit, true)) {
        return 0;
    }

    /* Port number */
    if (*buf == ':') {
        buf++;
        buf += sip_digits_len(buf, limit, shift);
    }
    return buf - aux;
}

/*
 * sip_skp_epaddr_len
 *
 * Get address length, skiping user info
 */ 
int sip_skp_epaddr_len(uint16_t af, const char *buf, const char *limit,
                       int *shift)
{
    const char *start = buf;
    int s = *shift;

    /*
     * Search for @, but stop at the end of the line.
     * We are inside a sip: URI, so we don't need to worry about
     * continuation lines.
     */
    while (buf < limit && *buf != '@' && *buf != '\r' && *buf != '\n') {
        (*shift)++;
        buf++;
    }

    if (buf < limit && *buf == '@') {
        buf++;
        (*shift)++;
    } else {
        buf = start;
        *shift = s;
    }

    return sip_epaddr_len(af, buf, limit, shift);
}


}  // namespace alg_sip
}  // namespace plugins
}  // namespace hal
