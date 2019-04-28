#! /usr/bin/python3
import pdb
from pprint import pformat
from scapy.all import *
import iris.config.resmgr as resmgr
from infra.common.logging import logger
from ctypes import *

import infra.factory.base as base


IONIC_RX_MAX_SG_ELEMS = 8
IONIC_TX_MAX_SG_ELEMS = 8


def to_dict(obj):
    if isinstance(obj, Array):
        return [to_dict(x) for x in obj]
    elif isinstance(obj, Structure) or isinstance(obj, Union):
        return {x[0]: to_dict(getattr(obj, x[0])) for x in obj._fields_}
    elif isinstance(obj, int):
        return "0x%x" % obj
    else:
        return obj


def ctypes_pformat(cstruct):
    return cstruct.__class__.__name__ + '\n' + pformat(to_dict(cstruct)) 


class EthRxDescriptor(LittleEndianStructure):
    _fields_ = [
        ("opcode", c_uint8),
        ("rsvd", c_uint8 * 5),
        ("len", c_uint16),
        ("addr", c_uint64),
    ]


class EthRxSgElement(LittleEndianStructure):
    _fields_ = [
        ("addr", c_uint64),
        ("len", c_uint16),
        ("rsvd", c_uint16 * 3),
    ]


class EthRxSgDescriptor(LittleEndianStructure):
    _fields_ = [
        ("elems", EthRxSgElement * IONIC_RX_MAX_SG_ELEMS),
    ]


class EthRxCqDescriptor(LittleEndianStructure):
    _fields_ = [
        ("status", c_uint8),
        ("num_sg_elems", c_uint8),
        ("comp_index", c_uint16),
        ("rss_hash", c_uint32),
        ("csum", c_uint16),
        ("vlan_tci", c_uint16),
        ("len", c_uint16),
        ("csum_tcp_ok", c_uint8, 1),
        ("csum_tcp_bad", c_uint8, 1),
        ("csum_udp_ok", c_uint8, 1),
        ("csum_udp_bad", c_uint8, 1),
        ("csum_ip_ok", c_uint8, 1),
        ("csum_ip_bad", c_uint8, 1),
        ("vlan_strip", c_uint8, 1),
        ("csum_calc", c_uint8, 1),
        ("pkt_type", c_uint8, 7),
        ("color", c_uint8, 1),
    ]


class EthTxDescriptor(LittleEndianStructure):
    _fields_ = [
        ("vlan_insert", c_uint64, 1),
        ("encap", c_uint64, 1),
        ("csum_l3_or_sot", c_uint64, 1),
        ("csum_l4_or_eot", c_uint64, 1),
        ("opcode", c_uint64, 4),
        ("num_sg_elems", c_uint64, 4),
        ("addr", c_uint64, 52),
        ("len", c_uint16),
        ("vlan_tci", c_uint16),
        ("csum_start_or_hdr_len", c_uint16),
        ("csum_offset_or_mss", c_uint16),
    ]


class EthTxSgElement(LittleEndianStructure):
    _fields_ = [
        ("addr", c_uint64),
        ("len", c_uint16),
        ("rsvd", c_uint16 * 3),
    ]


class EthTxSgDescriptor(LittleEndianStructure):
    _fields_ = [
        ("elems", EthTxSgElement * IONIC_TX_MAX_SG_ELEMS),
    ]


class EthTxCqDescriptor(LittleEndianStructure):
    _fields_ = [
        ("status", c_uint8),
        ("rsvd", c_uint8),
        ("comp_index", c_uint16),
        ("rsvd2", c_uint8 * 11),
        ("rsvd3", c_uint8, 7),
        ("color", c_uint8, 1),
    ]


class AdminDescriptor(LittleEndianStructure):
    _fields_ = [
        ("opcode", c_uint16),
        ("lif_index", c_uint16),
        ("cmd_data", c_uint8 * 60),
    ]


class AdminCqDescriptor(LittleEndianStructure):
    _fields_ = [
        ("status", c_uint8),
        ("rsvd", c_uint8),
        ("comp_index", c_uint16),
        ("cmd_data", c_uint8 * 11),
        ("rsvd2", c_uint8, 7),
        ("color", c_uint8, 1),
    ]


