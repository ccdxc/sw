// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include <openssl/pem.h>
#include <openssl/x509.h>
#include "nic/sdk/lib/utils/ssl/ssl_utils.hpp"
#include "nic/hal/hal_trace.hpp"

#define KEY_WIDTH    2048
#define CSR_FILE     "./cert.csr"

int
main (int argc, char** argv)
{
    int rv = -1;
    EVP_PKEY *pkey;
    X509_REQ *x509_req;
    BIO *out;

    // generate key
    pkey = sdk::utils::generate_key(KEY_WIDTH);
    if (!pkey) {
        printf("Failed to generate private key");
        return -1;
    }

    // generate a signed csr
    x509_req = sdk::utils::generate_x509_csr(pkey);
    if (!x509_req) {
        printf("Failed to genearte x509 csr");
        goto end;
    }

    // write to a file (for debugging)
    out = BIO_new_file(CSR_FILE, "w");
    if (!out) {
        printf("Failed to create file %s", CSR_FILE);
    }
    rv = PEM_write_bio_X509_REQ(out, x509_req);
    if (rv <= 0) {
        printf("Failed to write csr to %s file", CSR_FILE);
        rv = -1;
    }

end:

    if (pkey) {
        EVP_PKEY_free(pkey);
    }
    if (x509_req) {
        X509_REQ_free(x509_req);
    }

    return rv;
}
