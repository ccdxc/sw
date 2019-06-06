#! /usr/bin/python3
import pdb

import scapy.all                as scapy
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
from iris.config.store               import Store
from infra.common.logging       import logger

import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs

from infra.common.glopts        import GlobalOptions

from iris.config.objects.session     import SessionHelper

from scapy.utils import inet_aton, inet_ntoa, inet_pton
import struct
import socket

import model_sim.src.model_wrap as model_wrap
from scapy.all import *

class NvmeResourcecb(Packet):
    name = "NvmeResourcecb"
    fields_desc = [
        LEShortField("rx_pduid_ring_pi", 0),
        LEShortField("rx_pduid_ring_proxy_ci", 0),
        LEShortField("rx_pduid_ring_ci", 0),
        BitField("rx_pduid_ring_log_sz", 0, 5),
        BitField("rx_pduid_ring_rsvd", 0, 3),
        ByteField("rx_pduid_ring_choke_counter", 0),

        LEShortField("tx_pduid_ring_pi", 0),
        LEShortField("tx_pduid_ring_proxy_ci", 0),
        LEShortField("tx_pduid_ring_ci", 0),
        BitField("tx_pduid_ring_log_sz", 0, 5),
        BitField("tx_pduid_ring_rsvd", 0, 3),
        ByteField("tx_pduid_ring_choke_counter", 0),

        LEShortField("cmdid_ring_pi", 0),
        LEShortField("cmdid_ring_proxy_ci", 0),
        LEShortField("cmdid_ring_ci", 0),
        BitField("cmdid_ring_log_sz", 0, 5),
        BitField("cmdid_ring_rsvd", 0, 3),
        ByteField("cmdid_ring_choke_counter", 0),

        BitField("pad", 0, 320),
    ]

class NvmeHwXtsTxcb(Packet):
    name = "NvmeHwXtsTxcb"
    fields_desc = [
        IntField("pi", 0),
        IntField("ci", 0),
        XBitField("xts_ring_base_addr", 0, 34),
        BitField("log_sz", 0, 5),
        BitField("rsvd", 0, 1),
        ByteField("choke_counter", 0),
        BitField("pad", 0, 400),
    ]

class NvmeHwDgstTxcb(Packet):
    name = "NvmeHwDgstTxcb"
    fields_desc = [
        IntField("pi", 0),
        IntField("ci", 0),
        XBitField("dgst_ring_base_addr", 0, 34),
        BitField("log_sz", 0, 5),
        BitField("rsvd", 0, 1),
        ByteField("choke_counter", 0),
        BitField("pad", 0, 400),
    ]

class NvmeGlobalObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('NVME_GLOBAL'))
        self.GID("Nvme Global")
        return

    def main(self, spec):
        self.spec = spec
        halapi.NvmeEnable([self])
        self.Show()
        return
         
    def Show(self):
        logger.info('Nvme Global')
        logger.info('   - max_cmd_context: %d max_ns: %d max_sess: %d \
                          tx_max_pdu_context: %d rx_max_pdu_context: %d' \
                     %(self.spec.max_cmd_context, self.spec.max_ns, \
                       self.spec.max_sess, self.spec.tx_max_pdu_context, \
                       self.spec.rx_max_pdu_context))
        logger.info('   - cmd_context_ring_base: 0x%x cmd_context_page_base: 0x%x' \
                     %(self.cmd_context_ring_base, self.cmd_context_page_base))
        logger.info('   - resourcecb_addr: 0x%x' \
                     %(self.resourcecb_addr))
        logger.info('   - tx_pdu_context_ring_base: 0x%x tx_pdu_context_page_base: 0x%x' \
                     %(self.tx_pdu_context_ring_base, self.tx_pdu_context_page_base))
        logger.info('   - rx_pdu_context_ring_base: 0x%x rx_pdu_context_page_base: 0x%x' \
                     %(self.rx_pdu_context_ring_base, self.rx_pdu_context_page_base))
        logger.info('   - tx_hwxtscb_addr: 0x%x tx_hwdgstcb_addr: 0x%x' \
                     %(self.tx_hwxtscb_addr, self.tx_hwdgstcb_addr))
        logger.info('   - rx_hwxtscb_addr: 0x%x rx_hwdgstcb_addr: 0x%x' \
                     %(self.rx_hwxtscb_addr, self.rx_hwdgstcb_addr))
        return

    def PrepareHALRequestSpec(self, req_spec):
        if (GlobalOptions.dryrun): return
        req_spec.max_cmd_context = self.spec.max_cmd_context
        req_spec.max_ns = self.spec.max_ns
        req_spec.max_sess = self.spec.max_sess
        req_spec.tx_max_pdu_context = self.spec.tx_max_pdu_context
        req_spec.rx_max_pdu_context = self.spec.rx_max_pdu_context
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("ProcessHALResponse:: NVMe Global Init")
        self.cmd_context_ring_base = resp_spec.cmd_context_ring_base
        self.cmd_context_page_base = resp_spec.cmd_context_page_base
        self.resourcecb_addr = resp_spec.resourcecb_addr
        self.tx_pdu_context_ring_base = resp_spec.tx_pdu_context_ring_base
        self.tx_pdu_context_page_base = resp_spec.tx_pdu_context_page_base
        self.rx_pdu_context_ring_base = resp_spec.rx_pdu_context_ring_base
        self.rx_pdu_context_page_base = resp_spec.rx_pdu_context_page_base
        self.tx_hwxtscb_addr = resp_spec.tx_hwxtscb_addr
        self.tx_hwdgstcb_addr = resp_spec.tx_hwdgstcb_addr
        self.rx_hwxtscb_addr = resp_spec.rx_hwxtscb_addr
        self.rx_hwdgstcb_addr = resp_spec.rx_hwdgstcb_addr
        return

    def ResourcecbRead(self, debug=True):
        cb_size = len(NvmeResourcecb())
        if (GlobalOptions.dryrun):
            return NvmeResourcecb(bytes(cb_size))
               
        if debug is True:
            logger.info("Read Resourcecb @0x%x size: %d" % (self.resourcecb_addr, cb_size))

        cb = NvmeResourcecb(model_wrap.read_mem(self.resourcecb_addr, cb_size))
        logger.ShowScapyObject(cb)
        return cb
        
    def HwxtstxcbRead(self, debug=True):
        cb_size = len(NvmeHwXtsTxcb())
        if (GlobalOptions.dryrun):
            return NvmeHwXtsTxcb(bytes(cb_size))
               
        if debug is True:
            logger.info("Read Hwxtstxcb @0x%x size: %d" % (self.tx_hwxtscb_addr, cb_size))

        cb = NvmeHwXtsTxcb(model_wrap.read_mem(self.tx_hwxtscb_addr, cb_size))
        logger.ShowScapyObject(cb)
        return cb
        
    def HwdgsttxcbRead(self, debug=True):
        cb_size = len(NvmeHwDgstTxcb())
        if (GlobalOptions.dryrun):
            return NvmeHwDgstTxcb(bytes(cb_size))
               
        if debug is True:
            logger.info("Read Hwdgsttxcb @0x%x size: %d" % (self.tx_hwdgstcb_addr, cb_size))

        cb = NvmeHwDgstTxcb(model_wrap.read_mem(self.tx_hwdgstcb_addr, cb_size))
        logger.ShowScapyObject(cb)
        return cb

#NvmeGlobalHelper = NvmeGlobalObject()
