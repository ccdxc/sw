#! /usr/bin/python3
import pdb
import copy
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.hal.api           as halapi
import config.hal.defs          as haldefs

from infra.common.logging       import cfglogger
from config.store               import Store

class DosPolicyObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('DOS_POLICY'))
        return

    def Init(self, tenant, spec):
        self.id = resmgr.DosPolicyAllocator.get()
        self.GID("DP%04d" % self.id)
        self.tenant = tenant
        self.spec = copy.deepcopy(spec)
        self.label = spec.id
        
        self.__init_from_spec()
        self.Show()
        return

    def __init_icmp_msg(self, obj, spec):
        icmpspec = getattr(spec, 'icmp_msg', None)
        obj.service.icmp_msg = icmpspec
        if obj.service.icmp_msg is not None:
            obj.service.icmp_msg.code = spec.icmp_msg.code.get()
            obj.service.icmp_msg.type = spec.icmp_msg.type.get()
        return

    def __init_service(self, obj, spec):
        svcspec = getattr(spec, 'service', None)
        if svcspec is None:
            return
        self.__init_icmp_msg(obj, spec.service)
        obj.service.proto = spec.service.proto.upper()
        obj.service.port = spec.service.port
        if obj.service.port is not None:
            obj.service.port = obj.service.port.get()
        return

    def __init_flood_limits_common(self, obj, spec):
        obj.restrict.pps = spec.restrict_limits.pps.get()
        obj.restrict.burst = spec.restrict_limits.burst.get()
        obj.restrict.duration = spec.restrict_limits.duration.get()

        obj.protect.pps = spec.protect_limits.pps.get()
        obj.protect.burst = spec.protect_limits.burst.get()
        obj.protect.duration = spec.protect_limits.duration.get()
        return

    def __init_tcp_syn_flood_limits(self, obj, spec):
        tcpspec = getattr(spec, 'tcp_syn_flood_limits', None)
        if tcpspec is None:
            return
        self.__init_flood_limits_common(obj.tcp_syn_flood_limits,
                                        spec.tcp_syn_flood_limits)
        return

    def __init_udp_flood_limits(self, obj, spec):
        udpspec = getattr(spec, 'udp_flood_limits', None)
        if udpspec is None:
            return
        self.__init_flood_limits_common(obj.udp_flood_limits,
                                        spec.udp_flood_limits)
        return

    def __init_icmp_flood_limits(self, obj, spec):
        tcpspec = getattr(spec, 'icmp_flood_limits', None)
        if tcpspec is None:
            return
        self.__init_flood_limits_common(obj.icmp_flood_limits,
                                        spec.icmp_flood_limits)
        return

    def __init_other_flood_limits(self, obj, spec):
        tcpspec = getattr(spec, 'other_flood_limits', None)
        if tcpspec is None:
            return
        self.__init_flood_limits_common(obj.other_flood_limits,
                                        spec.other_flood_limits)
        return

    def __init_common(self, obj, spec):
        peersg = getattr(spec, 'peersg', None)
        obj.peersg = peersg
        self.__init_service(obj, spec)
        self.__init_tcp_syn_flood_limits(obj, spec)
        self.__init_udp_flood_limits(obj, spec)
        self.__init_icmp_flood_limits(obj, spec)
        self.__init_other_flood_limits(obj, spec)

    def __init_ingress(self):
        ispec = getattr(self.spec, 'ingress', None)
        if ispec is None:
            return
        self.__init_common(self.ingress, ispec)
        return

    def __init_egress(self):
        espec = getattr(self.spec, 'egress', None)
        if espec is None:
            return
        self.__init_common(self.egress, espec)
        return

    def __init_sgs(self):
        self.sglabels = []
        self.sgs = []
        sglist = getattr(self.spec, 'sgs', None)
        if sglist is None:
            return
        for sgl in sglist:
            self.sglabels.append(sgl.label)

        sgs = self.tenant.GetSecurityGroups()
        for s in sgs:
            if s.GetLabel() in self.sglabels:
                self.sgs.append(s)
        return

    def __init_from_spec(self):
        self.__init_sgs()
        self.__init_ingress()
        self.__init_egress()
        return

    def __show_service(self, obj):
        cfglogger.info("  - Service")
        if obj.icmp_msg is not None:
            cfglogger.info("    - IcmpMsgType=%s" % obj.icmp_msg.type)
            cfglogger.info("    - IcmpMsgCode=%s" % obj.icmp_msg.code)
        if obj.proto is not None:
            cfglogger.info("    - Proto     = %s" % obj.proto)
        if obj.port is not None:
            cfglogger.info("    - Port      = %s" % obj.port)
        return

    def __show_flood_limits_common(self, obj, typestr):
        if obj.pps is not None:
            cfglogger.info("    - %s PPS       : %d" % (typestr, obj.pps))
        if obj.burst is not None:
            cfglogger.info("    - %s Burst PPS : %d" % (typestr, obj.burst))
        if obj.duration is not None:
            cfglogger.info("    - %s Duration  : %d" % (typestr, obj.duration))
        return

    def __show_flood_limits(self, obj, typestr):
        cfglogger.info("  - %s" % typestr)
        self.__show_flood_limits_common(obj.restrict, 'Restrict')
        self.__show_flood_limits_common(obj.protect, 'Protect')
        return

    def __show_dir_common(self, obj, direction):
        cfglogger.info("- %s" % direction)
        if obj.peersg is not None:
            cfglogger.info("  - PeerSG     = %s" % obj.peersg)
        else:
            cfglogger.info("  - PeerSG     = None")
        self.__show_service(obj.service)
        cfglogger.info("- Flood Limits")
        self.__show_flood_limits(obj.tcp_syn_flood_limits, 'TCP')
        self.__show_flood_limits(obj.udp_flood_limits, 'UDP')
        self.__show_flood_limits(obj.icmp_flood_limits, 'ICMP')
        self.__show_flood_limits(obj.other_flood_limits, 'OTHER')
        return

    def Show(self, detail = False):
        cfglogger.info("DosPolicy = %s(%d)" % (self.GID(), self.id))
        cfglogger.info("- Tenant  = %s" % self.tenant.GID())
        cfglogger.info("- Label   = %s" % self.label)
        cfglogger.info("- Security Groups:")
        for s in self.sgs:
            cfglogger.info("  - %s %s" % (s.GID(), s.GetLabel()))
        self.__show_dir_common(self.ingress, 'Ingress')
        self.__show_dir_common(self.egress, 'Egress')
        return

    def __phrs_limits_common(self, req_spec, obj):
        if obj.pps is None:
            return
        req_spec.pps = obj.pps
        req_spec.burst_pps = obj.burst
        req_spec.duration = obj.duration
        return

    def __phrs_flood_limits_common(self, req_spec, obj):
        self.__phrs_limits_common(req_spec.restrict_limits, obj.restrict)
        self.__phrs_limits_common(req_spec.protect_limits, obj.protect)
        return

    def __get_hal_ipproto(self, proto):
        if proto is None:
            return
        #hal_ipproto_str = 'IPPROTO_' + proto
        #hal_ipproto = haldefs.common.IPProtocol.Value(hal_ipproto_str)
        #return hal_ipproto
        return defs.ipprotos.id(proto)

    def __phrs_svc_common_icmp(self, req_spec, obj):
        if obj is None:
            return
        if obj.type is not None:
            req_spec.type = obj.type
        if obj.code is not None:
            req_spec.code = obj.code

    def __phrs_svc_common(self, req_spec, obj):
        if obj.proto is not None:
            req_spec.ip_protocol = self.__get_hal_ipproto(obj.proto) 
        if obj.port is not None:
            req_spec.dst_port = obj.port
        self.__phrs_svc_common_icmp(req_spec.icmp_msg, obj.icmp_msg)
        return

    def __phrs_dir_common(self, req_spec, obj):
        self.__phrs_svc_common(req_spec.svc, obj.service)
        self.__phrs_flood_limits_common(req_spec.tcp_syn_flood_limits,
                                        obj.tcp_syn_flood_limits)
        self.__phrs_flood_limits_common(req_spec.udp_flood_limits,
                                        obj.udp_flood_limits)
        self.__phrs_flood_limits_common(req_spec.icmp_flood_limits,
                                        obj.icmp_flood_limits)
        self.__phrs_flood_limits_common(req_spec.other_flood_limits,
                                        obj.other_flood_limits)
        if obj.peersg is not None:
            #sgs = self.tenant.GetRemoteSecurityGroups()
            sgs = self.tenant.GetSecurityGroups()
            if sgs:
                req_spec.peer_sg_handle = sgs[0].hal_handle
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.vrf_id = self.tenant.id
        self.__phrs_dir_common(req_spec.ingress_policy.dos_protection,
                               self.ingress)
        self.__phrs_dir_common(req_spec.egress_policy.dos_protection,
                               self.egress)
        for s in self.sgs:
            req_spec.sg_handle.append(s.hal_handle)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("- DosPolicy %s = %s" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        self.hal_handle = resp_spec.status.dos_handle
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        return        

    def ProcessHALGetResponse(self, get_req_spec, get_resp_spec):
        return

    def Get(self):
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Network Objects.
class DosPolicyObjectHelper:
    def __init__(self):
        self.dps = []
        return

    def Configure(self):
        if len(self.dps) == 0: return
        cfglogger.info("Configuring %d Security Groups" % len(self.dps))
        halapi.ConfigureDosPolicies(self.dps)
        return

    def Generate(self, tenant):
        dpspec = getattr(tenant.spec, 'dos', None)
        if dpspec is None:
            return

        cfglogger.info("Creating Dos Policies for Tenant = %s" %\
                       (tenant.GID()))
        dpspec = dpspec.Get(Store)
        for espec in dpspec.entries:
            dspec = espec.entry
            dp = DosPolicyObject()
            dp.Init(tenant, dspec)
            Store.objects.Set(dp.GID(), dp)
            self.dps.append(dp)
        return

    def main(self, tenant):
        self.Generate(tenant)
        self.Configure()
        return

def GetMatchingObjects(selectors):
    dps =  Store.objects.GetAllByClass(DosPolicyObject)
    return [dp for dp in dps if dp.IsFilterMatch(selectors.security_group)]
