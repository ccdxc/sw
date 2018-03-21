# Testcase definition file.

import pdb
import copy
import test.tcp_tls_proxy.tcp_proxy as tcp_proxy
import test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy

from config.store import Store
from config.objects.proxycb_service import ProxyCbServiceHelper
from config.objects.tcp_proxy_cb import TcpCbHelper
import test.callbacks.networking.modcbs as modcbs
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger


def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    skip_config = False
    if hasattr(tc.module.args, 'skip_config') and tc.module.args.skip_config:
        print("skipping config")
        skip_config = True

    tc.pvtdata = ObjectDatabase()

    tcp_proxy.SetupProxyArgs(tc)

    id1, id2 = ProxyCbServiceHelper.GetSessionQids(tc.config.flow._FlowObject__session)
    if tc.config.flow.IsIflow():
        id = id1
        other_fid = id2
    else:
        id = id2
        other_fid = id1

    TcpCbHelper.main(id)
    tcbid = "TcpCb%04d" % id
    logger.info("Configuring %s" % tcbid)
    # 1. Configure TCB in HBM before packet injection
    tcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    if not skip_config:
        tcp_proxy.init_tcb_inorder(tc, tcb)
        if tc.pvtdata.serq_full:
            tcb.serq_pi = 5
        tcb.SetObjValPd()
    else:
        tc.pvtdata.flow1_bytes_rxed = 0
        tc.pvtdata.flow1_bytes_txed = 0
        tc.pvtdata.flow2_bytes_rxed = 0
        tc.pvtdata.flow2_bytes_txed = 0

    TcpCbHelper.main(other_fid)
    tcbid2 = "TcpCb%04d" % (other_fid)
    logger.info("Configuring %s" % tcbid2)
    tcb2 = tc.infra_data.ConfigStore.objects.db[tcbid2]
    if not skip_config:
        tcp_proxy.init_tcb_inorder2(tc, tcb2)
        tcb2.SetObjValPd()

    tc.pvtdata.tcb1 = tcb
    tc.pvtdata.tcb2 = tcb2

    # 2. Configure TLS CB in HBM before packet injection
    tlscbid = "TlsCb%04d" % id
    tlscbid2 = "TlsCb%04d" % (other_fid)
    tlscb = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb2 = tc.infra_data.ConfigStore.objects.db[tlscbid2]
    if tc.pvtdata.serq_full:
        tlscb.serq_pi = 5
        tlscb.serq_ci = 6
        tlscb2.serq_pi = 5
        tlscb2.serq_ci = 6
    else:
        tlscb.serq_pi = 0
        tlscb.serq_pi = 0
        tlscb.serq_ci = 0
        tlscb.serq_ci = 0
        tlscb2.serq_pi = 0
        tlscb2.serq_pi = 0
        tlscb2.serq_ci = 0
        tlscb2.serq_ci = 0

    tlscb.debug_dol = 0
    tlscb2.debug_dol = 0
    if tc.pvtdata.bypass_barco:
        print("Bypassing Barco")
        tlscb.is_decrypt_flow = False
        tlscb2.is_decrypt_flow = False
        tlscb.debug_dol |= tcp_tls_proxy.tls_debug_dol_bypass_barco
        tlscb2.debug_dol |= tcp_tls_proxy.tls_debug_dol_bypass_barco
    if tc.pvtdata.same_flow:
        print("Same flow")
        tlscb.debug_dol |= tcp_tls_proxy.tls_debug_dol_bypass_proxy
        tlscb2.debug_dol |= tcp_tls_proxy.tls_debug_dol_bypass_proxy
        tlscb.other_fid = 0xffff
        tlscb2.other_fid = 0xffff
    else:
        print("Other flow")
        tlscb.other_fid = other_fid
        tlscb2.other_fid = id

    if not skip_config:
        tlscb.SetObjValPd()
        tlscb2.SetObjValPd()

    if skip_config:
        return

    # 3. Clone objects that are needed for verification
    tcpcb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tcbid])
    tcpcb.GetObjValPd()
    tc.pvtdata.Add(tcpcb)

    tcpcb2 = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tcbid2])
    tcpcb2.GetObjValPd()
    tc.pvtdata.Add(tcpcb2)

    tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid])
    tlscb.GetObjValPd()
    tc.pvtdata.Add(tlscb)

    other_tlscb = copy.deepcopy(tc.infra_data.ConfigStore.objects.db[tlscbid2])
    other_tlscb.GetObjValPd()
    tc.pvtdata.Add(other_tlscb)

    rnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDR"])
    rnmdr.GetMeta()
    if tc.pvtdata.sem_full and tc.pvtdata.sem_full == 'nmdr':
        rnmdr.pi = 0
        rnmdr.ci = 2    # ring size of 2, so can hold 1 entry
        rnmdr.SetMeta()
    tc.pvtdata.Add(rnmdr)

    tnmdr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDR"])
    tnmdr.GetMeta()
    tc.pvtdata.Add(tnmdr)

    rnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMPR"])
    rnmpr.GetMeta()
    tc.pvtdata.Add(rnmpr)
        
    tnmpr = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMPR"])
    tnmpr.GetMeta()
    tc.pvtdata.Add(tnmpr)

    return

