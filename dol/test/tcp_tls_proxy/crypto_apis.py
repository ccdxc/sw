# Testcase definition file.
import pdb
import copy
import binascii

import types_pb2                as types_pb2

from config.store               import Store
from infra.common.objects       import ObjectDatabase as ObjectDatabase
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions
import infra.config.base        as base
import config.hal.api           as halapi
import test.tcp_tls_proxy.crypto_asym_apis as crypto_asym_apis
import test.tcp_tls_proxy.crypto_sym_apis  as crypto_sym_apis


def Setup(infra, module):

    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    return

def TestCaseVerify(tc):

    print("Crypto API: %s" % tc.module.args.api)

    if halapi.IsHalDisabled():
        return True

    if tc.module.args.api == "CRYPTO_API_ASYM_ECC_POINT_MUL_P256":
        return crypto_asym_apis.ecc_point_mul_p256_test()
    elif tc.module.args.api == "CRYPTO_API_ASYM_ECDSA_P256_SIG_GEN":
        return crypto_asym_apis.ecdsa_p256_sig_gen_test()
    elif tc.module.args.api == "CRYPTO_API_ASYM_ECDSA_P256_SIG_VERIFY":
        return crypto_asym_apis.ecdsa_p256_sig_verify_test()
    elif tc.module.args.api == "CRYPTO_API_ASYM_RSA2K_ENCRYPT":
        return crypto_asym_apis.rsa2k_encrypt_test()
    elif tc.module.args.api == "CRYPTO_API_ASYM_RSA2K_DECRYPT":
        return crypto_asym_apis.rsa2k_decrypt_test()
    elif tc.module.args.api == "CRYPTO_API_ASYM_RSA2K_CRT_DECRYPT":
        return crypto_asym_apis.rsa2k_crt_decrypt_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA1_GENERATE":
        return crypto_sym_apis.hmac_sha1_generate_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA1_VERIFY":
        return crypto_sym_apis.hmac_sha1_verify_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA224_GENERATE":
        return crypto_sym_apis.hmac_sha224_generate_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA224_VERIFY":
        return crypto_sym_apis.hmac_sha224_verify_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA256_GENERATE":
        return crypto_sym_apis.hmac_sha256_generate_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA256_VERIFY":
        return crypto_sym_apis.hmac_sha256_verify_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA384_GENERATE":
        return crypto_sym_apis.hmac_sha384_generate_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA384_VERIFY":
        return crypto_sym_apis.hmac_sha384_verify_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA512_GENERATE":
        return crypto_sym_apis.hmac_sha512_generate_test()
    elif tc.module.args.api == "CRYPTO_API_HMAC_SHA512_VERIFY":
        return crypto_sym_apis.hmac_sha512_verify_test()
    else:
        print("Invalid API in module definition")
        return False


def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
