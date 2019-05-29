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
        return

#NvmeGlobalHelper = NvmeGlobalObject()
