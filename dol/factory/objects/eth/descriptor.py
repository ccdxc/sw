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


class EthRxCqDescriptor(Packet):
    fields_desc = [
        LEShortField("completion_index", 0),
        ByteField("queue_id", 0),
        ByteField("rss_type", 0),
        LEIntField("rss_hash", 0),
        LEIntField("checksum", 0),
        LEShortField("bytes_written", 0),
        LEShortField("vlan_tci", 0),
        LEShortField("flags", 0),
        BitField("rsvd0", 0, 96),
        BitField("rsvd1", 0, 10),
        BitField("checksum_valid", 0, 1),
        BitField("vlan_valid", 0, 1),
        BitField("encap", 0, 1),
        BitField("pkt_error", 0, 1),
        BitField("dma_error", 0, 1),
        BitField("color", 0, 1),
    ]


class EthTxCqDescriptor(Packet):
    fields_desc = [
        LEShortField("completion_index", 0),
        ByteField("rsvd0", 0),
        ByteField("queue_id", 0),
        BitField("rsvd1", 0, 80),
        ByteField("err_code", 0),
        BitField("rsvd2", 0, 7),
        BitField("color", 0, 1),
    ]


class AdminDesciptor(Packet):
    fields_desc = [
        LEShortField("opcode", 0),
        LEShortField("rsvd", 0),
        LEIntField("cmd_data0", 0),
        LEIntField("cmd_data1", 0),
        LEIntField("cmd_data2", 0),
        LEIntField("cmd_data3", 0),
        LEIntField("cmd_data4", 0),
        LEIntField("cmd_data5", 0),
        LEIntField("cmd_data6", 0),
        LEIntField("cmd_data7", 0),
        LEIntField("cmd_data8", 0),
        LEIntField("cmd_data9", 0),
        LEIntField("cmd_data10", 0),
        LEIntField("cmd_data11", 0),
        LEIntField("cmd_data12", 0),
        LEIntField("cmd_data13", 0),
        LEIntField("cmd_data14", 0),
        LEIntField("cmd_data15", 0),
    ]


class AdminCqDescriptor(Packet):
    fields_desc = [
        ByteField("cmd_status", 0),
        LEShortField("cpl_id", 0),
        LEIntField("cmd_data0", 0),
        LEIntField("cmd_data1", 0),
        LEIntField("cmd_data2", 0),
        BitField("rsvd", 7, 0),
        BitField("color", 1, 0),
    ]


class EthDescriptorObject(base.FactoryObjectBase):

    __descriptor_class__ = None

    def __init__(self):
        super().__init__()
        self.logger = cfglogger
        self.desc = None

    def Init(self, spec):
        super().Init(spec)
        self.buf = getattr(spec.fields, 'buf', None)       # Buffer Object
        self.buf_addr = getattr(spec.fields, 'addr', 0)    # Buffer Address
        self.buf_len = getattr(spec.fields, 'len', 0)     # Buffer Length
        self._mem = None
        self.logger.info("Init %s" % self)

    def Write(self):
        """
        Writes a Descriptor to Memory
        :return:
        """
        if self._mem is None: return

        self.logger.info("Writing %s" % self)
        self.desc = self.__descriptor_class__(addr=self.buf_addr, len=self.buf_len)
        resmgr.HostMemoryAllocator.write(self._mem, bytes(self.desc))

    def Read(self):
        """
        Reads a Descriptor from Memory
        :return:
        """
        if self._mem is None: return

        self.desc = self.__descriptor_class__(resmgr.HostMemoryAllocator.read(self._mem, 16))
        self.logger.ShowScapyObject(self.desc)

    def __str__(self):
        if self.buf:
            return "%s GID:%s/Id:0x%x/Memory:%s/Buf:%s/Buf.Id:0x%x/Buf_Addr:0x%x/Buf_Len:%d" % (
                    self.__class__.__name__, self.GID(), id(self), self._mem,
                    self.buf.GID(), id(self.buf), self.buf_addr, self.buf_len)
        else:
            return "%s GID:%s/Id:0x%x/Memory:%s/Buf:None" % (
                    self.__class__.__name__, self.GID(), id(self), self._mem)

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        # Todo: Need to compare non-None fields
        return True

    def __copy__(self):
        obj = super().__copy__()
        obj.GID('ACTUAL_' + obj.GID())
        # [obj._mem] must point to the original descriptor's memory handle
        obj.buf = copy.copy(self.buf) if self.buf else None
        # The following fields are filled in by Read()
        obj.buf_addr = 0
        obj.buf_len = 0
        return obj

    def GetBuffer(self):
        return self.buf


class EthRxDescriptorObject(EthDescriptorObject):
    __descriptor_class__ = EthRxDescriptor

    def Read(self):
        # This is a write-only descriptor type
        return


class EthRxCqDescriptorObject(EthDescriptorObject):
    __descriptor_class__ = EthRxCqDescriptor

    def Write(self):
        # This is a read-only descriptor type
        return

    def Read(self):
        super().Read()
        if self.buf is not None and self.buf._mem is not None:
            self.buf_addr = self.buf._mem.pa
            self.buf_len = self.desc[EthRxCqDescriptor].bytes_written
            self.logger.info("Read %s" % self)


class EthTxDescriptorObject(EthDescriptorObject):
    __descriptor_class__ = EthTxDescriptor

    def Read(self):
        # This is a write-only descriptor type
        return


class EthTxCqDescriptorObject(EthDescriptorObject):
    __descriptor_class__ = EthTxCqDescriptor

    def Write(self):
        # This is a read-only descriptor type
        return

    def Read(self):
        super().Read()
        if self.buf is not None and self.buf._mem is not None:
            self.buf_addr = self.buf._mem.pa
            self.buf_len = None
            self.logger.info("Read %s" % self)


class AdminDescriptorObject(EthDescriptorObject):
    __descriptor_class__ = AdminDesciptor

    def Read(self):
        # This is a write-only descriptor type
        return


class AdminCqDescriptorObject(EthDescriptorObject):
    __descriptor_class__ = AdminCqDescriptor

    def Write(self):
        # This is a read-only descriptor type
        return
