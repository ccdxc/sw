# Testcase definition file.

import time
import socket
import pdb
import copy
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy

import types_pb2                as types_pb2
import crypto_keys_pb2          as crypto_keys_pb2

from config.objects.proxycb_service       import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb          import TcpCbHelper
from config.objects.proxy_redir_cb        import ProxyrCbHelper
from config.objects.proxy_chain_cb        import ProxycCbHelper
import test.callbacks.networking.modcbs as modcbs
import test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger
import test.app_redir.app_redir_shared as app_redir_shared


def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    modcbs.Teardown(infra, module)
    return

def TestCaseSetup(tc):

    tc.pvtdata = ObjectDatabase(logger)
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

    _proxyrcb_id = app_redir_shared.proxyr_oper_cb_id(app_redir_shared.proxyr_tls_proxy_dir, id)
    ProxyrCbHelper.main(_proxyrcb_id)
    proxyrcbid = "ProxyrCb%04d" % _proxyrcb_id
    # 1. Configure PROXYRCB in HBM before packet injection
    proxyrcb = tc.infra_data.ConfigStore.objects.db[proxyrcbid]
    # let HAL fill in defaults for chain_rxq_base, etc.
    proxyrcb.chain_rxq_base = 0
    proxyrcb.proxyrcb_flags = app_redir_shared.app_redir_dol_pipeline_loopbk_en

    # fill in flow key
    app_redir_shared.proxyrcb_flow_key_build(tc, proxyrcb)
    print("vrf %d flow sport %d dport %d" % 
          (proxyrcb.vrf, proxyrcb.sport, proxyrcb.dport))
    proxyrcb.SetObjValPd()

    brq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"])
    brq.Configure()

    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])

    tlscb.debug_dol = tcp_tls_proxy.tls_debug_dol_bypass_proxy
    tlscb.other_fid = 0xffff
    tlscb.l7_proxy_type = tcp_proxy.l7_proxy_type_REDIR 

    if tc.module.args.key_size == 16:
        tcp_tls_proxy.tls_aes128_decrypt_setup(tc, tlscb)
    elif tc.module.args.key_size == 32:
        tcp_tls_proxy.tls_aes256_decrypt_setup(tc, tlscb)

    _proxyccb_id = app_redir_shared.proxyc_oper_cb_id(app_redir_shared.proxyc_tcp_proxy_dir, id)
    ProxycCbHelper.main(_proxyccb_id)
    proxyccbid = "ProxycCb%04d" % _proxyccb_id
    # 1. Configure PROXYCCB in HBM before packet injection
    proxyccb = tc.infra_data.ConfigStore.objects.db[proxyccbid]
    # let HAL fill in defaults for my_txq_base, etc.
    proxyccb.my_txq_base = 0
    proxyccb.chain_txq_lif = app_redir_shared.service_lif_tcp_proxy
    proxyccb.chain_txq_qtype = 0
    proxyccb.chain_txq_qid = id
    proxyccb.chain_txq_ring = 0
    proxyccb.proxyccb_flags = app_redir_shared.app_redir_chain_desc_add_aol_offset | \
                              app_redir_shared.app_redir_dol_skip_chain_doorbell
    proxyccb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.Configure()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.Configure()
    rnmpr_small = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"])
    rnmpr_small.Configure()

    proxyrcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[proxyrcbid])
    proxyrcb.GetObjValPd()
    proxyccb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[proxyccbid])
    proxyccb.GetObjValPd()
    arq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["ARQ"])
    arq.Configure()
    proxyccbq = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["PROXYCCBQ"])
    proxyccbq.Configure()
    
    tc.pvtdata.Add(tlscb)
    tc.pvtdata.Add(rnmdr)
    tc.pvtdata.Add(rnmpr)
    tc.pvtdata.Add(rnmpr_small)
    tc.pvtdata.Add(arq)
    tc.pvtdata.Add(brq)
    tc.pvtdata.Add(proxyrcb)
    tc.pvtdata.Add(proxyccb)
    tc.pvtdata.Add(proxyccbq)
    return

