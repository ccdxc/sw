#! /usr/bin/python3
import pdb
import sys
import binascii

import infra.penscapy.penscapy  as penscapy
import infra.common.objects     as objects
import infra.common.defs        as defs
import infra.common.utils       as utils

from infra.factory.store        import FactoryStore as FactoryStore
from infra.common.logging       import logger as logger
from infra.common.glopts        import GlobalOptions as GlobalOptions

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

def TranslateTcpUdpOptions(builder, hdr):
    scapy_options = []
    for opt in hdr.fields.options:
        scapy_opt = None
        if isinstance(opt, tuple):
            scapy_options.append(opt)
            continue
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
        TranslateTcpUdpOptions(self, hdr) 
        return

    def __is_native_format(self, flags):
        flaglist = flags.split(',')
        for tup in self.flags_map:
            if tup[0] in flaglist:
                return True
        return False

    def __translate_flags(self, hdr):
        if hdr.fields.flags == None:
            return
        if self.__is_native_format(hdr.fields.flags):
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


class ScapyHeaderBuilder_ICRC(ScapyHeaderBuilder_BASE):

    def build(self, hdr):
        logger.info("Updating ICRC header of size: %d" % hdr.meta.size)
        return super().build(hdr)

ICRC_builder = ScapyHeaderBuilder_ICRC()

class ScapyHeaderBuilder_SUNRPC_4_PORTMAP_DUMP_REPLY(ScapyHeaderBuilder_BASE):
    def __translate_data(self, hdr):
        scapy_data = None 
        for dat in hdr.fields.data:
            scapy_hdr = penscapy.SUNRPC_4_PORTMAP_DUMP_REPLY_DATA(pgm=dat.pgm, 
                            netid=dat.netid, addr=dat.addr, 
                            owner=dat.owner, ValFollows=dat.ValFollows)
            scapy_hdr.netid_len = len(scapy_hdr.netid)
            l = len(scapy_hdr.netid)%4
            if l:
               opq = ''
               for i in range(0, (4-l)):
                   opq = opq + '\x00'
               scapy_hdr.opaque_data1 = opq
            scapy_hdr.addr_len = len(scapy_hdr.addr)
            l = len(scapy_hdr.addr)%4
            if l:
               opq = ''
               for i in range(0, (4-l)):
                   opq = opq + '\x00'
               scapy_hdr.opaque_data2 = opq
            scapy_hdr.owner_len = len(scapy_hdr.owner)
            l = len(scapy_hdr.owner)%4
            if l:
               opq = ''
               for i in range(0, (4-l)):
                   opq = opq + '\x00'
               scapy_hdr.opaque_data3 = opq
            if scapy_data is None:
                scapy_data = scapy_hdr
            else:
                scapy_data = scapy_data / scapy_hdr
        hdr.fields.data = scapy_data 
        return

    def build(self, hdr):
        if hdr.fields.ValFollows == 1:
            self.__translate_data(hdr)
        return super().build(hdr)

SUNRPC_4_PORTMAP_DUMP_REPLY_builder = ScapyHeaderBuilder_SUNRPC_4_PORTMAP_DUMP_REPLY()

class ScapyHeaderBuilder_UDP_OPTIONS(ScapyHeaderBuilder_BASE):
    def __translate_options(self, hdr):
        TranslateTcpUdpOptions(self, hdr)
        return
    def build(self, hdr):
        self.__translate_options(hdr)
        return super().build(hdr)
UDP_OPTIONS_builder = ScapyHeaderBuilder_UDP_OPTIONS()


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
        if hdr.fields.options is None:
            hdr.fields.options = []
        return super().build(hdr)
IPV4_builder = ScapyHeaderBuilder_IPV4()

