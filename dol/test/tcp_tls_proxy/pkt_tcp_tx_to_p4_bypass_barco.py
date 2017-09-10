# Testcase definition file.

import pdb
import copy

from config.store               import Store

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db["TcpCb0000"]
    tcb.rcv_nxt = 0xBABABABA
    tcb.snd_nxt = 0xEFEFEFF0
    tcb.snd_una = 0xEFEFEFEF
    tcb.rcv_tsval = 0xFAFAFAFA
    tcb.ts_recent = 0xFAFAFAF0
    tcb.debug_dol = 0
    tcb.SetObjValPd()

    # 2. Configure TLS CB in HBM before packet injection
    tlscb = tc.infra_data.ConfigStore.objects.db["TlsCb0000"]
    tlscb.debug_dol = 1
    tlscb.SetObjValPd()

    return

def TestCaseVerify(tc):
    # 1. Verify pi/ci got update got updated
    tcpcb_cur = tc.infra_data.ConfigStore.objects.db["TcpCb0000"]
    old_sesq_pi = tcpcb_cur.sesq_pi
    old_sesq_ci = tcpcb_cur.sesq_ci
    tcpcb_cur.GetObjValPd()
    if (tcpcb_cur.sesq_pi != old_sesq_pi + 1 or tcpcb_cur.sesq_ci != old_sesq_ci + 1):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (old_sesq_pi, old_sesq_ci,
                 tcpcb_cur.sesq_pi, tcpcb_cur.sesq_ci))
        return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
