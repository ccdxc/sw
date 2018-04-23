#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import config.resmgr            as resmgr
import config.objects.endpoint  as endpoint

from infra.common.logging       import logger
from config.store               import Store

import config.hal.api            as halapi
import config.hal.defs           as haldefs

ENIC_TYPE_DIRECT= 'DIRECT'
ENIC_TYPE_USEG  = 'USEG'
ENIC_TYPE_PVLAN = 'PVLAN'

class L4LbBackendObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.id = resmgr.L4LbBackendIdAllocator.get()
        self.GID("L4LbBknd%04d" % self.id)
        return

    def Init(self, service, spec, pick_ep_in_l2seg_vxlan):
        self.service    = service
        self.port       = spec.port
        self.remote     = spec.remote
        if hasattr(spec, 'tnnled'):
            self.tnnled = spec.tnnled
        else:
            self.tnnled  = False
        self.ep         = service.tenant.AllocL4LbBackend(self.remote, self.tnnled, pick_ep_in_l2seg_vxlan)
        self.ep.AttachL4LbBackend(self)
        return

    def GetInterface(self):
        return self.ep.GetInterface()

    def IsTunneled(self):
        return self.tnnled

    def GetIpAddress(self):
        return self.ep.GetIpAddress()

    def GetIpv6Address(self):
        return self.ep.GetIpv6Address()

    def Show(self):
        logger.info("- Backend = %s  %s:%d" %\
                       (self.GID(), self.ep.GID(), self.port.get()))
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.vrf_id = self.service.tenant.id
        req_spec.backend_key_or_handle.backend_key.backend_ip_address.ip_af = haldefs.common.IP_AF_INET
        req_spec.backend_key_or_handle.backend_key.backend_ip_address.v4_addr = self.GetIpAddress.getnum()
        req_spec.backend_key_or_handle.backend_key.backend_port = self.port.get()
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("- Backend %s = %s" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        self.hal_handle = resp_spec.status.service_handle
        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/Port:%d' % self.port.get()
        summary += '/EP:' + self.ep.Summary()
        return summary

class L4LbBackendObjectHelper:
    def __init__(self):
        self.bends  = []
        return

    def Configure(self):
        loggers("Configuring %d L4LbBackends" % len(self.bends))
        halapi.ConfigureL4LbBackends(self.bends)
        return

    def Generate(self, service, bspec_list):
        for bspec in bspec_list:
            count = bspec.count.get()
            for c in range(count):
                bend = L4LbBackendObject()
                if c < count/2:
                    logger.info("Picking up non-vxlan EP for Backend");
                    pick_ep_in_l2seg_vxlan = False
                else:
                    logger.info("Picking up vxlan EP for Backend");
                    pick_ep_in_l2seg_vxlan = True
                bend.Init(service, bspec, pick_ep_in_l2seg_vxlan)
                self.bends.append(bend)
        return

class L4LbServiceObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('L4LB_SERVICE'))
        self.id = resmgr.L4LbServiceIdAllocator.get()
        self.GID("L4LbService%04d" % self.id)
        return

    def Init(self, tenant, spec):
        self.tenant     = tenant
        self.spec       = spec
        self.label      = spec.label.upper()
        self.proto      = spec.proto.upper()

        self.vip        = resmgr.L4LbServiceIpAllocator.get()
        self.vip6       = resmgr.L4LbServiceIpv6Allocator.get()
        self.macaddr    = resmgr.L4LbVMacAllocator.get()

        self.port       = spec.port
        self.mode       = spec.mode.upper()
        self.snat_ips   = []
        self.snat_ipv6s = []
        self.snat_ports = []
        if self.IsTwiceNAT():
            for c in range(spec.snat_ips.GetCount()):
                sip = spec.snat_ips.get()
                sipv6 = spec.snat_ipv6s.get()
                self.snat_ips.append(sip)
                self.snat_ipv6s.append(sipv6)
                # Handle for ports 0 for IP only Twice NAT
                if spec.snat_ports is not None:
                    sport = spec.snat_ports.get()
                    self.snat_ports.append(sport)

        self.bend_idx = 0
        self.snat_idx = 0
        self.obj_helper_bend = L4LbBackendObjectHelper()
        self.obj_helper_bend.Generate(self, spec.backends)

        self.__create_ep()
        self.Show()
        return

    def IsNAT(self):
        return self.mode == 'NAT'

    def IsTwiceNAT(self):
        return self.mode == 'TWICE_NAT'

    def IsNATDSR(self):
        return self.mode == 'NAT_DSR'

    def SelectBackend(self):
        bend = self.obj_helper_bend.bends[self.bend_idx]
        self.bend_idx = (self.bend_idx + 1) % len(self.obj_helper_bend.bends)
        return bend

    def SelectServiceSNat(self):
        # Valid only for twice NAT
        snat_ip   = self.snat_ips[self.snat_idx]
        snat_ipv6 = self.snat_ipv6s[self.snat_idx]

        # Handle if no elements in snat_ports. IP Only twice NAT
        snat_port = 0
        if len(self.snat_ports) != 0:
            snat_port = self.snat_ports[self.snat_idx]
        self.snat_idx = (self.snat_idx + 1) % len(self.snat_ips)
        return snat_ip, snat_ipv6, snat_port

    def __create_ep(self):
        # Create a dummy endpoint for the service
        self.ep = endpoint.EndpointObject()
        self.ep.macaddr = self.macaddr
        self.ep.SetIpAddress(self.vip)
        self.ep.SetIpv6Address(self.vip6)
        self.ep.SetL4LbService()
        self.ep.tenant = self.tenant
        return


    def Show(self):
        logger.info("L4LbService: %s" % self.GID())
        logger.info("- Label   = %s" % self.label)
        logger.info("- Mode    = %s" % self.mode)
        logger.info("- Tenant  = %s" % self.tenant.GID())
        logger.info("- Proto   = %s" % self.proto)
        logger.info("- VIP     = %s" % self.vip.get())
        logger.info("- VMac    = %s" % self.macaddr.get())
        logger.info("- Port    = %s" % self.port.get())
        for s in range(len(self.snat_ips)):
            if len(self.snat_ports) != 0:
                logger.info("- SNAT Ipaddr:Ipv6addr:Port = %s:%s:%d" %\
                              (self.snat_ips[s].get(), self.snat_ipv6s[s].get(),\
                              self.snat_ports[s]))
            else:
                logger.info("- SNAT Ipaddr:Port = %s:%s:%d" %\
                              (self.snat_ips[s].get(), self.snat_ipv6s[s].get(), 0))
        for bend in self.obj_helper_bend.bends:
            bend.Show()

        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/Label:%s' % self.label
        summary += '/Mode:%s' % self.mode
        summary += '/VIP:%s' % self.vip.get()
        summary += '/VIP6:%s' % self.vip6.get()
        summary += '/VMac:%s' % self.macaddr.get()
        summary += '/Proto:%s' % self.proto
        return summary

    def PrepareHALRequestSpec(self, req_spec):
        if gl_l4lb_config_ipv4:
            logger.info("Configuring IPv4 L4LbService: %s" % self.GID())
        else:
            logger.info("Configuring IPv4 L4LbService: %s" % self.GID())

        req_spec.meta.vrf_id = self.tenant.id

        hal_ipproto_str = 'IPPROTO_' + self.proto
        hal_ipproto = haldefs.common.IPProtocol.Value(hal_ipproto_str)
        req_spec.key_or_handle.service_key.ip_protocol = hal_ipproto
        req_spec.key_or_handle.service_key.service_port = self.port.get()
        req_spec.service_mac = self.macaddr.getnum()

        if gl_l4lb_config_ipv4:
            req_spec.key_or_handle.service_key.service_ip_address.ip_af = haldefs.common.IP_AF_INET
            req_spec.key_or_handle.service_key.service_ip_address.v4_addr = self.vip.getnum()
        else:
            req_spec.key_or_handle.service_key.service_ip_address.ip_af = haldefs.common.IP_AF_INET6
            req_spec.key_or_handle.service_key.service_ip_address.v6_addr = self.vip6.getnum().to_bytes(16, 'big')

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        if gl_l4lb_config_ipv4:
            self.ipv4_hal_handle = resp_spec.status.service_handle
            handle = self.ipv4_hal_handle
        else:
            self.ipv6_hal_handle = resp_spec.status.service_handle
            handle = self.ipv4_hal_handle

        status = haldefs.common.ApiStatus.Name(resp_spec.api_status)
        logger.info("- L4LbService %s = %s (HDL = 0x%x)" % (self.GID(), status, handle))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

class L4LbServiceObjectHelper:
    def __init__(self):
        self.svcs = []
        return

    def Generate(self, tenant, spec):
        for espec in spec.entries:
            svc = L4LbServiceObject()
            svc.Init(tenant, espec.entry)
            self.svcs.append(svc)
        Store.objects.SetAll(self.svcs)
        return

    def Configure(self):
        global gl_l4lb_config_ipv4
        if not len(self.svcs): return
        logger.info("Configuring %d IPv4 L4LbServices." % len(self.svcs))
        gl_l4lb_config_ipv4 = True
        halapi.ConfigureL4LbServices(self.svcs)

        logger.info("Configuring %d IPv6 L4LbServices." % len(self.svcs))
        gl_l4lb_config_ipv4 = False
        halapi.ConfigureL4LbServices(self.svcs)

        gl_l4lb_config_ipv4 = None
        return
