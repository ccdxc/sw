#ifndef __PSE_MEM_H__
#define __PSE_MEM_H__

#include <openssl/bn.h>

typedef struct pse_buffer_s {
    uint32_t    len;
    uint8_t     *data;
} pse_buffer_t;


#define LOG_BUFFER(name, buf)       \
    INFO(name);                     \
    INFO("\t len: %d", buf.len);   \
    HEX_DUMP(buf.data, buf.len);

int pse_BN_to_buffer(const BIGNUM* bn, pse_buffer_t *buf);

void pse_free_buffer(pse_buffer_t* buf);

#endif /* __PSE_MEM_H__ */
