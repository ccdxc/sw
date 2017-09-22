#! /usr/bin/python3
import pdb

from third_party.scapy import *
from third_party.scapy.all import *
from third_party.scapy.layers.inet6 import _ICMPv6

PAYLOAD_SIGNATURE = 0xDA1A
class PAYLOAD(Packet):
    name = "PAYLOAD"
    fields_desc = [
        XShortField("sig", PAYLOAD_SIGNATURE),
        FieldLenField("len", None, length_of="data"),
        FieldListField("data", [], ByteField("", 0))
    ]

    def show(self, indent=3, lvl="", label_lvl=""):
        print("%s###[ PAYLOAD ]###" % label_lvl)
        print("%s  sig  = %X" % (label_lvl, self.sig))
        if self.len:
            print("%s  len  = %d" % (label_lvl, self.len))
        else:
            print("%s  len  = None" % label_lvl)
        print("%s  data = " % label_lvl, end='')
        PrintPayload(self.data, prefix=label_lvl + "         ")
        self.payload.show(indent, lvl, label_lvl)
        return


class PADDING(Packet):
    name = "PADDING"
    fields_desc = [
        StrLenField("data", "")
    ]


class CRC(Packet):
    name = "CRC"
    fields_desc = [
        XIntField("crc", 0)
    ]


class PENDOL(Packet):
    name = "PENDOL"
    fields_desc = [
        BitField("ts",          0,      32),
        BitField("id",          0,      32),
        BitField("opcode",      0,      8),
        BitField("step_id",     0,      8),
        BitField("log",         0,      1),
        BitField("logdrop",     0,      1),
        BitField("rsvd",        0,      14),
    ]

class GRH(Packet):
    name = "GRH"
    fields_desc = [
        BitField("ipver",       0,      4),
        BitField("tclass",      0,      8),
        BitField("flowlabel",   0,      20),
        BitField("paylen",      0,      16),
        BitField("nxthdr",      0,      8),
        BitField("hoplmt",      0,      8),
        IP6Field("sgid",        "::"),
        IP6Field("dgid",        "::"),
    ]

class VXLAN(Packet):
    name = "VXLAN"
    fields_desc = [
        BitField("flags_r0",    0,      4),
        BitField("flag_i",      1,      1),
        BitField("flags_r1",    0,      3),
        BitField("rsvd0",       0,      24),
        BitField("vni",         0,      24),
        BitField("rsvd1",       0,      8),
    ]
bind_layers(UDP, VXLAN, dport=4789)
bind_layers(VXLAN, Ether, flag_i=1)
class RDETH(Packet):
    name = "RDETH"
    fields_desc = [
        BitField("rsvd",        0,      8),
        BitField("eecnxt",      0,      24),
    ]


class DETH(Packet):
    name = "DETH"
    fields_desc = [
        BitField("qkey",        0,      32),
        BitField("rsvd",        0,      8),
        BitField("srcqp",       0,      24),
    ]

class ImmDT(Packet):
    name = "ImmDT"
    fields_desc = [
        BitField("imm_data",    0,      32),
    ]


class RETH(Packet):
    name = "RETH"
    fields_desc = [
        BitField("va",          0,      64),
        BitField("r_key",       0,      32),
        BitField("dma_len",     0,      32),
    ]

    next_hdr = { 6: Raw, #write-first
                 10: Raw, #write-only
                 11: ImmDT, #write-only-with-imm
                 12: Raw, #read-request
    }

    def guess_payload_class(self, payload):
        if (self.next_hdr[self.underlayer.opcode]): 
            return self.next_hdr[self.underlayer.opcode]
        else:
            assert(0);
            return super().guess_payload_class()

class AtomicETH(Packet):
    name = "AtomicETH"
    fields_desc = [
        BitField("va",          0,      64),
        BitField("r_key",       0,      32),
        BitField("swapdt",      0,      64),
        BitField("cmpdt",       0,      64),
    ]


class XRCETH(Packet):
    name = "XRCETH"
    fields_desc = [
        BitField("rsvd",        0,      8),
        BitField("xrcsrq",      0,      24),
    ]

class AtomicAckETH(Packet):
    name = "AtomicAckETH"
    fields_desc = [
        BitField("orig_remdt",  0,      64),
    ]

class AETH(Packet):
    name = "AETH"
    fields_desc = [
        BitField("syndrome",    0,      8),
        BitField("msn",         0,      24),
    ]
    next_hdr = {
                 13: Raw, #read-response-first
                 15: Raw, #read-response-last
                 16: Raw, #read-response-only
                 17: Raw, #ack
                 18: AtomicAckETH, #atomic-ack
    }
    def guess_payload_class(self, payload):
        if (self.next_hdr[self.underlayer.opcode]): 
            return self.next_hdr[self.underlayer.opcode]
        else:
            assert(0);
            return super().guess_payload_class()

class IETH(Packet):
    name = "IETH"
    fields_desc = [
        BitField("r_key",       0,      32),
    ]

