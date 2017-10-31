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
import config.hal.defs          as haldefs
import config.hal.api           as halapi
import crypto_apis_pb2          as crypto_apis_pb2

import crypto_keys_pb2      as crypto_keys_pb2

# ECC P256

p = \
    b'\xFF\xFF\xFF\xFF\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF'
a = \
    b'\xFF\xFF\xFF\xFF\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFC'
b = \
    b'\x5A\xC6\x35\xD8\xAA\x3A\x93\xE7\xB3\xEB\xBD\x55\x76\x98\x86\xBC\x65\x1D\x06\xB0\xCC\x53\xB0\xF6\x3B\xCE\x3C\x3E\x27\xD2\x60\x4B'
Gx = \
    b'\x6B\x17\xD1\xF2\xE1\x2C\x42\x47\xF8\xBC\xE6\xE5\x63\xA4\x40\xF2\x77\x03\x7D\x81\x2D\xEB\x33\xA0\xF4\xA1\x39\x45\xD8\x98\xC2\x96'
Gy = \
    b'\x4F\xE3\x42\xE2\xFE\x1A\x7F\x9B\x8E\xE7\xEB\x4A\x7C\x0F\x9E\x16\x2B\xCE\x33\x57\x6B\x31\x5E\xCE\xCB\xB6\x40\x68\x37\xBF\x51\xF5'
n = \
    b'\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xBC\xE6\xFA\xAD\xA7\x17\x9E\x84\xF3\xB9\xCA\xC2\xFC\x63\x25\x51'
k = \
    b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x09'
exp_qx = \
    b'\xEA\x68\xD7\xB6\xFE\xDF\x0B\x71\x87\x89\x38\xD5\x1D\x71\xF8\x72\x9E\x0A\xCB\x8C\x2C\x6D\xF8\xB3\xD7\x9E\x8A\x4B\x90\x94\x9E\xE0'
exp_qy = \
    b'\x2A\x27\x44\xC9\x72\xC9\xFC\xE7\x87\x01\x4A\x96\x4A\x8E\xA0\xC8\x4D\x71\x4F\xEA\xA4\xDE\x82\x3F\xE8\x5A\x22\x4A\x4D\xD0\x48\xFA'
computed_qx = 0
computed_qy = 0

def ecc_point_mul_p256(p, n, a, b, gx, gy, k):

    stub = crypto_apis_pb2.CryptoApisStub(halapi.HalChannel)
    req_msg = crypto_apis_pb2.CryptoApiRequestMsg()
    req_spec = req_msg.request.add()

    req_spec.api_type = crypto_apis_pb2.ASYMAPI_ECC_POINT_MUL_FP
    req_spec.ecc_point_mul_fp.ecc_domain_params.KeySize = 32
    req_spec.ecc_point_mul_fp.ecc_domain_params.p = p
    req_spec.ecc_point_mul_fp.ecc_domain_params.a = a
    req_spec.ecc_point_mul_fp.ecc_domain_params.b = b
    req_spec.ecc_point_mul_fp.ecc_domain_params.G.x = gx
    req_spec.ecc_point_mul_fp.ecc_domain_params.G.y = gy
    req_spec.ecc_point_mul_fp.ecc_domain_params.n = n

    req_spec.ecc_point_mul_fp.k = k

    print("Invoking CryptoAPI: ECC Point Multiplication P-256");
    resp_msg = stub.CryptoApiInvoke(req_msg)

    num_resp_specs = len(resp_msg.response)
    if num_resp_specs != 1:
        assert(0)

    resp_spec = resp_msg.response[0]

    if resp_spec.api_status == types_pb2.API_STATUS_OK:
        return resp_spec.api_status, resp_spec.ecc_point_mul_fp.Q.x, \
        resp_spec.ecc_point_mul_fp.Q.y
    else:
        print ("API Failed")
        return resp_spec.api_status, 0, 0



def Setup(infra, module):

    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    return

def TestCaseVerify(tc):
    global p
    global a
    global b
    global Gx
    global Gy
    global n
    global  exp_qx
    global  exp_qy
    global computed_qx
    global computed_qy


    computed_qx = 0
    computed_qy = 0
    if halapi.IsHalDisabled():
        return True

    ret, computed_qx, computed_qy  = ecc_point_mul_p256(p, n, a, b, Gx, Gy, k)

    if (computed_qx == exp_qx) and (computed_qy == exp_qy):
        print ("Response Matched:")
        print("Qx: %s" % binascii.hexlify(computed_qx))
        print("Qy: %s" % binascii.hexlify(computed_qy))
        return True
    else:
        return False

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
