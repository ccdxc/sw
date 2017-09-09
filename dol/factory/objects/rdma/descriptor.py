#! /usr/bin/python3

from scapy.all import *

import config.resmgr as resmgr

import infra.factory.base as base
from infra.common.logging   import cfglogger

import model_sim.src.model_wrap as model_wrap


class RdmaSqDescriptorBase(Packet):
    fields_desc = [
        LELongField("wrid", 0),
        BitField("op_type", 0, 4),
        BitField("complete_notify", 0, 1),
        BitField("fence", 0, 1),
        BitField("solicited_event", 0, 1),
        BitField("rsvd0", 0, 1),
        ByteField("num_sges", 0),
        LEShortField("rsvd1", 0),
    ]

class RdmaSqDescriptorSend(Packet):
    fields_desc = [
        LEIntField("imm_data", 0),
        LEIntField("inv_key", 0),
        LEIntField("rsvd0", 0),
        LEIntField("rsvd1", 0),
        LEIntField("rsvd2", 0),
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
        LELongField("va", 0),
        LEIntField("len", 0),
        LEIntField("l_key", 0),
    ]



class RdmaSqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.logger = cfglogger

    def Init(self, spec):
        super().__init__()
        self.spec = spec

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        cfglogger.info("Writing Desciptor @0x%x = op_type: 0x%d wrid: 0x%x num_sges: %d" % 
                       (self.address, self.spec.fields.op_type, self.spec.fields.wrid, self.spec.fields.num_sges))
        desc = RdmaSqDescriptorBase(op_type=self.spec.fields.op_type, wrid=self.spec.fields.wrid,
                                    num_sges=self.spec.fields.num_sges)
        for sge in spec.fields.sges:
            sge_entry = RdmaSge(va=sge.va, len=sge.len, l_key=sge.l_key)
            desc = desc/sge_entry
              
        print(desc)
        #model_wrap.write_mem(self.address, bytes(desc), len(desc))
        resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        desc = RdmaSqDescriptor(model_wrap.read_mem(self.address, 32))
        desc.show()
        self.wrid = desc.wrid
        self.op_type = desc.op_type
        self.num_sges = desc.num_sges
        cfglogger.info("Read Desciptor @0x%x = op_type: %d wrid: 0x%x len: %d" % 
                       (self.address, self.op_type, self.wrid, self.num_sges))
        # no need to read sges for SQs, ignore
        #for i in num_sges:
        #     sge = RdmaSge(model_wrap.read_mem(self.address + 32 + i * 16, 16))
        #     cfglogger.info("Read Sge[%d] = va: 0x%x len: %d l_key: %d" % 
        #                         (i, sge.va, sge.len, sge.l_key))

    def Show(self):
        desc = RdmaSqDescriptor(wrid=self.wrid, op_type=self.op_type, num_sges=self.num_sges)
        desc.show()
        #TODO: Check if we need to show SGEs

class RdmaRqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.logger = cfglogger

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
            sge_entry = RdmaSge(va=sge.va, len=sge.len, l_key=sge.l_key)
            desc = desc/sge_entry
              
        print(bytes(desc))
        #model_wrap.write_mem(self.address, bytes(desc), len(desc))
        resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        #self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        desc = RdmaRqDescriptorBase(model_wrap.read_mem(self.address, 32))
        desc.show()
        self.wrid = desc.wrid
        self.num_sges = desc.num_sges
        cfglogger.info("Read Desciptor @0x%x = wrid: 0x%x len: %d" % 
                       (self.address, self.wrid, self.num_sges))
        # no need to read sges for RQs, ignore
        #for i in num_sges:
        #     sge = RdmaSge(model_wrap.read_mem(self.address + 32 + i * 16, 16))
        #     cfglogger.info("Read Sge[%d] = va: 0x%x len: %d l_key: %d" % 
        #                         (i, sge.va, sge.len, sge.l_key))

    def Show(self):
        desc = RdmaRqDescriptor(addr=self.wrid, len=self.num_sges)
        desc.show()
        #TODO: Check if we need to show SGEs

