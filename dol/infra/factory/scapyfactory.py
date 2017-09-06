#! /usr/bin/python3
import pdb
import sys

import infra.penscapy.penscapy          as penscapy
import infra.common.objects             as objects
import infra.common.defs                as defs
import infra.common.utils               as utils

from infra.common.logging import logger as logger

class ScapyHeaderBuilder_BASE:
    def build(self, hdr):
        if hasattr(penscapy, hdr.meta.scapy) == False:
            logger.error("SCAPY has no header = %s" %
                         hdr.meta.scapy)
            assert(0)
        logger.debug("Adding Header: %s" % hdr.meta.scapy)
        scapyhdl = getattr(penscapy, hdr.meta.scapy)
        shdr = scapyhdl()
        
        for key, value in hdr.fields.__dict__.items():
            if objects.IsFrameworkObjectInternalAttr(key): continue
            logger.verbose("  - %-10s =" % key, value)
            try:
                shdr.__setattr__(key, value)
            except:
                logger.error("ScapyHeaderBuilder: Failed to set %s.%s to" %\
                             (hdr.meta.id, key), value)
                assert(0)

        return shdr
BASE_builder = ScapyHeaderBuilder_BASE()

class ScapyHeaderBuilder_ICMPV6(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        if hdr.fields.type in penscapy.icmp6typescls:
            hdr.meta.scapy = penscapy.icmp6typescls[hdr.fields.type]
        else:
            hdr.meta.scapy = 'ICMPv6Unknown'
        return super().build(hdr)
ICMPV6_builder = ScapyHeaderBuilder_ICMPV6()

class ScapyHeaderBuilder_TCP(ScapyHeaderBuilder_BASE):
    flags_map = [
        ('fin', 'F'),
        ('syn', 'S'),
        ('rst', 'R'),
        ('psh', 'P'),
        ('ack', 'A'),
        ('urg', 'U'),
        ('ece', 'E'),
        ('cwr', 'C')
    ]
    def __translate_options(self, hdr):
        scapy_options = []
        for opt in hdr.fields.options:
            scapy_opt = None
            if opt.kind == None: continue
            if opt.data == None:
                scapy_opt = (opt.kind, None)
            elif opt.kind == 'SAckOK':
                scapy_opt = (opt.kind, '')
            else:
                opt.data = utils.ParseIntegerList(opt.data)
                scapy_data_tup = None
                for df in opt.data:
                    if scapy_data_tup:
                        scapy_data_tup += (df,)
                    else:
                        scapy_data_tup = (df,)
                scapy_opt = (opt.kind, scapy_data_tup)
            scapy_options.append(scapy_opt)
        hdr.fields.options = scapy_options
        return
    def __translate_flags(self, hdr):
        if hdr.fields.flags == None:
            return
        hdr.fields.flags = hdr.fields.flags.lower()
        scapy_tcp_flags = ''
        for tup in self.flags_map:
            if tup[0] in hdr.fields.flags:
                scapy_tcp_flags += tup[1]
        hdr.fields.flags = scapy_tcp_flags
        return 
    def build(self, hdr):
        self.__translate_options(hdr)
        self.__translate_flags(hdr)
        return super().build(hdr)
TCP_builder = ScapyHeaderBuilder_TCP()

class ScapyHeaderBuilder_PENDOL(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        hdr.fields.ts = 0xFF2233FF
        hdr.fields.id = 0x0001CCCC0002DDDD
        return super().build(hdr)
PENDOL_builder = ScapyHeaderBuilder_PENDOL()

class ScapyHeaderBuilder_PAYLOAD(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        logger.verbose("Updating PAYLOAD of size: %d" % hdr.meta.size)
        return super().build(hdr)
PAYLOAD_builder = ScapyHeaderBuilder_PAYLOAD()

class ScapyHeaderBuilder_CRC(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        # CRC is being used by mock model to test. 
        #hdr.fields.crc = 0xBEEF
        return super().build(hdr)
CRC_builder = ScapyHeaderBuilder_CRC()

class ScapyHeaderBuilder_PADDING(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        return super().build(hdr)
PADDING_builder = ScapyHeaderBuilder_PADDING()

class ScapyHeaderBuilder_IPV4(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        hdr.fields.options = []
        return super().build(hdr)
IPV4_builder = ScapyHeaderBuilder_IPV4()

class ScapyPacket:
    def __init__(self, packet):
        self.spkt = None
        self.packet = packet
        self.__process()
        self.spkt.show_indent = 0
        return

    def __build_header(self, hdr):
        builder_name = '%s_builder' % hdr.meta.id
        if hasattr(sys.modules[__name__], builder_name):
            logger.verbose("Using builder %s" % builder_name)
            builder = getattr(sys.modules[__name__], builder_name)
            return builder.build(hdr)
        return BASE_builder.build(hdr)

    def __add_header(self, hdr):
        shdr = self.__build_header(hdr)
        if self.spkt != None:
            self.spkt = self.spkt / shdr
        else:
            self.spkt = shdr
        return

    def __process(self):
        for hdrid in self.packet.headers_order:
            hdrdata = self.packet.headers.__dict__[hdrid]
            if objects.IsFrameworkObject(hdrdata):
                self.__add_header(hdrdata)
        return

def main(packet):
    logger.debug("Generating SCAPY Packet.")
    scapy_packet = ScapyPacket(packet)
    #scapy_packet.spkt.build()
    #scapy_packet.spkt.show(label_lvl = "SCAPY :")
    #penscapy.PrintPacket(scapy_packet.spkt)
    #penscapy.hexdump(scapy_packet.spkt)
    #penscapy.linehexdump(scapy_packet.spkt, onlyhex = 1)
    return scapy_packet.spkt
