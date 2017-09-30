# Testcase definition file.

import pdb
from infra.common.logging       import logger
import types_pb2                as types_pb2

import infra.common.defs as defs

# Need to match defines in tls-constants.h
tls_debug_dol_bypass_barco = 1
tls_debug_dol_sesq_stop = 0x2
tls_debug_dol_bypass_proxy = 0x4
tls_debug_dol_leave_in_arq = 0x8

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
tls_aes128_gcm_explicit_iv = 0x627788b54033b07f
def tls_aes128_gcm_decrypt_setup(tc, tlscb):
    tc.module.logger.info("AES128-GCM Setup:")
    
    # Key Setup
    key_type = types_pb2.CRYPTO_KEY_TYPE_AES128
    key_size = 16
    key = b'\x19\xe4\xa3\x26\xa5\x0a\xf1\x29\x06\x3c\x11\x0c\x7f\x03\xf9\x5e'
    tlscb.crypto_key.Update(key_type, key_size, key)

    # TLS-CB Setup
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
