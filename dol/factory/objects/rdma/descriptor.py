#! /usr/bin/python3

from scapy.all import *

import config.resmgr as resmgr

import infra.factory.base as base
from infra.common.logging   import logger

import model_sim.src.model_wrap as model_wrap

from factory.objects.rdma import buffer as rdmabuffer

from infra.factory.store    import FactoryStore

class RdmaSqDescriptorBase(Packet):
    fields_desc = [
        LongField("wrid", 0),
        BitField("op_type", 0, 4),
        BitField("complete_notify", 0, 1),
        BitField("fence", 0, 1),
        BitField("solicited_event", 0, 1),
        BitField("inline_data_vld", 0, 1),
        ByteField("num_sges", 0),
        BitField("color", 0, 1),
        BitField("rsvd1", 0, 15),
    ]

class RdmaSqDescriptorSend(Packet):
    fields_desc = [
        IntField("imm_data", 0),
        IntField("inv_key", 0),
        IntField("rsvd1", 0),      # rsvd1 is to ensure len field in Sq Send and Write defined at same offset
        IntField("len", 0),
        IntField("rsvd2", 0),
    ]

class RdmaSqDescriptorUDSend(Packet):
    fields_desc = [
        IntField("imm_data", 0),
        IntField("q_key", 0),
        IntField("length", 0),
        X3BytesField("dst_qp", 0),
        ByteField("ah_size", 0),
        IntField("ah_handle", 0),
    ]

class RdmaSqDescriptorWrite(Packet):
    fields_desc = [
        IntField("imm_data", 0),
        LongField("va", 0),
        IntField("len", 0),    # ensure that len is defined at same offset b/w Sq Send and Write descriptors
        IntField("r_key", 0),
    ]

class RdmaSqDescriptorRead(Packet):
    fields_desc = [
        IntField("rsvd", 0),
        LongField("va", 0),
        IntField("len", 0),
        IntField("r_key", 0),
    ]

class RdmaSqDescriptorAtomic(Packet):
    fields_desc = [
        IntField("r_key", 0),
        LongField("va", 0),
        LongField("swapdt", 0),
        LongField("cmpdt", 0),
        LongField("pad", 0),
    ]

class RdmaSqDescriptorLocalInv(Packet):
    fields_desc = [
        IntField("l_key", 0),
        BitField("pad", 0, 128),
    ]

class RdmaSqDescriptorBindMw(Packet):
     fields_desc = [
         LongField("va", 0),
         IntField("len", 0),
         IntField("l_key", 0),
         IntField("r_key", 0),
         ByteField("new_r_key_key", 0),
         ByteField("access_ctrl", 0),
         BitField("mw_type", 0, 2),
         BitField("zbva", 0, 1),
         BitField("pad", 0, 237),
     ]

class RdmaRrqDescriptorBase(Packet):
    fields_desc = [
        BitField("read_resp_or_atomic", 0, 1),
        BitField("num_sges", 0, 7),
        X3BytesField("psn", 0),
        X3BytesField("msn", 0),
        ByteField("rsvd", 0),
    ]

class RdmaRrqDescriptorRead(Packet):
    fields_desc = [
        IntField("len", 0),
        LongField("wqe_sge_list_ptr", 0),
        IntField("rsvd0", 0),
        IntField("rsvd1", 0),
        IntField("rsvd2", 0),
        IntField("rsvd3", 0),
        IntField("rsvd4", 0),
        IntField("rsvd5", 0),
        IntField("rsvd6", 0),
        IntField("rsvd7", 0),
        IntField("rsvd8", 0),
        IntField("rsvd9", 0),
        IntField("rsvd10", 0),
    ]

class RdmaRrqDescriptorAtomic(Packet):
    fields_desc = [
        LongField("va", 0),
        IntField("len", 0),
        IntField("l_key", 0),
        ByteField("op_type", 0),
        ByteField("rsvd0", 0),
        ByteField("rsvd1", 0),
        ByteField("rsvd2", 0),
        IntField("rsvd3", 0),
        IntField("rsvd4", 0),
        IntField("rsvd5", 0),
        IntField("rsvd6", 0),
        IntField("rsvd7", 0),
        IntField("rsvd8", 0),
        IntField("rsvd9", 0),
        IntField("rsvd10", 0),
        IntField("rsvd11", 0),
        IntField("rsvd12", 0),
    ]

