#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment
import config.objects.tenant    as tenant

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from infra.common.glopts        import GlobalOptions
from infra.common.logging       import logger
from config.store               import Store

class GftTranspositionProfileObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('GFT_TRSPN_PROFILE'))
        return

    def Init(self, spec):
        self.spec = spec
        self.GID(spec.name)
        self.id = resmgr.GftTranspositionProfileIdAllocator.get()
        self.groups = []
        self.redirect_to_vport_ingress_queue = getattr(spec, 'redirect_to_vport_ingress_queue',
                                                       self.redirect_to_vport_ingress_queue)
        self.redirect_to_vport_egress_queue = getattr(spec, 'redirect_to_vport_egress_queue',
                                                       self.redirect_to_vport_egress_queue)
        self.redirect_to_vport_ingress_queue_if_ttl_is_one = getattr(spec, 'redirect_to_vport_ingress_queue_if_ttl_is_one',
                                                                     self.redirect_to_vport_ingress_queue_if_ttl_is_one)
        self.redirect_to_vport_egress_queue_if_ttl_is_one = getattr(spec, 'redirect_to_vport_egress_queue_if_ttl_is_one',
                                                                    self.redirect_to_vport_egress_queue_if_ttl_is_one)
        self.copy_all_packets = getattr(spec, 'copy_all_packets',
                                        self.copy_all_packets)
        self.copy_first_packet = getattr(spec, 'copy_first_packet',
                                         self.copy_first_packet)
        self.copy_when_tcp_flag_set = getattr(spec, 'copy_when_tcp_flag_set',
                                              self.copy_when_tcp_flag_set)
        self.copy_after_tcp_fin_flag_set = getattr(spec, 'copy_after_tcp_fin_flag_set',
                                                   self.copy_after_tcp_fin_flag_set)
        self.copy_after_tcp_rst_flag_set = getattr(spec, 'copy_after_tcp_rst_flag_set',
                                                    self.copy_after_tcp_rst_flag_set)
        self.custom_action_present = getattr(spec, 'custom_action_present',
                                         self.custom_action_present)
        self.meta_action_before_transposition = getattr(spec, 'meta_action_before_transposition',
                                                        self.meta_action_before_transposition)
        self.__process_groups()
        self.Show()
        return

    def __process_groups(self):
        spec_groups = getattr(self.spec, 'groups', [])
        for g in spec_groups:
            group = g.Get(Store)
            self.groups.append(group)
        return

    def FillTranspositionActions(self, req_spec):
        req_spec.redirect_to_vport_ingress_queue = self.redirect_to_vport_ingress_queue
        req_spec.redirect_to_vport_egress_queue = self.redirect_to_vport_egress_queue
        req_spec.redirect_to_vport_ingress_queue_if_ttl_is_one = self.redirect_to_vport_ingress_queue_if_ttl_is_one
        req_spec.redirect_to_vport_egress_queue_if_ttl_is_one = self.redirect_to_vport_egress_queue_if_ttl_is_one
        req_spec.copy_all_packets = self.copy_all_packets
        req_spec.copy_first_packet = self.copy_first_packet
        req_spec.copy_when_tcp_flag_set = self.copy_when_tcp_flag_set
        req_spec.copy_after_tcp_fin_flag_set = self.copy_after_tcp_fin_flag_set
        req_spec.copy_after_tcp_rst_flag_set = self.copy_after_tcp_rst_flag_set
        req_spec.custom_action_present = self.custom_action_present
        req_spec.meta_action_before_transposition = self.meta_action_before_transposition
        return

    def Show(self):
        logger.info("Transposition Profile : %s" % self.GID())
        logger.info("- ID: ", self.id)

        redirect_actions = ""
        if self.redirect_to_vport_ingress_queue:
            redirect_actions += "VportIngressQ,"
        if self.redirect_to_vport_egress_queue:
            redirect_actions += "VportEgressQ,"
        if self.redirect_to_vport_ingress_queue_if_ttl_is_one:
            redirect_actions += "VportIngressQTtl1,"
        if self.redirect_to_vport_egress_queue_if_ttl_is_one:
            redirect_actions += "VportEgressQTtl1,"
        logger.info("- Redirect: %s" % redirect_actions)

        copy_actions = ""
        if self.copy_all_packets:
            copy_actions += "CopyAll,"
        if self.copy_first_packet:
            copy_actions += "CopyFirstPacket,"
        if self.copy_when_tcp_flag_set:
            copy_actions += "CopyWhenTcpFlagSet,"
        if self.copy_after_tcp_fin_flag_set:
            copy_actions += "CopyAfterFinFlagSet,"
        if self.copy_after_tcp_rst_flag_set:
            copy_actions += "CopyAfterRstFlagSet,"
        logger.info("- Copy: %s" % copy_actions)
        if self.custom_action_present:
            logger.info("- Custom Action: True")
        if self.meta_action_before_transposition:
            logger.info("- Meta Action Before Transposition: True")

        return

class GftTranspositionProfileObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Generate(self, topospec):
        gftexm_spec = getattr(topospec, 'gftexm', None)
        if gftexm_spec is None: return

        spec = getattr(gftexm_spec, 'transpositions', None)
        trs = spec.Get(Store)

        logger.info("Adding GFT Transposition Profiles to Store.")
        for entry in trs.transposition_profiles:
            trp = entry.profile
            obj = GftTranspositionProfileObject()
            obj.Init(trp)
            self.objlist.append(obj)
            Store.objects.Set(obj.GID(), obj)
        return

    def main(self, topospec):
        self.Generate(topospec)
        return

    def GetAll(self):
        return self.objlist

GftTranspositionProfileHelper = GftTranspositionProfileObjectHelper()
