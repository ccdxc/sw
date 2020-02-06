#! /usr/bin/python3

import os
import pdb
import copy
import binascii

import infra.common.defs            as defs
import infra.common.utils           as utils
import infra.common.objects         as objects
import infra.factory.template       as template
import infra.factory.scapyfactory   as scapyfactory

from infra.factory.store            import FactoryStore
from infra.common.logging           import logger
from infra.penscapy                 import penscapy

scapy_hdr_map = {}

def IsVariableSizedField(field):
    if isinstance(field, objects.PatternFixed) or\
       isinstance(field, objects.PatternIncrement) or\
       isinstance(field, objects.PatternDecrement) or\
       isinstance(field, objects.PatternRandom):
        return True
    return False

def get_factory_id_from_scapy_id(scapy_id):
    return scapy_hdr_map.get(scapy_id)

def init():
    logger.info("Loading HEADER templates.")
    objlist = template.ParseHeaderTemplates()
    FactoryStore.headers.SetAll(objlist)
    global scapy_hdr_map
    for hdr in objlist:
        scapy_hdr_map[hdr.meta.scapy] = hdr.meta.id

    logger.info("Loading PACKET templates.")
    objlist = template.ParsePacketTemplates()
    FactoryStore.packets.SetAll(objlist)

    logger.info("Loading PAYLOAD templates.")
    objlist = template.ParsePayloadTemplates()
    FactoryStore.payloads.SetAll(objlist)
    return


class PacketHeaderFields(objects.FrameworkObject):
    def __init__(self, inst = None):
        super().__init__()
        if inst:
            self.__instantiate(inst)
        return

    def __instantiate(self, fields):
        for key,data in fields.__dict__.items():
            self.__dict__[key] = self.__get_field_value(data)
        return

    def __get_field_value(self, data):
        if not objects.IsFrameworkFieldObject(data):
            return data

        if IsVariableSizedField(data):
            return data

        if objects.IsAutoField(data):
            return None

        if objects.IsReference(data):
            # This will be resolved in 2nd phase.
            return data

        if objects.IsCallback(data):
            # This will be resolved in 2nd phase.
            return data

        return data.get()

    def __build_list(self, data, testcase, packet):
        for e in data:
            if objects.IsFrameworkObject(e):
                self.__build(e, testcase, packet)
        return

    def __build(self, obj, testcase, packet):
        for key,data in obj.__dict__.items():
            if objects.IsReference(data):
                if data.GetRootID() == defs.ref_roots.TESTCASE:
                    obj.__dict__[key] = data.Get(testcase)
                elif data.GetRootID() == defs.ref_roots.FACTORY:
                    obj.__dict__[key] = data.Get(FactoryStore)
            elif objects.IsCallback(data):
                retval = data.call(testcase, packet)
                obj.__dict__[key] = obj.__get_field_value(retval)
            elif isinstance(data, list):
                self.__build_list(data, testcase, packet)
        return

    def Build(self, testcase, packet):
        self.__build(self, testcase, packet)
        return

class PacketHeader(objects.FrameworkObject):
    def __init__(self, inst):
        super().__init__()
        self.fields = PacketHeaderFields()
        if inst:
            self.__instantiate(inst)
        self.start  = 0
        self.size   = 0
        return

    def __instantiate(self, header):
        for key,data in header.__dict__.items():
            self.__dict__[key] = data
        self.fields = PacketHeaderFields(header.fields)
        return

    def Build(self, testcase, packet):
        self.fields.Build(testcase, packet)
        return

def IsPacketHeader(obj):
    return isinstance(obj, PacketHeader)

class PacketHeaders(objects.FrameworkObject):
    def __init__(self, inst):
        super().__init__()
        if inst:
            self.__instantiate(inst)
        return

    def __instantiate(self, headers):
        for key,data in headers.__dict__.items():
            if objects.IsFrameworkObject(data):
                hdr = PacketHeader(data)
                self.__dict__[key] = hdr
            else:
                self.__dict__[key] = data
        return

class PacketSpec(objects.FrameworkObject):
    def __init__(self, spec):
        super().__init__()
        self.Clone(FactoryStore.testobjects.Get('PACKET'))
        # Convert the Testspec format to Header Template format
        # i.e. add the 'fields' level
        self.paddingsize    = spec.paddingsize
        self.payloadsize    = spec.payloadsize
        self.template       = spec.template
        self.encaps         = spec.encaps
        self.id             = spec.id
        self.clone          = spec.clone
        self.headers        = PacketHeaders(None)
        self.pendol         = getattr(spec, 'pendol', False)
        self.icrc           = getattr(spec, 'icrc', False)
        self.pcap           = getattr(spec, 'pcap', None)
        self.ConvertHeaders(spec)
        return

    def ConvertHeader(self, spec):
        hdr = PacketHeader(None)
        hdr.fields.Clone(spec)
        return hdr

    def ConvertHeaders(self, spec):
        if spec.headers == None: return
        for key,data in spec.headers.__dict__.items():
            if objects.IsFrameworkObject(data):
                hdr = self.ConvertHeader(data)
            else:
                hdr = data
            self.headers.__dict__[key] = hdr
        return

