# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import config.resmgr            as resmgr

from config.store               import Store
from infra.common.logging       import cfglogger
from config.objects.swdr        import SwDscrRingHelper
from config.objects.tls_proxy_cb     import TlsCbHelper

import config.hal.defs          as haldefs
import config.hal.api           as halapi

class TcpCbObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('TCPCB'))
        return

    #def Init(self, spec_obj):
    def Init(self, qid):
        if halapi.IsHalDisabled(): qid = resmgr.TcpCbIdAllocator.get()
        self.id = qid
        gid = "TcpCb%04d" % qid
        self.GID(gid)
        # self.spec = spec_obj
        # cfglogger.info("  - %s" % self)

        # self.uplinks = objects.ObjectDatabase()
        # for uplink_spec in self.spec.uplinks:
            # uplink_obj = uplink_spec.Get(Store)
            # self.uplinks.Set(uplink_obj.GID(), uplink_obj)

        # assert(len(self.uplinks) > 0)
        cfglogger.info("  - %s" % self)
        self.tlscb = TlsCbHelper.main(self)
        self.sesq = SwDscrRingHelper.main("SESQ", gid, self.id)
        self.asesq = SwDscrRingHelper.main("ASESQ", gid, self.id)

        return


    def PrepareHALRequestSpec(self, req_spec):
        #req_spec.meta.tcpcb_id             = self.id
        req_spec.key_or_handle.tcpcb_id    = self.id
        if req_spec.__class__.__name__ != 'TcpCbGetRequest':
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
           req_spec.snd_cwnd_cnt              = self.snd_cwnd_cnt
           req_spec.rcv_mss                   = self.rcv_mss
           req_spec.source_port               = self.source_port
           req_spec.dest_port                 = self.dest_port
           req_spec.state                     = self.state
           req_spec.source_lif                = self.source_lif
           req_spec.header_len                = self.header_len
           req_spec.l7_proxy_type             = self.l7_proxy_type
           req_spec.serq_pi                   = self.serq_pi
           if hasattr(self, 'header_template'):
               req_spec.header_template           = self.header_template
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - TcpCb %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        if resp_spec.__class__.__name__ != 'TcpCbResponse':
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
            self.snd_cwnd_cnt = resp_spec.spec.snd_cwnd_cnt
            self.rcv_mss = resp_spec.spec.rcv_mss
            self.source_port = resp_spec.spec.source_port
            self.dest_port = resp_spec.spec.dest_port
            self.state = resp_spec.spec.state
            self.source_lif = resp_spec.spec.source_lif
            self.l7_proxy_type = resp_spec.spec.l7_proxy_type
            self.retx_xmit_cursor = resp_spec.spec.retx_xmit_cursor
            self.retx_snd_una = resp_spec.spec.retx_snd_una
            self.serq_pi = resp_spec.spec.serq_pi

            self.bytes_rcvd = resp_spec.stats.bytes_rcvd
            self.pkts_rcvd = resp_spec.stats.pkts_rcvd
            self.pages_alloced = resp_spec.stats.pages_alloced
            self.desc_alloced = resp_spec.stats.desc_alloced

            self.bytes_sent = resp_spec.stats.bytes_sent
            self.pkts_sent = resp_spec.stats.pkts_sent
            self.debug_num_phv_to_pkt = resp_spec.stats.debug_num_phv_to_pkt
            self.debug_num_mem_to_pkt = resp_spec.stats.debug_num_mem_to_pkt
            self.debug_num_pkt_to_mem = resp_spec.stats.debug_num_pkt_to_mem
            self.debug_num_phv_to_mem = resp_spec.stats.debug_num_phv_to_mem

            self.debug_atomic_delta = resp_spec.stats.debug_atomic_delta
            self.debug_atomic0_incr1247 = resp_spec.stats.debug_atomic0_incr1247
            self.debug_atomic1_incr247 = resp_spec.stats.debug_atomic1_incr247
            self.debug_atomic2_incr47 = resp_spec.stats.debug_atomic2_incr47
            self.debug_atomic3_incr47 = resp_spec.stats.debug_atomic3_incr47
            self.debug_atomic4_incr7 = resp_spec.stats.debug_atomic4_incr7
            self.debug_atomic5_incr7 = resp_spec.stats.debug_atomic5_incr7
            self.debug_atomic6_incr7 = resp_spec.stats.debug_atomic6_incr7

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
        cfglogger.info("Configuring %d TcpCbs." % len(objlist))
        halapi.ConfigureTcpCbs(objlist)
        return

    def __gen_one(self, qid):
        cfglogger.info("Creating TcpCb")
        tcpcb_obj = TcpCbObject()
        tcpcb_obj.Init(qid)
        Store.objects.Add(tcpcb_obj)
        return tcpcb_obj

    def Generate(self, qid):
        obj = self.__gen_one(qid)
        self.objlist.append(obj)
        lst = []
        lst.append(obj)
        self.Configure(lst)
        return self.objlist

    def main(self, qid):
        gid = "TcpCb%04d" % qid
        if Store.objects.IsKeyIn(gid):
            return Store.objects.Get(gid)
        objlist = self.Generate(qid)
        return objlist

TcpCbHelper = TcpCbObjectHelper()