class BTH(Packet):
    name = "BTH"
    fields_desc = [
        BitField("opcode",      0,      8),
        BitField("se",          0,      1),
        BitField("m",           0,      1),
        BitField("padcnt",      0,      2),
        BitField("tver",        0,      4),
        BitField("p_key",       0,      16),
        BitField("f_r",         0,      1),
        BitField("b_r",         0,      1),
        BitField("rsvd",        0,      6),
        BitField("destqp",      0,      24),
        BitField("a",           0,      1),
        BitField("rsvd2",       0,      7),
        BitField("psn",         0,      24),
    ]

    next_hdr = { 0: Raw, #send-first
                 1: Raw, #send-middle
                 2: Raw, #send-last
                 3: ImmDT, #send-last-with-imm
                 4: Raw, #send-only
                 5: ImmDT, #send-only-with-imm
                 6: RETH, #write-first
                 7: Raw, #write-middle
                 8: Raw, #write-last
                 9: ImmDT, #write-last-with-imm
                 10: RETH, #write-only
                 11: RETH, #write-only-with-imm
                 12: RETH, #read-request
                 13: AETH, #read-response-first
                 14: Raw, #read-response-middle
                 15: AETH, #read-response-last
                 16: AETH, #read-response-only
                 17: AETH, #ack
                 18: AETH, #atomic-ack
                 19: AtomicETH, #compare-n-swap
                 20: AtomicETH, #fetch-add
                 21: Raw, #rsvd
                 22: IETH, #send-last-with-inv
                 23: IETH, #send-only-with-inv
    }

    def guess_payload_class(self, payload):
        if (self.next_hdr[self.opcode]): 
            return self.next_hdr[self.opcode]
        else:
            assert(0);
            return super().guess_payload_class()

bind_layers(UDP, BTH, dport=4791)



def PrintPayload(data, prefix=""):
    size = len(data)
    i = 0
    while i < size:
        if i != 0 and i % 16 == 0:
            print("\n%s" % prefix, end='')
        print("%02X " % data[i], end='')
        i += 1
    print("", flush=True)
    return


def PrintPacket(pkt):
    raw = bytes(pkt)
    size = len(raw)
    i = 0
    while i < size:
        if i % 16 == 0:
            if i != 0:
                print("")
            print("%04X " % i, end='')
        if i % 8 == 0:
            print(" ", end='')

        print("%02X " % raw[i], end='')
        i += 1
    print("", flush=True)

def PrintRawPacket(pkt):
    hexdump(pkt)
    return

def PrintHeaders(pkt):
    pkt.show(indent=0)
    return


class RawPacketParserObject:
    def __init__(self):
        return

    def __parse_ether(self, rawpkt):
        pkt = Ether(rawpkt)
        return pkt

    def __get_raw_hdr(self, pkt):
        if Raw in pkt:
            return pkt[Raw]
        if Padding in pkt:
            return pkt[Padding]
        return None

    def __process_pendol(self, pyld):
        data = pyld.data
        penbytes = bytes(data[pyld.len:])
        del data[pyld.len:]
        pyld.data = data
            
        pen = PENDOL(penbytes)
        nxthdrs = pen
        if Raw in pen:
            crcbytes = bytes(pen[Raw])
            pen[PENDOL].remove_payload()
            crc = CRC(crcbytes)
            nxthdrs = pen/crc
        return nxthdrs

    def __process_payload(self, rawhdr):
        pyld = PAYLOAD(bytes(rawhdr))
        if pyld.sig != PAYLOAD_SIGNATURE:
            return rawhdr
        nxthdrs = self.__process_pendol(pyld)
        return pyld / nxthdrs

    def __process_raw_bytes(self, pkt):
        rawhdr = self.__get_raw_hdr(pkt)
        if rawhdr is None:
            return pkt
        rawhdr.underlayer.remove_payload()

        nxthdrs = self.__process_payload(rawhdr)
        pkt = pkt / nxthdrs
        return pkt

    def __process_vxlan(self, pkt):
        if VXLAN in pkt:
            pkt[VXLAN].underlayer.sport = 0
        return

    def Parse(self, rawpkt):
        pkt = self.__parse_ether(rawpkt)
        pkt = self.__process_raw_bytes(pkt)
        self.__process_vxlan(pkt) 
        return pkt


def Parse(rawpkt):
    prs = RawPacketParserObject()
    return prs.Parse(rawpkt)

def ShowRawPacket(spkt, logger):
    logger.info("--------------------- RAW PACKET ---------------------")
    raw = bytes(spkt)
    size = len(raw)
    i = 0
    line = ''
    while i < size:
        if i % 16 == 0:
            if i != 0:
                logger.info(line)
                line = ''
            line += "%04X " % i
        if i % 8 == 0:
            line += ' '
        line += "%02X " % raw[i]
        i += 1
    if line != '':
        logger.info(line)
    return


def ShowScapyPacket(spkt, logger):
    logger.info("------------------- SCAPY PACKET ---------------------")
    spkt.show(indent = 0,
              label_lvl = logger.GetLogPrefix())
    return


def ShowPacket(spkt, logger):
    ShowScapyPacket(spkt, logger)
    ShowRawPacket(spkt, logger)
    return

