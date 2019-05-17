#! /usr/bin/python3

import pdb
import math
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs
from infra.common.glopts import GlobalOptions
from iris.test.callbacks.common.pktslicer import *
import binascii
from random import *
import iris.config.resmgr            as resmgr
import os
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend

def GetPrp2Offset (tc, pkt, args):
    return (tc.config.nvmesession.lif.spec.host_page_size) - args.nentries * 8

def GetDataSize (tc, pkt, args):
    return (tc.config.nvmesession.ns.lba_size) * args.nlb

def PickPktTemplate(tc, pkt, args = None):
    if tc.config.nvmesession.session.IsIPV6():
       template = args.v6
    else:
       template = args.v4
    logger.info("ChoosePacketTemplateByFlow: Picking %s" % (template))
    return infra_api.GetPacketTemplate(template)

# args - 0 based pkt_id
def __get_pkt_payload_size(tc, pkt, args):
    basepkt = tc.packets.Get(args.basepkt)
    total_payload_size = basepkt.payloadsize
    if total_payload_size <= tc.config.nvmesession.mtu:
       assert args.pkt_id == 0
       return total_payload_size
    pkt_payload_size = total_payload_size - args.pkt_id * tc.config.nvmesession.mtu
    assert pkt_payload_size > 0
    if pkt_payload_size > tc.config.nvmesession.mtu:
       pkt_payload_size = tc.config.nvmesession.mtu
    return pkt_payload_size

def GetPktPayloadSize(tc, pkt, args): 
    pkt_payload_size = __get_pkt_payload_size(tc, pkt, args)
    assert pkt_payload_size % tc.config.nvmesession.ns.lba_size == 0
    return pkt_payload_size

def xts_encrypt(iv, key, plaintext):
    cipher = Cipher(algorithms.AES(key), modes.XTS(iv), backend=default_backend())
    encryptor = cipher.encryptor()
    ciphertext = encryptor.update(plaintext) + encryptor.finalize()
    return ciphertext

def xts_decrypt(iv, key, ciphertext):
    cipher = Cipher(algorithms.AES(key), modes.XTS(iv), backend=default_backend())
    decryptor = cipher.decryptor()
    decodetext = decryptor.update(ciphertext) + decryptor.finalize()
    return decodetext

def encrypt_lba(tc, pkt, args):
    key = tc.config.nvmesession.ns.crypto_key.key[0:tc.config.nvmesession.ns.crypto_key.key_size*2]
    lba_num = (args.lba_num << 64)
    iv = lba_num.to_bytes(16, byteorder='big')
    pkt = tc.packets.Get(args.pktid)
    data = bytearray(pkt.headers.payload.fields.data)
    cipher_text = xts_encrypt(iv, key, data)
    return list(cipher_text)

def decrypt_lba(tc, pkt, args):
    key = tc.config.nvmesession.ns.crypto_key.key[0:tc.config.nvmesession.ns.crypto_key.key_size*2]
    lba_num = (args.lba_num << 64)
    iv = lba_num.to_bytes(16, byteorder='big')
    pkt = tc.packets.Get(args.pktid)
    data = bytearray(pkt.headers.payload.fields.data)
    plain_text = xts_decrypt(iv, key, data)
    return list(plain_text)
