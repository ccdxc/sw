# Testcase definition file.

import pdb

from config.store               import Store
from infra.common.logging       import logger
import infra.common.defs as defs
import config.hal.api           as halapi

def tcp_inorder_rx_verify (fwdata, usrdata):
    # usrdata will have initial and expected object usrdata.initial and
    # usrdata.expected. Potentially usrdata.final can also be there (after
    # reading hw)
    #print(fwdata.CfgObjectStore.objects.db["Session1"])
    return True

def tls_cb_verify (fwdata, usrdata):
    return True

def setup (fwdata, usrdata):
    lst = []
    cb = fwdata.CfgObjectStore.objects.db["TcpCb0001"]
    logger.info("my data %d" % cb.id)
    cb.rcv_nxt = 999
    lst.append(cb)
    #halapi.ConfigureTcpCbs(lst)
    return defs.status.SUCCESS

def teardown (fwdata, usrdata):
    return defs.status.SUCCESS
