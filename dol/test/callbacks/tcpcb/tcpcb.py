#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs

def mycallback (tc, pkt):
    return 999

def mycallback_verify (tc, dummy):
    logger.info("mycallback_verify!!!")
    return True, "Passed"

