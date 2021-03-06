# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import iris.config.resmgr            as resmgr

from iris.config.store               import Store
from infra.common.logging       import logger
from iris.config.objects.swdr        import SwDscrRingHelper
from iris.config.objects.tls_proxy_cb     import TlsCbHelper

import iris.config.hal.defs          as haldefs
import iris.config.hal.api           as halapi

import iris.test.tcp_tls_proxy.tcp_proxy as tcp_proxy
import iris.test.nvme.nvme_o_tcp as nvme_o_tcp

class TcpCbObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('TCPCB'))
        return

    #def Init(self, spec_obj):
    def Init(self, qid, other_qid = None, session = None, is_iflow = None):
        if halapi.IsHalDisabled(): qid = resmgr.TcpCbIdAllocator.get()
        self.id = qid
        if other_qid != None:
            self.other_qid = other_qid
        else:
            self.other_qid = 0xffff
        gid = "TcpCb%04d" % qid
        self.GID(gid)
        # self.spec = spec_obj
        # logger.info("  - %s" % self)

        # self.uplinks = objects.ObjectDatabase()
        # for uplink_spec in self.spec.uplinks:
            # uplink_obj = uplink_spec.Get(Store)
            # self.uplinks.Set(uplink_obj.GID(), uplink_obj)

        # assert(len(self.uplinks) > 0)
        logger.info("  - %s" % self)
        if session is not None: 
            if session.iflow.label == 'NVME-PROXY':
                logger.info("skipping TLS cb creation for nvme..")
            else:
                self.tlscb = TlsCbHelper.main(self)
        self.sesq = SwDscrRingHelper.main("SESQ", gid, self.id)
        self.asesq = SwDscrRingHelper.main("ASESQ", gid, self.id)

        if is_iflow:
            logger.info("%s is iflow" % gid)
            tcp_proxy.init_tcb1(self, session)
        elif is_iflow != None:
            logger.info("%s is rflow" % gid)
            if session.iflow.label == 'NVME-PROXY':
                nvme_o_tcp.init_tcb2(self, session)
            else:
                tcp_proxy.init_tcb2(self, session)

        self.debug_dol = tcp_proxy.tcp_debug_dol_dont_send_ack | \
                            tcp_proxy.tcp_debug_dol_bypass_barco
        self.debug_dol_tx = tcp_proxy.tcp_tx_debug_dol_dont_send_ack | \
                                tcp_proxy.tcp_tx_debug_dol_bypass_barco

        return


    def PrepareHALRequestSpec(self, req_spec):
        #req_spec.meta.tcpcb_id             = self.id
        req_spec.key_or_handle.tcpcb_id    = self.id
        if req_spec.__class__.__name__ != 'TcpCbGetRequest':
           req_spec.other_qid                 = self.other_qid
           req_spec.proxy_type                = self.proxy_type
           req_spec.rcv_nxt                   = self.rcv_nxt
           req_spec.snd_nxt                   = self.snd_nxt
           req_spec.snd_una                   = self.snd_una
           req_spec.rcv_tsval                 = self.rcv_tsval
           req_spec.ts_recent                 = self.ts_recent
           req_spec.debug_dol                 = self.debug_dol
           req_spec.debug_dol_tx              = self.debug_dol_tx
           req_spec.sesq_pi                   = self.sesq_pi
           req_spec.sesq_ci                   = self.sesq_ci
           req_spec.asesq_pi                  = self.asesq_pi
           req_spec.asesq_ci                  = self.asesq_ci
           req_spec.snd_wnd                   = self.snd_wnd
           req_spec.snd_cwnd                  = self.snd_cwnd
           req_spec.initial_window            = self.initial_window
           req_spec.snd_ssthresh              = self.snd_ssthresh
           req_spec.rcv_mss                   = self.rcv_mss
           req_spec.smss                      = self.smss
           req_spec.source_port               = self.source_port
           req_spec.dest_port                 = self.dest_port
           req_spec.state                     = self.state
           req_spec.source_lif                = self.source_lif
           req_spec.header_len                = self.header_len
           req_spec.l7_proxy_type             = self.l7_proxy_type
           req_spec.serq_pi                   = self.serq_pi
           req_spec.serq_ci                   = self.serq_ci
           req_spec.pred_flags                = self.pred_flags
           req_spec.rto_backoff               = self.rto_backoff
           req_spec.rcv_wnd                   = self.rcv_wnd
           req_spec.rcv_wup                   = self.rcv_wup
           req_spec.rcv_nxt                   = self.rcv_nxt
           req_spec.snd_wscale                = self.snd_wscale
           req_spec.rcv_wscale                = self.rcv_wscale
           req_spec.delay_ack                 = self.delay_ack
           req_spec.ato                       = self.ato
           req_spec.abc_l_var                 = self.abc_l_var
           req_spec.ooo_queue                 = self.ooo_queue
           req_spec.sack_perm                 = self.sack_perm
           if hasattr(self, 'header_template'):
               req_spec.header_template           = self.header_template
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("  - TcpCb %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        if resp_spec.__class__.__name__ != 'TcpCbResponse':
            self.other_qid = resp_spec.spec.other_qid
            self.proxy_type = resp_spec.spec.proxy_type
            self.rx_ts = resp_spec.spec.rx_ts
            self.rcv_nxt = resp_spec.spec.rcv_nxt
            self.snd_nxt = resp_spec.spec.snd_nxt
            self.snd_una = resp_spec.spec.snd_una
            self.rcv_tsval = resp_spec.spec.rcv_tsval
            self.ts_recent = resp_spec.spec.ts_recent
            self.debug_dol = resp_spec.spec.debug_dol
            self.debug_dol_tx = resp_spec.spec.debug_dol_tx
            self.debug_dol_tblsetaddr = resp_spec.spec.debug_dol_tblsetaddr
            self.sesq_pi = resp_spec.spec.sesq_pi
            self.sesq_ci = resp_spec.spec.sesq_ci
            self.asesq_pi = resp_spec.spec.asesq_pi
            self.asesq_ci = resp_spec.spec.asesq_ci
            self.snd_wnd = resp_spec.spec.snd_wnd
            self.snd_cwnd = resp_spec.spec.snd_cwnd
            self.initial_window = resp_spec.spec.initial_window
            self.snd_ssthresh = resp_spec.spec.snd_ssthresh
            self.snd_recover = resp_spec.spec.snd_recover
            self.rcv_mss = resp_spec.spec.rcv_mss
            self.smss = resp_spec.spec.smss
            self.source_port = resp_spec.spec.source_port
            self.dest_port = resp_spec.spec.dest_port
            self.state = resp_spec.spec.state
            self.source_lif = resp_spec.spec.source_lif
            self.l7_proxy_type = resp_spec.spec.l7_proxy_type
            self.sesq_retx_ci = resp_spec.spec.sesq_retx_ci
            self.retx_snd_una = resp_spec.spec.retx_snd_una
            self.serq_pi = resp_spec.spec.serq_pi
            self.serq_ci = resp_spec.spec.serq_ci
            self.pred_flags = resp_spec.spec.pred_flags
            self.packets_out = resp_spec.spec.packets_out
            self.rto_backoff = resp_spec.spec.rto_backoff
            self.rcv_wnd = resp_spec.spec.rcv_wnd
            self.snd_wscale = resp_spec.spec.snd_wscale
            self.rcv_wscale = resp_spec.spec.rcv_wscale
            self.delay_ack = resp_spec.spec.delay_ack
            self.ato = resp_spec.spec.ato
            self.abc_l_var = resp_spec.spec.abc_l_var
            self.ooo_queue = resp_spec.spec.ooo_queue
            self.sack_perm = resp_spec.spec.sack_perm

            self.bytes_rcvd = resp_spec.stats.bytes_rcvd
            self.pkts_rcvd = resp_spec.stats.pkts_rcvd

            self.bytes_sent = resp_spec.stats.bytes_sent
            self.pkts_sent = resp_spec.stats.pkts_sent
            self.cc_flags = resp_spec.stats.cc_flags

            self.ooq_not_empty = resp_spec.status.ooq_not_empty
            i = 0
            for ooq_status in resp_spec.status.ooq_status:
                self.ooq_status[i].queue_addr = ooq_status.queue_addr
                self.ooq_status[i].start_seq = ooq_status.start_seq
                self.ooq_status[i].end_seq = ooq_status.end_seq
                self.ooq_status[i].num_entries = ooq_status.num_entries
                i += 1

        return

    def GetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.GetTcpCbs(lst)
        return

    def SetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.UpdateTcpCbs(lst)
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def Read(self):
        return

    def Write(self):
        return



# Helper Class to Generate/Configure/Manage TcpCb Objects.
class TcpCbObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(TcpCbObject)
        logger.info("Configuring %d TcpCbs." % len(objlist))
        halapi.ConfigureTcpCbs(objlist)
        return

    def __gen_one(self, qid, other_qid = None, session = None, is_iflow = None):
        logger.info("Creating TcpCb")
        tcpcb_obj = TcpCbObject()
        tcpcb_obj.Init(qid, other_qid, session, is_iflow)
        Store.objects.Add(tcpcb_obj)
        return tcpcb_obj

    def Generate(self, qid, other_qid = None, session = None, is_iflow = None):
        obj = self.__gen_one(qid, other_qid, session, is_iflow)
        self.objlist.append(obj)
        lst = []
        lst.append(obj)
        self.Configure(lst)
        return self.objlist

    def main(self, qid, other_qid = None, session = None, is_iflow = None):
        gid = "TcpCb%04d" % qid
        if Store.objects.IsKeyIn(gid):
            return Store.objects.Get(gid)
        objlist = self.Generate(qid, other_qid, session, is_iflow)
        return objlist

TcpCbHelper = TcpCbObjectHelper()
