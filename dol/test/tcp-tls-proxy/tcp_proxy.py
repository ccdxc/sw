# Testcase definition file.

import pdb

from config.store               import Store


def Setup(infra, module):
    print("Setup(): Sample Implementation")
    cb = infra.ConfigStore.objects.db["TcpCb0000"]
    cb.rcv_nxt = 0xbabababa
    cb.SetObjValPd()
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    print("TestCaseSetup(): Sample Implementation.")
    return

def TestCaseVerify(tc):
    cb = tc.infra_data.ConfigStore.objects.db["TcpCb0000"]
    print("rcv_nxt value pre-sync from HBM 0x%x" % cb.rcv_nxt)
    cb.GetObjValPd()
    print("rcv_nxt value post-sync from HBM 0x%x" % cb.rcv_nxt)

    if cb.rcv_nxt is '0xBABABAFA':
        print("Returning true")
        return True
    else:
        print("Returning false")
        return False

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
