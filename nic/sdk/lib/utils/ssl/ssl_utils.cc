// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include "include/sdk/base.hpp"
#include "lib/utils/ssl/ssl_utils.hpp"

namespace sdk {
namespace utils {

// generate key with specified width (in bits)
// NOTE: EVP_PKEY_free() should be called to free the private key
EVP_PKEY *
generate_key (uint32_t key_width)
{
    RSA *rsa = NULL;
    BIGNUM *bne;
    EVP_PKEY *pkey;

    // allocate memory for the key
    pkey = EVP_PKEY_new();
    if (unlikely(pkey == NULL)) {
        SDK_TRACE_ERR("Unable to create EVP_PKEY structure");
        return NULL;
    }

    // generate the RSA key pair
    bne = BN_new();
    if (unlikely(!BN_set_word(bne, RSA_F4))) {
        SDK_TRACE_ERR("Failed to set word");
        goto error;
    }

    rsa = RSA_new();
    if (!RSA_generate_key_ex(rsa, key_width, bne, NULL)) {
        SDK_TRACE_ERR("Failed to generate RSA key");
        goto error;
    }

    // set public key of x509 req
    if (unlikely(!EVP_PKEY_assign_RSA(pkey, rsa))) {
        SDK_TRACE_ERR("Failed to public key");
        goto error;
    }
    rsa = NULL;
    return pkey;

error:

    if (bne) {
        BN_free(bne);
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
    }
    if (rsa) {
        RSA_free(rsa);
    }
    return NULL;
}

// generate a signed CSR given a private key
// NOTE: X509_REQ_free() should be called to release the certificate
X509_REQ *
generate_x509_csr (EVP_PKEY *pkey)
{
    uint32_t i;
    X509_REQ *x509_req;
    X509_NAME *x509_name;
    // declare array of entries to assign to cert
    typedef struct cert_entry_s {
        const char *key;
        const char *value;
    } cert_entry_t;
    cert_entry_t cert_entries[] = {
        // country
        {"C", "US"},
        // state
        {"ST", "CA"},
        // city
        {"L", "San Jose"},
        // organization
        {"O", "Pensando"},
        // common name
        {"CN", "localhost"},
    };

    // allocate and initialize an empty csr
    x509_req = X509_REQ_new();
    if (x509_req == NULL) {
        SDK_TRACE_ERR("Failed to allocate an empty csr");
        return NULL;
    }

    if (unlikely(!X509_REQ_set_version(x509_req, 0))) {
        SDK_TRACE_ERR("Failed to set x590 request version");
        goto error;
    }

    // set the subject in the req
    x509_name = X509_REQ_get_subject_name(x509_req);
    for (i = 0; i < SDK_ARRAY_SIZE(cert_entries); i++) {
        if (unlikely(!X509_NAME_add_entry_by_txt(x509_name, cert_entries[i].key,
                          MBSTRING_ASC,
                          (const unsigned char *)cert_entries[i].value,
                          -1, -1, 0))) {
            SDK_TRACE_ERR("Failed to add cert entry key %s, value %s",
                          cert_entries[i].key, cert_entries[i].value);
            goto error;
        }
    }

    if (unlikely(!X509_REQ_set_pubkey(x509_req, pkey))) {
        SDK_TRACE_ERR("Failed to set public key of x509 request");
        goto error;
    }

    // sign the CSR using private key and message digest
    if (unlikely(!X509_REQ_sign(x509_req, pkey, EVP_sha1()))) {
        SDK_TRACE_ERR("Failed to sign the CSR");
        goto error;
    }
    return x509_req;

error:

    if (x509_req) {
        X509_REQ_free(x509_req);
    }
    return NULL;
}

}    // namespace utils
}    // namespace sdk
