#! /usr/bin/python3
import pdb
import json

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr

import iris.config.hal.api           as halapi
import iris.config.agent.api         as agentapi
import iris.config.hal.defs          as haldefs

from iris.config.store               import Store
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions

import iris.config.objects.pd        as pd
import iris.config.objects.cq        as cq
import iris.config.objects.slab      as slab

class AgentEndpointObjectSpec:
    def __init__(self, ep):
        self.EndpointUUID = ep.GID()
        self.WorkloadUUID = ep.GID()
        self.WorkloadName = ep.GID()
        self.NetworkName = ep.segment.GID()
        self.IPv4Address = ep.ipaddrs[0].get() + "/32"
        #self.IPv6Address = ep.ipv6addrs[0].get()
        self.MacAddress = ep.macaddr.get()
        self.NodeUUID = "remote"
        return

class AgentEndpointObjectStatus:
    def __init__(self, ep):
        return


class AgentEndpointObject(base.AgentObjectBase):
    def __init__(self, ep):
        super().__init__("Endpoint", ep.GID(), ep.tenant.GID())
        self.spec = AgentEndpointObjectSpec(ep)
        self.status = AgentEndpointObjectStatus(ep)
        return
    def to_JSON(self):
        ep = dict()
        ep["kind"] = "Endpoint"
        ep["meta"] = {"name":self.meta.Name, "tenant":self.meta.Tenant, "namespace": self.meta.Namespace}
        ep["spec"] = {\
            "endpoint-uuid": self.spec.EndpointUUID,\
            "workload-uuid": self.spec.WorkloadUUID,\
            "workload-name": self.spec.WorkloadName,\
            "network-name": self.spec.NetworkName,\
            "ipv4-address": self.spec.IPv4Address,\
            "mac-address": self.spec.MacAddress,\
            "node-uuid": self.spec.NodeUUID,\
        }
        return json.dumps(ep)

class EndpointObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('ENDPOINT'))
        self.id = resmgr.EpIdAllocator.get()
        self.GID("EP%04d" % self.id)
        self.ipaddrs = []
        self.ipv6addrs = []
        self.security_groups = []
        self.udqps = []
        return

    def Init(self, segment, backend):
        self.label              = None
        self.segment            = segment
        self.local              = True
        self.access             = False
        self.tenant             = segment.tenant
        self.segment            = segment
        self.remote             = False
        self.segment_id         = segment.id
        self.tenant_id          = segment.tenant.id
        self.attachif           = None
        self.pinintf            = None

        self.is_l4lb_backend = backend
        self.is_l4lb_service = False
        self.l4lb_backend = None
        self.last_slab_id = -1
        self.total_mr_slabs = 0

        self.sgs = []

        num_ip_addrs = defs.HAL_NUM_IPADDRS_PER_ENDPOINT
        num_ipv6_addrs = defs.HAL_NUM_IPV6ADDRS_PER_ENDPOINT
        if segment.IsInfraSegment():
            num_ip_addrs = 1
            num_ipv6_addrs = 1

        for ipidx in range(num_ip_addrs):
            ipaddr = segment.AllocIpv4Address(self.IsL4LbBackend())
            self.SetIpAddress(ipaddr)

        for ipidx in range(num_ipv6_addrs):
            ipv6addr = segment.AllocIpv6Address(self.IsL4LbBackend())
            self.SetIpv6Address(ipv6addr)
        self.useg_vlan_id = 0
        return

    def IsBehindTunnel(self):
        # EP is remote & segment's fabencap is vxlan
        return self.remote and self.segment.IsFabEncapVxlan()

    def IsInL2SegVxlan(self):
        return self.segment.IsFabEncapVxlan()

    def IsNative(self):
        return self.segment.native or self.access

    def SetAccessMode(self):
        self.access = True
        return

    def SetLabel(self, label):
        self.label = label
        return

    def IsL4LbBackend(self):
        return self.is_l4lb_backend

    def IsL4LbService(self):
        return self.is_l4lb_service

    def SetL4LbService(self):
        self.is_l4lb_service = True
        return

    def AttachL4LbBackend(self, backend):
        self.l4lb_backend = backend
        return

    def AddSecurityGroup(self, sg):
        if self.segment.IsEndpointSgEnabled():
            self.sgs.append(sg)
        return

    def IsRemote(self):
        return self.remote

    def SetRemote(self):
        self.remote = True
        return

    def SetMacAddress(self, mac):
        self.macaddr = mac
        return
    def GetMacAddress(self):
        return self.macaddr

    def SetIpAddress(self, ip):
        self.ipaddrs.append(ip)
        return
    def GetIpAddress(self, idx = 0):
        return self.ipaddrs[idx]

    def SetIpv6Address(self, ip6):
        self.ipv6addrs.append(ip6)
        return
    def GetIpv6Address(self, idx = 0):
        return self.ipv6addrs[idx]

    def GetInterface(self):
        return self.intf

    def GetUdQps(self):
        return self.udqps

    def AttachInterface(self, intf):
        self.intf = intf
        if self.remote:
            self.SetMacAddress(resmgr.RemoteEpMacAllocator.get())
        else:
            self.attachif = intf
            logger.info("- %s Saving if %s" % (self.GID(), self.attachif))
            self.SetMacAddress(intf.macaddr)
        self.Show()
        return

    def GetTxQosCos(self):
        return self.intf.GetTxQosCos()

    def GetRxQosCos(self):
        return self.intf.GetRxQosCos()

    def GetTxQosDscp(self):
        return self.intf.GetTxQosDscp()

    def GetRxQosDscp(self):
        return self.intf.GetRxQosDscp()

    def __copy__(self):
        endpoint = EndpointObject()
        endpoint.id = self.id
        endpoint.hal_handle = self.hal_handle
        endpoint.tenant_id = self.tenant_id
        endpoint.segment_id = self.segment_id
        endpoint.intf_id = self.intf_id
        endpoint.macaddr = self.macaddr
        endpoint.ipaddrs = self.ipaddrs[:]
        endpoint.ipv6addrs = self.ipv6addrs[:]
        endpoint.udqps = self.udqps[:]
        return endpoint

    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False

        fields = ["tenant_id", "hal_handle", "segment_id", "intf_id",
                  "macaddr", "ipaddrs", "ipv6addrs"]
        if not self.CompareObjectFields(other, fields, lgh):
            return False

        return True

    def Show(self):
        logger.info("Endpoint = %s(%d)" % (self.GID(), self.id))
        logger.info("- IsBackend = %s" % self.IsL4LbBackend())
        logger.info("- Access    = %s" % self.access)
        logger.info("- Tenant    = %s" % self.tenant)
        logger.info("- Macaddr   = %s" % self.macaddr.get())
        logger.info("- Interface = %s" % self.intf.GID())
        for ipaddr in self.ipaddrs:
            logger.info("- Ipaddr    = %s" % ipaddr.get())
        for ipv6addr in self.ipv6addrs:
            logger.info("- Ipv6addr  = %s" % ipv6addr.get())
        for qp in self.udqps:
            logger.info("- UdQp: QP: %s PD: %s " % (qp.GID(), qp.pd.GID()))
        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/Mac:%s' % self.macaddr.get()
        summary += '/Remote:%s' % self.remote
        summary += '/Bkend:%s' % self.IsL4LbBackend()
        summary += '/Acc:%s' % self.access
        return summary

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.vrf_key_handle.vrf_id     = self.tenant.id
        req_spec.key_or_handle.endpoint_key.l2_key.l2segment_key_handle.segment_id = self.segment.id
        req_spec.endpoint_attrs.interface_key_handle.interface_id   = self.intf.id
        req_spec.key_or_handle.endpoint_key.l2_key.mac_address = self.macaddr.getnum()
        #Interface should be created by now.
        self.segment_id     = self.segment.id
        self.intf_id        = self.intf.id

        if GlobalOptions.classic is False:
            if not self.segment.IsIPV4EpLearnEnabled(remote_ep = self.IsRemote()):
                for ipaddr in self.ipaddrs:
                    ip = req_spec.endpoint_attrs.ip_address.add()
                    ip.ip_af = haldefs.common.IP_AF_INET
                    ip.v4_addr = ipaddr.getnum()

            if not self.segment.IsIPV6EpLearnEnabled(remote_ep = self.IsRemote()):
                for ipv6addr in self.ipv6addrs:
                    ip = req_spec.endpoint_attrs.ip_address.add()
                    ip.ip_af = haldefs.common.IP_AF_INET6
                    ip.v6_addr = ipv6addr.getnum().to_bytes(16, 'big')

            for sg in self.sgs:
                sg_key_handle = req_spec.endpoint_attrs.sg_key_handle.add()
                sg_key_handle.security_group_handle = sg.hal_handle
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.endpoint_status.key_or_handle.endpoint_handle
        if self.attachif is not None:
            self.pinintf = self.attachif.pinnedif
        logger.info("- Endpoint %s = %s (HDL = 0x%x) (Pinned Uplink = %s)" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hal_handle, self.pinintf))
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        get_req_spec.vrf_key_handle.vrf_id = self.tenant.id
        get_req_spec.key_or_handle.endpoint_handle = self.hal_handle
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        self.tenant_id = get_resp.spec.vrf_key_handle.vrf_id
        self.segment_id = get_resp.spec.key_or_handle.endpoint_key.l2_key.l2segment_key_handle.segment_id
        self.intf_id = get_resp.spec.endpoint_attrs.interface_key_handle.interface_id
        self.macaddr = objects.MacAddressBase(integer=get_resp.spec.key_or_handle.endpoint_key.l2_key.mac_address)

        self.ipaddrs = []
        self.ipv6addrs = []
        for ipaddr in get_resp.spec.endpoint_attrs.ip_address:
            if ipaddr.ip_af == haldefs.common.IP_AF_INET:
                self.ipaddrs.append(objects.IpAddress(integer=ipaddr.v4_addr))
            else:
                self.ipv6addrs.append(objects.Ipv6Address(integer=ipaddr.v6_addr))

    def Get(self):
        halapi.GetEndpoints([self])

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def CreateCqs(self, spec):
        self.cqs = objects.ObjectDatabase()
        self.obj_helper_cq = cq.CqObjectHelper()
        self.obj_helper_cq.Generate(self, spec)
        if len(self.obj_helper_cq.cqs):
            self.cqs.SetAll(self.obj_helper_cq.cqs)
        logger.debug("In CreateCqs, Endpoint %s" % (self.GID()))

    def CreatePds(self, spec):
        self.pds = objects.ObjectDatabase()
        self.obj_helper_pd = pd.PdObjectHelper()
        self.obj_helper_pd.Generate(self, spec)
        self.pds.SetAll(self.obj_helper_pd.pds)
        logger.debug("In CreatePds, Endpoint %s" % (self.GID()))
        for eppd in self.obj_helper_pd.pds:
            logger.debug("   Adding QPs for PD %s, Num of Qps %d" % (eppd.GID(), len(eppd.udqps)))
            pdudqps = eppd.udqps.GetAll()
            for qp in pdudqps:
                logger.debug("      Adding QP: PD %s, QP %s" % (eppd.GID(), qp.GID()))
                self.udqps.append(qp)
        logger.debug("   Total UDQPs in this endpoint: Qps %d" % (len(self.udqps)))

    def ConfigureCqs(self):
        self.obj_helper_cq.Configure()

    def ConfigurePds(self):
        self.obj_helper_pd.Configure()

    def CreateSlabs(self, spec):
        logger.debug("In CreateSlabs, Endpoint %s" % (self.GID()))
        self.slab_allocator = objects.TemplateFieldObject("range/0/2048")
        self.slabs = objects.ObjectDatabase()
        self.mr_slabs = objects.ObjectDatabase()
        self.obj_helper_slab = slab.SlabObjectHelper()
        self.obj_helper_slab.Generate(self.intf.lif, spec)
        self.slabs.SetAll(self.obj_helper_slab.slabs)
        self.mr_slabs.SetAll(self.obj_helper_slab.slabs)
        self.total_mr_slabs += len(self.mr_slabs)

    def AddSlab(self, slab):
        self.obj_helper_slab.AddSlab(slab)
        self.slabs.Add(slab)

    def ConfigureSlabs(self):
        self.obj_helper_slab.Configure()

    def GetNewSlab(self):
        self.last_slab_id = (self.last_slab_id + 1) % self.total_mr_slabs
        logger.info("- # New slab on EP %s assigned: %s" % (self.GID(), 'SLAB%04d' % self.last_slab_id))
        return self.slabs.Get('SLAB%04d' % self.last_slab_id)

    def PrepareAgentObject(self):
        return AgentEndpointObject(self)


