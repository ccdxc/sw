#ifndef __PSE_INTF_H__
#define __PSE_INTF_H__

#include <openssl/evp.h>
#ifdef __cplusplus
extern "C" {
#endif 

typedef struct pse_buffer_s {
    uint32_t    len;
    uint8_t     *data;
} PSE_BUFFER;

typedef struct PSE_rsa_key_st {
    PSE_BUFFER  rsa_n;
    PSE_BUFFER  rsa_e;
} PSE_RSA_KEY;

typedef struct PSE_ec_key_st {
    
} PSE_EC_KEY;

/* PSE Key Handle */
typedef struct PSE_key_st {
    const char       *label;    // Label to identify the key
    int              type;      // Underlying type of the KEY from openssl
    uint32_t         index;     // index where key is stored (e.g. hw)
    union {
        PSE_RSA_KEY  rsa_key;   // Public parameters for RSA
        PSE_EC_KEY   ec_key;    // Public parameters for EC
    } u;
}PSE_KEY;


#ifdef __cplusplus
}
#endif /* __cpluspls */

#endif /* __PSE_INTF_H__ */
