# Testcase definition file.

import pdb

from config.store               import Store
from infra.common.logging       import logger
import infra.common.defs as defs
import config.hal.api           as halapi


def Setup(infra, module):
    print("Setup(): Sample Implementation")
    #if module.args:
    #    for arg in module.args:
    #        print("- Arg: ", arg)
    lst = []
    #cb = fwdata.CfgObjectStore.objects.db["TcpCb0001"]
    cb = infra.ConfigStore.objects.db["TcpCb0001"]
    logger.info("my data %d" % cb.id)
    cb.rcv_nxt = 0xbabababa
    lst.append(cb)
    halapi.UpdateTcpCbs(lst)
    #return defs.status.SUCCESS
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    print("TestCaseSetup(): Sample Implementation.")
    return

def TestCaseVerify(tc):
    cb = tc.infra_data.ConfigStore.objects.db["TcpCb0001"]
    logger.info("TestCaseVerify(): Sample Implementation my implementation. 0x%x" % cb.rcv_nxt)
    return

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