def TestCaseVerify(tc):

    num_pkts = 1
    if hasattr(tc.module.args, 'num_pkts'):
        num_pkts = int(tc.module.args.num_pkts)

    proxyr_meta_pages = 0
    if hasattr(tc.module.args, 'proxyr_meta_pages'):
        proxyr_meta_pages = int(tc.module.args.proxyr_meta_pages)

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    tlscbid = "TlsCb%04d" % id
    tlscb = tc.pvtdata.db[tlscbid]

    # Verify chain_rxq_base
    _proxyrcb_id = app_redir_shared.proxyr_oper_cb_id(app_redir_shared.proxyr_tls_proxy_dir, id)
    proxyrcbid = "ProxyrCb%04d" % _proxyrcb_id
    proxyrcb = tc.pvtdata.db[proxyrcbid]
    proxyrcb_cur = tc.infra_data.ConfigStore.objects.db[proxyrcbid]
    proxyrcb_cur.GetObjValPd()
    if proxyrcb_cur.chain_rxq_base == 0:
        print("chain_rxq_base not set")
        return False

    print("chain_rxq_base value post-sync from HBM 0x%x" % proxyrcb_cur.chain_rxq_base)

    # Verify my_txq_base
    _proxyccb_id = app_redir_shared.proxyc_oper_cb_id(app_redir_shared.proxyc_tcp_proxy_dir, id)
    proxyccbid = "ProxycCb%04d" % _proxyccb_id
    proxyccb = tc.pvtdata.db[proxyccbid]
    proxyccb_cur = tc.infra_data.ConfigStore.objects.db[proxyccbid]
    proxyccb_cur.GetObjValPd()
    if proxyccb_cur.my_txq_base == 0:
        print("my_txq_base not set")
        return False

    print("my_txq_base value post-sync from HBM 0x%x" % proxyccb_cur.my_txq_base)

    # Fetch current values from Platform
    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    rnmpr_small = tc.pvtdata.db["RNMPR_SMALL"]
    brq = tc.pvtdata.db["BRQ_ENCRYPT"]
    arq = tc.pvtdata.db["ARQ"]
    proxyccbq = tc.pvtdata.db["PROXYCCBQ"]

    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.Configure()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.Configure()
    rnmpr_small_cur = tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"]
    rnmpr_small_cur.Configure()
    arq_cur = tc.infra_data.ConfigStore.objects.db["ARQ"]
    arq_cur.Configure()
    proxyccbq_cur = tc.infra_data.ConfigStore.objects.db["PROXYCCBQ"]
    proxyccbq_cur.Configure()

    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb_cur.GetObjValPd()

    brq_cur = tc.infra_data.ConfigStore.objects.db["BRQ_ENCRYPT"]
    brq_cur.Configure()

    # Verify PI for RNMDR got incremented
    if (rnmdr_cur.pi != rnmdr.pi+num_pkts):
        print("RNMDR pi check failed old %d new %d expected %d" %
                     (rnmdr.pi, rnmdr_cur.pi, rnmdr.pi+num_pkts))
        return False
    print("RNMDR pi old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))

    # Verify PI for RNMPR or RNMPR_SMALL got incremented
    if ((rnmpr_cur.pi+rnmpr_small_cur.pi) != (rnmpr.pi+rnmpr_small.pi+num_pkts+proxyr_meta_pages)):
        print("RNMPR pi check failed old %d new %d expected %d" %
                  (rnmpr.pi+rnmpr_small.pi, rnmpr_cur.pi+rnmpr_small_cur.pi,
                   rnmpr.pi+rnmpr_small.pi+num_pkts+proxyr_meta_pages))
        return False
    print("RNMPR pi old %d new %d" % (rnmpr.pi, rnmpr_cur.pi))
    print("RNMPR_SMALL old %d new %d" % (rnmpr_small.pi, rnmpr_small_cur.pi))

    # Rx: verify PI for ARQ got incremented
    if (arq_cur.pi != arq.pi+num_pkts):
        print("ARQ pi check failed old %d new %d expected %d" %
                   (arq.pi, arq_cur.pi, arq.pi+num_pkts))
        #return False
    print("ARQ pi old %d new %d" % (arq.pi, arq_cur.pi))

    # Tx: verify PI for PROXYCCB got incremented
    time.sleep(1)
    proxyccb_cur.GetObjValPd()
    if (proxyccb_cur.pi != proxyccb.pi+num_pkts):
        print("PROXYCCB pi check failed old %d new %d expected %d" %
                      (proxyccb.pi, proxyccb_cur.pi, proxyccb.pi+num_pkts))
        return False
    print("PROXYCCB pi old %d new %d" % (proxyccb.pi, proxyccb_cur.pi))

    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    modcbs.TestCaseTeardown(tc)
    return

def TestCaseStepSetup(tc, step):
    return modcbs.TestCaseStepSetup(tc, step)

def TestCaseStepTrigger(tc, step):
    return modcbs.TestCaseStepTrigger(tc, step)

def TestCaseStepVerify(tc, step):
    return modcbs.TestCaseStepVerify(tc, step)

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)

