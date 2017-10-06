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
from infra.common.logging           import pktlogger
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
    pktlogger.info("Loading HEADER templates.")
    objlist = template.ParseHeaderTemplates()
    FactoryStore.headers.SetAll(objlist)
    global scapy_hdr_map
    for hdr in objlist:
        scapy_hdr_map[hdr.meta.scapy] = hdr.meta.id

    pktlogger.info("Loading PACKET templates.")
    objlist = template.ParsePacketTemplates()
    FactoryStore.packets.SetAll(objlist)

    pktlogger.info("Loading PAYLOAD templates.")
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

    def Build(self, testcase, packet):
        for key,data in self.__dict__.items():
            if objects.IsReference(data):
                if data.GetRootID() == defs.ref_roots.TESTCASE:
                    self.__dict__[key] = data.Get(testcase)
                elif data.GetRootID() == defs.ref_roots.FACTORY:
                    self.__dict__[key] = data.Get(FactoryStore)
            elif objects.IsCallback(data):
                retval = data.call(testcase, packet)
                self.__dict__[key] = self.__get_field_value(retval)
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
        self.payloadsize    = spec.payloadsize
        self.template       = spec.template
        self.encaps         = spec.encaps
        self.id             = spec.id
        self.clone          = spec.clone
        self.headers        = PacketHeaders(None)
        self.pendol         = getattr(spec, 'pendol', False)
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
        self.pendol = None
        self.LockAttributes()

        pktspec = PacketSpec(testspec_packet)
        self.pktspec = pktspec
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
        self.pendol     = pktspec.pendol
        return

    def IsDolHeaderRequired(self):
        return self.pendol

    def IsCrcHeaderRequired(self):
        return True

    def __get_packet_base(self, tc, pktspec):
        if pktspec.clone:
            basepkt = pktspec.clone.Get(tc)
            self.Clone(basepkt)
            return

        if pktspec.template == None:
            pktlogger.error("No Template or Clone specified for packet.")
            return None

        if objects.IsReference(pktspec.template):
            self.template = pktspec.template.Get(FactoryStore)
        elif objects.IsCallback(pktspec.template):
            self.template = pktspec.template.call(tc, self)
        else:
            assert(0)
        if self.template == None:
            pktlogger.error("Template NOT FOUND for ID:%s" %\
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
        elif objects.IsCallback(spec_size):
            size = spec_size.call(tc, self)
        else:
            size = spec_size
        
        self.payloadsize = size
        return

    def __len__(self):
        size = 0
        for h in self.hdrsorder:
            hdr = self.headers.__dict__[h]
            if IsPacketHeader(hdr):
                pktlogger.verbose("Size of HEADER: %s = " %\
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

    def __merge_headers(self, tc):
        spec_hdrs = PacketHeaders(self.spec.headers)
        template_hdrs = PacketHeaders(self.template.headers)
        if self.headers:
            self.headers = objects.MergeObjects(spec_hdrs, self.headers)
        else:
            self.headers = spec_hdrs
        self.headers = objects.MergeObjects(self.headers, template_hdrs)
        self.hdrsorder = copy.deepcopy(self.template.hdrsorder)
        self.headers.show()
        return

    def __fixup_payloadsize(self, tc):
        datasize = len(self.headers.payload.fields.data)
        if datasize != self.payloadsize:
            tc.warn("- Testspec payload data size:%d payloadsize:%d" %\
                    (datasize, self.payloadsize))
            tc.warn("- Fixing payloadsize to data size: %d" % datasize)
            self.payloadsize = datasize
        return

    def __fixup_payload(self, tc):
        currdata = self.headers.payload.fields.data
        datasize = len(self.headers.payload.fields.data)
        if datasize != self.payloadsize:
            tc.warn("- Testspec payload data size:%d payloadsize:%d" %\
                (len(self.headers.payload.fields.data), self.payloadsize))
            tc.warn("- Fixing payload data.")
            fac = int(self.payloadsize / datasize) + 1
            newdata = currdata * fac
            self.headers.payload.fields.data = newdata[:self.payloadsize]
        return

    def __add_payload(self, tc):
        tc.info("- Adding Payload of Size = %d" % self.payloadsize)
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
        else:
            self.__fixup_payload(tc)
        return

    def __process(self, tc):
        self.__process_headers(tc)
        self.__merge_headers(tc)
        self.__merge_encaps(tc)
        self.__add_payload(tc)
        return

    def __resolve(self, tc):
        for h in self.hdrsorder:
            hdr = self.headers.__dict__[h]
            if not IsPacketHeader(hdr): continue
            hdr.Build(tc, self)
        return

    def Build(self, tc):
        if self.spktobj is not None:
            return
        self.__resolve(tc)
        self.spktobj = scapyfactory.ScapyPacketObject()
        self.spktobj.Build(packet = self)
        self.rawbytes = self.spktobj.GetRawBytes()
        self.size = self.spktobj.GetSize()
        return

    def Show(self, logger):
        logger.info("################## %-16s ##################" %
                    self.GID())
        self.spktobj.Show(logger)
        return
