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