class EthDescriptorObject(base.FactoryObjectBase):

    __data_class__ = None

    def __init__(self):
        super().__init__()
        self.size = sizeof(self.__data_class__)
        if (self.size & (self.size - 1)) != 0:    # Size must be power of 2
            raise ValueError('Size must be a power of 2.'
                             'Size of type %s is %d' % (self.__data_class__.__name__,
                                                        self.size))
        self.fields = None  # Set when Init method is called
        self._mem = None    # Set when self.Bind method is called
        self._data = None   # Set when self.Read/self.Write method is called
        self._buf = None    # Set when Init method is called
        self._more = None   # Set when Init method is called

    def Init(self, spec):
        super().Init(spec)

        # Make sure fields are specified
        assert(hasattr(spec, 'fields'))
        if spec.fields is None:
            logger.info("Uninitialized Descriptor %s" % self)
            return

        # Make sure none of the fields are None
        assert(not any(field is None for field in spec.fields.keys()))
        # Make sure all field names are valid
        #field_names = [field.name for field in self.__data_class__.fields_desc]
        #assert(all(name in field_names for name in spec.fields.keys()))

        self.fields = {k: getattr(spec.fields, k) for k in spec.fields.keys()}
        self._buf = getattr(spec.fields, '_buf', None)
        self._more = getattr(spec.fields, '_more', None)

        logger.info("Init %s" % self)

    def Write(self):
        """
        Writes a Descriptor to Memory
        :return:
        """
        if self._mem is None: return

        logger.info("Writing %s" % self)
        self._data = self.__data_class__(**self.fields)
        resmgr.HostMemoryAllocator.write(self._mem, bytes(self._data))
        logger.info(ctypes_pformat(self._data))

    def Read(self):
        """
        Reads a Descriptor from Memory
        :return:
        """
        if self._mem is None: return

        logger.info("Reading %s" % self)
        ba = resmgr.HostMemoryAllocator.read(self._mem, self.size)
        self._data = self.__data_class__.from_buffer_copy(ba)
        # Fill in the fields from data
        self.fields = {k[0]: getattr(self._data, k[0])
                       for k in self.__data_class__._fields_}
        logger.info(ctypes_pformat(self._data))

    def __str__(self):
        return "%s GID:%s/Memory:%s" % (
                self.__class__.__name__, self.GID(), self._mem)

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
        logger.info('/'.join(msg))
        return all(self.fields[x] == other.fields[x] for x in compare_fields)

    def __copy__(self):
        obj = super().__copy__()
        obj.GID('ACTUAL_' + obj.GID())
        obj.fields = {}     # Set when self.Read method is called
        if self._buf:
            obj._buf = copy.copy(obj._buf)
        return obj

    def GetBuffer(self):
        return self._buf

    def GetCompletionIndex(self):
        return None


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

    def GetCompletionIndex(self):
        return self._data.comp_index

    def GetColor(self):
        return self._data.color


class EthTxDescriptorObject(EthDescriptorObject):
    __data_class__ = EthTxDescriptor

    def Read(self):
        # This is a write-only descriptor type
        return

    def GetTxPktByteStats(self):
        flds = getattr(self.spec, 'fields', None)
        if flds is None:
            return (None, None)
        nbytes = getattr(flds, 'len', None)
        if nbytes is None:
            return (None, None)
        return (1, nbytes)

    def GetTxPacket(self):
        flds = getattr(self.spec, 'fields', None)
        if flds is None:
            return None
        buf = getattr(flds, '_buf', None)
        if buf is not None:
            return buf.spec.fields.data
        return None

class EthTxSgDescriptorObject(EthDescriptorObject):
    __data_class__ = EthTxSgDescriptor

    def Read(self):
        # This is a write-only descriptor type
        return


class EthTxCqDescriptorObject(EthDescriptorObject):
    __data_class__ = EthTxCqDescriptor

    def Write(self):
        # This is a read-only descriptor type
        return

    def GetCompletionIndex(self):
        return self._data.comp_index

    def GetColor(self):
        return self._data.color


class AdminDescriptorObject(EthDescriptorObject):
    __data_class__ = AdminDescriptor

    def Read(self):
        # This is a write-only descriptor type
        return


class AdminCqDescriptorObject(EthDescriptorObject):
    __data_class__ = AdminCqDescriptor

    def Write(self):
        # This is a read-only descriptor type
        return

    def GetCompletionIndex(self):
        return self._data[self.__data_class__].completion_index

    def GetColor(self):
        return self._data[self.__data_class__].color
