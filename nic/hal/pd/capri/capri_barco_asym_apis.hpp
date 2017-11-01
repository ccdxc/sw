#ifndef __CAPRI_BARCO_ASYM_APIS_H__
#define __CAPRI_BARCO_ASYM_APIS_H__

namespace hal {
namespace pd {


#define _API_PARAM_DEBUG_

#ifdef _API_PARAM_DEBUG_


#define MAX_LINE_SZ 128
static inline void capri_hex_dump_trace(char *label, char *buf, uint16_t len)
{
    char            line[MAX_LINE_SZ];
    char            *lineptr;
    uint16_t        idx = 0;
    uint16_t        lineoffset = 0;

    lineptr = &line[0];
    HAL_TRACE_DEBUG("{}:", label);
    for (idx = 0; idx < len; idx++) {

        lineoffset += snprintf(lineptr + lineoffset, (MAX_LINE_SZ - lineoffset - 1),
                "%02hhx ", buf[idx]);

        if (((idx + 1) % 16) == 0) {
            HAL_TRACE_DEBUG("{}", line);
            lineoffset = 0;
        }
    }
    if (lineoffset) {
        HAL_TRACE_DEBUG("{}", line);
    }
}

#define CAPRI_BARCO_API_PARAM_HEXDUMP(label, buf, len) capri_hex_dump_trace(label, buf, len)

#else

#define CAPRI_BARCO_API_PARAM_HEXDUMP(label, buf, len)

#endif /* _API_PARAM_DEBUG_ */

hal_ret_t capri_barco_asym_ecc_point_mul_p256(uint8_t *p, uint8_t *n,
        uint8_t *xg, uint8_t *yg, uint8_t *a, uint8_t *b, uint8_t *x1, uint8_t *y1,
        uint8_t *k, uint8_t *x3, uint8_t *y3);

hal_ret_t capri_barco_asym_ecdsa_p256_sig_gen(uint8_t *p, uint8_t *n,
        uint8_t *xg, uint8_t *yg, uint8_t *a, uint8_t *b, uint8_t *da,
        uint8_t *k, uint8_t *h, uint8_t *r, uint8_t *s);

hal_ret_t capri_barco_asym_ecdsa_p256_sig_verify(uint8_t *p, uint8_t *n,
        uint8_t *xg, uint8_t *yg, uint8_t *a, uint8_t *b, uint8_t *xq,
        uint8_t *yq, uint8_t *r, uint8_t *s, uint8_t *h);

}    // namespace pd
}    // namespace hal

#endif /* __CAPRI_BARCO_ASYM_APIS_H__ */