# Helper Class to Generate/Configure/Manage Endpoint Objects.
class EndpointObjectHelper:
    def __init__(self):
        self.direct     = []
        self.useg       = []
        self.pvlan      = []
        self.local      = []
        self.remote     = []
        self.eps        = []

        self.backend_direct     = []
        self.backend_useg       = []
        self.backend_pvlan      = []
        self.backend_local      = []
        self.backend_remote     = []
        self.backend_eps        = []
        return

    def Show(self):
        string = ''
        if len(self.direct):
            string += 'Direct:%d ' % len(self.direct)
        if len(self.useg):
            string += 'Useg:%d ' % len(self.useg)
        if len(self.pvlan):
            string += 'Pvlan:%d ' % len(self.pvlan)
        if len(self.local):
            string += 'Local:%d ' % len(self.local)
        if len(self.remote):
            string += 'Remote:%d ' % len(self.remote)
        if len(self.eps):
            string += 'Total:%d' % len(self.eps)
        logger.info("- # EP: %s" % string)
        return

    def Configure(self):
        logger.info("Configuring %d Endpoints." % len(self.eps))
        api = agentapi if GlobalOptions.agent else halapi
        if GlobalOptions.classic:
            api.ConfigureEndpoints(self.local)
        else:
            api.ConfigureEndpoints(self.eps)
            api.ConfigureEndpoints(self.backend_eps)

        if self.rdma:
            for ep in self.local:
                ep.ConfigureSlabs()
            for ep in self.eps:
                ep.ConfigureCqs()
                ep.ConfigurePds()
        return

    def __create(self, segment, intfs, count,
                 remote = False, backend = False,
                 access = False):
        eps = []
        #if count > len(intfs):
        #    count = len(intfs)
        for e in range(count):
            intf = intfs[e % len(intfs)]
            ep = EndpointObject()
            # Setup EP <--> ENIC association
            ep.Init(segment, backend)
            if remote:
                ep.SetRemote()
            if access:
                ep.SetAccessMode()
            ep.AttachInterface(intf)
            if not remote:
                intf.AttachEndpoint(ep)
            eps.append(ep)
        return eps

    def __create_remote(self, segment, spec):
        if spec.remote:
            logger.info("Creating %d REMOTE EPs in Segment = %s" %\
                           (spec.remote, segment.GID()))
            if GlobalOptions.classic:
                remote_intfs = [ segment.tenant.GetPinIf() ]
            elif segment.IsFabEncapVlan():
                remote_intfs = Store.GetTrunkingUplinks()
            elif segment.IsFabEncapVxlan():
                remote_intfs = Store.GetTunnelsVxlan()
            self.remote = self.__create(segment, remote_intfs,
                                        spec.remote,
                                        remote = True,
                                        backend = False)
            if segment.l4lb:
                logger.info("Creating %d REMOTE L4LB Backend EPs in Segment = %s" %\
                               (spec.remote, segment.GID()))
                self.backend_remote = self.__create(segment, remote_intfs,
                                                    spec.remote,
                                                    remote = True,
                                                    backend = True)
        return

    def __create_local(self, segment, spec, typestr, enic_cb):
        eps = []
        bend_eps = []
        count = getattr(spec, typestr, None)
        if count is None: return eps,bend_eps
        logger.info("Creating %d %s EPs in Segment = %s" %\
                       (count, typestr, segment.GID()))
        enics = enic_cb()
        eps = self.__create(segment, enics, count)
        self.local += eps

        if segment.l4lb:
            logger.info("Creating %d %s L4LB Backend EPs in Segment = %s" %\
                           (count, typestr, segment.GID()))
            enics = enic_cb(backend = True)
            bend_eps = self.__create(segment, enics, count, backend = True)
            self.backend_local += bend_eps

        return eps,bend_eps

    def __create_local_all(self, segment, spec):
        self.direct,self.backend_direct = self.__create_local(segment, spec, 'direct',
                                                              segment.GetDirectEnics)
        self.pvlan,self.backend_pvlan = self.__create_local(segment, spec, 'pvlan',
                                                            segment.GetPvlanEnics)
        self.useg,self.backend_useg = self.__create_local(segment, spec, 'useg',
                                                          segment.GetUsegEnics)

        if GlobalOptions.classic:
            logger.info("Creating %d CLASSIC EPs in Segment = %s" %\
                           (spec.classic, segment.GID()))
            classic_enics = segment.tenant.GetClassicEnics()
            self.classic = self.__create(segment, classic_enics, spec.classic,
                                         access = getattr(spec, 'access', False))
            self.local += self.classic
        return

    def __create_cqs(self, spec):
        for ep in self.eps:
            ep.CreateCqs(spec)

    def __create_pds(self, spec):
        for ep in self.eps:
            ep.CreatePds(spec)

    def __create_slabs(self, spec):
        # create slabs only on local eps
        for ep in self.local:
            ep.CreateSlabs(spec)

    def Generate(self, segment, spec):
        self.__create_remote(segment, spec)
        self.__create_local_all(segment, spec)
        self.eps += self.local
        self.eps += self.remote

        self.backend_eps += self.backend_local
        self.backend_eps += self.backend_remote
        if len(self.eps):
            Store.objects.SetAll(self.eps)
        if len(self.backend_eps):
            Store.objects.SetAll(self.backend_eps)

        self.rdma = getattr(spec, 'rdma', False)

        if self.rdma:
            if spec.rdma.slab:
                slab_spec = spec.rdma.slab.Get(Store)
                self.__create_slabs(slab_spec)

            # Create CQs before PDs
            if spec.rdma.cq:
                cq_spec = spec.rdma.cq.Get(Store)
                self.__create_cqs(cq_spec)

            if spec.rdma.pd:
                pd_spec = spec.rdma.pd.Get(Store)
                self.__create_pds(pd_spec)

        return

def GetMatchingObjects(selectors):
    endpoints =  Store.objects.GetAllByClass(EndpointObject)
    eps = []
    for ep in endpoints:
        if ep.IsFilterMatch(selectors.endpoint) and \
            ep.segment.IsFilterMatch(selectors.segment) and \
            ep.segment.tenant.IsFilterMatch(selectors.tenant):
            logger.info("Selecting Endpoint : %s" % ep.GID())
            eps.append(ep)

    return eps