def TestCaseTrigger(tc):
    if GlobalOptions.dryrun:
        return True
    if tc.pvtdata.test_timer:
        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(41)
    if tc.pvtdata.test_retx_timer:
        timer = tc.infra_data.ConfigStore.objects.db['SLOW_TIMER']
        timer.Step(101)

    return

def TestCaseVerify(tc):

    if GlobalOptions.dryrun:
        return True

    if hasattr(tc.module.args, 'skip_verify') and tc.module.args.skip_verify:
        print("skipping verify")
        return True


    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    if tc.config.flow.IsIflow():
        print("This is iflow")
        tcbid = "TcpCb%04d" % id
        tlscbid = "TlsCb%04d" % id
        other_tcbid = "TcpCb%04d" % (id + 1)
        other_tlscbid = "TlsCb%04d" % (id + 1)
    else:
        print("This is rflow")
        tcbid = "TcpCb%04d" % (id + 1)
        tlscbid = "TlsCb%04d" % (id + 1)
        other_tcbid = "TcpCb%04d" % id
        other_tlscbid = "TlsCb%04d" % id

    same_flow = False
    if tc.pvtdata.same_flow:
        other_tcbid = tcbid
        other_tlscbid = tlscbid
        same_flow = True

    tcpcb = tc.pvtdata.db[tcbid]
    tcpcb_cur = tc.infra_data.ConfigStore.objects.db[tcbid]
    tcpcb_cur.GetObjValPd()

    other_tcpcb = tc.pvtdata.db[other_tcbid]
    other_tcpcb_cur = tc.infra_data.ConfigStore.objects.db[other_tcbid]
    other_tcpcb_cur.GetObjValPd()

    tlscb = tc.pvtdata.db[tlscbid]
    tlscb_cur = tc.infra_data.ConfigStore.objects.db[tlscbid]
    tlscb_cur.GetObjValPd()

    other_tlscb = tc.pvtdata.db[other_tlscbid]
    other_tlscb_cur = tc.infra_data.ConfigStore.objects.db[other_tlscbid]
    other_tlscb_cur.GetObjValPd()

    rnmdr = tc.pvtdata.db["RNMDR"]
    rnmpr = tc.pvtdata.db["RNMPR"]
    rnmdr_cur = tc.infra_data.ConfigStore.objects.db["RNMDR"]
    rnmdr_cur.GetMeta()
    rnmpr_cur = tc.infra_data.ConfigStore.objects.db["RNMPR"]
    rnmpr_cur.GetMeta()

    tnmdr = tc.pvtdata.db["TNMDR"]
    tnmpr = tc.pvtdata.db["TNMPR"]
    tnmdr_cur = tc.infra_data.ConfigStore.objects.db["TNMDR"]
    tnmdr_cur.GetMeta()
    tnmpr_cur = tc.infra_data.ConfigStore.objects.db["TNMPR"]
    tnmpr_cur.GetMeta()

    # Print stats
    if same_flow:
        other_tcpcb = tcpcb
        other_tcpcb_cur = tcpcb_cur
    else:
        print("bytes_rcvd = %d:" % tcpcb_cur.bytes_rcvd)
        print("pkts_rcvd = %d:" % tcpcb_cur.pkts_rcvd)
        print("debug_num_phv_to_mem = %d:" % tcpcb_cur.debug_num_phv_to_mem)
        print("debug_num_pkt_to_mem = %d:" % tcpcb_cur.debug_num_pkt_to_mem)

    print("bytes_sent = %d:" % other_tcpcb_cur.bytes_sent)
    print("pkts_sent = %d:" % other_tcpcb_cur.pkts_sent)
    print("debug_num_phv_to_pkt = %d:" % other_tcpcb_cur.debug_num_phv_to_pkt)
    print("debug_num_mem_to_pkt = %d:" % other_tcpcb_cur.debug_num_mem_to_pkt)

    num_rx_pkts = tc.pvtdata.num_pkts
    if (tc.pvtdata.test_retx_timer or tc.pvtdata.test_retx_timer_full) \
            and not tc.pvtdata.test_cancel_retx_timer:
        num_tx_pkts = tc.pvtdata.num_pkts * 2
    else:
        num_tx_pkts = tc.pvtdata.num_pkts

    if hasattr(tc.module.args, 'num_retx_pkts'):
        tc.pvtdata.flow2_bytes_txed += (tc.pvtdata.flow2_bytes_txed * int(tc.module.args.num_retx_pkts))

    if hasattr(tc.module.args, 'fin'):
        tc.pvtdata.flow1_bytes_rxed -= int(tc.module.args.fin)
        tc.pvtdata.flow2_bytes_txed -= int(tc.module.args.fin)

    if tc.pvtdata.serq_full:
        # SERQ is full, pi/ci should not move
        if tlscb_cur.serq_pi != tlscb.serq_pi or \
                    tlscb_cur.serq_ci != tlscb.serq_ci:
            print("serq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                    (tlscb.serq_pi, tlscb.serq_ci,
                     tlscb_cur.serq_pi, tlscb_cur.serq_ci))
            return False
        return True


    # 1. Verify SERQ pi got updated
    if (tlscb_cur.serq_pi != tlscb.serq_pi + num_rx_pkts):
        print("serq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (tlscb.serq_pi, tlscb.serq_ci,
                 tlscb_cur.serq_pi, tlscb_cur.serq_ci))
        return False

    # 2. Verify SERQ ci got updated
    if (tlscb_cur.serq_ci != tlscb.serq_ci + num_rx_pkts):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (tlscb.serq_pi, tlscb.serq_ci,
                 tlscb_cur.serq_pi, tlscb_cur.serq_ci))
        return False

    # 3. Verify SESQ pi got updated
    if (other_tcpcb_cur.sesq_pi != other_tcpcb.sesq_pi + num_rx_pkts):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (other_tcpcb.sesq_pi, other_tcpcb.sesq_ci,
                 other_tcpcb_cur.sesq_pi, other_tcpcb_cur.sesq_ci))
        return False

    # 4. Verify SESQ ci got updated
    if (other_tcpcb_cur.sesq_ci != other_tcpcb.sesq_ci + num_rx_pkts):
        print("sesq pi/ci not as expected old (%d, %d), new (%d, %d)" %
                (other_tcpcb.sesq_pi, other_tcpcb.sesq_ci,
                 other_tcpcb_cur.sesq_pi, other_tcpcb_cur.sesq_ci))
        return False

    # 5. Verify pkt rx stats
    if not tc.pvtdata.final_fin and tcpcb_cur.pkts_rcvd != tcpcb.pkts_rcvd + num_rx_pkts:
        print("pkt rx stats not as expected")
        return False

    print("Stats - 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x" % (tc.pvtdata.flow2_bytes_txed, tc.pvtdata.flow2_bytes_rxed, tc.pvtdata.flow1_bytes_txed, tc.pvtdata.flow1_bytes_rxed, num_tx_pkts, num_rx_pkts))

    if tcpcb_cur.bytes_rcvd - tcpcb.bytes_rcvd != \
            tc.pvtdata.flow1_bytes_rxed:
        print("Warning! pkt rx byte stats not as expected %d %d" % \
                (tcpcb_cur.bytes_rcvd, tcpcb.bytes_rcvd))

        # In the error case of running out of descriptors, we increment stats and
        # update rcv_nxt before discovering too late that we cannot continue.
        # Since we will likely kill the connection in this case, ignore this
        # check
        if not tc.pvtdata.sem_full:
            return False
    
    # 6. Verify pkt tx stats
    if other_tcpcb_cur.pkts_sent != other_tcpcb.pkts_sent + num_tx_pkts:
        print("pkt tx stats (%d) not as expected (%d)" % (other_tcpcb_cur.pkts_sent, other_tcpcb.pkts_sent))
        return False

    if other_tcpcb_cur.bytes_sent - other_tcpcb.bytes_sent != \
            tc.pvtdata.flow2_bytes_txed:
        print("Warning! pkt tx byte stats not as expected 0x%x 0x%x" % (other_tcpcb_cur.bytes_sent, other_tcpcb.bytes_sent))
        return False

    # 7. Verify phv2pkt
    if other_tcpcb_cur.debug_num_phv_to_pkt != other_tcpcb.debug_num_phv_to_pkt + 2 * num_tx_pkts:
        print("Num phv2pkt not as expected")
        return False

    # 8. Verify mem2pkt
    if other_tcpcb_cur.debug_num_mem_to_pkt != other_tcpcb.debug_num_mem_to_pkt + 2 * num_tx_pkts:
        print("Num mem2pkt not as expected")
        return False

    if same_flow and other_tcpcb_cur.snd_nxt != tc.pvtdata.flow1_snd_nxt + \
            tc.pvtdata.flow2_bytes_txed:
        print("mem2pkt failed snd_nxt = 0x%x" % other_tcpcb_cur.snd_nxt)
        print("mem2pkt failed pvtdata snd_nxt = 0x%x" % tc.pvtdata.flow1_snd_nxt)
        return False
    elif not same_flow and other_tcpcb_cur.snd_nxt != \
            tc.pvtdata.flow2_snd_nxt + tc.pvtdata.flow2_bytes_txed:
        print("mem2pkt failed snd_nxt = 0x%x" % other_tcpcb_cur.snd_nxt)

    # 10. Verify pkt tx (in testspec)

    # 11.
    print("RNMDR old pi=%d,ci=%d / new pi=%d,ci=%d" %
            (rnmdr.pi, rnmdr.ci, rnmdr_cur.pi, rnmdr_cur.ci))
    print("RNMPR old pi=%d,ci=%d / new pi=%d,ci=%d" %
            (rnmpr.pi, rnmpr.ci, rnmpr_cur.pi, rnmpr_cur.ci))

    print("TNMDR old pi=%d,ci=%d / new pi=%d,ci=%d" %
            (tnmdr.pi, tnmdr.ci, tnmdr_cur.pi, tnmdr_cur.ci))
    print("TNMPR old pi=%d,ci=%d / new pi=%d,ci=%d" %
            (tnmpr.pi, tnmpr.ci, tnmpr_cur.pi, tnmpr_cur.ci))

    print("retx_xmit_cursor before 0x%lx after 0x%lx" % \
            (tcpcb_cur.retx_xmit_cursor, other_tcpcb_cur.retx_xmit_cursor))
    print("retx_snd_una before 0x%x after 0x%x" % \
            (tcpcb_cur.retx_snd_una, other_tcpcb_cur.retx_snd_una))

    if tc.pvtdata.test_retx and tc.pvtdata.test_retx == 'partial':
        if other_tcpcb_cur.retx_xmit_cursor == 0:
            print("retx_xmit_cursor is 0")
            return False
        if other_tcpcb_cur.retx_snd_una != tc.pvtdata.flow2_snd_una + \
                 tc.pvtdata.flow2_bytes_txed / 2:
            print("retx_snd_una 0x%x is not 0x%x" % 
                    (other_tcpcb_cur.retx_snd_una, tc.pvtdata.flow2_bytes_txed))
            return False

    if tc.pvtdata.test_retx and tc.pvtdata.test_retx == 'complete':
        if other_tcpcb_cur.retx_xmit_cursor != 0:
            print("retx_xmit_cursor is not 0")
            return False
        if other_tcpcb_cur.retx_snd_una != tc.pvtdata.flow2_snd_una + \
                 tc.pvtdata.flow1_bytes_rxed:
            print("retx_snd_una 0x%x is not 0x%x" % 
                    (other_tcpcb_cur.retx_snd_una, tc.pvtdata.flow1_bytes_rxed))
            return False

    if tc.pvtdata.test_retx and (tc.pvtdata.test_retx == 'partial' \
            or tc.pvtdata.test_retx == 'complete'):
        if rnmdr_cur.pi != rnmdr.pi + tc.pvtdata.pkt_alloc:
            print("rnmdr cur %d pi does not match expected %d" % \
                    (rnmdr_cur.pi, rnmdr.pi + tc.pvtdata.pkt_alloc))
            return False
        if rnmpr_cur.pi != rnmpr.pi + tc.pvtdata.pkt_alloc:
            print("rnmpr cur %d pi does not match expected %d" % \
                    (rnmpr_cur.pi, rnmpr.pi + tc.pvtdata.pkt_alloc))
            return False
        if tc.pvtdata.bypass_barco:
            if rnmdr_cur.ci != rnmdr.ci + tc.pvtdata.pkt_free:
                print("rnmdr cur %d pi does not match expected %d" % \
                        (rnmdr_cur.pi, rnmdr.pi + tc.pvtdata.pkt_free))
                return False
            if rnmpr_cur.ci != rnmpr.ci + tc.pvtdata.pkt_free:
                print("rnmpr cur ci %d does not match expected %d" % \
                        (rnmpr_cur.ci, rnmpr.ci + tc.pvtdata.pkt_free))
                return False

    if tc.pvtdata.test_cong_avoid:
        if other_tcpcb_cur.snd_cwnd != other_tcpcb.snd_cwnd + 1:
            print("cong_avoid: failed to increment cwnd (%d)" % \
                    other_tcpcb_cur.snd_cwnd)
            return False
        if other_tcpcb_cur.snd_cwnd_cnt != 0:
            print("cong_avoid: failed to set snd_cwnd_cnt to 0")
            return False

    if tc.pvtdata.fin:
        print("tcpcb state = %s, other_tcpcb state = %s" % \
                (tcpcb.state, other_tcpcb.state))
        print("tcpcb_cur state = %s, other_tcpcb_cur state = %s" % \
                (tcpcb_cur.state, other_tcpcb_cur.state))
        if not tc.pvtdata.final_fin and tcpcb_cur.state != tcp_proxy.tcp_state_CLOSE_WAIT:
            print("flow 1 state not CLOSE_WAIT as expected")
            return False
        elif tc.pvtdata.final_fin and tcpcb_cur.state != tcp_proxy.tcp_state_CLOSE:
            print("flow 1 state not CLOSE as expected")
            return False
        if not tc.pvtdata.fin_ack and not tc.pvtdata.final_fin:
            # flow 2 should be in fin_wait1, but since it doesn't receive any
            # packet the state doesn't move from ESTABLISHED (state is only
            # maintained in rxdma)
            if other_tcpcb_cur.state != tcp_proxy.tcp_state_ESTABLISHED:
                print("flow 2 state not FIN_WAIT1 as expected")
                return False
        elif not tc.pvtdata.final_fin:
            if other_tcpcb_cur.state != tcp_proxy.tcp_state_FIN_WAIT2:
                print("flow 2 state not FIN_WAIT2 as expected")
                return False
        else:
            if other_tcpcb_cur.state != tcp_proxy.tcp_state_TIME_WAIT:
                print("flow 2 state not TIME_WAIT as expected")
                return False

    if tc.pvtdata.test_mpu_tblsetaddr:
        print("debug_dol_tblsetaddr = 0x%x" % \
                other_tcpcb_cur.debug_dol_tblsetaddr)
        if (other_tcpcb_cur.debug_dol_tblsetaddr != \
                tcp_proxy.tcp_ddol_TBLADDR_VALUE):
            print("tblsetaddr value not as expected")
            return False

    return True

