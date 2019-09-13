// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __SDK_SSL_UTILS_HPP__
#define __SDK_SSL_UTILS_HPP__

#include <openssl/pem.h>
#include <openssl/x509.h>

namespace sdk {
namespace utils {

// generate key with specified width (in bits)
// NOTE: EVP_PKEY_free() should be called to free the private key
EVP_PKEY *generate_key(uint32_t key_width);

// generate a signed CSR given a key
// NOTE: X509_REQ_free() should be called to release the certificate
X509_REQ *generate_x509_csr(EVP_PKEY *pkey);

}    // namespace utils
}    // namespace sdk

#endif    // __SDK_SSL_UTILS_HPP__
