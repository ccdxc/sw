#! /usr/bin/python3

import copy
from scapy.all import *
import config.resmgr            as resmgr
import infra.penscapy.penscapy  as penscapy
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
        LEShortField("packet_len", 0),
        LEIntField("rsvd0", 0),
    ]


class EthRxDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.logger = cfglogger

    def Init(self, spec):
        super().Init(spec)
        self.buf = spec.fields.buf
        self.buf_addr = getattr(spec.fields, 'addr', 0)    # Buffer Address
        self.buf_len = spec.fields.len      # Buffer Length
        self._mem = None
        self.logger.info("Init %s" % self)

    def Write(self):
        """
        Writes a Descriptor to Memory
        :return:
        """
        if self._mem is None: return

        self.logger.info("Writing %s" % self)
        desc = EthRxDescriptor(addr=self.buf_addr, len=self.buf_len)
        resmgr.HostMemoryAllocator.write(self._mem, bytes(desc))

    def Read(self):
        """
        Reads a Descriptor from Memory
        :return:
        """
        if self._mem is None: return

        desc = EthRxDescriptor(resmgr.HostMemoryAllocator.read(self._mem, 16))
        self.logger.ShowScapyObject(desc)
        self.buf_addr = desc.addr
        self.buf_len = desc.packet_len
        self.buf.Bind(resmgr.HostMemoryAllocator.p2v(self.buf_addr))
        self.logger.info("Read %s" % self)

    def __str__(self):
        return "%s GID:%s/Id:0x%x/Memory:%s/Buf:%s/Buf.Id:0x%x/Buf_Addr:0x%x/Buf_Len:%d" % (
                self.__class__.__name__, self.GID(), id(self), self._mem,
                self.buf.GID(), id(self.buf), self.buf_addr, self.buf_len)

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False

        result = True
        if self.buf_len != other.buf_len:
            self.logger.info("  Buf Len Mismatch: Expected %s Actual %s" %\
                             (str(self.buf_len), str(other.buf_len)))
            other.buf_len = other.buf.size
        return result

    def __copy__(self):
        obj = super().__copy__()
        obj.GID('ACTUAL_' + obj.GID())
        # [obj._mem] must point to the original descriptor's memory handle
        obj.buf = copy.copy(self.buf)
        # The following fields are filled in by Read()
        obj.buf_addr = 0
        obj.buf_len = 0
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
        self.buf_addr = getattr(spec.fields, 'addr', 0)    # Buffer Address
        self.buf_len = spec.fields.len      # Buffer Length
        self._mem = None
        self.logger.info("Init %s" % self)

    def Write(self):
        """
        Writes a Descriptor to Memory
        :return:
        """
        if self._mem is None: return

        self.logger.info("Writing %s" % self)
        desc = EthTxDescriptor(addr=self.buf_addr, len=self.buf_len)
        resmgr.HostMemoryAllocator.write(self._mem, bytes(desc))
        # self.Read()

    def Read(self):
        """
        Reads a Descriptor from Memory
        :return:
        """
        if self._mem is None: return

        desc = EthTxDescriptor(resmgr.HostMemoryAllocator.read(self._mem, 16))
        self.logger.ShowScapyObject(desc)
        self.buf_addr = desc.addr
        self.buf_len = desc.len
        self.buf.Bind(resmgr.HostMemoryAllocator.p2v(self.buf_addr))
        self.logger.info("Read %s" % self)

    def Bind(self, _mem):
        assert '_mem' not in self.__dict__ or self._mem is None
        super().Bind(_mem)

    def __str__(self):
        return "%s GID:%s/Id:0x%x/Memory:%s/Buf:%s/Buf.Id:0x%x/Buf_Addr:0x%x/Buf_Len:%d" % (
                self.__class__.__name__, self.GID(), id(self), self._mem,
                self.buf.GID(), id(self.buf), self.buf_addr, self.buf_len)

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
        obj.buf_addr = 0
        obj.buf_len = 0
        return obj

    def GetBuffer(self):
        return self.buf
