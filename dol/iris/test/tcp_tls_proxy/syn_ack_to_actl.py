# Testcase definition file.

import pdb
import copy

from iris.config.store               import Store
from iris.config.objects.proxycb_service    import ProxyCbServiceHelper
from iris.config.objects.tcp_proxy_cb        import TcpCbHelper
from infra.common.objects import ObjectDatabase as ObjectDatabase
import iris.test.callbacks.networking.modcbs as modcbs
from infra.common.logging import logger
from iris.config.objects.cpucb        import CpuCbHelper
import iris.test.tcp_tls_proxy.tcp_proxy as tcp_proxy
import iris.test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy
from infra.common.glopts import GlobalOptions

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    tc.pvtdata = ObjectDatabase()
    tcp_proxy.SetupProxyArgs(tc)

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    TcpCbHelper.main(id)
    tcbid = "TcpCb%04d" % id
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcp_proxy.init_tcb_inorder(tc, tcb)
    tcb.debug_dol |= tcp_proxy.tcp_debug_dol_leave_in_arq
    # set tcb state to SYN_SENT(2)
    tcb.state = tcp_proxy.tcp_state_SYN_SENT
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdpr_big = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"])
    rnmdpr_big.GetMeta()
    rnmdpr_big.GetRingEntries([rnmdpr_big.pi])
    tnmdpr_big = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDPR_BIG"])
    tnmdpr_big.GetMeta()
    actl = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["CPU0000_TCP_ACTL"])
    actl.GetMeta()

    sesqid = "TCPCB%04d_SESQ" % id
    sesq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[sesqid])
    tlscbid = "TlsCb%04d" % id
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb_cur.debug_dol = tcp_tls_proxy.tls_debug_dol_bypass_barco
    tlscb_cur.SetObjValPd()
    tlscb = copy.deepcopy(tlscb_cur)
    tcpcb = copy.deepcopy(tcb)

    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdpr_big)
    tc.pvtdata.Add(tnmdpr_big)
    tc.pvtdata.Add(tcpcb)
    tc.pvtdata.Add(sesq)
    tc.pvtdata.Add(actl)
    return

def TestCaseVerify(tc):
    if GlobalOptions.dryrun:
        return True

    rnmdpr_big = tc.pvtdata.db["RNMDPR_BIG"]
    actl = tc.pvtdata.db["CPU0000_TCP_ACTL"]

    # 1. Fetch current values from Platform
    rnmdpr_big_cur = tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"]
    rnmdpr_big_cur.GetMeta()
    actl_cur = tc.infra_data.ConfigStore.objects.db["CPU0000_TCP_ACTL"]
    actl_cur.GetMeta()
    actl_cur.GetRingEntries([actl.pi])
    actl_cur.GetRingEntryAOL([0])

    print("actl pi 0x%x" % (actl.pi))
    print("actl cur pi 0x%x" % (actl_cur.pi))
    # 2. Verify descriptor
    if rnmdpr_big.ringentries[rnmdpr_big.pi].handle != actl_cur.ringentries[actl.pi].GetHandle():
    #if rnmdpr.ringentries[rnmdpr.pi].handle != actl_cur.ringentries[0].handle:
        print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdpr_big.ringentries[rnmdpr_big.pi].handle, actl_cur.ringentries[actl.pi].handle))
        #print("Descriptor handle not as expected in ringentries 0x%x 0x%x" % (rnmdpr.ringentries[rnmdpr.pi].handle, actl_cur.ringentries[actl.pi].handle))
        return False

    if ((rnmdpr_big.ringentries[rnmdpr_big.pi].handle is not None) and (actl_cur.ringentries[actl.pi].handle is not None)):
        print("Descriptor handle as expected in ringentries 0x%x 0x%x" % (rnmdpr_big.ringentries[rnmdpr_big.pi].handle, actl_cur.ringentries[actl.pi].handle))

    return True


def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
