#! /usr/bin/python3

import copy
from scapy.all import *
import config.resmgr            as resmgr
from infra.common.logging       import cfglogger

import infra.factory.base as base

class EthTxDescriptor(Packet):
    fields_desc = [
        LELongField("addr", 0),
        LEShortField("len", 0),
        LEShortField("vlan_tag", 0xffff),
        BitField("mss", 0, 14),
        BitField("encap", 0, 2),
        BitField("hdr_len", 0, 10),
        BitField("offload_mode", 0, 2),
        BitField("eop", 1, 1),
        BitField("cq_entry", 1, 1),
        BitField("vlan_insert", 1, 1),
        BitField("rsvd0", 1, 1),
    ]


class EthRxDescriptor(Packet):
    fields_desc = [
        LELongField("addr", 0),
        LEShortField("len", 0),
        LEIntField("rsvd0", 0),
        LEShortField("__pad0", 0),
    ]


class EthRxDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.logger = cfglogger

    def Init(self, spec):
        super().Init(spec)
        self.buf = spec.fields.buf
        self.buf_addr = getattr(spec.fields, 'addr', None)    # Buffer Address
        self.buf_len = spec.fields.len      # Buffer Length

    def Write(self):
        """
        Writes a Descriptor to Memory
        :return:
        """
        if self._mem == None: return

        self.logger.info("Writing EthRxDescriptor:%s" % self.GID())
        self.logger.info("- Memory: %s = addr: 0x%x len: %d" % (self._mem, self.buf_addr, self.buf_len))
        desc = EthRxDescriptor(addr=self.buf_addr, len=self.buf_len)
        resmgr.HostMemoryAllocator.write(self._mem, bytes(desc))

    def Read(self):
        """
        Reads a Descriptor from Memory
        :return:
        """
        if self._mem == None: return

        desc = EthRxDescriptor(resmgr.HostMemoryAllocator.read(self._mem, 16))
        # self.logger.ShowScapyObject(desc)
        self.buf_addr = desc.addr
        self.buf_len = 130  #desc.rsvd0
        self.buf.Bind(resmgr.HostMemoryAllocator.p2v(self.buf_addr))
        self.logger.info("Read EthRxDescriptor:%s" % self.GID())
        self.logger.info("- Memory: %s = addr: 0x%x len: %d" % (self._mem, self.buf_addr, self.buf_len))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False

        result = True
        if self.buf_len != other.buf_len:
            self.logger.info("  Buf Len Mismatch: Expected %s Actual %s" %\
                             (str(self.buf_len), str(other.buf_len)))
            result = False
        return result

    def __copy__(self):
        obj = super().__copy__()
        obj.GID('ACTUAL_' + obj.GID())
        # [obj._mem] must point to the original descriptor's memory handle
        obj.buf = copy.copy(self.buf)
        # The following fields are filled in by Read()
        obj.buf_addr = None
        obj.buf_len = None
        return obj

    def GetBuffer(self):
        return self.buf


class EthTxDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.logger = cfglogger

    def Init(self, spec):
        super().Init(spec)
        self.buf = spec.fields.buf
        self.buf_addr = getattr(spec.fields, 'addr', None)    # Buffer Address
        self.buf_len = spec.fields.len      # Buffer Length

    def Write(self):
        """
        Writes a Descriptor to Memory
        :return:
        """
        if self.buf_addr == None: return

        self.logger.info("Writing EthTxDescriptor:%s" % self.GID())
        self.logger.info("- Memory: %s = addr: 0x%x len: %d" % (self._mem, self.buf_addr, self.buf_len))
        desc = EthTxDescriptor(addr=self.buf_addr, len=self.buf_len)
        resmgr.HostMemoryAllocator.write(self._mem, bytes(desc))
        # self.Read()

    def Read(self):
        """
        Reads a Descriptor from Memory
        :return:
        """
        if self._mem == None: return

        desc = EthTxDescriptor(resmgr.HostMemoryAllocator.read(self._mem, 16))
        # self.logger.ShowScapyObject(desc)
        self.buf_addr = desc.addr
        self.buf_len = desc.len
        self.buf.Bind(resmgr.HostMemoryAllocator.p2v(self.buf_addr))
        self.logger.info("Read EthTxDescriptor:%s" % self.GID())
        self.logger.info("- Memory: %s = addr: 0x%x len: %d" % (self._mem, self.buf_addr, self.buf_len))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False

        result = True
        if self.buf_len != other.buf_len:
            self.logger.info("  Buf Len Mismatch: Expected %s Actual %s" %\
                             (str(self.buf_len), str(other.buf_len)))
            result = False
        return result

    def __copy__(self):
        obj = super().__copy__()
        obj.GID('ACTUAL_' + obj.GID())
        # [obj._mem] must point to the original descriptor's memory handle
        obj.buf = copy.copy(self.buf)
        # The following fields are filled in by Read()
        obj.buf_addr = None
        obj.buf_len = None
        return obj

    def GetBuffer(self):
        return self.buf
