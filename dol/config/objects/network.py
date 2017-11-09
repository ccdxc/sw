#! /usr/bin/python3
import pdb
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.hal.api           as halapi
import config.hal.defs          as haldefs

from infra.common.logging       import cfglogger
from config.store               import Store

class NetworkObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('NETWORK'))
        return

    def Init(self, segment, prefix, af):
        self.segment = segment
        self.id = self.segment.id
        self.tenant = segment.tenant
        self.GID("Nw%04d" % self.segment.id)

        self.af = af
        self.prefix = prefix
        if self.IsIpv4():
            self.prefix_len = 24
        else:
            self.prefix_len = 96
        self.rmac = segment.macaddr
        self.hal_handle = None
        self.security_groups = []

        self.Show()
        return

    def IsIpv4(self):
        return self.af == 'IPV4'
    def IsIpv6(self):
        return self.af == 'IPV6'

    def AddSecurityGroup(self, sg):
        self.security_groups.append(sg)
        cfglogger.info("- Adding SecurityGroup:%s to Network:%s" %\
                       (sg.GID(), self.GID()))
        return

    def __copy__(self):
        nw = NetworkObject()
        nw.tenant = self.tenant
        nw.hal_handle = self.hal_handle
        nw.rmac = self.rmac
        nw.prefix = self.prefix
        nw.prefix_len = self.prefix_len
        nw.af = self.af
        nw.id = self.id
        nw.security_groups = self.security_groups[:]
        return nw


    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False
        fields = ["id", "rmac", "prefix", "prefix_len", "af", "hal_handle",
                  "security_groups"]

        if not self.CompareObjectFields(other, fields, lgh):
            return False

        return True


    def Show(self, detail = False):
        cfglogger.info("- Network = %s(%d)" % (self.GID(), self.id))
        cfglogger.info("  - RMAC       = %s" % self.rmac.get())
        cfglogger.info("  - Prefix     = %s/%d" %\
                       (self.prefix.get(), self.prefix_len))
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.vrf_id = self.segment.tenant.id
        req_spec.vrf_key_handle.vrf_id = self.segment.tenant.id
        req_spec.rmac = self.rmac.getnum()
        if self.IsIpv4():
            req_spec.key_or_handle.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET
            req_spec.key_or_handle.ip_prefix.address.v4_addr = self.prefix.getnum()
            req_spec.key_or_handle.ip_prefix.prefix_len = self.prefix_len
        else:
            req_spec.key_or_handle.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET6
            req_spec.key_or_handle.ip_prefix.address.v6_addr = self.prefix.getnum().to_bytes(16, 'big')
            req_spec.key_or_handle.ip_prefix.prefix_len = self.prefix_len
        for sg in self.security_groups:
            req_spec.sg_handle.append(sg.hal_handle)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("- Network %s = %s" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        self.hal_handle = resp_spec.status.nw_handle
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        get_req_spec.meta.vrf_id = self.tenant.id
        if self.IsIpv4():
            get_req_spec.key_or_handle.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET
            get_req_spec.key_or_handle.ip_prefix.address.v4_addr = self.prefix.getnum()
            get_req_spec.key_or_handle.ip_prefix.prefix_len = self.prefix_len
        else:
            get_req_spec.key_or_handle.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET6
            get_req_spec.key_or_handle.ip_prefix.address.v6_addr = self.prefix.getnum().to_bytes(16, 'big')
            get_req_spec.key_or_handle.ip_prefix.prefix_len = self.prefix_len
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp_spec):
        if get_resp_spec.api_status == haldefs.common.ApiStatus.Value('API_STATUS_OK'):
            self.rmac = objects.MacAddressBase(integer=get_resp_spec.spec.rmac)
        else:
            cfglogger.error("- Network  %s = %s is missing." %\
                       (self.GID(), haldefs.common.ApiStatus.Name(get_resp_spec.api_status)))
            self.hal_handle = None

    def Get(self):
        halapi.GetNetworks([self])

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Network Objects.
class NetworkObjectHelper:
    def __init__(self):
        self.nws = []
        return

    def AddSecurityGroup(self, sg):
        for nw in self.nws:
            nw.AddSecurityGroup(sg)
        return

    def Configure(self):
        cfglogger.info("Configuring %d Networks" % len(self.nws))
        halapi.ConfigureNetworks(self.nws)
        return

    def Generate(self, segment):
        cfglogger.info("Creating Network Objects for Segment = %s" %\
                       (segment.GID()))
        nw = NetworkObject()
        nw.Init(segment, segment.subnet, 'IPV4')
        self.nws.append(nw)

        nw = NetworkObject()
        nw.Init(segment, segment.subnet6, 'IPV6')
        self.nws.append(nw)
        Store.objects.Set(nw.GID(), nw)
        return

def GetMatchingObjects(selectors):
    networks =  Store.objects.GetAllByClass(NetworkObject)
    return [nw for nw in networks if nw.IsFilterMatch(selectors.network)]
