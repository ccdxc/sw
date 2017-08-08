#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs

def mycallback (tc, pkt):
    return 999
    #tc.fwdata.CfgObjectStore.objects.db["TcpCb0001"].rcv_nxt
    #return defs.status.SUCCESS


def mycallback2 (tc, pkt):
    logger.info("mycallback2!!!");
    return 0

def mycallback_verify (tc, dummy):
    logger.info("mycallback_verify!!!");
    return True, "Passed"

