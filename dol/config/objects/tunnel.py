#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from infra.common.logging       import cfglogger
from config.store               import Store

class TunnelObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('UPLINK'))
        return

    def Init(self, tenant, spec, remote_ep):
        self.id         = resmgr.InterfaceIdAllocator.get()
        self.GID("Tunnel%d" % self.id)
        self.tenant     = tenant
        self.type       = haldefs.interface.IF_TYPE_TUNNEL
        self.status     = haldefs.interface.IF_STATUS_UP
        self.encap_type = spec.encap_type.upper()
        self.remote_ep  = remote_ep
        self.rtep       = remote_ep.GetIpAddress()
        if 'local_tep' in self.tenant.__dict__:
            self.ltep       = self.tenant.local_tep
        else:
            leps = self.tenant.GetLocalEps()
            self.ltep = leps[0].ipaddrs[0]
        self.ports      = self.remote_ep.GetInterface().ports
        self.vlan_id    = self.remote_ep.segment.vlan_id
        self.macaddr    = self.remote_ep.segment.macaddr
        self.rmacaddr   = self.remote_ep.macaddr
        self.__init_qos()
        self.Show()
        return

    def __init_qos(self):
        self.txqos.cos = 7
        self.rxqos.cos = 7
        self.txqos.dscp = 7
        self.rxqos.dscp = 7
        return

    def GetTxQosCos(self):
        return self.txqos.cos

    def GetRxQosCos(self):
        return self.rxqos.cos

    def GetTxQosDscp(self):
        return self.txqos.dscp

    def GetRxQosDscp(self):
        return self.rxqos.dscp

    def GetDestIp(self):
        return self.remote_ep.ipaddrs[0]

    def GetSrcIp(self):
        return self.ltep

    def Show(self):
        cfglogger.info("Tunnel = %s" % self.GID())
        cfglogger.info("- Tenant      = %s" % self.tenant.GID())
        cfglogger.info("- EncapType   = %s" % self.encap_type)
        if 'local_tep' in self.tenant.__dict__:
            cfglogger.info("- LocalTep    = %s" % self.ltep.get())
        cfglogger.info("- RemoteTep   = %s" % self.rtep.get())
        cfglogger.info("- Interface   = %s" % self.remote_ep.GetInterface().GID())
        cfglogger.info("- Ports       =", self.ports)
        cfglogger.info("- txqos: Cos:%s/Dscp:%s" %\
                       (str(self.txqos.cos), str(self.txqos.dscp)))
        cfglogger.info("- rxqos: Cos:%s/Dscp:%s" %\
                       (str(self.rxqos.cos), str(self.rxqos.dscp)))
        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/ID:%d' % self.id
        if 'local_tep' in self.tenant.__dict__:
            summary += '/LocTEP:%s' % self.ltep.get()
        summary += '/RemTEP:%s' % self.rtep.get()
        summary += '/Intf:%s' % self.remote_ep.GetInterface().GID()
        summary += '/Ports:' + str(self.ports)
        return summary


    def IsVxlan(self):
        return self.encap_type == 'VXLAN'

    def IsGRE(self):
        return self.encap_type == 'GRE'

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.vrf_key_handle.vrf_id = self.tenant.id
        req_spec.key_or_handle.interface_id = self.id
        req_spec.type = self.type
        req_spec.admin_status = self.status
        if self.encap_type == "VXLAN":
            req_spec.if_tunnel_info.encap_type = haldefs.interface.IF_TUNNEL_ENCAP_TYPE_VXLAN
            # Local TEP
            req_spec.if_tunnel_info.vxlan_info.local_tep.ip_af = haldefs.common.IP_AF_INET 
            req_spec.if_tunnel_info.vxlan_info.local_tep.v4_addr = self.ltep.getnum()
            # Remote TEP
            req_spec.if_tunnel_info.vxlan_info.remote_tep.ip_af = haldefs.common.IP_AF_INET
            req_spec.if_tunnel_info.vxlan_info.remote_tep.v4_addr = self.rtep.getnum()
        elif self.encap_type == "GRE":
            req_spec.if_tunnel_info.encap_type = haldefs.interface.IF_TUNNEL_ENCAP_TYPE_GRE
            req_spec.if_tunnel_info.gre_info.source.ip_af = haldefs.common.IP_AF_INET
            req_spec.if_tunnel_info.gre_info.source.v4_addr = self.ltep.getnum()
            req_spec.if_tunnel_info.gre_info.destination.ip_af = haldefs.common.IP_AF_INET
            req_spec.if_tunnel_info.gre_info.destination.v4_addr = self.remote_ep.ipaddrs[0].getnum()

        # QOS stuff
        if self.txqos.cos is not None:
            req_spec.tx_qos_actions.marking_spec.pcp_rewrite_en = True
            req_spec.tx_qos_actions.marking_spec.pcp = self.txqos.cos
        if self.txqos.dscp is not None:
            req_spec.tx_qos_actions.marking_spec.dscp_rewrite_en = True
            req_spec.tx_qos_actions.marking_spec.dscp = self.txqos.dscp
 
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.if_handle
        cfglogger.info("- Tunnel %s = %s (HDL = 0x%x)" %\
                       (self.GID(),\
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),\
                        self.hal_handle))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

class TunnelObjectHelper:
    def __init__(self):
        self.tunnels = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d Tunnels." % len(self.tunnels))
        halapi.ConfigureInterfaces(self.tunnels)
        return

    def Generate(self, tenant, spec, eps):
        cfglogger.info("Creating %d Tunnels for Tenant = %s" %\
                       (len(eps), tenant.GID()))
        for ep in eps:
            tunnel = TunnelObject()
            tunnel.Init(tenant, spec, ep)
            self.tunnels.append(tunnel)
        return

    def AddToStore(self):
        Store.objects.SetAll(self.tunnels)
        Store.SetTunnels(self.tunnels)
        return

TunnelHelper = TunnelObjectHelper()
