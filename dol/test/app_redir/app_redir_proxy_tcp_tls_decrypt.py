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

rnmdr = 0
rnmpr = 0
rnmpr_small = 0
proxyrcbid = ""
proxyccbid = ""
proxyrcb = 0
proxyccb = 0
redir_span = False

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    modcbs.Teardown(infra, module)
    return

def TestCaseSetup(tc):
    global rnmdr
    global rnmpr
    global rnmpr_small
    global proxyrcbid
    global proxyccbid
    global proxyrcb
    global proxyccb
    global redir_span

    redir_span = getattr(tc.module.args, 'redir_span', False)

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
    if redir_span:
        tcb.debug_dol_tx |= tcp_proxy.tcp_tx_debug_dol_dont_tx

    tcb.SetObjValPd()

    _proxyrcb_id = id
    ProxyrCbHelper.main(_proxyrcb_id)
    proxyrcbid = "ProxyrCb%04d" % _proxyrcb_id
    # 1. Configure PROXYRCB in HBM before packet injection
    proxyrcb = tc.infra_data.ConfigStore.objects.db[proxyrcbid]
    # let HAL fill in defaults for chain_rxq_base, etc.
    proxyrcb.my_txq_base = 0
    proxyrcb.chain_rxq_base = 0
    proxyrcb.redir_span = redir_span
    proxyrcb.proxyrcb_flags = app_redir_shared.app_redir_dol_pipeline_loopbk_en

    # fill in flow key
    proxyrcb.FlowKeyBuild(tc.config.flow)
    print("vrf %d flow sport %d dport %d" % 
          (proxyrcb.vrf, proxyrcb.sport, proxyrcb.dport))
    proxyrcb.SetObjValPd()

    tlscbid = "TlsCb%04d" % id
    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])

    tlscb.debug_dol = tcp_tls_proxy.tls_debug_dol_bypass_proxy
    tlscb.other_fid = 0xffff
    tlscb.l7_proxy_type = tcp_proxy.l7_proxy_type_REDIR
    if redir_span:
        tlscb.l7_proxy_type = tcp_proxy.l7_proxy_type_SPAN

    if tc.module.args.key_size == 16:
        tcp_tls_proxy.tls_aes128_decrypt_setup(tc, tlscb)
    elif tc.module.args.key_size == 32:
        tcp_tls_proxy.tls_aes256_decrypt_setup(tc, tlscb)

    _proxyccb_id = id
    ProxycCbHelper.main(_proxyccb_id)
    proxyccbid = "ProxycCb%04d" % _proxyccb_id
    # 1. Configure PROXYCCB in HBM before packet injection
    proxyccb = tc.infra_data.ConfigStore.objects.db[proxyccbid]
    # let HAL fill in defaults for my_txq_base, etc.
    proxyccb.redir_span = redir_span
    proxyccb.my_txq_base = 0
    proxyccb.chain_txq_base = 0
    proxyccb.chain_txq_lif = app_redir_shared.service_lif_tcp_proxy
    proxyccb.chain_txq_qtype = 0
    proxyccb.chain_txq_qid = id
    proxyccb.chain_txq_ring = 0
    proxyccb.proxyccb_flags = app_redir_shared.app_redir_chain_desc_add_aol_offset | \
                              app_redir_shared.app_redir_dol_skip_chain_doorbell
    proxyccb.SetObjValPd()

    # 2. Clone objects that are needed for verification
    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.GetMeta()
    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.GetMeta()
    rnmpr_small = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"])
    rnmpr_small.GetMeta()

    proxyrcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[proxyrcbid])
    proxyrcb.GetObjValPd()
    proxyccb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[proxyccbid])
    proxyccb.GetObjValPd()
    
    return

