# Crypto Symm API DoL support routines
import pdb
import copy
import binascii

import types_pb2                as types_pb2

from config.store               import Store
from infra.common.objects       import ObjectDatabase as ObjectDatabase
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions
import infra.config.base        as base
import config.hal.defs          as haldefs
import config.hal.api           as halapi
import crypto_apis_pb2          as crypto_apis_pb2

'''
    HMAC SHA Tests
'''
# HMAC-SHA input parameters
# 
SHA224_params = {'key': '123456', 'keylen': 6, 'data':'My test data', 'datalen':12,
                 'digest': \
                 b'\x96\xa3\x54\x46\x71\xc6\x5d\xaf' \
                 b'\x80\xe6\xfb\xae\xd9\xc1\x5f\xee' \
                 b'\x37\xf9\x83\x5b\x2b\x65\x5c\xc2' \
                 b'\x2b\x70\xab\x3c',
                 'digestlen':28
              }

SHA256_params = {'key': '123456', 'keylen': 6, 'data':'My test data', 'datalen':12,
                 'digest': \
                 b'\xba\xb5\x30\x58\xae\x86\x1a\x7f' \
                 b'\x19\x1a\xbe\x2d\x01\x45\xcb\xb1' \
                 b'\x23\x77\x6a\x63\x69\xee\x3f\x9d' \
                 b'\x79\xce\x45\x56\x67\xe4\x11\xdd',
                 'digestlen':32
              }
 
SHA384_params = {'key': '123456', 'keylen': 6, 'data':'My test data', 'datalen':12,
                 'digest': \
                 b'\x51\x75\xf6\x21\x47\x0a\x03\x98' \
                 b'\x32\xf2\xc2\xf5\x15\x04\xa5\x81' \
                 b'\x32\xc1\x5b\x52\x57\x34\x67\xb8' \
                 b'\x34\x99\x6e\xda\xc3\x11\x05\x72' \
                 b'\xb4\x90\x85\x2b\x0a\xe2\x48\xf2' \
                 b'\xc9\xd7\xb9\x33\xd2\xe1\xda\xdf',
                 'digestlen':48
              }

SHA512_params = {'key': '123456', 'keylen': 6, 'data':'My test data', 'datalen':12,
                 'digest': \
                 b'\xef\x6e\x05\x97\xb0\x91\x70\x20' \
                 b'\xec\xda\x1d\x79\xb1\x9a\xa3\x18' \
                 b'\xca\xf2\xd2\x40\x29\x79\x7c\xca' \
                 b'\xfb\xe5\xbb\x2a\x7d\xbc\x56\x7d' \
                 b'\x2f\x3e\x41\x92\xb3\x2e\xbc\x4e' \
                 b'\xfc\xa8\x56\x07\xd0\x46\xab\xd9' \
                 b'\xd9\xce\x98\x07\xe0\x14\x3f\xb6' \
                 b'\xdd\xca\xb1\xcb\xd4\x30\x81\x2d',
                 'digestlen':64
              }

SHA1_params = {'key': '12345', 'keylen': 5, 'data':'My test data again', 'datalen':18,
               'digest': \
               b'\xa1\x23\x96\xce\xdd\xd2\xa8\x5f' \
               b'\x4c\x65\x6b\xc1\xe0\xaa\x50\xc7' \
               b'\x8c\xff\xde\x3e',
               'digestlen':20
           }

