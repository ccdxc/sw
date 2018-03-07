#include "pse_mem.h"
#include "pse_utils.h"

int pse_BN_to_buffer(const BIGNUM* bn, pse_buffer_t *buf)
{
    if(!bn || !buf) {
        return 0;    
    }

    buf->len = BN_num_bytes(bn);
    if(buf->len == 0) {
        INFO("BN size is 0");
        return 1;
    }

    // Allocate the size. TODO: Move to slab mechanism
    buf->data = (uint8_t *)malloc(buf->len);
    if(NULL == buf->data) {
        INFO("Failed to allocate buffer data of len: %d", buf->len);
        return 0;
    }

    BN_bn2bin(bn, buf->data);
    return 1;
}

void pse_free_buffer(pse_buffer_t *buf) 
{
    if(buf && buf->data) {
        free(buf->data);
        buf->data = NULL;
    }     
}
