# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import config.resmgr            as resmgr

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.defs          as haldefs
import config.hal.api           as halapi

class TlsCbObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('TLS_PROXY_CB'))
        return
        
    # def Init(self, spec_obj):
    def Init(self):
        self.id = resmgr.TlsCbIdAllocator.get()
        gid = "TlsCb%04d" % self.id
        self.GID(gid)
"""
TODO:
        self.spec = spec_obj
        cfglogger.info("  - %s" % self)

        self.uplinks = objects.ObjectDatabase(cfglogger)
        for uplink_spec in self.spec.uplinks:
            uplink_obj = uplink_spec.Get(Store)
            self.uplinks.Set(uplink_obj.GID(), uplink_obj)

        assert(len(self.uplinks) > 0) """
        return

    def PrepareHALRequestSpec(self, reqspec):
        reqspec.meta.id             = self.id
        reqspec.key_or_handle.id    = self.id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - TlsCb %s = %s" %\
                       (self.name, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def Read(self):
        return

    def Write(self):
        return



# Helper Class to Generate/Configure/Manage TlsCb Objects.
class TlsCbObjectHelper:
    def __init__(self):
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(TlsCbObject)
        cfglogger.info("Configuring %d TlsCbs." % len(objlist)) 
        halapi.ConfigureTlsCbs(objlist)
        return
        
    def __gen_one(self, topospec, tcp_tlsspec):
        """
        TODO : figure out how to use spec object
        spec_obj = tenant_spec.spec.Get(Store)
        cfglogger.info("Creating %d TcpCbs" % tenant_spec.count)
        objlist = []
        for c in range(tenant_spec.count):
            tenant_obj = TenantObject()
            tenant_obj.Init(spec_obj)
        objlist.append(tenant_obj)
        """
        cfglogger.info("Creating %d TlsCbs" % tcp_tlsspec.count)
        objlist = []
        for c in range(1..100):
            tlsobj = TlsCbObject()
            tlsobj.Init()
        objlist.append(tlsobj)
        return objlist

    def Generate(self, topospec):
        objlist = []
        for tcp_tlsspec in topospec.tcp_tls_proxys:
            objlist += self.__gen_one(topospec, tcp_tlsspec)
        return objlist

    def main(self, topospec):
        objlist = self.Generate(topospec)
        self.Configure(objlist)
        Store.objects.SetAll(objlist)
        return objlist
