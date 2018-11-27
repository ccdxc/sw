# Testcase definition file.

import pdb
import copy
import iris.test.tcp_tls_proxy.tcp_proxy as tcp_proxy

import types_pb2                as types_pb2
import internal_pb2          as internal_pb2

from iris.config.store                       import Store
from iris.config.objects.proxycb_service     import ProxyCbServiceHelper
from iris.config.objects.tcp_proxy_cb        import TcpCbHelper
import iris.test.callbacks.networking.modcbs as modcbs
import iris.test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger
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
    # set tcb state to ESTABLISHED(1)
    tcb.state = 1
    tcb.l7_proxy_type = 0
    tcb.debug_dol = 0
    tcb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdpr_big = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"])
    rnmdpr_big.GetMeta()
    tnmdpr_big = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDPR_BIG"])
    #tnmdpr_big.GetMeta()

    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_GCM"])
    brq.GetMeta()

    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])

    tlscb.debug_dol = tcp_tls_proxy.tls_debug_dol_bypass_proxy | \
                            tcp_tls_proxy.tls_debug_dol_sesq_stop
    tlscb.other_fid = 0xffff
    tlscb.l7_proxy_type = tcp_proxy.l7_proxy_type_SPAN 
    tlscb.serq_pi = 0
    tlscb.serq_ci = 0

    if tc.module.args.key_size == 16:
        tcp_tls_proxy.tls_aes128_decrypt_setup(tc, tlscb)
    elif tc.module.args.key_size == 32:
        tcp_tls_proxy.tls_aes256_decrypt_setup(tc, tlscb)

    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdpr_big)
    tc.pvtdata.Add(tnmdpr_big)
    tc.pvtdata.Add(brq)

    return

def TestCaseVerify(tc):
    if GlobalOptions.dryrun:
        return True

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)


    # Retrieve saved state
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.pvtdata.db[tlscbid]
    rnmdpr_big = tc.pvtdata.db["RNMDPR_BIG"]
    tnmdpr_big = tc.pvtdata.db["TNMDPR_BIG"]
    brq = tc.pvtdata.db["BRQ_ENCRYPT_GCM"]

    #  Fetch current values from Platform
    rnmdpr_big_cur = tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"]
    rnmdpr_big_cur.GetMeta()

    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.GetMeta()

    tnmdpr_big_cur = tc.infra_data.ConfigStore.objects.db["TNMDPR_BIG"]
    #tnmdpr_big_cur.GetMeta()


    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb_cur.GetObjValPd()

    brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT_GCM"]
    #brq_cur.GetMeta()

    # Verify PI for RNMDPR_BIG got incremented by 1
    if (rnmdpr_big_cur.pi != rnmdpr_big.pi+2):
        print("RNMDPR_BIG pi check failed old %d new %d" % (rnmdpr_big.pi, rnmdpr_big_cur.pi))
        return False
    print("Old RNMDPR_BIG PI: %d, New RNMDPR_BIG PI: %d" % (rnmdpr_big.pi, rnmdpr_big_cur.pi))

    # Verify page
    #if rnmpr.ringentries[0].handle != brq_cur.swdre_list[0].Addr1:
    #    print("Page handle not as expected in brq_cur.swdre_list")
        #return False

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