# Hash generate/verify API
def crypto_hash_api_invoke(hashtype, generate, key, key_len, data, data_len, digest, digest_len):

    stub = crypto_apis_pb2.CryptoApisStub(halapi.HalChannel)
    req_msg = crypto_apis_pb2.CryptoApiRequestMsg()
    req_spec = req_msg.request.add()

    if generate:
        req_spec.api_type = crypto_apis_pb2.SYMMAPI_HASH_GENERATE
        req_spec.hash_generate.hashtype = hashtype
        req_spec.hash_generate.key = str.encode(key)
        req_spec.hash_generate.key_len = key_len
        req_spec.hash_generate.data = str.encode(data)
        req_spec.hash_generate.data_len = data_len
        req_spec.hash_generate.digest_len = digest_len
        print("Invoking CryptoAPI: %s-generate" % \
              crypto_apis_pb2.CryptoApiHashType.Name(hashtype))

    else:
        req_spec.api_type = crypto_apis_pb2.SYMMAPI_HASH_VERIFY
        req_spec.hash_verify.hashtype = hashtype
        req_spec.hash_verify.key = str.encode(key)
        req_spec.hash_verify.key_len = key_len
        req_spec.hash_verify.data = str.encode(data)
        req_spec.hash_verify.data_len = data_len
        req_spec.hash_verify.digest = digest
        req_spec.hash_verify.digest_len = digest_len
        print("Invoking CryptoAPI: %s-verify" % \
              crypto_apis_pb2.CryptoApiHashType.Name(hashtype))

    resp_msg = stub.CryptoApiInvoke(req_msg)

    num_resp_specs = len(resp_msg.response)
    if num_resp_specs != 1:
        assert(0)

    resp_spec = resp_msg.response[0]

    if resp_spec.api_status != types_pb2.API_STATUS_OK:
        print ("API Failed")
        return resp_spec.api_status, 0

    if generate:
        return resp_spec.api_status, resp_spec.hash_generate.digest
    else:
        return resp_spec.api_status, req_spec.hash_verify.digest


def symm_crypto_hash_request(hashtype, generate):
    global SHA1_params
    global SHA224_params
    global SHA256_params
    global SHA384_params
    global SHA512_params

    if hashtype == crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA1:
        params = SHA1_params
    elif hashtype == crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA224:
        params = SHA224_params
    elif hashtype == crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA256:
        params = SHA256_params
    elif hashtype == crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA384:
        params = SHA384_params
    elif hashtype == crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA512:
        params = SHA512_params
    else:
        print("FAIL: Unsupported Hash-type %s" % \
              crypto_apis_pb2.CryptoApiHashType.Name(hashtype))
        return False

    ret, output_digest = crypto_hash_api_invoke(hashtype,
                                                generate,
                                                params['key'],
                                                params['keylen'],
                                                params['data'],
                                                params['datalen'],
                                                params['digest'],
                                                params['digestlen'])

    if generate == False:
        if (ret != types_pb2.API_STATUS_OK):
            print("FAIL: API %s-Verify error" % \
                  crypto_apis_pb2.CryptoApiHashType.Name(hashtype))
            return False
        print("Verified digest: %s" % \
              binascii.hexlify(params['digest']))
        print("PASS: API %s-Verify digest accepted!" % \
                  crypto_apis_pb2.CryptoApiHashType.Name(hashtype))
        return True

    if (ret != types_pb2.API_STATUS_OK):
            print("FAIL: API %s-Generate error" % \
                  crypto_apis_pb2.CryptoApiHashType.Name(hashtype))
            return False
    else:
        print("PASS: API %s-Generate succeeded" % \
                  crypto_apis_pb2.CryptoApiHashType.Name(hashtype))
        print("Expected digest: %s" % \
              binascii.hexlify(params['digest']))
        print("Received digest: %s" % \
              binascii.hexlify(output_digest))
        if (output_digest != params['digest']):
            print("FAIL: Digests do not match!");
            return False
        else:
            print("PASS: %s-Generate digests match!" % \
                  crypto_apis_pb2.CryptoApiHashType.Name(hashtype))
            return True


def hmac_sha1_generate_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA1, True)

def hmac_sha1_verify_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA1, False)

def hmac_sha224_generate_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA224, True)

def hmac_sha224_verify_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA224, False)

def hmac_sha256_generate_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA256, True)

def hmac_sha256_verify_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA256, False)

def hmac_sha384_generate_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA384, True)

def hmac_sha384_verify_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA384, False)

def hmac_sha512_generate_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA512, True)

def hmac_sha512_verify_test():
  return symm_crypto_hash_request(crypto_apis_pb2.CRYPTOAPI_HASHTYPE_HMAC_SHA512, False)
  
