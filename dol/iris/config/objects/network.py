#! /usr/bin/python3
import pdb
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs

from infra.common.glopts        import GlobalOptions
from infra.common.logging       import logger
from iris.config.store               import Store

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
        self.ip_prefix = None

        self.Show()
        return

    def IsIpv4(self):
        return self.af == 'IPV4'
    def IsIpv6(self):
        return self.af == 'IPV6'

    def AddSecurityGroup(self, sg):
        self.security_groups.append(sg)
        logger.info("- Adding SecurityGroup:%s to Network:%s" %\
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
        nw.ip_prefix = self.ip_prefix
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
        logger.info("- Network = %s(%d)" % (self.GID(), self.id))
        logger.info("  - RMAC       = %s" % self.rmac.get())
        logger.info("  - Prefix     = %s/%d" %\
                       (self.prefix.get(), self.prefix_len))
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.rmac = self.rmac.getnum()
        req_spec.key_or_handle.nw_key.vrf_key_handle.vrf_id = self.segment.tenant.id
        if self.IsIpv4():
            req_spec.key_or_handle.nw_key.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET
            req_spec.key_or_handle.nw_key.ip_prefix.address.v4_addr = self.prefix.getnum()
            req_spec.key_or_handle.nw_key.ip_prefix.prefix_len = self.prefix_len
        else:
            req_spec.key_or_handle.nw_key.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET6
            req_spec.key_or_handle.nw_key.ip_prefix.address.v6_addr = self.prefix.getnum().to_bytes(16, 'big')
            req_spec.key_or_handle.nw_key.ip_prefix.prefix_len = self.prefix_len
        # Store the key generated.
        self.ip_prefix = req_spec.key_or_handle.nw_key.ip_prefix
        for sg in self.security_groups:
            key_handle = req_spec.sg_key_handle.add()
            key_handle.security_group_id = sg.id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("- Network %s = %s" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        self.hal_handle = resp_spec.status.key_or_handle.nw_handle
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        req_spec.key_or_handle.nw_key.vrf_key_handle.vrf_id = self.tenant.id
        if self.IsIpv4():
            get_req_spec.key_or_handle.nw_key.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET
            get_req_spec.key_or_handle.nw_key.ip_prefix.address.v4_addr = self.prefix.getnum()
            get_req_spec.key_or_handle.nw_key.ip_prefix.prefix_len = self.prefix_len
        else:
            get_req_spec.key_or_handle.nw_key.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET6
            get_req_spec.key_or_handle.nw_key.ip_prefix.address.v6_addr = self.prefix.getnum().to_bytes(16, 'big')
            get_req_spec.key_or_handle.nw_key.ip_prefix.prefix_len = self.prefix_len
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp_spec):
        if get_resp_spec.api_status == haldefs.common.ApiStatus.Value('API_STATUS_OK'):
            self.rmac = objects.MacAddressBase(integer=get_resp_spec.spec.rmac)
        else:
            logger.error("- Network  %s = %s is missing." %\
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
        logger.info("Configuring %d Networks" % len(self.nws))
        if not GlobalOptions.agent:
            halapi.ConfigureNetworks(self.nws)
        else:
            logger.info(" - Skipping in agent mode.")
        return

    def Generate(self, segment):
        logger.info("Creating Network Objects for Segment = %s" %\
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
