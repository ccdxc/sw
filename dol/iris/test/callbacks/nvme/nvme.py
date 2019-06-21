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

def GetPrp2Offset (tc, pkt, args):
    return (tc.config.nvmens.lif.spec.host_page_size) - args.nentries * 8

def PickPktTemplate(tc, pkt, args = None):
    if tc.config.nvmesession.session.IsIPV6():
       template = args.v6
    else:
       template = args.v4
    logger.info("ChoosePacketTemplateByFlow: Picking %s" % (template))
    return infra_api.GetPacketTemplate(template)

# args - 0 based pkt_id
def __get_pkt_payload_size_n_offset(tc, pkt_id):
    total_size = tc.pvtdata.capsulecmd_len
    if total_size <= tc.config.nvmesession.mtu:
        assert pkt_id == 0
        logger.info('pkt_id: %d size: %d offset: %d' %(pkt_id, total_size, 0))
        return total_size, 0
    
    offset = pkt_id * tc.config.nvmesession.mtu

    #pkt_id 0 would have shipped header bytes as well
    if pkt_id != 0:
        offset = offset + tc.pvtdata.capsulecmd_pdo

    size = total_size - offset
    assert size > 0

    #if remaining size without ddgst is more than mtu, ship only mtu bytes
    if (size-4) > tc.config.nvmesession.mtu:
        size = tc.config.nvmesession.mtu

    #pkt_id 0 should ship header bytes as well
    if pkt_id == 0:
        size = size + tc.pvtdata.capsulecmd_pdo
    
    logger.info('pkt_id: %d size: %d offset: %d' %(pkt_id, size, offset))
    return size, offset

def GetPktPayloadSize(tc, pkt, args): 
    size, _ = __get_pkt_payload_size_n_offset(tc, args.pkt_id)
    return size

def GetPktPayloadBytes(tc, pkt, args):
    size, offset = __get_pkt_payload_size_n_offset(tc, args.pkt_id)
    return list(tc.pvtdata.capsulecmd_bytes[offset:(offset+size)])

def GetScalePktPayloadSize(tc, pkt): 
    logger.info('GetScalePktPayloadSize called with pkt_id: %d' %tc.pvtdata.curr_pkt_id)
    size, _ = __get_pkt_payload_size_n_offset(tc, tc.pvtdata.curr_pkt_id)
    return size

def GetScalePktPayloadBytes(tc, pkt):
    logger.info('GetScalePktPayloadBytes called with pkt_id: %d' %tc.pvtdata.curr_pkt_id)
    size, offset = __get_pkt_payload_size_n_offset(tc, tc.pvtdata.curr_pkt_id)
    return list(tc.pvtdata.capsulecmd_bytes[offset:(offset+size)])