def TestCaseTeardown(tc):
    if GlobalOptions.dryrun:
        return True
    if tc.pvtdata.test_timer:
        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(0)
    if tc.pvtdata.sem_full and tc.pvtdata.sem_full == 'nmdr':
        rnmdr = tc.pvtdata.db["RNMDR"]
        rnmdr.pi = 0
        rnmdr.ci = 1024
        rnmdr.SetMeta()
    if tc.pvtdata.test_retx_timer_full:
        #
        # Reset the debug_dol_tx to 0, so that we reset the temporary
        # timer config
        #
        id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
        if tc.config.flow.IsIflow():
            tcbid = "TcpCb%04d" % id
            other_tcbid = "TcpCb%04d" % (id + 1)
        else:
            tcbid = "TcpCb%04d" % (id + 1)
            other_tcbid = "TcpCb%04d" % id

        tcpcb = tc.pvtdata.db[tcbid]
        tcpcb = tc.infra_data.ConfigStore.objects.db[tcbid]
        tcpcb.debug_dol_tx = 0
        tcpcb.SetObjValPd()

        other_tcpcb = tc.pvtdata.db[other_tcbid]
        other_tcpcb = tc.infra_data.ConfigStore.objects.db[other_tcbid]
        other_tcpcb.debug_dol_tx = 0
        other_tcpcb.SetObjValPd()
    return
