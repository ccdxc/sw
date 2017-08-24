#! /usr/bin/python3

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
        self.ltep       = self.tenant.local_tep
        self.ports      = self.remote_ep.GetInterface().ports
        self.vlan_id    = self.remote_ep.segment.vlan_id
        self.macaddr    = self.remote_ep.segment.macaddr
        self.rmacaddr   = self.remote_ep.macaddr
        self.Show()
        return

    def Show(self):
        cfglogger.info("Tunnel = %s" % self.GID())
        cfglogger.info("- Tenant      = %s" % self.tenant.GID())
        cfglogger.info("- EncapType   = %s" % self.encap_type)
        cfglogger.info("- LocalTep    = %s" % self.ltep.get())
        cfglogger.info("- RemoteTep   = %s" % self.rtep.get())
        cfglogger.info("- Interface   = %s" % self.remote_ep.GetInterface().GID())
        cfglogger.info("- Ports       =", self.ports)
        return

    def IsVxlan(self):
        return self.encap_type == 'VXLAN'

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.tenant_id = self.tenant.id
        req_spec.key_or_handle.interface_id = self.id
        req_spec.type = self.type
        req_spec.admin_status = self.status
        if self.IsVxlan():
            req_spec.if_tunnel_info.encap_type = haldefs.interface.IF_TUNNEL_ENCAP_TYPE_VXLAN
            # Local TEP
            req_spec.if_tunnel_info.vxlan_info.local_tep.ip_af = haldefs.common.IP_AF_INET 
            req_spec.if_tunnel_info.vxlan_info.local_tep.v4_addr = self.ltep.getnum()
            # Remote TEP
            req_spec.if_tunnel_info.vxlan_info.remote_tep.ip_af = haldefs.common.IP_AF_INET
            req_spec.if_tunnel_info.vxlan_info.remote_tep.v4_addr = self.rtep.getnum()
        else:
            assert(0)
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
