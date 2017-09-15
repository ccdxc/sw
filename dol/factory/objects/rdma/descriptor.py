#! /usr/bin/python3

from scapy.all import *

import config.resmgr as resmgr

import infra.factory.base as base
from infra.common.logging   import cfglogger

import model_sim.src.model_wrap as model_wrap

from factory.objects.rdma import buffer as rdmabuffer

class RdmaSqDescriptorBase(Packet):
    fields_desc = [
        LongField("wrid", 0),
        BitField("op_type", 0, 4),
        BitField("complete_notify", 0, 1),
        BitField("fence", 0, 1),
        BitField("solicited_event", 0, 1),
        BitField("rsvd1", 0, 1),
        ByteField("num_sges", 0),
        ShortField("rsvd1", 0),
    ]

class RdmaSqDescriptorSend(Packet):
    fields_desc = [
        IntField("imm_data", 0),
        IntField("inv_key", 0),
        IntField("rsvd0", 0),
        IntField("rsvd1", 0),
        IntField("rsvd2", 0),
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
        LEIntField("len", 0),
        LELongField("wqe_sge_list_ptr", 0),
        LEIntField("rsvd0", 0),
        LEIntField("rsvd1", 0),
        LEIntField("rsvd2", 0),
        LEIntField("rsvd3", 0),
        LEIntField("rsvd4", 0),
        LEIntField("rsvd5", 0),
        LEIntField("rsvd6", 0),
        LEIntField("rsvd7", 0),
        LEIntField("rsvd8", 0),
        LEIntField("rsvd9", 0),
        LEIntField("rsvd10", 0),
    ]

class RdmaRrqDescriptorAtomic(Packet):
    fields_desc = [
        LELongField("va", 0),
        LEIntField("len", 0),
        LEIntField("l_key", 0),
        ByteField("op_type", 0),
        ByteField("rsvd0", 0),
        ByteField("rsvd1", 0),
        ByteField("rsvd2", 0),
        LEIntField("rsvd3", 0),
        LEIntField("rsvd4", 0),
        LEIntField("rsvd5", 0),
        LEIntField("rsvd6", 0),
        LEIntField("rsvd7", 0),
        LEIntField("rsvd8", 0),
        LEIntField("rsvd9", 0),
        LEIntField("rsvd10", 0),
        LEIntField("rsvd11", 0),
        LEIntField("rsvd12", 0),
    ]

class RdmaRqDescriptorBase(Packet):
    fields_desc = [
        LELongField("wrid", 0),
        ByteField("num_sges", 0),
        ByteField("rsvd0", 0),
        LEShortField("rsvd1", 0),
        LEIntField("rsvd2", 0),
        LELongField("rsvd3", 0),
        LELongField("rsvd4", 0),
    ]

class RdmaRsqDescriptorBase(Packet):
    fields_desc = [
        BitField("read_resp_or_atomic", 0, 1),
        BitField("rsvd0", 0, 7),
        X3BytesField("psn", 0),
        LELongField("rsvd1", 0),
    ]

class RdmaRsqDescriptorRead(Packet):
    fields_desc = [
        LEIntField("r_key", 0),
        LELongField("va", 0),
        LEIntField("len", 0),
        LEIntField("offset", 0),
    ]

class RdmaRsqDescriptorAtomic(Packet):
    fields_desc = [
        LEIntField("r_key", 0),
        LELongField("va", 0),
        LELongField("orig_data", 0),
    ]

class RdmaSge(Packet):
    fields_desc = [
        LongField("va", 0),
        IntField("len", 0),
        IntField("l_key", 0),
    ]



class RdmaSqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.logger = cfglogger
        self.sges = []

    def Init(self, spec):
        super().Init(spec)

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        cfglogger.info("Writing Desciptor @0x%x = op_type: 0x%d wrid: 0x%x num_sges: %d" % 
                       (self.address, self.spec.fields.op_type, self.spec.fields.wrid, self.spec.fields.num_sges))
        desc = RdmaSqDescriptorBase(op_type=self.spec.fields.op_type, wrid=self.spec.fields.wrid,
                                    num_sges=self.spec.fields.num_sges)
        if hasattr(self.spec.fields, 'send'):
           print("Reading Send")
           imm_data = self.spec.fields.imm_data if hasattr(self.spec.fields, 'imm_data') else 0
           inv_key = self.spec.fields.inv_key if hasattr(self.spec.fields, 'inv_key') else 0
           send = RdmaSqDescriptorSend(imm_data=imm_data, inv_key=inv_key)
           desc = desc/send

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

class RdmaRqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.logger = cfglogger
        self.sges = []

    def Init(self, spec):
        super().Init(spec)

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        cfglogger.info("Writing Desciptor @0x%x = wrid: 0x%x num_sges: %d" % 
                       (self.address, self.spec.fields.wrid, self.spec.fields.num_sges))
        desc = RdmaRqDescriptorBase(wrid=self.spec.fields.wrid,
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
