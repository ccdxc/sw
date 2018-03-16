#include "pse_mem.h"
#include "pse_utils.h"

int 
pse_BN_to_buffer_pad(const BIGNUM* bn, pse_buffer_t *buf, int tolen)
{
    if(tolen == 0) {
        INFO("BN size is 0");
        return 1;
    }

    // Allocate the size. TODO: Move to slab mechanism
    buf->data = (uint8_t *)PSE_MALLOC(tolen);
    if(NULL == buf->data) {
        INFO("Failed to allocate buffer data of len: %d", tolen);
        return 0;
    }
    
    buf->len = tolen;
    return BN_bn2binpad(bn, buf->data, tolen);
}


int pse_BN_to_buffer(const BIGNUM* bn, pse_buffer_t *buf)
{
    if(!bn || !buf) {
        return 0;    
    }

    int len = BN_num_bytes(bn);

    return pse_BN_to_buffer_pad(bn, buf, len);
}

int pse_BN_to_buffer_pad256(const BIGNUM* bn, pse_buffer_t *buf)
{
    return pse_BN_to_buffer_pad(bn, buf, 256);    
}

void pse_free_buffer(pse_buffer_t *buf) 
{
    if(buf && buf->data) {
        free(buf->data);
        buf->data = NULL;
    }     
}