class RdmaRqDescriptorBase(Packet):
    fields_desc = [
        LongField("wrid", 0),
        ByteField("num_sges", 0),
        ByteField("rsvd0", 0),
        ShortField("rsvd1", 0),
        IntField("rsvd2", 0),
        LongField("rsvd3", 0),
        LongField("rsvd4", 0),
    ]

class RdmaRsqDescriptorBase(Packet):
    fields_desc = [
        BitField("read_resp_or_atomic", 0, 1),
        BitField("rsvd0", 0, 7),
        X3BytesField("psn", 0),
        LongField("rsvd1", 0),
    ]

class RdmaRsqDescriptorRead(Packet):
    fields_desc = [
        IntField("r_key", 0),
        LongField("va", 0),
        IntField("len", 0),
        IntField("offset", 0),
    ]

class RdmaRsqDescriptorAtomic(Packet):
    fields_desc = [
        IntField("r_key", 0),
        LongField("va", 0),
        LongField("orig_data", 0),
    ]

class RdmaSge(Packet):
    fields_desc = [
        LongField("va", 0),
        IntField("len", 0),
        IntField("l_key", 0),
    ]

class RdmaCqDescriptorRecv(Packet):
    fields_desc = [
        LongField("wrid", 0),
        ByteField("op_type", 0),
        X3BytesField("src_qp", 0),
        MACField("smac", ETHER_ANY),
        ShortField("pkey_index", 0),
        IntField("imm_data_or_r_key", 0),
        IntField("status_or_length", 0),
        X3BytesField("qid", 0),
        BitField("type", 0, 3),
        BitField("imm_data_vld", 0, 1),
        BitField("rkey_inv_vld", 0, 1),
        BitField("ipv4", 0, 1),
        BitField("error", 0, 1),
        BitField("color", 0, 1),
    ]

class RdmaCqDescriptorSend(Packet):
    fields_desc = [
        BitField("rsvd1", 0, 32),
        IntField("msn", 0),
        BitField("rsvd2", 0, 64),
        LongField("wrid", 0),
        IntField("status", 0),
        X3BytesField("qid", 0),
        BitField("type", 0, 3),
        BitField("flags", 0, 3),
        BitField("error", 0, 1),
        BitField("color", 0, 1),
    ]

class RdmaCqDescriptorAdmin(Packet):
    fields_desc = [
        BitField("rsvd", 0, 160),
        ShortField("old_sq_cindex", 0),
        ShortField("old_rq_cq_cindex", 0),
        IntField("status", 0),
        X3BytesField("qid", 0),
        BitField("type", 0, 3),
        BitField("flags", 0, 3),
        BitField("error", 0, 1),
        BitField("color", 0, 1),
    ]

class RdmaEqDescriptor(Packet):
    fields_desc = [
        X3BytesField("qid", 0),
        BitField("type", 0, 3),
        BitField("code", 0, 4),
        BitField("color", 0, 1),
    ]

class RdmaSqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.sges = []

    def Init(self, spec):
        super().Init(spec)
        if hasattr(self.spec.fields, 'wrid'):
            self.wrid = self.spec.fields.wrid

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        inline_data_vld = self.spec.fields.inline_data_vld if hasattr(self.spec.fields, 'inline_data_vld') else 0
        num_sges = self.spec.fields.num_sges if hasattr(self.spec.fields, 'num_sges') else 0
        color = self.spec.fields.color if hasattr(self.spec.fields, 'color') else 0
        fence = self.spec.fields.fence if hasattr(self.spec.fields, 'fence') else 0
        logger.info("Writing SQ Descriptor @0x%x = op_type: %d wrid: 0x%x inline_data_vld: %d num_sges: %d color: %d fence: %d" % 
                       (self.address, self.spec.fields.op_type, self.wrid, inline_data_vld, num_sges, color, fence))
        desc = RdmaSqDescriptorBase(op_type=self.spec.fields.op_type, wrid=self.wrid,
                                    inline_data_vld = inline_data_vld, num_sges=num_sges, color=color, fence=fence)
        inline_data_len = 0
        inline_data = None

        # Make sure Inline data is specificied only for Send and Write, assert in other operations
        if hasattr(self.spec.fields, 'send'):
           logger.info("Reading Send")
           imm_data = self.spec.fields.send.imm_data if hasattr(self.spec.fields.send, 'imm_data') else 0
           inv_key = self.spec.fields.send.inv_key if hasattr(self.spec.fields.send, 'inv_key') else 0
           data_len = self.spec.fields.send.len if hasattr(self.spec.fields.send, 'len') else 0
           if inline_data_vld:
               inline_data_len = data_len
               # Create the Inline data of size provided
               inline_data = bytearray(inline_data_len)
               inline_data[0:inline_data_len] = self.spec.fields.send.inline_data[0:inline_data_len]
           send = RdmaSqDescriptorSend(imm_data=imm_data, inv_key=inv_key, len=data_len)
           desc = desc/send

        if hasattr(self.spec.fields, 'ud_send'):
           logger.info("Reading UD Send")
           dst_qp = self.spec.fields.ud_send.dst_qp if hasattr(self.spec.fields.ud_send, 'dst_qp') else 0
           q_key = self.spec.fields.ud_send.q_key if hasattr(self.spec.fields.ud_send, 'q_key') else 0
           ah_handle = self.spec.fields.ud_send.ah_handle if hasattr(self.spec.fields.ud_send, 'ah_handle') else 0
           #right shift ah_handle by 3 bits to keep it within 32 bits
           ah_handle >>= 3
           ah_size = self.spec.fields.ud_send.ah_size if hasattr(self.spec.fields.ud_send, 'ah_size') else 0
           imm_data = self.spec.fields.ud_send.imm_data if hasattr(self.spec.fields.ud_send, 'imm_data') else 0
           logger.info("UD Descriptor fields: dst_qp: %d q_key: 0x%x ah_handle: 0x%x ah_size: %d imm_data: 0x%x" % \
                       (dst_qp, q_key, ah_handle, ah_size, imm_data))
           send = RdmaSqDescriptorUDSend(imm_data=imm_data, q_key=q_key, dst_qp=dst_qp, ah_size=ah_size, ah_handle=ah_handle)
           desc = desc/send

        if hasattr(self.spec.fields, 'write'):
           logger.info("Reading Write")
           imm_data = self.spec.fields.write.imm_data if hasattr(self.spec.fields.write, 'imm_data') else 0
           va = self.spec.fields.write.va if hasattr(self.spec.fields.write, 'va') else 0
           dma_len = self.spec.fields.write.len if hasattr(self.spec.fields.write, 'len') else 0
           if inline_data_vld:
               inline_data_len = dma_len
               # Create the Inline data of size provided
               inline_data = bytearray(inline_data_len)
               inline_data[0:inline_data_len] = self.spec.fields.write.inline_data[0:inline_data_len]
           r_key = self.spec.fields.write.r_key if hasattr(self.spec.fields.write, 'r_key') else 0
           write = RdmaSqDescriptorWrite(imm_data=imm_data, va=va, len=dma_len, r_key=r_key)
           desc = desc/write

        if hasattr(self.spec.fields, 'read'):
           logger.info("Reading Read")
           assert(inline_data_vld == 0)
           rsvd = self.spec.fields.read.rsvd if hasattr(self.spec.fields.read, 'rsvd') else 0
           va = self.spec.fields.read.va if hasattr(self.spec.fields.read, 'va') else 0
           data_len = self.spec.fields.read.len if hasattr(self.spec.fields.read, 'len') else 0
           r_key = self.spec.fields.read.r_key if hasattr(self.spec.fields.read, 'r_key') else 0
           read = RdmaSqDescriptorRead(rsvd=rsvd, va=va, len=data_len, r_key=r_key)
           desc = desc/read

        if hasattr(self.spec.fields, 'atomic'):
           logger.info("Reading Atomic")
           assert(inline_data_vld == 0)
           r_key = self.spec.fields.atomic.r_key if hasattr(self.spec.fields.atomic, 'r_key') else 0
           va = self.spec.fields.atomic.va if hasattr(self.spec.fields.atomic, 'va') else 0
           cmpdt = self.spec.fields.atomic.cmpdt if hasattr(self.spec.fields.atomic, 'cmpdt') else 0
           swapdt = self.spec.fields.atomic.swapdt if hasattr(self.spec.fields.atomic, 'swapdt') else 0
           atomic = RdmaSqDescriptorAtomic(r_key=r_key, va=va, cmpdt=cmpdt, swapdt=swapdt)
           desc = desc/atomic

        if hasattr(self.spec.fields, 'local_inv'):
           logger.info("Reading Local Invalidate")
           assert(inline_data_vld == 0)
           l_key = self.spec.fields.local_inv.l_key if hasattr(self.spec.fields.local_inv, 'l_key') else 0
           local_inv = RdmaSqDescriptorLocalInv(l_key=l_key)
           desc = desc/local_inv

        if hasattr(self.spec.fields, 'bind_mw'):
           logger.info("Reading Bind MW")
           assert(inline_data_vld == 0)
           va = self.spec.fields.bind_mw.va if hasattr(self.spec.fields.bind_mw, 'va') else 0
           logger.info("va = 0x%x" % va)
           data_len = self.spec.fields.bind_mw.len if hasattr(self.spec.fields.bind_mw, 'len') else 0
           logger.info("len = 0x%x" % data_len)
           l_key = self.spec.fields.bind_mw.l_key if hasattr(self.spec.fields.bind_mw, 'l_key') else 0
           logger.info("l_key = 0x%x" % l_key)
           r_key = self.spec.fields.bind_mw.r_key if hasattr(self.spec.fields.bind_mw, 'r_key') else 0
           logger.info("r_key = 0x%x" % r_key)
           new_r_key_key = self.spec.fields.bind_mw.new_r_key_key if hasattr(self.spec.fields.bind_mw, 'new_r_key_key') else 0
           logger.info("new_r_key_key = 0x%x" % new_r_key_key)
           access_ctrl = self.spec.fields.bind_mw.access_ctrl if hasattr(self.spec.fields.bind_mw, 'access_ctrl') else 0
           logger.info("access_ctrl = 0x%x" % access_ctrl)
           mw_type = self.spec.fields.bind_mw.mw_type if hasattr(self.spec.fields.bind_mw, 'mw_type') else 0
           logger.info("mw_type = 0x%x" % mw_type)
           zbva = self.spec.fields.bind_mw.zbva if hasattr(self.spec.fields.bind_mw, 'zbva') else 0
           logger.info("zbva = 0x%x" % zbva)
            
           bind_mw = RdmaSqDescriptorBindMw(va=va, len=data_len, l_key=l_key, r_key=r_key, new_r_key_key=new_r_key_key, access_ctrl=access_ctrl, mw_type=mw_type, zbva=zbva)
           desc = desc/bind_mw

        if inline_data_vld:
           logger.info("Inline Data: %s " % bytes(inline_data[0:inline_data_len]))
           desc = desc/bytes(inline_data)
        elif (num_sges and (num_sges > 0)):
            for sge in self.spec.fields.sges:
                sge_entry = RdmaSge(va=sge.va, len=sge.len, l_key=sge.l_key)
                logger.info("Read Sge[] = va: 0x%x len: %d l_key: %d" % 
                               (sge.va, sge.len, sge.l_key))
                desc = desc/sge_entry
        
        logger.ShowScapyObject(desc)
        if self.mem_handle:
            resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        else:
            model_wrap.write_mem_pcie(self.address, bytes(desc), len(desc))

        self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        if self.mem_handle:
            self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
            mem_handle = resmgr.MemHandle(self.address, self.phy_address)
            desc = RdmaSqDescriptorBase(resmgr.HostMemoryAllocator.read(mem_handle, 32))
        else:
            hbm_addr = self.address
            desc = RdmaSqDescriptorBase(model_wrap.read_mem(hbm_addr, 32))

        logger.ShowScapyObject(desc)
        self.wrid = desc.wrid
        self.num_sges = desc.num_sges
        self.op_type = desc.op_type
        self.fence   = desc.fence
        logger.info("Read Desciptor @0x%x = wrid: 0x%x num_sges: %d op_type: %d fence: %d" % 
                       (self.address, self.wrid, self.num_sges, self.op_type, self.fence))
        self.sges = []
        if self.mem_handle:
            mem_handle.va += 32
            #for atomic descriptor, skip 16 bytes to access SGE
            if self.op_type in [6, 7]:
                mem_handle.va += 16
        else:
            hbm_addr += 32
            if self.op_type in [6, 7]:
                hbm_addr += 16

        for i in range(self.num_sges):
            
            if self.mem_handle:
                self.sges.append(RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16)))
            else:
                self.sges.append(RdmaSge(model_wrap.read_mem(hbm_addr, 16)))

            #sge = RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16))
            #self.sges.append(sge)
            #logger.info("Read Sge[%d] = va: 0x%x len: %d l_key: %d" % 
            #               (i, sge.va, sge.len, sge.l_key))
            if self.mem_handle:
                mem_handle.va += 16
            else:
                hbm_addr += 16

        for sge in self.sges:
            logger.info('%s' % type(sge))
            logger.info('0x%x' % sge.va)
        logger.info('id: %s' %id(self))


    def Show(self):
        desc = RdmaSqDescriptor(wrid=self.wrid, op_type=self.op_type, num_sges=self.num_sges)
        logger.ShowScapyObject(desc)
        #TODO: Check if we need to show SGEs

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on descriptor, ignoring for now..")
        return True

    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on descriptor")

        #if hasattr(self.spec.fields, 'buff'):
        if not hasattr(self, 'address'):
            logger.info("Reading from buff")
            return self.spec.fields.buff

        rdmabuff = rdmabuffer.RdmaBufferObject()
        logger.info("wrid: %d num_sges: %d len: %d" % (self.wrid, self.num_sges, len(self.sges)));
        total_data = bytearray()
        total_size = 0 
        for idx in range(self.num_sges):
            sge = self.sges[idx]
            logger.info("Reading sge content : 0x%x  len: %d" %(sge.va, sge.len))
            mem_handle = resmgr.MemHandle(sge.va,
                                          resmgr.HostMemoryAllocator.v2p(sge.va))
            sge_data = resmgr.HostMemoryAllocator.read(mem_handle, sge.len)
            logger.info("     sge data: %s" % bytes(sge_data))
            total_data.extend(sge_data)
            total_size += sge.len
        rdmabuff.data = bytes(total_data)
        rdmabuff.size = total_size
        logger.info("Total data: %s" % bytes(total_data))
        return rdmabuff

class RdmaRqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.sges = []

    def Init(self, spec):
        super().Init(spec)
        if hasattr(self.spec.fields, 'wrid'):
            self.wrid = self.spec.fields.wrid

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        if self.mem_handle:
            logger.info("Writing RQ Descriptor @(va:0x%x, pa:0x%x) = wrid: 0x%x num_sges: %d" % 
                       (self.mem_handle.va, self.mem_handle.pa, self.wrid, self.spec.fields.num_sges))
        else:
            logger.info("Writing RQ Descriptor @(address:0x%x) = wrid: 0x%x num_sges: %d" % 
                       (self.address, self.wrid, self.spec.fields.num_sges))

        desc = RdmaRqDescriptorBase(wrid=self.wrid,
                                    num_sges=self.spec.fields.num_sges)
        for sge in self.spec.fields.sges:
            logger.info("sge: va: 0x%x len: %d l_key: %d" %(sge.va, sge.len, sge.l_key))
            sge_entry = RdmaSge(va=sge.va, len=sge.len, l_key=sge.l_key)
            desc = desc/sge_entry
        
        logger.ShowScapyObject(desc)
        if self.mem_handle:
            resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        else:
            model_wrap.write_mem_pcie(self.address, bytes(desc), len(desc))

        self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """

        if self.mem_handle:
            self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
            mem_handle = resmgr.MemHandle(self.address, self.phy_address)
            desc = RdmaRqDescriptorBase(resmgr.HostMemoryAllocator.read(mem_handle, 32))
        else:
            hbm_addr = self.address
            desc = RdmaRqDescriptorBase(model_wrap.read_mem(hbm_addr, 32))

        logger.ShowScapyObject(desc)
        self.wrid = desc.wrid
        self.num_sges = desc.num_sges
        logger.info("Read Desciptor @0x%x = wrid: 0x%x num_sges: %d" % 
                       (self.address, self.wrid, self.num_sges))

        self.sges = []
        if self.mem_handle:
            mem_handle.va += 32
        else:
            hbm_addr += 32

        for i in range(self.num_sges):
            
            if self.mem_handle:
                self.sges.append(RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16)))
            else:
                self.sges.append(RdmaSge(model_wrap.read_mem(hbm_addr, 16)))

            if self.mem_handle:
                mem_handle.va += 16
            else:
                hbm_addr += 16

        for sge in self.sges:
            logger.info('%s' % type(sge))
            logger.info('va: 0x%x' % sge.va)
            logger.info('len: 0x%x' % sge.len)
            logger.info('lkey: 0x%x' % sge.l_key)
        logger.info('id: %s' %id(self))

    def Show(self):
        desc = RdmaRqDescriptor(addr=self.wrid, len=self.num_sges)
        logger.ShowScapyObject(desc)
        #TODO: Check if we need to show SGEs

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on descriptor, ignoring for now..")
        return True

    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on descriptor")

        #if hasattr(self.spec.fields, 'buff'):
        if not hasattr(self, 'address'):
            return self.spec.fields.buff

        rdmabuff = rdmabuffer.RdmaBufferObject()
        logger.info("wrid: %d num_sges: %d len: %d" % (self.wrid, self.num_sges, len(self.sges)));
        total_data = bytearray()
        total_size = 0 
        for idx in range(self.num_sges):
            sge = self.sges[idx]
            logger.info("Reading sge content : 0x%x  len: %d" %(sge.va, sge.len))
            mem_handle = resmgr.MemHandle(sge.va,
                                          resmgr.HostMemoryAllocator.v2p(sge.va))
            sge_data = resmgr.HostMemoryAllocator.read(mem_handle, sge.len)
            logger.info("     sge data: %s" % bytes(sge_data))
            total_data.extend(sge_data)
            total_size += sge.len
        rdmabuff.data = bytes(total_data)
        rdmabuff.size = total_size
        logger.info("Total data: %s" % bytes(total_data))
        return rdmabuff

class RdmaCqDescriptorRecvObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(FactoryStore.templates.Get('DESC_RDMA_CQ_RECV'))

    def Init(self, spec):
        super().Init(spec)
        self.wrid = self.spec.fields.wrid if hasattr(self.spec.fields, 'wrid') else 0
        self.op_type = self.spec.fields.op_type if hasattr(self.spec.fields, 'op_type') else 0
        self.rkey_inv_vld = self.spec.fields.rkey_inv_vld if hasattr(self.spec.fields, 'rkey_inv_vld') else 0
        self.imm_data_vld = self.spec.fields.imm_data_vld if hasattr(self.spec.fields, 'imm_data_vld') else 0
        self.ipv4 = self.spec.fields.ipv4 if hasattr(self.spec.fields, 'ipv4') else 0
        self.src_qp = self.spec.fields.src_qp if hasattr(self.spec.fields, 'src_qp') else 0
        self.smac = self.spec.fields.smac if hasattr(self.spec.fields, 'smac') else 0
        self.pkey_index = self.spec.fields.pkey_index if hasattr(self.spec.fields, 'pkey_index') else 0
        self.imm_data_or_r_key = self.spec.fields.imm_data if hasattr(self.spec.fields, 'imm_data') else self.spec.fields.r_key if hasattr(self.spec.fields, 'r_key') else 0
        self.status_or_length = self.spec.fields.status if hasattr(self.spec.fields, 'status') else self.spec.fields.length if hasattr(self.spec.fields, 'length') else 0
        self.qid = self.spec.fields.qid if hasattr(self.spec.fields, 'qid') else 0
        self.type = self.spec.fields.type if hasattr(self.spec.fields, 'type') else 1 #CQE_TYPE_RECV
        self.error = self.spec.fields.error if hasattr(self.spec.fields, 'error') else 0
        self.color = self.spec.fields.color if hasattr(self.spec.fields, 'color') else 0
        #CQE_TYPE_RECV
        assert(self.type == 1)

        self.__create_desc()

    def __create_desc(self):
        self.desc = RdmaCqDescriptorRecv(
            wrid=self.wrid,
            op_type=self.op_type,
            imm_data_vld=self.imm_data_vld,
            rkey_inv_vld=self.rkey_inv_vld,
            ipv4=self.ipv4,
            src_qp=self.src_qp,
            smac=self.smac,
            pkey_index=self.pkey_index,
            imm_data_or_r_key=self.imm_data_or_r_key,
            qid=self.qid,
            status_or_length=self.status_or_length,
            type=self.type,
            error=self.error,
            color=self.color)
        
    def __set_desc(self, desc):
        self.desc = desc
    
    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        logger.info("Writing CQ(Recv) Desciptor @0x%x = wrid: 0x%x " % 
                       (self.address, self.wrid))
        resmgr.HostMemoryAllocator.write(self.mem_handle, 
                                         bytes(self.desc))

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        logger.info("Reading CQ(Recv) Desciptor @0x%x = wrid: 0x%x " % 
                       (self.address, self.wrid))
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        mem_handle = resmgr.MemHandle(self.address, self.phy_address)
        self.__set_desc(RdmaCqDescriptorRecv(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaCqDescriptorRecv()))))

    def Show(self):
        logger.ShowScapyObject(self.desc)

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on CQ(Recv) descriptor..")

        logger.info('self(expected):')
        self.Show()
        logger.info('other(actual):')
        other.Show()

        if self.desc.error == 0 and other.desc.error == 0: #CQ_STATUS_SUCCESS
            return self.desc == other.desc

        logger.info('error is not 0\n')

        if self.desc.error != other.desc.error:
            return False

        logger.info('error matched\n')

        if self.desc.wrid != other.desc.wrid:
            return False

        logger.info('wrid matched\n')

        if self.desc.op_type != other.desc.op_type:
            return False

        logger.info('op_type matched\n')

        if self.desc.status_or_length != other.desc.status_or_length:
            return False

        logger.info('status_or_length matched\n')

        #Do not verify other fields if error is set
        if self.desc.error == 1:
           return True

        if self.desc.imm_data_vld != other.desc.imm_data_vld:
            return False

        logger.info('imm_data_vld matched\n')

        if self.desc.rkey_inv_vld != other.desc.rkey_inv_vld:
            return False

        logger.info('rkey_inv_vld matched\n')

        if self.desc.ipv4 != other.desc.ipv4:
            return False

        logger.info('ipv4 matched\n')

        if self.desc.src_qp != other.desc.src_qp:
            return False

        logger.info('src_qp matched\n')

        if self.desc.smac != other.desc.smac:
            return False

        logger.info('smac matched\n')

        if self.desc.pkey_index != other.desc.pkey_index:
            return False

        logger.info('pkey_index matched\n')

        if self.desc.imm_data_or_r_key != other.desc.imm_data_or_r_key:
            return False

        logger.info('imm_data_or_r_key matched\n')

        if self.desc.qid != other.desc.qid:
            return False

        logger.info('qid matched\n')

        if self.desc.type != other.desc.type:
            return False

        logger.info('type matched\n')

        if self.desc.color != other.desc.color:
            return False

        logger.info('color matched\n')

        return True

        #no need to compare other params as they are meaningful only incase of SUCCESS


    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on CQ(Recv) descriptor")
        # CQ is not associated with any buffer and hence simply create
        # default RDMABuffer object so that ebuf == abuf check passes
        return rdmabuffer.RdmaBufferObject()


class RdmaCqDescriptorSendObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(FactoryStore.templates.Get('DESC_RDMA_CQ_SEND'))

    def Init(self, spec):
        super().Init(spec)
        self.wrid = self.spec.fields.wrid if hasattr(self.spec.fields, 'wrid') else 0
        self.msn = self.spec.fields.msn if hasattr(self.spec.fields, 'msn') else 0
        self.status = self.spec.fields.status if hasattr(self.spec.fields, 'status') else 0
        self.qid = self.spec.fields.qid if hasattr(self.spec.fields, 'qid') else 0
        self.type = self.spec.fields.type if hasattr(self.spec.fields, 'type') else 2 #CQE_TYPE_SEND_MSN
        self.error = self.spec.fields.error if hasattr(self.spec.fields, 'error') else 0
        self.color = self.spec.fields.color if hasattr(self.spec.fields, 'color') else 0
        #CQE_TYPE_SEND_MSN or CQE_TYPE_SEND_NPG
        logger.info("CQ Descriptor type: %d" % self.type)
        assert(self.type == 2 or self.type == 3)

        #CQE_TYPE_SEND_MSN and msn go together
        if self.type == 2:
           assert(hasattr(self.spec.fields, 'msn'))

        #CQE_TYPE_SEND_NPG and wrid go together
        if self.type == 3:
           assert(hasattr(self.spec.fields, 'wrid'))
           #keep the wrid to 16 bit value
           assert(self.wrid < 65536)

        self.__create_desc()

    def __create_desc(self):
        self.desc = RdmaCqDescriptorSend(
            wrid=self.wrid,
            msn=self.msn,
            qid=self.qid,
            status=self.status,
            type=self.type,
            error=self.error,
            color=self.color)
        
    def __set_desc(self, desc):
        self.desc = desc
    
    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        logger.info("Writing CQ(Send) Desciptor @0x%x = wrid: 0x%x " % 
                       (self.address, self.wrid))
        resmgr.HostMemoryAllocator.write(self.mem_handle, 
                                         bytes(self.desc))

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        logger.info("Reading CQ(Send) Desciptor @0x%x = wrid: 0x%x " % 
                       (self.address, self.wrid))
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        mem_handle = resmgr.MemHandle(self.address, self.phy_address)
        self.__set_desc(RdmaCqDescriptorSend(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaCqDescriptorSend()))))

    def Show(self):
        logger.ShowScapyObject(self.desc)

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on CQ(Send) descriptor..")

        logger.info('self(expected):')
        self.Show()
        logger.info('other(actual):')
        other.Show()

        if self.desc.error == 0 and other.desc.error == 0: #CQ_STATUS_SUCCESS
            return self.desc == other.desc

        logger.info('error is not 0\n')

        if self.desc.error != other.desc.error:
            return False

        logger.info('error matched\n')

        if self.desc.wrid != other.desc.wrid:
            return False

        logger.info('wrid matched\n')

        if self.desc.msn != other.desc.msn:
            return False

        logger.info('msn matched\n')

        if self.desc.qid != other.desc.qid:
            return False

        logger.info('qid matched\n')

        if self.desc.type != other.desc.type:
            return False

        logger.info('type matched\n')

        if self.desc.status != other.desc.status:
            return False

        logger.info('status matched\n')

        if self.desc.color != other.desc.color:
            return False

        logger.info('color matched\n')

        return True

        #no need to compare other params as they are meaningful only incase of SUCCESS


    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on CQ(Send) descriptor")
        # CQ is not associated with any buffer and hence simply create
        # default RDMABuffer object so that ebuf == abuf check passes
        return rdmabuffer.RdmaBufferObject()



class RdmaEqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(FactoryStore.templates.Get('DESC_RDMA_EQ'))

    def Init(self, spec):
        super().Init(spec)
        if hasattr(spec.fields, 'qid'):
            self.qid = spec.fields.qid
        if hasattr(spec.fields, 'type'):
            self.type = spec.fields.type
        if hasattr(spec.fields, 'code'):
            self.code = spec.fields.code
        if hasattr(spec.fields, 'color'):
            self.color = spec.fields.color
        self.__create_desc()

    def __create_desc(self):
        self.desc = RdmaEqDescriptor(
            qid=self.qid,
            type=self.type,
            code=self.code,
            color=self.color)
        
    def __set_desc(self, desc):
        self.desc = desc
    
    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        logger.info("Writing EQ Desciptor @0x%x = qid: %d " % 
                       (self.address, self.qid))
        resmgr.HostMemoryAllocator.write(self.mem_handle, 
                                         bytes(self.desc))

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        logger.info("Reading EQ Desciptor @ 0x%x " % (self.address))
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        mem_handle = resmgr.MemHandle(self.address, self.phy_address)
        self.__set_desc(RdmaEqDescriptor(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaEqDescriptor()))))

    def Show(self):
        logger.ShowScapyObject(self.desc)

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on Eq descriptor..")

        logger.info('\nself(expected):')
        self.Show()
        logger.info('\nother(actual):')
        other.Show()

        if self.desc.qid != other.desc.qid:
            return False

        logger.info('qid matched\n')

        if self.desc.color != other.desc.color:
            return False

        logger.info('color matched\n')

        if self.desc.type != other.desc.type:
            return False

        logger.info('type matched\n')

        if self.desc.code != other.desc.code:
            return False

        logger.info('code matched\n')

        logger.info('EQ descriptor matched\n')
        return True

        #no need to compare other params as they are meaningful only incase of SUCCESS


    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on EQ descriptor")
        # EQ is not associated with any buffer and hence simply create
        # default RDMABuffer object so that ebuf == abuf check passes
        return rdmabuffer.RdmaBufferObject()