class Packet(objects.FrameworkObject):
    def __init__(self, tc, testspec_packet):
        super().__init__()
        self.Clone(FactoryStore.testobjects.Get('PACKET'))
        #self.tc = tc
        self.rawbytes = None
        self.pendol = tc.IsPendolHeaderEnabled()
        self.icrc   = False
        self.inherit_icrc   = False
        self.basepkt = None
        self.tcid = 0
        self.step_id = 0
        self.LockAttributes()

        pktspec = PacketSpec(testspec_packet)
        self.pktspec = pktspec
        self.icrc = pktspec.icrc

        self.__get_packet_base(tc, pktspec)
        self.encaps = []
        self.__get_packet_encaps(tc, pktspec)

        self.spec = pktspec
        self.__get_payload_size(tc)

        self.GID(self.spec.id)
        self.__process(tc)

        self.spktobj    = None
        self.rawbytes   = None
        self.size       = None
        self.pendol     = pktspec.pendol or self.pendol
        return

    def GetPcap(self):
        return self.pktspec.pcap

    def IsDolHeaderRequired(self):
        return self.pendol

    def IsCrcHeaderRequired(self):
        return True

    def __get_packet_base(self, tc, pktspec):
        if pktspec.clone:
            basepkt = pktspec.clone.Get(tc)
            self.Clone(basepkt)

            if pktspec.paddingsize != 0:
                self.pktspec.paddingsize = pktspec.paddingsize
            if objects.IsCallback(self.pktspec.paddingsize):
                self.pktspec.paddingsize = self.pktspec.paddingsize.call(tc, self)
            if objects.IsReference(self.pktspec.paddingsize):
                self.pktspec.paddingsize = self.pktspec.paddingsize.Get(tc)
            self.basepkt = basepkt

            # Special handling for ICRC inheritance.
            # ICRC is not carried in the hdrsorder metadata,
            # hence it must be explicitly inherited. Current 'icrc'
            # knob is used to calculate new icrc for this packet.
            # Hence adding a new knob for inherit case.
            self.inherit_icrc = False
            if self.basepkt.icrc is True and pktspec.icrc is False:
                self.inherit_icrc = True
            self.icrc = pktspec.icrc
            return

        if objects.IsReference(self.pktspec.paddingsize):
            self.pktspec.paddingsize = self.pktspec.paddingsize.Get(tc)
        if objects.IsCallback(self.pktspec.paddingsize):
            self.pktspec.paddingsize = self.pktspec.paddingsize.call(tc, self)

        if pktspec.template == None:
            logger.error("No Template or Clone specified for packet.")
            return None

        if objects.IsReference(pktspec.template):
            self.template = pktspec.template.Get(FactoryStore)
        elif objects.IsCallback(pktspec.template):
            self.template = pktspec.template.call(tc, self)
        else:
            assert(0)
        if self.template == None:
            logger.error("Template NOT FOUND for ID:%s" %\
                            self.spec.template.GetInstID())
            assert(self.template != None)
        return 

    def __get_packet_encaps(self, tc, pktspec):
        if pktspec.encaps == None:
            return

        if objects.IsCallback(pktspec.encaps):
            self.encaps = pktspec.encaps.call(tc, self)
            return

        for encspec in pktspec.encaps:
            encap = encspec.Get(FactoryStore)
            self.encaps.append(encap)
        return

    def SetStepId(self, step_id):
        #self.headers.pendol.step_id = step_id
        return

    def GetPayloadSize(self):
        return self.payloadsize
    def GetPaddingSize(self):
        return self.pktspec.paddingsize

    def __get_payload_size(self, tc):
        if self.spec.payloadsize == None:
            return
        size = 0
        spec_size = self.spec.payloadsize
        if objects.IsReference(spec_size):
            if spec_size.GetRootID() == defs.ref_roots.FACTORY:
                size = spec_size.Get(FactoryStore)
            elif spec_size.GetRootID() == defs.ref_roots.TESTCASE:
                size = spec_size.Get(tc)
            else:
                assert(0)
        elif objects.IsCallback(spec_size):
            size = spec_size.call(tc, self)
        elif objects.IsFrameworkFieldObject(spec_size):
            size = spec_size.get()
        else:
            size = spec_size

        self.payloadsize = size
        return

    def __len__(self):
        size = 0
        for h in self.hdrsorder:
            hdr = self.headers.__dict__[h]
            if IsPacketHeader(hdr):
                logger.verbose("Size of HEADER: %s = " %\
                                  h, hdr.meta.size)
                size += hdr.meta.size
        return size

    def __merge_encaps(self, tc):
        if self.encaps == None:
            return

        self.encaps.reverse()
        for enc in self.encaps:
            enc.headers = PacketHeaders(enc.headers)
            self.headers = objects.MergeObjects(self.headers,
                                                enc.headers)
            enc.meta.callback.call(self, enc)
        return

    def __process_headers(self, tc):
        return

    def __copy_header_meta(self, template_hdrs):
        for k,template_hdr in template_hdrs.__dict__.items():
            hdr = getattr(self.headers, k, None)
            if hdr is None: continue
            hdr.meta = copy.deepcopy(template_hdr.meta)
        return

    def __merge_headers(self, tc):
        spec_hdrs = PacketHeaders(self.spec.headers)
        template_hdrs = PacketHeaders(self.template.headers)
        if self.headers:
            self.headers = objects.MergeObjects(spec_hdrs, self.headers)
        else:
            self.headers = spec_hdrs

        if self.GetPcap():
            # For PCAP based packets, dont inherit the default values from templates.
            # Any field that is not specified in testspec should retain its value from 
            # the PCAP file.
            self.__copy_header_meta(template_hdrs)
        else:
            self.headers = objects.MergeObjects(self.headers, template_hdrs)
        self.hdrsorder = copy.deepcopy(self.template.hdrsorder)
        self.headers.show()
        return

    def __fixup_payloadsize(self, tc):
        datasize = len(self.headers.payload.fields.data)
        if datasize != self.payloadsize:
            logger.warn("- Testspec payload data size:%d payloadsize:%d" %\
                    (datasize, self.payloadsize))
            logger.warn("- Fixing payloadsize to data size: %d" % datasize)
            self.payloadsize = datasize
        return

    def __fixup_payload(self, tc):
        currdata = self.headers.payload.fields.data
        datasize = len(self.headers.payload.fields.data)
        if datasize != self.payloadsize:
            logger.warn("- Testspec payload data size:%d payloadsize:%d" %\
                (len(self.headers.payload.fields.data), self.payloadsize))
            logger.warn("- Fixing payload data.")
            fac = int(self.payloadsize / datasize) + 1
            newdata = currdata * fac
            self.headers.payload.fields.data = newdata[:self.payloadsize]
        return

    def __add_payload(self, tc):
        if getattr(self.headers, 'payload', None) is None:
            self.payloadsize = 0
            return

        if self.GetPcap():
            self.payloadsize = 0
            return
        self.headers.payload.meta.size = self.payloadsize
        if self.headers.payload.fields.data is None:
            self.headers.payload.fields.data =\
                         self.headers.payload.meta.pattern.get(self.payloadsize)
        elif objects.IsCallback(self.headers.payload.fields.data):
            cb = self.headers.payload.fields.data
            self.headers.payload.fields.data = cb.call(tc,self)
            self.__fixup_payload(tc)
        elif objects.IsReference(self.headers.payload.fields.data):
            ref = self.headers.payload.fields.data
            if ref.GetRootID() == defs.ref_roots.FACTORY:
                data = ref.Get(FactoryStore)
            elif ref.GetRootID() == defs.ref_roots.TESTCASE:
                data = ref.Get(tc)
            else:
                assert(0)
            self.headers.payload.fields.data = list(data)
            self.__fixup_payload(tc)
        elif isinstance(self.headers.payload.fields.data, str):
            self.headers.payload.fields.data =\
                    utils.ParseIntegerList(self.headers.payload.fields.data)
        elif objects.IsFrameworkFieldObject(self.headers.payload.fields.data):
            self.headers.payload.fields.data =\
                    self.headers.payload.fields.data.get(self.payloadsize)
            self.__fixup_payload(tc)
        else:
            self.__fixup_payload(tc)
        logger.info("- Added Payload of Size = %d" % self.payloadsize)
        return

    def __process(self, tc):
        self.__process_headers(tc)
        self.__merge_headers(tc)
        self.__merge_encaps(tc)
        self.__add_payload(tc)
        return

    def __resolve(self, tc):
        if self.hdrsorder is None:
            return
        for h in self.hdrsorder:
            if h not in self.headers.__dict__: continue
            hdr = self.headers.__dict__[h]
            if not IsPacketHeader(hdr): continue
            hdr.Build(tc, self)
        return

    def IsIcrcEnabled(self):
        return self.icrc or self.inherit_icrc
    def IsNewIcrcRequired(self):
        return self.icrc
    def IsInheritIcrcRequired(self):
        return self.inherit_icrc
    def GetBasePacketIcrc(self):
        return self.basepkt.GetIcrc()
    def GetIcrc(self):
        return self.spktobj.GetIcrc()

    def GetTcId(self):
        return self.tcid
    def GetStepId(self):
        return self.step_id

    def GetScapyPacket(self):
        return self.spktobj.GetScapyPacket()

    def Build(self, tc):
        if self.spktobj is not None:
            return
        self.__resolve(tc)
        self.tcid = tc.GetTcId()
        self.step_id = tc.GetStepId()
        self.spktobj = scapyfactory.ScapyPacketObject()
        self.spktobj.Build(packet = self)

        self.rawbytes = self.spktobj.GetRawBytes()
        self.size = self.spktobj.GetSize()
        return

    def Show(self):
        logger.info("################## %-16s ##################" %
                    self.GID())
        self.spktobj.Show()
        return
