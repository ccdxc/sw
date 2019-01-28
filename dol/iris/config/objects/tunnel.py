#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr               as resmgr
import iris.config.objects.segment      as segment

import iris.config.hal.api              as halapi
import iris.config.hal.defs             as haldefs

from infra.common.logging               import logger
from iris.config.store                  import Store

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
            logger.info(" found %d local eps for tunnel using %s" % (len(leps), leps[0].ipaddrs[0].get()))
            self.ltep = leps[0].ipaddrs[0]

        if remote_ep.IsRemote():
            self.local_dest = False
        else:
            self.local_dest = True
        if 'ports' not in self.remote_ep.GetInterface().__dict__:
            self.ports = []
            self.ports.append(self.remote_ep.GetInterface())
        else:
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

    def GetDistLabel(self):
        intf = self.remote_ep.GetInterface()
        return intf.GetDistLabel()

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
        logger.info("Tunnel = %s" % self.GID())
        logger.info("- Tenant      = %s" % self.tenant.GID())
        logger.info("- EncapType   = %s" % self.encap_type)
        if 'local_tep' in self.tenant.__dict__:
            logger.info("- LocalTep    = %s" % self.ltep.get())
        logger.info("- RemoteTep   = %s" % self.rtep.get())
        logger.info("- Local Dest  = ", self.local_dest)
        logger.info("- Interface   = %s" % self.remote_ep.GetInterface().GID())
        logger.info("- Ports       =", self.ports)
        logger.info("- txqos: Cos:%s/Dscp:%s" %\
                       (str(self.txqos.cos), str(self.txqos.dscp)))
        logger.info("- rxqos: Cos:%s/Dscp:%s" %\
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
        req_spec.key_or_handle.interface_id = self.id
        req_spec.type = self.type
        req_spec.admin_status = self.status
        req_spec.if_tunnel_info.vrf_key_handle.vrf_id = self.tenant.id
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
        elif self.encap_type == "MPLS_UDP":
            #import pdb; pdb.set_trace()
            # Substrate IP (DIPo incoming)
            req_spec.key_or_handle.interface_id = 3
            req_spec.if_tunnel_info.encap_type = haldefs.interface.IF_TUNNEL_ENCAP_TYPE_PROPRIETARY_MPLS
            req_spec.if_tunnel_info.prop_mpls_info.substrate_ip.ip_af = haldefs.common.IP_AF_INET
            req_spec.if_tunnel_info.prop_mpls_info.substrate_ip.v4_addr = self.ltep.getnum() # 64.0.0.2
            
            # Tunnel dest IP (DIPo outgoing)
            req_spec.if_tunnel_info.prop_mpls_info.tunnel_dest_ip.ip_af = haldefs.common.IP_AF_INET
            req_spec.if_tunnel_info.prop_mpls_info.tunnel_dest_ip.v4_addr = self.rtep.getnum() # 64.0.0.1
            
            # Overlay IP (DIPi incoming)
            oip = req_spec.if_tunnel_info.prop_mpls_info.overlay_ip.add()
            oip.ip_af = haldefs.common.IP_AF_INET
            oip.v4_addr = 336860180 #20.20.20.20
            oip2 = req_spec.if_tunnel_info.prop_mpls_info.overlay_ip.add()
            oip2.ip_af = haldefs.common.IP_AF_INET
            oip2.v4_addr = 505290270 #30.30.30.30
            
            # MPLS if (MPLS-tag incoming)
            mplsif = req_spec.if_tunnel_info.prop_mpls_info.mpls_if.add()
            mplsif.label = 12345
            mplsif.exp = 0
            mplsif.ttl = 64
            mplsif2 = req_spec.if_tunnel_info.prop_mpls_info.mpls_if.add()
            mplsif2.label = 67890
            mplsif2.exp = 0
            mplsif2.ttl = 64
            
            # MPLS tag (MPLS-tag outgoing)
            req_spec.if_tunnel_info.prop_mpls_info.mpls_tag.label = 54321
            req_spec.if_tunnel_info.prop_mpls_info.mpls_tag.exp = 0
            req_spec.if_tunnel_info.prop_mpls_info.mpls_tag.ttl = 64
            
            # Source GW (IPv4 prefix) 10.10.10.0/24
            req_spec.if_tunnel_info.prop_mpls_info.source_gw.prefix.ipv4_subnet.address.ip_af = haldefs.common.IP_AF_INET
            req_spec.if_tunnel_info.prop_mpls_info.source_gw.prefix.ipv4_subnet.address.v4_addr = 168430080 #10.10.10.0/24
            req_spec.if_tunnel_info.prop_mpls_info.source_gw.prefix.ipv4_subnet.prefix_len = 24
            
            # Ingress/Egress BW
            req_spec.if_tunnel_info.prop_mpls_info.ingress_bw = 100000
            req_spec.if_tunnel_info.prop_mpls_info.egress_bw = 100000
            
            # GW mac
            req_spec.if_tunnel_info.prop_mpls_info.gw_mac_da = 12345
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.if_handle
        logger.info("- Tunnel %s = %s (HDL = 0x%x)" %\
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
        logger.info("Configuring %d Tunnels." % len(self.tunnels))
        halapi.ConfigureInterfaces(self.tunnels)
        return

    def Generate(self, tenant, spec, eps):
        logger.info("Creating %d Tunnels for Tenant = %s" %\
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
