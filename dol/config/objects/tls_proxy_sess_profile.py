# /usr/bin/python3
import pdb
import os
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import config.resmgr            as resmgr

from config.store               import Store
from infra.common.logging       import logger

import config.hal.defs          as haldefs
import config.hal.api           as halapi
import crypto_apis_pb2          as crypto_apis_pb2

class CryptoCertObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('CRYPTO_CERT'))
        return
        
    def Init(self, certfile):
        self.id = resmgr.CryptoCertIdAllocator.get()
        gid = "CRYPTOCERT%04d" % self.id
        self.GID(gid)
        logger.info("  - %s" % self)
        # read cert from the file
        self.path = certfile.Get()
        logger.info("  - file %s" % self.path)
        f = open(self.path, "r")
        assert(f != None)
        self.cert_body = f.read()
        f.close()
        return

    def PrepareHALRequestSpec(self, reqspec):
        if reqspec.__class__.__name__ == 'CryptoApiRequest':
            reqspec.api_type = crypto_apis_pb2.ASYMAPI_SETUP_CERT
            reqspec.setup_cert.update_type = crypto_apis_pb2.ADD_UPDATE 
            reqspec.setup_cert.cert_id = self.id
            reqspec.setup_cert.body = self.cert_body
            #reqspec.setup_cert.next_cert_id = None
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        if resp_spec.__class__.__name__ == 'CryptoApiResponse':
            logger.info("CRYPTO_CERT Get %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

# Helper Class to Generate/Configure/Manage CryptoCertObject Objects
class CryptoCertObjectHelper:
    def __init__(self):
        return

    def Configure(self, obj):
        lst = []
        lst.append(obj)
        logger.info("Configuring CryptoCert")
        halapi.GetCryptoCert(lst)
        return
        
    def __gen_one(self, certfile):
        logger.info("Creating CryptoCert")
        obj = CryptoCertObject()
        obj.Init(certfile)
        Store.objects.Add(obj)
        return obj

    def Generate(self, certfile):
        return self.__gen_one(certfile)

    def main(self, certfile):
        obj = self.Generate(certfile)
        self.Configure(obj)
        return obj

CryptoCertHelper = CryptoCertObjectHelper()

class CryptoAsymKeyObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('CRYPTO_ASYM_KEY'))
        return
        
    def Init(self, keyfile):
        self.id = resmgr.CryptoAsymKeyIdAllocator.get()
        gid = "CRYPTOASYMKEY%04d" % self.id
        self.GID(gid)
        logger.info("  - %s" % self)
        # read key from the file
        self.path = keyfile.Get()
        logger.info("  - file %s" % self.path)
        f = open(self.path, "r")
        assert(f != None)
        self.key = f.read()
        f.close()
        return

    def PrepareHALRequestSpec(self, reqspec):
        if reqspec.__class__.__name__ == 'CryptoApiRequest':
            reqspec.api_type = crypto_apis_pb2.ASYMAPI_SETUP_PRIV_KEY
            reqspec.setup_priv_key.key = self.key
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        if resp_spec.__class__.__name__ == 'CryptoApiResponse':
            logger.info("CRYPTO_KEY Get key type %s = %s" %\
                       (resp_spec.setup_priv_key.key_type, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
            self.key_type = resp_spec.setup_priv_key.key_type
            if self.key_type == 0:
                # ECDSA key
                self.ecdsa_sign_key_idx = resp_spec.setup_priv_key.ecdsa_key_info.sign_key_idx
                logger.info("CRYPTO_KEY sign_key_idx: %s" % (self.ecdsa_sign_key_idx))
            elif self.key_type == 1:
                self.rsa_sign_key_idx = resp_spec.setup_priv_key.rsa_key_info.sign_key_idx
                self.rsa_decrypt_key_idx = resp_spec.setup_priv_key.rsa_key_info.decrypt_key_idx
                logger.info("CRYPTO_KEY sign_key_idx: %s, decrypt_key_idx %s" % \
                            (self.rsa_sign_key_idx, self.rsa_decrypt_key_idx))
        return

# Helper Class to Generate/Configure/Manage CryptoCertObject Objects
class CryptoAsymKeyObjectHelper:
    def __init__(self):
        return

    def Configure(self, obj):
        lst = []
        lst.append(obj)
        logger.info("Configuring CryptoAsymKey")
        halapi.GetCryptoAsymKey(lst)
        return
        
    def __gen_one(self, keyfile):
        logger.info("Creating CryptoAsymKey")
        obj = CryptoAsymKeyObject()
        obj.Init(keyfile)
        Store.objects.Add(obj)
        return obj

    def Generate(self, keyfile):
        return self.__gen_one(keyfile)

    def main(self, keyfile):
        obj = self.Generate(keyfile)
        self.Configure(obj)
        return obj

CryptoAsymKeyHelper = CryptoAsymKeyObjectHelper()

# Helper Class to Generate/Configure/Manage Tls Proxy Session Objects
class TlsProxySessProfileHelper:
    def __init__(self):
        return

    def main(self, tls_sess_profile):
        self.tls_sess_profile = tls_sess_profile
        self.tls_sess_profile.cert = \
                    CryptoCertHelper.main(tls_sess_profile.cert_file)
        self.tls_sess_profile.key = \
                    CryptoAsymKeyHelper.main(tls_sess_profile.key_file)
        return

TlsProxySessProfileHelper = TlsProxySessProfileHelper()