def TestCaseVerify(tc):
    global rnmdr
    global rnmpr
    global rnmpr_small
    global proxyrcbid
    global proxyccbid
    global proxyrcb
    global proxyccb
    global redir_span

    num_pkts = 1
    if hasattr(tc.module.args, 'num_pkts'):
        num_pkts = int(tc.module.args.num_pkts)

    proxyr_meta_pages = 0
    if hasattr(tc.module.args, 'proxyr_meta_pages'):
        proxyr_meta_pages = int(tc.module.args.proxyr_meta_pages)

    num_flow_miss_pkts = 0
    if hasattr(tc.module.args, 'num_flow_miss_pkts'):
        num_flow_miss_pkts = int(tc.module.args.num_flow_miss_pkts)

    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)

    # Verify chain_rxq_base
    proxyrcb_cur = tc.infra_data.ConfigStore.objects.db[proxyrcbid]
    proxyrcb_cur.GetObjValPd()
    if proxyrcb_cur.chain_rxq_base == 0:
        print("chain_rxq_base not set")
        return False

    print("chain_rxq_base value post-sync from HBM 0x%x" % proxyrcb_cur.chain_rxq_base)

    # Verify my_txq_base
    proxyccb_cur = tc.infra_data.ConfigStore.objects.db[proxyccbid]
    proxyccb_cur.GetObjValPd()
    if proxyccb_cur.my_txq_base == 0:
        print("my_txq_base not set")
        return False

    print("my_txq_base value post-sync from HBM 0x%x" % proxyccb_cur.my_txq_base)

    # Fetch current values from Platform
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.GetMeta()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.GetMeta()
    rnmpr_small_cur = tc.infra_data.ConfigStore.objects.db["RNMPR_SMALL"]
    rnmpr_small_cur.GetMeta()

    # Verify PI for RNMDR got incremented
    # when span is in effect, TLS would have allocated 2 descs per packet,
    # one for forwarding to TCP and one for L7
    num_exp_descs = num_pkts
    if redir_span:
        num_exp_descs *= 2

    if (rnmdr_cur.pi != rnmdr.pi+num_exp_descs):
        print("RNMDR pi check failed old %d new %d expected %d" %
                     (rnmdr.pi, rnmdr_cur.pi, rnmdr.pi+num_exp_descs))
        proxyrcb_cur.StatsPrint()
        proxyccb_cur.StatsPrint()
        return False
    print("RNMDR pi old %d new %d" % (rnmdr.pi, rnmdr_cur.pi))

    # Verify PI for RNMPR or RNMPR_SMALL got incremented
    if ((rnmpr_cur.pi+rnmpr_small_cur.pi) != (rnmpr.pi+rnmpr_small.pi+num_pkts+proxyr_meta_pages)):
        print("RNMPR pi check failed old %d new %d expected %d" %
                  (rnmpr.pi+rnmpr_small.pi, rnmpr_cur.pi+rnmpr_small_cur.pi,
                   rnmpr.pi+rnmpr_small.pi+num_pkts+proxyr_meta_pages))
        proxyrcb_cur.StatsPrint()
        proxyccb_cur.StatsPrint()
        return False
    print("RNMPR pi old %d new %d" % (rnmpr.pi, rnmpr_cur.pi))
    print("RNMPR_SMALL old %d new %d" % (rnmpr_small.pi, rnmpr_small_cur.pi))

    # Rx: verify # packets redirected
    num_redir_pkts = num_pkts - num_flow_miss_pkts
    if (proxyrcb_cur.stat_pkts_redir != proxyrcb.stat_pkts_redir+num_redir_pkts):
        print("stat_pkts_redir check failed old %d new %d expected %d" %
              (proxyrcb.stat_pkts_redir, proxyrcb_cur.stat_pkts_redir, proxyrcb.stat_pkts_redir+num_redir_pkts))
        proxyrcb_cur.StatsPrint()
        proxyccb_cur.StatsPrint()
        return False
    print("stat_pkts_redir old %d new %d" % 
          (proxyrcb.stat_pkts_redir, proxyrcb_cur.stat_pkts_redir))

    # Tx: verify PI for PROXYCCB got incremented
    time.sleep(1)
    proxyccb_cur.GetObjValPd()

    num_exp_proxyccb_pkts = num_redir_pkts
    if redir_span:
        num_exp_proxyccb_pkts = 0

    if (proxyccb_cur.pi != proxyccb.pi+num_exp_proxyccb_pkts):
        print("PROXYCCB pi check failed old %d new %d expected %d" %
                      (proxyccb.pi, proxyccb_cur.pi, proxyccb.pi+num_exp_proxyccb_pkts))
        proxyrcb_cur.StatsPrint()
        proxyccb_cur.StatsPrint()
        return False
    print("PROXYCCB pi old %d new %d" % (proxyccb.pi, proxyccb_cur.pi))

    # Tx: verify # packets chained
    if (proxyccb_cur.stat_pkts_chain != proxyccb.stat_pkts_chain+num_exp_proxyccb_pkts):
        print("stat_pkts_chain check failed old %d new %d expected %d" %
              (proxyccb.stat_pkts_chain, proxyccb_cur.stat_pkts_chain, proxyccb.stat_pkts_chain+num_exp_proxyccb_pkts))
        proxyrcb_cur.StatsPrint()
        proxyccb_cur.StatsPrint()
        return False
    print("stat_pkts_chain old %d new %d" % 
          (proxyccb.stat_pkts_chain, proxyccb_cur.stat_pkts_chain))

    proxyrcb_cur.StatsPrint()
    proxyccb_cur.StatsPrint()
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

