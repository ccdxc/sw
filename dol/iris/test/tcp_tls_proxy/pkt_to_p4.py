# Testcase definition file.

import pdb
import copy
import iris.test.tcp_tls_proxy.tcp_proxy as tcp_proxy
import iris.test.tcp_tls_proxy.tcp_tls_proxy as tcp_tls_proxy

from iris.config.store import Store
from iris.config.objects.proxycb_service import ProxyCbServiceHelper
from iris.config.objects.tcp_proxy_cb import TcpCbHelper
import iris.test.callbacks.networking.modcbs as modcbs
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
            tcb.serq_ci = 6
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

    rnmdpr_big = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"])
    rnmdpr_big.GetMeta()
    if tc.pvtdata.sem_full and tc.pvtdata.sem_full == 'nmdr':
        rnmdpr_big.pi = 0
        rnmdpr_big.ci = 2    # ring size of 2, so can hold 1 entry
        rnmdpr_big.SetMeta()
    tc.pvtdata.Add(rnmdpr_big)

    tnmdpr_big = copy.deepcopy(tc.infra_data.ConfigStore.objects.db["TNMDPR_BIG"])
    tnmdpr_big.GetMeta()
    tc.pvtdata.Add(tnmdpr_big)

    return

def TestCaseTrigger(tc):
    if GlobalOptions.dryrun:
        return True
    if tc.pvtdata.test_del_ack_timer:
        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(41)
    if tc.pvtdata.test_retx_timer:
        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(101)

        num_retx_pkts = 0
        if hasattr(tc.module.args, 'num_retx_pkts'):
            num_retx_pkts = tc.module.args.num_retx_pkts
        curr_tick = 101
        timeout = 200
        while num_retx_pkts >= 2:
            curr_tick += (timeout + 1)
            timer.Step(curr_tick)
            num_retx_pkts -= 1
            timeout <<= 1

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

    rnmdpr_big = tc.pvtdata.db["RNMDPR_BIG"]
    rnmdpr_big_cur = tc.infra_data.ConfigStore.objects.db["RNMDPR_BIG"]
    rnmdpr_big_cur.GetMeta()

    tnmdpr_big = tc.pvtdata.db["TNMDPR_BIG"]
    tnmdpr_big_cur = tc.infra_data.ConfigStore.objects.db["TNMDPR_BIG"]
    tnmdpr_big_cur.GetMeta()

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

    num_pkts = tc.pvtdata.num_pkts
    num_ack_pkts = tc.pvtdata.num_ack_pkts
    num_rx_pkts = tc.pvtdata.num_rx_pkts
    num_retx_pkts = tc.pvtdata.num_retx_pkts
    num_tx_pkts = tc.pvtdata.num_tx_pkts

    if hasattr(tc.module.args, 'num_retx_pkts'):
        tc.pvtdata.flow2_bytes_txed += (tc.packets.Get('PKT1').payloadsize * int(tc.module.args.num_retx_pkts))

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
        #
        # ignore stats in rst case as well
        if not tc.pvtdata.sem_full and not tc.pvtdata.rst:
            return False
    
    # 6. Verify pkt tx stats
    if other_tcpcb_cur.pkts_sent != other_tcpcb.pkts_sent + num_tx_pkts:
        print("pkt tx stats (%d) not as expected (%d)" % (other_tcpcb_cur.pkts_sent,
            (other_tcpcb.pkts_sent + num_tx_pkts)))
        return False

    if hasattr(tc.module.args, 'byte_calc_based_on_tx_pkts'):
        if other_tcpcb_cur.bytes_sent - other_tcpcb.bytes_sent != \
                tc.packets.Get('PKT1').payloadsize * num_tx_pkts:
            print("Warning! pkt tx byte stats not as expected %d %d" % \
                (other_tcpcb_cur.bytes_sent - other_tcpcb.bytes_sent,
                    tc.packets.Get('PKT1').payloadsize * num_tx_pkts))
            return False
    else:
        if other_tcpcb_cur.bytes_sent - other_tcpcb.bytes_sent != \
                tc.pvtdata.flow2_bytes_txed:
            print("Warning! pkt tx byte stats not as expected %d %d" % \
                (other_tcpcb_cur.bytes_sent - other_tcpcb.bytes_sent,
                    tc.pvtdata.flow2_bytes_txed))
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
    print("RNMDPR_BIG old pi=%d,ci=%d / new pi=%d,ci=%d" %
            (rnmdpr_big.pi, rnmdpr_big.ci, rnmdpr_big_cur.pi, rnmdpr_big_cur.ci))

    print("TNMDPR_BIG old pi=%d,ci=%d / new pi=%d,ci=%d" %
            (tnmdpr_big.pi, tnmdpr_big.ci, tnmdpr_big_cur.pi, tnmdpr_big_cur.ci))

    print("sesq_retx_ci before 0x%lx after 0x%lx" % \
            (tcpcb_cur.sesq_retx_ci, other_tcpcb_cur.sesq_retx_ci))
    print("retx_snd_una before 0x%x after 0x%x" % \
            (tcpcb_cur.retx_snd_una, other_tcpcb_cur.retx_snd_una))

    if tc.pvtdata.test_retx and tc.pvtdata.test_retx == 'partial':
        if other_tcpcb_cur.sesq_retx_ci != other_tcpcb.sesq_retx_ci + 1:
            print("sesq_retx_ci is %d, expected %d" %
                    (other_tcpcb_cur.sesq_retx_ci, other_tcpcb.sesq_retx_ci + 1))
            return False
        if other_tcpcb_cur.retx_snd_una != tc.pvtdata.flow2_snd_una + \
                 tc.pvtdata.flow2_bytes_txed / 2:
            print("retx_snd_una 0x%x is not 0x%x" % 
                    (other_tcpcb_cur.retx_snd_una, tc.pvtdata.flow2_bytes_txed))
            return False

    if tc.pvtdata.test_retx and tc.pvtdata.test_retx == 'complete':
        if other_tcpcb_cur.sesq_retx_ci != other_tcpcb.sesq_retx_ci + 2:
            print("sesq_retx_ci is %d, expected %d" %
                    (other_tcpcb_cur.sesq_retx_ci, other_tcpcb.sesq_retx_ci + 2))
            return False
        if other_tcpcb_cur.retx_snd_una != tc.pvtdata.flow2_snd_una + \
                 tc.pvtdata.flow1_bytes_rxed:
            print("retx_snd_una 0x%x is not 0x%x" % 
                    (other_tcpcb_cur.retx_snd_una, tc.pvtdata.flow1_bytes_rxed))
            return False

    if tc.pvtdata.test_retx and (tc.pvtdata.test_retx == 'partial' \
            or tc.pvtdata.test_retx == 'complete'):
        if rnmdpr_big_cur.pi != rnmdpr_big.pi + tc.pvtdata.pkt_alloc:
            print("rnmdpr_big cur %d pi does not match expected %d" % \
                    (rnmdpr_big_cur.pi, rnmdpr_big.pi + tc.pvtdata.pkt_alloc))
            return False
        if tc.pvtdata.bypass_barco:
            if rnmdpr_big_cur.ci != rnmdpr_big.ci + tc.pvtdata.pkt_free:
                print("rnmdpr_big cur %d ci does not match expected %d" % \
                        (rnmdpr_big_cur.ci, rnmdpr_big.ci + tc.pvtdata.pkt_free))
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

    if tc.pvtdata.rst:
        if (other_tcpcb_cur.sesq_retx_ci != other_tcpcb.sesq_retx_ci + num_tx_pkts):
            print("sesq_retx_ci %d, not as expected %d" % \
                    (other_tcpcb_cur.sesq_retx_ci, other_tcpcb.sesq_retx_ci + num_tx_pkts))
            return False
        if (rnmdpr_big_cur.ci != rnmdpr_big.ci + num_tx_pkts + num_ack_pkts):
            print("rnmdpr ci  %d, not as expected %d" % \
                    (rnmdpr_big_cur.ci, rnmdpr_big.ci + num_tx_pkts))
            return False

    if tc.pvtdata.test_retx_timer:
        if num_retx_pkts == 0:
            #
            # All acks are received (no retransmit case)
            #
            if other_tcpcb_cur.packets_out != 0:
                print("packets_out (%d) not as expected (0)" %
                        other_tcpcb_cur.packets_out)
                return False
        else:
            #
            # retransmit case - no acks or partial acks received
            #
            if other_tcpcb_cur.packets_out != num_pkts - num_ack_pkts:
                print("packets_out (%d) not as expected (%d)" %
                        (other_tcpcb_cur.packets_out, num_pkts - num_ack_pkts))
                return False

        if tc.pvtdata.rto_backoff:
            if other_tcpcb_cur.rto_backoff != 0:
                print("rto_backoff (%d) is not 0 after the test as expected" % \
                        other_tcpcb_cur.rto_backoff)
        if other_tcpcb_cur.rto_backoff != num_retx_pkts:
            print("rto_backoff (%d) not as expected (%d)" %
                    (other_tcpcb_cur.rto_backoff, num_retx_pkts))

    return True