class ScapyHeaderBuilder_IPV6(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        if hdr.fields.extns is None:
            hdr.fields.extns = []
        basev6 = super().build(hdr)
        
        v6hdr = basev6
        orig_nh = v6hdr.nh
        for ext in hdr.fields.extns:
            v6hdr = v6hdr / ext
            v6hdr.nh = ext.overload_fields[penscapy.IPv6]['nh']
            ext.nh = orig_nh
        return v6hdr
IPV6_builder = ScapyHeaderBuilder_IPV6()

class ScapyHeaderBuilder_BOOTP(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        options =  hdr.fields.options
        hdr.fields.options = None
        bootphdr = super().build(hdr)
        if options:
            bootphdr = penscapy.BOOTP(bytes(bootphdr) + penscapy.dhcpmagic)
            bootphdr = bootphdr / options
        return bootphdr

BOOTP_builder = ScapyHeaderBuilder_BOOTP()

class ScapyHeaderBuilder_TFTP(ScapyHeaderBuilder_BASE):
    opcode_map = [
           ('RRQ', 0x1),
           ('WRQ', 0x2),
           ('DATA', 0x3),
           ('ACK', 0x4),
           ('ERROR', 0x5),
           ('OACK', 0x6)]
 
    def __translate_opcode(self, hdr):
        if hdr.fields.op == None:
            return
         
        op = 0x0
        for tup in self.opcode_map:
             if tup[0] == hdr.fields.op:
                 op = tup[1]
        hdr.fields.op = op
        return
     
    def build(self, hdr):
        self.__translate_opcode(hdr)
        return super().build(hdr)
TFTP_builder = ScapyHeaderBuilder_TFTP()

class ScapyHeaderBuilder_IPFIX(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        records = []
        for r in hdr.fields.records:
            records.append(penscapy.IpfixRecord(r.record))
        hdr.fields.records = records
        return super().build(hdr)
IPFIX_builder = ScapyHeaderBuilder_IPFIX()

class IcrcHeaderBuilder:
    def __init__(self, spkt):
        self.spkt = spkt.copy()
        if penscapy.IPv6 in self.spkt:
            self.spkt = self.spkt[penscapy.IPv6]
        elif penscapy.IP in self.spkt:
            self.spkt = self.spkt[penscapy.IP]
        else:
            assert(0)
        return

    def __get_icrc(self):
        self.rawbytes = bytes(self.spkt)
        # ICRC calculation starts with 64 bits of 1's followed
        # packet (IP header to payload until before the ICRC and excluding UDP options, if any
        self.rawbytes = bytes([0xFF] * 8) + bytes(self.spkt)
        #Skip optional variable UDP_OPTIONS hdr length and ICRC hdr 4B when calculating ICRC
        #starting from ICRC header would do the trick
        #ScapyPacketObject.ShowRawPacket(self.rawbytes[:-4], logger)
        skip_bytes = len(bytes(self.spkt[penscapy.ICRC]))
        #logger.info("skip_bytes: %d\n" % (skip_bytes))
        self.icrc = binascii.crc32(self.rawbytes[:-skip_bytes])
        #logger.info("Calculated ICRC: 0x%x" % self.icrc)
        return

    def __add_for_ipv6(self):
        self.spkt[penscapy.IPv6].hlim = 0xFF
        self.spkt[penscapy.IPv6].tc = 0xFF
        self.spkt[penscapy.IPv6].fl = 0xFFFFF
        self.spkt[penscapy.UDP].chksum = 0xFFFF
        self.spkt[penscapy.BTH].f_r = 0x1
        self.spkt[penscapy.BTH].b_r = 0x1
        self.spkt[penscapy.BTH].rsvd = 0x3F
        self.__get_icrc()
        return

    def __add_for_ipv4(self):
        self.spkt[penscapy.IP].tos = 0xFF
        self.spkt[penscapy.IP].ttl = 0xFF
        self.spkt[penscapy.IP].chksum = 0xFFFF
        self.spkt[penscapy.UDP].chksum = 0xFFFF
        self.spkt[penscapy.BTH].f_r = 0x1
        self.spkt[penscapy.BTH].b_r = 0x1
        self.spkt[penscapy.BTH].rsvd = 0x3F
        self.__get_icrc()
        return

    def GetIcrc(self):
        if penscapy.IPv6 in self.spkt:
            self.__add_for_ipv6()
        elif penscapy.IP in self.spkt:
            self.__add_for_ipv4()
        else:
            assert(0)
        return self.icrc

class ScapyPacketObject:
    def __init__(self):
        self.spkt       = None
        self.rawbytes   = None

        self.__dolhdr   = None
        self.__crchdr   = None
        self.padbytes   = None

        self.curroffset = 0
        return

    def GetScapyPacket(self):
        return self.spkt

    def __build_header(self, hdr):
        builder_name = '%s_builder' % hdr.meta.id
        if hasattr(sys.modules[__name__], builder_name):
            logger.verbose("Using builder %s" % builder_name)
            builder = getattr(sys.modules[__name__], builder_name)
            return builder.build(hdr)
        return BASE_builder.build(hdr)

    def __add_header(self, hdr):
        shdr = self.__build_header(hdr)
        hdr.start = self.curroffset
        hdr.size = len(shdr)
        self.curroffset += hdr.size
        if self.spkt != None:
            self.spkt = self.spkt / shdr
        else:
            self.spkt = shdr
        return

    def __add_dol_header(self, tcid, step_id):
        hdr = FactoryStore.headers.Get('PENDOL')
        hdr.fields.tcid = tcid
        hdr.fields.step_id = step_id
        self.__add_header(hdr)
        #self.__dolhdr = self.__build_header(hdr)
        #self.rawbytes += bytes(self.__dolhdr)
        return

    def __add_crc_header(self):
        #hdr = FactoryStore.headers.Get('CRC')
        #self.__add_header(hdr)
        crc = binascii.crc32(self.rawbytes)
        self.rawbytes += penscapy.struct.pack("I", crc)
        return

    def GetIcrc(self):
        return self.spkt[penscapy.ICRC].icrc

    def __update_icrc(self, packet):
        if packet.IsNewIcrcRequired():
            spkt = packet.GetScapyPacket()
            builder = IcrcHeaderBuilder(spkt)
            icrc = builder.GetIcrc()
            #convert to network byte format
            icrc = (((icrc << 24) & 0xFF000000) |\
                           ((icrc <<  8) & 0x00FF0000) |\
                           ((icrc >>  8) & 0x0000FF00) |\
                           ((icrc >> 24) & 0x000000FF))
        elif packet.IsInheritIcrcRequired():
            # no need to byte swap - as already swapped when icrc header constructed
            icrc = packet.GetBasePacketIcrc()

        logger.debug("ICRC after byte swap: 0x%x" % icrc) 
        self.spkt[penscapy.ICRC].icrc = icrc
        return
        
    def __build_from_packet_meta(self, packet):
        logger.debug("Generating SCAPY Packet.")
        for hdrid in packet.hdrsorder:
            hdrdata = packet.headers.__dict__[hdrid]
            if objects.IsFrameworkObject(hdrdata):
                self.__add_header(hdrdata)
        
        if packet.IsDolHeaderRequired():
            self.__add_dol_header(packet.GetTcId(), packet.GetStepId())

        if packet.IsIcrcEnabled():
            self.__update_icrc(packet)


        self.pktbytes = bytes(self.spkt)
        self.rawbytes = bytes(self.spkt)
        #self.__add_crc_header()
        padsize = packet.GetPaddingSize()
        if padsize < 0:
            self.rawbytes = self.rawbytes[:len(self.rawbytes) + padsize]
        else:  
            self.rawbytes += bytes([0xff] * padsize)

    def __build_from_scapypacket(self, spkt):
        self.spkt       = spkt
        self.rawbytes   = bytes(spkt)
        return

    def Build(self, packet = None, scapypacket = None):
        if packet is not None:
            self.__build_from_packet_meta(packet)
        elif scapypacket is not None:
            self.__build_from_scapypacket(scapypacket)
        else:
            assert(0)
        return

    def GetSize(self):
        return len(self.rawbytes)

    def GetRawBytes(self):
        return self.rawbytes

    @staticmethod
    def ShowRawPacket(rawbytes, logger):
        logger.info("--------------------- RAW PACKET ---------------------")
        i = 0
        line = ''
        size = len(rawbytes)
        while i < size:
            if i % 16 == 0:
                if i != 0:
                    logger.info(line)
                    line = ''
                line += "%04X " % i
            if i % 8 == 0:
                line += ' '
            line += "%02X " % rawbytes[i]
            i += 1
        if line != '':
            logger.info(line)
        return

    @staticmethod
    def ShowScapyPacket(rawbytes, logger):
        spktobj = Parse(rawbytes)
        spkt = spktobj.spkt
        logger.info("------------------- SCAPY PACKET ---------------------")
        spkt.show(indent = 0, label_lvl = logger.GetLogPrefix())
        return

    def Show(self, logger):
        ScapyPacketObject.ShowScapyPacket(self.rawbytes, logger)
        ScapyPacketObject.ShowRawPacket(self.rawbytes, logger)
        return

class RawPacketParser:
    def __init__(self):
        return

    def __parse_ether(self, rawpkt):
        pkt = penscapy.Ether(rawpkt)
        return pkt

    def __get_raw_hdr(self, pkt):
        if penscapy.Raw in pkt:
            return pkt[penscapy.Raw]
        if penscapy.Padding in pkt:
            return pkt[penscapy.Padding]
        return None

    def __process_pendol(self, rawbytes):
        if len(rawbytes) < penscapy.PENDOL_LENGTH:
            return None

        pdoffset = len(rawbytes) - penscapy.PENDOL_LENGTH
        pdbytes = rawbytes[pdoffset:]
        pen = penscapy.PENDOL(pdbytes)
        if pen.sig != penscapy.PENDOL_SIGNATURE:
            return None
        return pen

    def __process_nxthdrs(self, rawbytes):
        nxthdrs = None
        '''
        length = len(rawbytes)
        if length < penscapy.CRC_LENGTH:
            return penscapy.Raw(rawbytes)

        # CRC is always present, process it first.
        crcoffset = length - penscapy.CRC_LENGTH
        crcbytes = rawbytes[crcoffset:]
        nxthdrs = penscapy.CRC(crcbytes)
        
        rawbytes = rawbytes[:crcoffset]
        if len(rawbytes) == 0:
            # Only CRC is present in payload.
            return nxthdrs
        '''
        # Lets check if PENDOL header is present.
        pen = self.__process_pendol(rawbytes)
        if pen is not None:
            pyldbytes = rawbytes[:len(rawbytes) - penscapy.PENDOL_LENGTH]
            nxthdrs = pen / nxthdrs
        else:
            pyldbytes = rawbytes

        # If any bytes are remaining, treat them as payload.
        if len(pyldbytes) > 0:
            nxthdrs = penscapy.PAYLOAD(pyldbytes) / nxthdrs

        return nxthdrs

    def __process_raw_bytes(self, pkt):
        rawhdr = self.__get_raw_hdr(pkt)
        if rawhdr is None:
            return pkt
        if rawhdr.underlayer == None:
           pdb.set_trace()
        rawhdr.underlayer.remove_payload()

        nxthdrs = self.__process_nxthdrs(bytes(rawhdr))
        pkt = pkt / nxthdrs
        return pkt

    def __process_vxlan(self, pkt):
        if penscapy.VXLAN in pkt:
            pkt[penscapy.VXLAN].underlayer.sport = 0
        return

    def Parse(self, rawpkt):
        pkt = self.__parse_ether(rawpkt)
        pkt = self.__process_raw_bytes(pkt)
        self.__process_vxlan(pkt) 
        return pkt

def Parse(rawpkt):
    prs = RawPacketParser()
    spkt = prs.Parse(rawpkt)
    #return spkt

    obj = ScapyPacketObject()
    obj.Build(scapypacket = spkt)
    return obj

#class ScapyHeaderBuilder_MSRPC_SYNTAX(ScapyHeaderBuilder_BASE):
#    def build(self, hdr):
#        hdr.fields.if_uuid = penscapy.UUID(hdr.fields.if_uuid)
#        return super().build(hdr)
#MSRPC_SYNTAX_builder = ScapyHeaderBuilder_MSRPC_SYNTAX()

#class ScapyHeaderBuilder_MSRPC_CTXT_ELEM(ScapyHeaderBuilder_BASE):
#    def build(self, hdr):
#        records = []
#        for r in hdr.fields.xfer_syntax:
#            records.append(penscapy.MSRPC_SYNTAX(r.record))
#        hdr.fields.xfer_syntax = records
#        return super().build(hdr)
#MSRPC_CTXT_ELEM_builder = ScapyHeaderBuilder_MSRPC_CTXT_ELEM()

class ScapyHeaderBuilder_MSRPC_EPM_TWR(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        records = []
        for r in hdr.fields.floors:
            records.append(penscapy.MSRPC_EPM_FLOOR(r.record))
        hdr.fields.floors = records
        return super().build(hdr)
MSRPC_EPM_TWR_builder = ScapyHeaderBuilder_MSRPC_EPM_TWR()

class ScapyHeaderBuilder_MSRPC_EPM_RSP(ScapyHeaderBuilder_BASE):
    def build(self, hdr):
        records = []
        for r in hdr.fields.msrpc_twr:
            records.append(penscapy.MSRPC_EPM_TWR(r.record))
        hdr.fields.msrpc_twr = records
        return super().build(hdr)
MSRPC_EPM_RSP_builder = ScapyHeaderBuilder_MSRPC_EPM_RSP()

#class ScapyHeaderBuilder_MSRPC_BIND_REQ(ScapyHeaderBuilder_BASE):
#    def build(self, hdr):
#        records = []
#        for r in hdr.fields.p_cont_elm:
#            records.append(penscapy.MSRPC_CTXT_ELEM(r.record))
#        hdr.fields.p_cont_elm = records
#        return super().build(hdr)
#MSRPC_BIND_REQ_builder = ScapyHeaderBuilder_MSRPC_BIND_REQ()

#class ScapyHeaderBuilder_MSRPC_BIND_RSP(ScapyHeaderBuilder_BASE):
#    def build(self, hdr):
#        records = []
#        for r in hdr.fields.rslts:
#            records.append(penscapy.MSRPC_BIND_RSLT(r.record))
#        hdr.fields.rslts = records
#        return super().build(hdr)
#MSRPC_BIND_RSP_builder = ScapyHeaderBuilder_MSRPC_BIND_RSP()
