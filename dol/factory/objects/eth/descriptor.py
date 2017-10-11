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
        LEShortField("vlan_tag", 0),
        BitField("mss", 0, 14),
        BitField("encap", 0, 2),
        BitField("hdr_len", 0, 10),
        BitField("offload", 0, 2),
        BitField("eop", 0, 1),
        BitField("cq_entry", 0, 1),
        BitField("vlan_insert", 0, 1),
        BitField("rsvd0", 0, 1),
    ]


class EthTxCqDescriptor(Packet):
    fields_desc = [
        # Info
        LEShortField("completion_index", 0),
        ByteField("queue_id", 0),
        ByteField("err_code", 0),
        # END
        BitField("rsvd0", 0, 95),
        BitField("color", 0, 1),
    ]


class EthRxDescriptor(Packet):
    fields_desc = [
        LELongField("addr", 0),
        LEShortField("len", 0),
        BitField("rsvd0", 0, 48),
    ]


class EthRxCqDescriptor(Packet):
    fields_desc = [
        # Info
        LEShortField("completion_index", 0),
        ByteField("queue_id", 0),
        ByteField("err_code", 0),
        # From - P4
        LEShortField("flags", 0),
        LEShortField("vlan_tag", 0),
        LEIntField("checksum", 0),
        LEShortField("bytes_written", 0),
        # From - RSS table
        ByteField("rss_type", 0),
        LEIntField("rss_hash", 0),
        # END
        BitField("rsvd0", 0, 103),
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
    ]


class AdminCqDescriptor(Packet):
    fields_desc = [
        LEShortField("cmd_status", 0),
        LEShortField("cpl_id", 0),
        BitField("rsvd", 0, 7),
        BitField("color", 0, 1),
        LEIntField("cmd_data0", 0),
        LEIntField("cmd_data1", 0),
        LEIntField("cmd_data2", 0),
        LEIntField("cmd_data3", 0),
        LEIntField("cmd_data4", 0),
        LEIntField("cmd_data5", 0),
        BitField("rsvd0", 0, 24),
    ]


class EthDescriptorObject(base.FactoryObjectBase):

    __data_class__ = None

    def __init__(self):
        super().__init__()
        assert(issubclass(self.__data_class__, Packet))
        self.logger = cfglogger
        self.size = len(self.__data_class__())
        if (self.size & (self.size - 1)) != 0:    # Size must be power of 2
            raise ValueError('Size must be a power of 2.'
                             'Size of type %s is %d' % (self.__data_class__.__name__,
                                                        self.size))
        self.fields = None  # Set when Init method is called
        self._mem = None    # Set when self.Bind method is called
        self._data = None   # Set when self.Read/self.Write method is called
        self._buf = None    # Set when Init method is called

    def Init(self, spec):
        super().Init(spec)

        # Make sure fields are specified
        assert(hasattr(spec, 'fields'))
        # Make sure none of the fields are None
        assert(not any(field is None for field in spec.fields.keys()))
        # Make sure all field names are valid
        field_names = [field.name for field in self.__data_class__.fields_desc]
        assert(all(name in field_names for name in spec.fields.keys()))

        self.fields = {k: getattr(spec.fields, k) for k in spec.fields.keys()}
        self._buf = getattr(spec, '_buf', None)

        self.logger.info("Init %s" % self)

    def Write(self):
        """
        Writes a Descriptor to Memory
        :return:
        """
        if self._mem is None: return

        self.logger.info("Writing %s" % self)
        self._data = self.__data_class__(**self.fields)
        resmgr.HostMemoryAllocator.write(self._mem, bytes(self._data))
        self.logger.ShowScapyObject(self._data)

    def Read(self):
        """
        Reads a Descriptor from Memory
        :return:
        """
        if self._mem is None: return

        self.logger.info("Reading %s" % self)
        ba = resmgr.HostMemoryAllocator.read(self._mem, self.size)
        self._data = self.__data_class__(ba)
        # Fill in the fields from data
        self.fields = {k.name: getattr(self._data[self.__data_class__], k.name)
                       for k in self._data.fields_desc}
        self.logger.ShowScapyObject(self._data)

    def __str__(self):
        return "%s GID:%s/Id:0x%x/Memory:%s" % (
                self.__class__.__name__, self.GID(), id(self), self._mem)

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        # Get the set of common fields between the two objects and compare.
        # All other fields are treated as don't care.
        this_fields = set(self.fields.keys())
        other_fields = set(other.fields.keys())
        compare_fields = this_fields.intersection(other_fields)
        ignored_fields = this_fields.symmetric_difference(other_fields)
        msg = []
        for x in compare_fields:
            msg.append('%s: %s %s %s' % (x, self.fields[x],
                                         '==' if self.fields[x] == other.fields[x] else '!=',
                                         other.fields[x]))
        msg.append('ignored: %s' % ','.join(ignored_fields))
        self.logger.info('/'.join(msg))
        return all(self.fields[x] == other.fields[x] for x in compare_fields)

    def __copy__(self):
        obj = super().__copy__()
        obj.GID('ACTUAL_' + obj.GID())
        obj.fields = {}     # Set when self.Read method is called
        return obj

    def GetBuffer(self):
        return self._buf


class EthRxDescriptorObject(EthDescriptorObject):
    __data_class__ = EthRxDescriptor

    def Read(self):
        # This is a write-only descriptor type
        return


class EthRxCqDescriptorObject(EthDescriptorObject):
    __data_class__ = EthRxCqDescriptor

    def Write(self):
        # This is a read-only descriptor type
        return


class EthTxDescriptorObject(EthDescriptorObject):
    __data_class__ = EthTxDescriptor

    def Read(self):
        # This is a write-only descriptor type
        return


class EthTxCqDescriptorObject(EthDescriptorObject):
    __data_class__ = EthTxCqDescriptor

    def Write(self):
        # This is a read-only descriptor type
        return


class AdminDescriptorObject(EthDescriptorObject):
    __data_class__ = AdminDesciptor

    def Read(self):
        # This is a write-only descriptor type
        return


class AdminCqDescriptorObject(EthDescriptorObject):
    __data_class__ = AdminCqDescriptor

    def Write(self):
        # This is a read-only descriptor type
        return
