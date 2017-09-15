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
        if reqspec.__class__.__name__ == 'CryptoKeyCreateRequest':
            cfglogger.info("CryptoKeyCreateRequest: do nothing")
        if reqspec.__class__.__name__ == 'CryptoKeyUpdateRequest':
            reqspec.key.keyindex = self.keyindex
            reqspec.key.key_type = self.key_type
            reqspec.key.key_size = self.key_size
            reqspec.key.key = self.key
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        if req_spec.__class__.__name__ == 'CryptoKeyCreateResponse':
            self.keyindex = resp_spec.keyindex
            cfglogger.info("CRYPTO_KEY Get %s = %s" %\
                       (self.keyindex, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        if req_spec.__class__.__name__ == 'CryptoKeyUpdateResponse':
            assert(req_spec.key.keyindex == resp_spec.keyindex)
            cfglogger.info("CRYPTO_KEY Update %s = %s" %\
                       (self.keyindex, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

    def Update(self, key_type, key_size, key):
        self.key_type = key_type
        self.key_size = key_size
        self.key = key
        lst = []
        lst.append(self)
        halapi.UpdateCryptoKeys(lst) 
        return 

# Helper Class to Generate/Configure/Manage CryptoKeyObject Objects
class CryptoKeyObjectHelper:
    def __init__(self):
        return

    def Configure(self, obj):
        lst = []
        lst.append(obj)
        cfglogger.info("Creating CryptoKey")
        halapi.GetCryptoKey(lst)
        return
        
    def __gen_one(self):
        cfglogger.info("Creating Cryptokey")
        obj = CryptoKeyObject()
        obj.Init()
        Store.objects.Add(obj)
        return obj

    def Generate(self):
        return self.__gen_one()

    def main(self):
        obj = self.Generate()
        self.Configure(obj)
        return obj

CryptoKeyHelper = CryptoKeyObjectHelper() 
