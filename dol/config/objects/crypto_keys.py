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

class CryptoKeyObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('CRYPTO_KEY'))
        return
        
    def Init(self):
        self.id = resmgr.CryptoKeyIdAllocator.get()
        gid = "CRYPTOKEY%04d" % self.id
        self.GID(gid)
        cfglogger.info("  - %s" % self)
        return

    def PrepareHALRequestSpec(self, reqspec):
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        if req_spec.__class__.__name__ == 'CryptoKeyCreateResponse':
            self.idx = resp_spec.keyindex

        cfglogger.info("  - CRYPTO_KEY %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

    def CryptoKeyUpdate(self, type):
        obj = CryptoKeyObject()
        obj.Clone(Store.templates.Get(type))
        self.type = obj.type
        self.keySize = obj.keySize
        self.key = obj.key
        lst = []
        lst.append(self)
        pdb.set_trace()
        halapi.UpdateCryptoKeys(lst) 

# Helper Class to Generate/Configure/Manage CryptoKeyObject Objects
class CryptoKeyObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(CryptoKeyObject)
        cfglogger.info("Configuring %d CryptoKeys" % len(objlist)) 
        halapi.ConfigureCryptoKeys(objlist)
        return
        
    def __gen_one(self):
        cfglogger.info("Creating Cryptokey")
        obj = CryptoKeyObject()
        obj.Init()
        Store.objects.Add(obj)
        return obj

    def Generate(self):
        self.objlist.append(self.__gen_one())
        return self.objlist

    def main(self):
        objlist = self.Generate()
        self.Configure(self.objlist)
        return objlist

CryptoKeyHelper = CryptoKeyObjectHelper() 
