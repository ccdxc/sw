#! /usr/bin/python3

from scapy.all import *

import config.resmgr as resmgr

import infra.factory.base as base
from infra.common.logging   import cfglogger

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
        ShortField("rsvd1", 0),
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
        IntField("dst_qp", 0),
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

class RdmaCqDescriptor(Packet):
    fields_desc = [
        LongField("wrid", 0),
        ByteField("op_type", 0),
        ByteField("status", 0),
        BitField("rkey_inv_vld", 0, 1),
        BitField("imm_data_vld", 0, 1),
        BitField("color", 0, 1),
        BitField("ipv4", 0, 1),
        BitField("rsvd1", 0, 4),
        ByteField("rsvd2", 0),
        X3BytesField("qp", 0),
        X3BytesField("src_qp", 0),
        MACField("smac", ETHER_ANY),
        IntField("imm_data", 0),
        IntField("r_key", 0),
    ]

class RdmaSqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.logger = cfglogger
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
        cfglogger.info("Writing Descriptor @0x%x = op_type: %d wrid: 0x%x inline_data_vld: %d num_sges: %d" % 
                       (self.address, self.spec.fields.op_type, self.wrid, inline_data_vld, num_sges))
        desc = RdmaSqDescriptorBase(op_type=self.spec.fields.op_type, wrid=self.wrid,
                                    inline_data_vld = inline_data_vld, num_sges=num_sges)
        inline_data_len = 0
        inline_data = None

        # Make sure Inline data is specificied only for Send and Write, assert in other operations
        if hasattr(self.spec.fields, 'send'):
           print("Reading Send")
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
           print("Reading UD Send")
           dst_qp = self.spec.fields.ud_send.dst_qp if hasattr(self.spec.fields.ud_send, 'dst_qp') else 0
           q_key = self.spec.fields.ud_send.q_key if hasattr(self.spec.fields.ud_send, 'q_key') else 0
           ah_handle = self.spec.fields.ud_send.ah_handle if hasattr(self.spec.fields.ud_send, 'ah_handle') else 0
           imm_data = self.spec.fields.ud_send.imm_data if hasattr(self.spec.fields.ud_send, 'imm_data') else 0
           send = RdmaSqDescriptorUDSend(imm_data=imm_data, q_key=q_key, dst_qp=dst_qp, ah_handle=ah_handle)
           desc = desc/send

        if hasattr(self.spec.fields, 'write'):
           print("Reading Write")
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
           print("Reading Read")
           assert(inline_data_vld == 0)
           rsvd = self.spec.fields.read.rsvd if hasattr(self.spec.fields.read, 'rsvd') else 0
           va = self.spec.fields.read.va if hasattr(self.spec.fields.read, 'va') else 0
           len = self.spec.fields.read.len if hasattr(self.spec.fields.read, 'len') else 0
           r_key = self.spec.fields.read.r_key if hasattr(self.spec.fields.read, 'r_key') else 0
           read = RdmaSqDescriptorRead(rsvd=rsvd, va=va, len=len, r_key=r_key)
           desc = desc/read

        if hasattr(self.spec.fields, 'atomic'):
           print("Reading Atomic")
           assert(inline_data_vld == 0)
           r_key = self.spec.fields.atomic.r_key if hasattr(self.spec.fields.atomic, 'r_key') else 0
           va = self.spec.fields.atomic.va if hasattr(self.spec.fields.atomic, 'va') else 0
           cmpdt = self.spec.fields.atomic.cmpdt if hasattr(self.spec.fields.atomic, 'cmpdt') else 0
           swapdt = self.spec.fields.atomic.swapdt if hasattr(self.spec.fields.atomic, 'swapdt') else 0
           atomic = RdmaSqDescriptorAtomic(r_key=r_key, va=va, cmpdt=cmpdt, swapdt=swapdt)
           desc = desc/atomic

        if inline_data_vld:
           print("Inline Data: %s " % bytes(inline_data[0:inline_data_len]))
           desc = desc/bytes(inline_data)
        else:
            for sge in self.spec.fields.sges:
                sge_entry = RdmaSge(va=sge.va, len=sge.len, l_key=sge.l_key)
                cfglogger.info("Read Sge[] = va: 0x%x len: %d l_key: %d" % 
                               (sge.va, sge.len, sge.l_key))
                desc = desc/sge_entry
              
        desc.show()
        #model_wrap.write_mem(self.address, bytes(desc), len(desc))
        resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        #self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        self.phy_address = resmgr.HostMemoryAllocator.get_v2p(self.address)
        mem_handle = resmgr.MemHandle(self.address, self.phy_address)
        desc = RdmaSqDescriptorBase(resmgr.HostMemoryAllocator.read(mem_handle, 32))
        desc.show()
        self.wrid = desc.wrid
        self.num_sges = desc.num_sges
        self.op_type = desc.op_type
        cfglogger.info("Read Desciptor @0x%x = wrid: 0x%x num_sges: %d op_type: %d" % 
                       (self.address, self.wrid, self.num_sges, self.op_type))
        self.sges = []
        mem_handle.va += 32
        for i in range(self.num_sges):
            
            self.sges.append(RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16)))
            #sge = RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16))
            #self.sges.append(sge)
            #cfglogger.info("Read Sge[%d] = va: 0x%x len: %d l_key: %d" % 
            #               (i, sge.va, sge.len, sge.l_key))
            mem_handle.va += 16

        for sge in self.sges:
            print('%s' % type(sge))
            print('0x%x' % sge.va)
        print('id: %s' %id(self))


    def Show(self):
        desc = RdmaSqDescriptor(wrid=self.wrid, op_type=self.op_type, num_sges=self.num_sges)
        desc.show()
        #TODO: Check if we need to show SGEs

    def __eq__(self, other):
        cfglogger.info("__eq__ operator invoked on descriptor, ignoring for now..")
        return True

    def GetBuffer(self):
        cfglogger.info("GetBuffer() operator invoked on descriptor")

        #if hasattr(self.spec.fields, 'buff'):
        if not hasattr(self, 'address'):
            return self.spec.fields.buff

        rdmabuff = rdmabuffer.RdmaBufferObject()
        cfglogger.info("wrid: %d num_sges: %d len: %d" % (self.wrid, self.num_sges, len(self.sges)));
        total_data = bytearray()
        total_size = 0 
        for idx in range(self.num_sges):
            sge = self.sges[idx]
            cfglogger.info("Reading sge content : 0x%x  len: %d" %(sge.va, sge.len))
            mem_handle = resmgr.MemHandle(sge.va,
                                    resmgr.HostMemoryAllocator.get_v2p(sge.va))
            sge_data = resmgr.HostMemoryAllocator.read(mem_handle, sge.len)
            cfglogger.info("     sge data: %s" % bytes(sge_data))
            total_data.extend(sge_data)
            total_size += sge.len
        rdmabuff.data = bytes(total_data)
        rdmabuff.size = total_size
        cfglogger.info("Total data: %s" % bytes(total_data))
        return rdmabuff

class RdmaRqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.logger = cfglogger
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
        cfglogger.info("Writing Descriptor @(va:0x%x, pa:0x%x) = wrid: 0x%x num_sges: %d" % 
                       (self.mem_handle.va, self.mem_handle.pa, self.wrid, self.spec.fields.num_sges))
        desc = RdmaRqDescriptorBase(wrid=self.wrid,
                                    num_sges=self.spec.fields.num_sges)
        for sge in self.spec.fields.sges:
            cfglogger.info("sge: va: 0x%x len: %d l_key: %d" %(sge.va, sge.len, sge.l_key))
            sge_entry = RdmaSge(va=sge.va, len=sge.len, l_key=sge.l_key)
            desc = desc/sge_entry
              
        #model_wrap.write_mem(self.address, bytes(desc), len(desc))
        resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        #self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        self.phy_address = resmgr.HostMemoryAllocator.get_v2p(self.address)
        mem_handle = resmgr.MemHandle(self.address, self.phy_address)
        desc = RdmaRqDescriptorBase(resmgr.HostMemoryAllocator.read(mem_handle, 32))
        desc.show()
        self.wrid = desc.wrid
        self.num_sges = desc.num_sges
        cfglogger.info("Read Desciptor @0x%x = wrid: 0x%x num_sges: %d" % 
                       (self.address, self.wrid, self.num_sges))
        self.sges = []
        mem_handle.va += 32
        for i in range(self.num_sges):
            self.sges.append(RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16)))
            #sge = RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16))
            #self.sges.append(sge)
            #cfglogger.info("Read Sge[%d] = va: 0x%x len: %d l_key: %d" % 
            #               (i, sge.va, sge.len, sge.l_key))
            mem_handle.va += 16

        #for sge in self.sges:
        #    print('%s' % type(sge))
        #    print('0x%x' % sge.va)
        #print('id: %s' %id(self))

    def Show(self):
        desc = RdmaRqDescriptor(addr=self.wrid, len=self.num_sges)
        desc.show()
        #TODO: Check if we need to show SGEs

    def __eq__(self, other):
        cfglogger.info("__eq__ operator invoked on descriptor, ignoring for now..")
        return True

    def GetBuffer(self):
        cfglogger.info("GetBuffer() operator invoked on descriptor")

        #if hasattr(self.spec.fields, 'buff'):
        if not hasattr(self, 'address'):
            return self.spec.fields.buff

        rdmabuff = rdmabuffer.RdmaBufferObject()
        cfglogger.info("wrid: %d num_sges: %d len: %d" % (self.wrid, self.num_sges, len(self.sges)));
        total_data = bytearray()
        total_size = 0 
        for idx in range(self.num_sges):
            sge = self.sges[idx]
            cfglogger.info("Reading sge content : 0x%x  len: %d" %(sge.va, sge.len))
            mem_handle = resmgr.MemHandle(sge.va,
                                    resmgr.HostMemoryAllocator.get_v2p(sge.va))
            sge_data = resmgr.HostMemoryAllocator.read(mem_handle, sge.len)
            cfglogger.info("     sge data: %s" % bytes(sge_data))
            total_data.extend(sge_data)
            total_size += sge.len
        rdmabuff.data = bytes(total_data)
        rdmabuff.size = total_size
        cfglogger.info("Total data: %s" % bytes(total_data))
        return rdmabuff

class RdmaCqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(FactoryStore.templates.Get('DESC_RDMA_CQ'))
        self.logger = cfglogger

    def Init(self, spec):
        super().Init(spec)
        if hasattr(spec.fields, 'wrid'):
            self.wrid = spec.fields.wrid
        if hasattr(spec.fields, 'msn'):
            self.msn = spec.fields.msn
        if hasattr(spec.fields, 'op_type'):
            self.op_type = spec.fields.op_type
        if hasattr(spec.fields, 'status'):
            self.status = spec.fields.status
        if hasattr(spec.fields, 'rkey_inv_vld'):
            self.rkey_inv_vld = spec.fields.rkey_inv_vld
        if hasattr(spec.fields, 'imm_data_vld'):
            self.imm_data_vld = spec.fields.imm_data_vld
        if hasattr(spec.fields, 'color'):
            self.color = spec.fields.color
        if hasattr(spec.fields, 'qp'):
            self.qp = spec.fields.qp
        if hasattr(spec.fields, 'imm_data'):
            self.imm_data = spec.fields.imm_data
        if hasattr(spec.fields, 'r_key'):
            self.r_key = spec.fields.r_key
        if hasattr(spec.fields, 'ipv4'):
            self.ipv4 = spec.fields.ipv4
        if hasattr(spec.fields, 'src_qp'):
            self.src_qp = spec.fields.src_qp
        if hasattr(spec.fields, 'smac'):
            self.smac = bytes(
                spec.fields.smac)
            

        self.__create_desc()

    def __create_desc(self):
        self.desc = RdmaCqDescriptor(
            wrid=self.wrid,
            op_type=self.op_type,
            status=self.status,
            rkey_inv_vld=self.rkey_inv_vld,
            imm_data_vld=self.imm_data_vld,
            color=self.color,
            qp=self.qp,
            imm_data=self.imm_data,
            r_key=self.r_key,
            ipv4=self.ipv4,
            src_qp=self.src_qp,
            smac=self.smac)
        
    def __set_desc(self, desc):
        self.desc = desc
    
    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        cfglogger.info("Writing Desciptor @0x%x = wrid: 0x%x " % 
                       (self.address, self.wrid))
        resmgr.HostMemoryAllocator.write(self.mem_handle, 
                                         bytes(self.desc))

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        self.phy_address = resmgr.HostMemoryAllocator.get_v2p(self.address)
        mem_handle = resmgr.MemHandle(self.address, self.phy_address)
        self.__set_desc(RdmaCqDescriptor(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaCqDescriptor()))))

    def Show(self):
        self.desc.show()

    def __eq__(self, other):
        cfglogger.info("__eq__ operator invoked on cq descriptor..")

        cfglogger.info('self(expected):\n')
        self.Show()
        cfglogger.info('other(actual):\n')
        other.Show()

        if self.desc.status == 0: #CQ_STATUS_SUCCESS
            return self.desc == other.desc

        cfglogger.info('status is not 0\n')

        if self.desc.wrid != other.desc.wrid:
            return False

        cfglogger.info('wrid matched\n')

        if self.desc.op_type != other.desc.op_type:
            return False

        cfglogger.info('op_type matched\n')

        if self.desc.status != other.desc.status:
            return False

        cfglogger.info('status matched\n')

        return True

        #no need to compare other params as they are meaningful only incase of SUCCESS


    def GetBuffer(self):
        cfglogger.info("GetBuffer() operator invoked on cq descriptor")
        # CQ is not associated with any buffer and hence simply create
        # default RDMABuffer object so that ebuf == abuf check passes
        return rdmabuffer.RdmaBufferObject()

