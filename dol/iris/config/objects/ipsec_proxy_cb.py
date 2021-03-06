# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import iris.config.resmgr            as resmgr

from iris.config.store               import Store
from infra.common.logging       import logger
from iris.config.objects.swdr        import SwDscrRingHelper
from iris.config.objects.crypto_keys        import CryptoKeyHelper
import iris.config.objects.endpoint  as endpoint
import iris.config.objects.segment   as segment 
import types_pb2                as types_pb2

import iris.config.hal.defs          as haldefs
import iris.config.hal.api           as halapi

class IpsecCbObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('IPSECCB'))
        return
        
    def Init(self, qid):
        #self.id = resmgr.IpsecCbIdAllocator.get()
        self.id = qid
        gid = "IPSECCB%04d" % self.id
        self.GID(gid)
        self.ipseccbq_base = SwDscrRingHelper.main("IPSECCBQ", gid, self.id)
        logger.info("  - %s" % self)
        self.crypto_key = CryptoKeyHelper.main() 
        self.new_crypto_key = CryptoKeyHelper.main() 
        self.sip6 = resmgr.TepIpv6SubnetAllocator.get()
        self.dip6 = resmgr.TepIpv6SubnetAllocator.get()
        self.sip6.v6_addr = self.sip6.getnum().to_bytes(16, 'big')
        self.dip6.v6_addr = self.dip6.getnum().to_bytes(16, 'big')

        key_type = types_pb2.CRYPTO_KEY_TYPE_AES128
        key_size = 16
        key = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        sip6 = b'\x20\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xaa\xaa'
        dip6 = b'\x20\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xbb\xbb'
        self.crypto_key.Update(key_type, key_size, key)

        self.tunnel_sip4               = 0x0A010001
        self.tunnel_dip4               = 0x0A010002
        self.iv_size                   = 8
        self.icv_size                  = 16
        self.block_size                = 16
        self.key_index                 = 0
        self.barco_enc_cmd             = 0x30000000
        self.iv                        = 0xaaaaaaaaaaaaaaaa
        self.iv_salt                   = 0xbbbbbbbb
        self.esn_hi                    = 0
        self.esn_lo                    = 0
        self.spi                       = 0
        self.key_index                 = self.crypto_key.keyindex
        self.sip6.ip_af                = 2
        self.sip6.v6_addr              = sip6 
        self.dip6.ip_af                = 2
        self.dip6.v6_addr              = dip6 

        return


    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.ipseccb_id    = self.id
        if req_spec.__class__.__name__ != 'IpsecCbGetRequest':
            req_spec.tunnel_sip4               = self.tunnel_sip4
            req_spec.tunnel_dip4               = self.tunnel_dip4
            req_spec.iv_size                   = self.iv_size
            req_spec.icv_size                  = self.icv_size
            req_spec.block_size                = self.block_size
            req_spec.barco_enc_cmd             = self.barco_enc_cmd
            req_spec.iv                        = self.iv
            req_spec.iv_salt                   = self.iv_salt
            req_spec.esn_hi                    = self.esn_hi
            req_spec.esn_lo                    = self.esn_lo
            req_spec.spi                       = self.spi
            req_spec.new_spi                   = self.new_spi
            req_spec.key_index                 = self.key_index
            req_spec.new_key_index             = self.new_key_index
            req_spec.expected_seq_no           = self.expected_seq_no
            req_spec.seq_no_bmp                = self.seq_no_bmp
            req_spec.sip6.ip_af                = haldefs.common.IP_AF_INET6
            req_spec.sip6.v6_addr              = self.sip6.v6_addr
            req_spec.dip6.ip_af                = haldefs.common.IP_AF_INET6
            req_spec.dip6.v6_addr              = self.dip6.v6_addr
            req_spec.is_v6                     = self.is_v6
            req_spec.is_nat_t                  = self.is_nat_t
            req_spec.is_random                 = self.is_random
            req_spec.extra_pad                 = self.extra_pad
            req_spec.vrf_vlan                  = self.vrf_vlan
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("  - IPSECCB %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        if resp_spec.__class__.__name__ == 'IpsecCbGetResponse':
            self.tunnel_sip4       = resp_spec.spec.tunnel_sip4
            self.tunnel_dip4       = resp_spec.spec.tunnel_dip4
            self.iv_size           = resp_spec.spec.iv_size
            self.icv_size          = resp_spec.spec.icv_size 
            self.block_size        = resp_spec.spec.block_size 
            self.barco_enc_cmd     = resp_spec.spec.barco_enc_cmd 
            self.iv                = resp_spec.spec.iv 
            self.iv_salt           = resp_spec.spec.iv_salt 
            self.esn_hi            = resp_spec.spec.esn_hi 
            self.esn_lo            = resp_spec.spec.esn_lo 
            self.spi               = resp_spec.spec.spi 
            self.new_spi           = resp_spec.spec.new_spi 
            self.pi                = resp_spec.spec.pi
            self.ci                = resp_spec.spec.ci
            self.expected_seq_no   = resp_spec.spec.expected_seq_no
            self.seq_no_bmp        = resp_spec.spec.seq_no_bmp
            self.is_v6             = resp_spec.spec.is_v6
            self.is_nat_t          = resp_spec.spec.is_nat_t
            self.is_random         = resp_spec.spec.is_random
            self.vrf_vlan          = resp_spec.spec.vrf_vlan
            self.rx_pkts           = resp_spec.spec.rx_pkts
            self.rx_bytes          = resp_spec.spec.rx_bytes
            self.rx_drops          = resp_spec.spec.rx_drops
            self.tx_pkts           = resp_spec.spec.tx_pkts
            self.tx_bytes          = resp_spec.spec.tx_bytes
            self.tx_drops          = resp_spec.spec.tx_drops
        return

    def GetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.GetIpsecCbs(lst)
        return

    def SetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.UpdateIpsecCbs(lst)
        return


# Helper Class to Generate/Configure/Manage IpsecCb Objects.
class IpsecCbObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(IpsecCbObject)
        logger.info("Configuring %d IpsecCbs." % len(objlist)) 
        halapi.ConfigureIpsecCbs(objlist)
        return
        
    def __gen_one(self, qid):
        logger.info("Creating IpsecCb for id: %d" % qid)
        ipseccb_obj = IpsecCbObject()
        ipseccb_obj.Init(qid)
        Store.objects.Add(ipseccb_obj)
        return ipseccb_obj

    def Generate(self, qid):
        self.objlist.append(self.__gen_one(qid))
        return self.objlist 

    def main(self, qid):
        gid = "IPSECCB%04d" % qid
        if Store.objects.IsKeyIn(gid):
            return Store.objects.Get(gid)
        objlist = self.Generate(qid)
        self.Configure(self.objlist)
        return objlist

IpsecCbHelper = IpsecCbObjectHelper()