def TestCaseTeardown(tc):
    id = ProxyCbServiceHelper.GetFlowInfo(tc.config.flow._FlowObject__session)
    if tc.config.flow.IsIflow():
        tcbid = "TcpCb%04d" % id
        other_tcbid = "TcpCb%04d" % (id + 1)
    else:
        tcbid = "TcpCb%04d" % (id + 1)
        other_tcbid = "TcpCb%04d" % id

    tcpcb = tc.infra_data.ConfigStore.objects.db[tcbid]
    other_tcpcb = tc.infra_data.ConfigStore.objects.db[other_tcbid]

    if GlobalOptions.dryrun:
        return True
    if tc.pvtdata.test_del_ack_timer:
        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(0)
    if tc.pvtdata.test_retx_timer:
        tcpcb.debug_dol_tx &= ~tcp_proxy.tcp_tx_debug_dol_start_retx_timer
        tcpcb.SetObjValPd()
        other_tcpcb.debug_dol_tx &= ~tcp_proxy.tcp_tx_debug_dol_start_retx_timer
        other_tcpcb.SetObjValPd()

        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(0)
    if tc.pvtdata.sem_full and tc.pvtdata.sem_full == 'nmdr':
        rnmdpr_big = tc.pvtdata.db["RNMDPR_BIG"]
        rnmdpr_big.pi = 0
        rnmdpr_big.ci = 1024
        rnmdpr_big.SetMeta()
    if tc.pvtdata.test_retx_timer_full:
        tcpcb.debug_dol_tx = 0
        tcpcb.SetObjValPd()

        other_tcpcb.debug_dol_tx = 0
        other_tcpcb.SetObjValPd()
    return
