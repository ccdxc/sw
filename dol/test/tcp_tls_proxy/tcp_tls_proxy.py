# Testcase definition file.

import pdb
from infra.common.logging       import logger
import types_pb2                as types_pb2

import infra.common.defs as defs

# Need to match defines in tls-constants.h
tls_debug_dol_bypass_barco = 0x1
tls_debug_dol_sesq_stop = 0x2
tls_debug_dol_bypass_proxy = 0x4
tls_debug_dol_leave_in_arq = 0x8
tls_debug_dol_arm_loop_ctlr_pkts = 0x10
tls_debug_dol_explicit_iv_use_random = 0x20

#  HACK: used until the issue with payload generation is fixed in DoL framework
tls_debug_dol_fake_handshake_msg = 0x80


def tcp_inorder_rx_verify (fwdata, usrdata):
    # usrdata will have initial and expected object usrdata.initial and
    # usrdata.expected. Potentially usrdata.final can also be there (after
    # reading hw)
    return True

def tls_cb_verify (fwdata, usrdata):
    return True

# NOTE: this should match the payload from  PAYLOAD_TLS_AES128_GCM_RECORD payload definition
# This is already part of the actual payload

tls_aes128_gcm_explicit_iv = 0x627788b54033b07f
def tls_aes128_decrypt_setup(tc, tlscb):
    tc.module.logger.info("AES128-GCM Decrypt Setup:")
    
    # Key Setup
    key_type = types_pb2.CRYPTO_KEY_TYPE_AES128
    key_size = 16
    key = b'\x19\xe4\xa3\x26\xa5\x0a\xf1\x29\x06\x3c\x11\x0c\x7f\x03\xf9\x5e'
    tlscb.crypto_key.Update(key_type, key_size, key)

    # TLS-CB Setup
    if tc.module.args.cipher_suite == "CCM":
        tlscb.command = 0x05100c10
    else:
        tlscb.command = 0x30100000

    tlscb.crypto_key_idx = tlscb.crypto_key.keyindex
    # Salt: 0xaf 0x98 0xc5 0xe2
    tlscb.salt = 0xe2c598af

    # IV: 
    tlscb.explicit_iv = 0x0100000000000000
    tlscb.is_decrypt_flow = True
    tlscb.SetObjValPd()

    print("IS Decrypt Flow: %s" % tlscb.is_decrypt_flow)
    return

def tls_aes128_encrypt_setup(tc, tlscb):
    tc.module.logger.info("AES128-GCM Encrypt Setup:")
    
    # Key Setup
    key_type = types_pb2.CRYPTO_KEY_TYPE_AES128
    key_size = 16
    key = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    tlscb.crypto_key.Update(key_type, key_size, key)

    # TLS-CB Setup
    if tc.module.args.cipher_suite == "CCM":
        tlscb.command = 0x05000c10
    else:
        tlscb.command = 0x30000000

    tlscb.crypto_key_idx = tlscb.crypto_key.keyindex
    tlscb.salt = 0x12345678
    # IV: 
    tlscb.explicit_iv = 0x0100000000000000
    tlscb.is_decrypt_flow = False
    tlscb.SetObjValPd()
    print("IS Decrypt Flow: %s" % tlscb.is_decrypt_flow)
    return

# Explicit IV : 0x94 0xcb 0x98 0x62 0x80 0xff 0xdb 0x23
tls_aes256_gcm_explicit_iv = 0x23dbff806298cb94
def tls_aes256_decrypt_setup(tc, tlscb):
    tc.module.logger.info("AES256-GCM Decrypt Setup:")
    
    # Key Setup
    key_type = types_pb2.CRYPTO_KEY_TYPE_AES256
    key_size = 32
    key = b'\x30\xdb\x63\x16\x2b\x2f\x4e\xb6\xce\x4b\xbd\x21\x7e\xf7\x64\xc6\xdb\xfb\xf9\xec\x3b\x83\x8b\xa9\x19\xf8\xfd\xb8\x81\xbd\x9a\xc3'
    tlscb.crypto_key.Update(key_type, key_size, key)

    # TLS-CB Setup
    if tc.module.args.cipher_suite == "CCM":
        tlscb.command = 0x05100c10
    else:
        tlscb.command = 0x30100000

    tlscb.crypto_key_idx = tlscb.crypto_key.keyindex
    # Salt: 0x1a 0x8c 0x86 0x99
    tlscb.salt = 0x99868c1a

    # IV: 
    tlscb.explicit_iv = 0x0100000000000000
    tlscb.is_decrypt_flow = True
    tlscb.SetObjValPd()

    print("IS Decrypt Flow: %s" % tlscb.is_decrypt_flow)
    return

def tls_aes256_encrypt_setup(tc, tlscb):
    tc.module.logger.info("AES256-GCM Encrypt Setup:")
    
    # Key Setup
    key_type = types_pb2.CRYPTO_KEY_TYPE_AES256
    key_size = 32
    key = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    tlscb.crypto_key.Update(key_type, key_size, key)

    # TLS-CB Setup
    if tc.module.args.cipher_suite == "CCM":
        tlscb.command = 0x05000c10
    else:
        tlscb.command = 0x30000000

    tlscb.crypto_key_idx = tlscb.crypto_key.keyindex
    tlscb.salt = 0x12345678

    # IV: 
    tlscb.explicit_iv = 0x0100000000000000
    tlscb.is_decrypt_flow = False
    tlscb.SetObjValPd()
    print("IS Decrypt Flow: %s" % tlscb.is_decrypt_flow)
    return



def tls_explicit_iv(key_size):
    if key_size == 16:
        return tls_aes128_gcm_explicit_iv
    elif key_size == 32:
        return tls_aes256_gcm_explicit_iv
