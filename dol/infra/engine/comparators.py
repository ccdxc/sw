#! /usr/bin/python3
import copy
import pdb

import infra.penscapy.penscapy as penscapy
import infra.common.defs as defs
import infra.common.objects as objects
import infra.common.utils as utils

from infra.factory.store    import FactoryStore
from infra.common.glopts    import GlobalOptions
from infra.common.logging   import logger as logger

gl_ignore_fields = {
    'IP'    : [ 'chksum' ],
    'TCP'   : [ 'chksum' ],
    'UDP'   : [ 'chksum' ],
    'ICMP'  : [ 'chksum' ],
}

class CrPacket:
    def __init__(self, pktid, rawpkt, ports = None):
        self.ports      = ports
        self.rawpkt     = rawpkt
        self.pktlen     = len(rawpkt)
        self.hdrs       = []
        self.hdrnames   = []
        self.spkt       = penscapy.Parse(rawpkt)
        self.pktid      = pktid

        self.__build_hdr_stack()
        self.nhdrs = len(self.hdrs)
        return
    
    def __get_payload(self, hdr):
        return hdr.payload

    def __get_hdr_name(self, hdr):
        return hdr.__class__.__name__

    def __build_hdr_stack(self):
        hdr = self.spkt.copy()
        pyld = hdr.payload
        hdr.remove_payload()
        while hdr.name != 'NoPayload':
            self.hdrs.append(hdr)
            hdrname = self.__get_hdr_name(hdr)
            self.hdrnames.append(hdrname)
            hdr = pyld
            pyld = hdr.payload
            hdr.remove_payload()
        return

    def __is_port_match(self, rxpkt):
        # Handle ports=None case for PktBuffers
        if self.ports is None:
            if rxpkt.ports != None:
                return False
            return True

        # Now lets handle real packets.
        if rxpkt.ports is None:
            return False
        
        rxport = rxpkt.ports[0]
        if rxport in self.ports:
            return True

        return True

    def __is_pkt_len_match(self, rxpkt):
        return self.pktlen == rxpkt.pktlen
    
    def __is_pkt_hdr_count_match(self, rxpkt):
        # First check if the # of headers are same.
        if len(self.hdrs) != len(rxpkt.hdrs):
            return False
        return True

    def __is_pkt_hdr_stack_match(self, rxpkt):
        for hidx in range(len(self.hdrs)):
            if type(self.hdrs[hidx]) != type(rxpkt.hdrs[hidx]):
                return False
        return True
 
    def __get_pkt_match_degree(self, rxpkt):
        pktdeg = 0
        if self.__is_pkt_hdr_count_match(rxpkt) is False:
            # If # of headers is not same, then treat them
            # as different packets.
            return 0
       
        if self.__is_pkt_hdr_stack_match(rxpkt) is False:
            # If the header-stack is not same, then treat them
            # as different packets.
            return 0

        # Check if the header stackup is same.
        for hidx in range(len(self.hdrs)):
            if self.hdrs[hidx] == rxpkt.hdrs[hidx]:
                pktdeg += 1
                continue
        return pktdeg


    def GetMatchDegree(self, rxpkt):
        # The object passed MUST be a RX packet.
        port_match = self.__is_port_match(rxpkt)
        if port_match is False:
            return 0

        pkt_len_match = self.__is_pkt_len_match(rxpkt)
        if pkt_len_match is False:
            return 0

        return self.__get_pkt_match_degree(rxpkt)


    def Show(self, lgh):
        penscapy.ShowPacket(self.spkt, lgh)
        lgh.info("Packet Length = %d" % self.pktlen)
        return

class CrPacketPair:
    def __init__(self, expkt, rxpkt, pid, lg = logger):
        if expkt and isinstance(expkt, CrPacket) is False:
            pdb.set_trace()
        if rxpkt and isinstance(rxpkt, CrPacket) is False:
            pdb.set_trace()
        self.expkt  = expkt
        self.rxpkt  = rxpkt
        
        # Match and Mis-match headers
        self.match_hdrs      = []
        self.mismatch_hdrs   = []

        self.degree = self.__get_degree()
        self.match  = None
        self.lg     = lg
        return
    
    def __get_degree(self):
        if self.expkt == None or self.rxpkt == None:
            return 0
        return self.expkt.GetMatchDegree(self.rxpkt)

    def __expstr(self):
        if self.expkt is None: return None
        idstring = "Exp:%s/Ports:%s" % (self.expkt.pktid, str(self.expkt.ports))
        return idstring

    def __rxstr(self):
        if self.rxpkt is None: return None
        idstring = "Rx:%s/Port:%s" % (self.rxpkt.pktid, str(self.rxpkt.ports))
        return idstring

    def __get_pair_id(self):
        idstring = "%s <--> %s" % (self.__expstr(), self.__rxstr())
        return idstring

    def __process_ignore(self, ehdr, ahdr):
        global gl_ignore_fields
        for hdr,fields in gl_ignore_fields.items():
            if hdr != ehdr.__class__.__name__: continue
            for f in fields:
                setattr(ehdr, f, 0)
                setattr(ahdr, f, 0)
                self.lg.warn("- Header:%s Field:%s is ignored. !!" % (hdr, f))
        return

    def Match(self):
        self.lg.info("Matching Packets: %s" % self.__get_pair_id())
        if self.expkt is None or self.rxpkt is None:
            return False

        assert(self.expkt.nhdrs == self.rxpkt.nhdrs)
        self.match = True
        for hidx in range(self.expkt.nhdrs):
            ehdr = self.expkt.hdrs[hidx]
            ahdr = self.rxpkt.hdrs[hidx]
            assert(type(ehdr) == type(ahdr))
            self.__process_ignore(ehdr, ahdr)
            if ehdr != ahdr:
                self.mismatch_hdrs.append((ehdr,ahdr))
                self.match = False
            else:
                self.match_hdrs.append((ehdr,ahdr))
        return self.match

    def __show_missing(self):
        self.lg.error("Missing Packet: %s" % self.__expstr())
        return

    def __show_excess(self):
        self.lg.error("Excess Packet: %s" % self.__rxstr())
        return

    def __show_hdr_mismatch(self, ehdr, ahdr):
        efields = ehdr.fields
        afields = ahdr.fields

        self.lg.error("Header: %s" % ehdr.__class__.__name__)
        for efk,efv in efields.items():
            afv = afields[efk]
            if efv == afv: continue
            self.lg.error("  Field: %s : Exp = %s, Actual = %s" %\
                           (efk, str(efv), str(afv)))
        return

    def __show_mismatch(self):
        self.lg.error("Packets Mismatch: %s" % self.__get_pair_id())
        for hpair in self.mismatch_hdrs:
            ehdr = hpair[0]
            ahdr = hpair[1]
            assert(type(ehdr) == type(ahdr)) 
            self.__show_hdr_mismatch(ehdr, ahdr)
        return

    def __show_match(self):
        self.lg.info("Packets Match: %s" % self.__get_pair_id())
        return

    def ShowResults(self):
        if self.match == True:
            self.__show_match()
            return
        if self.expkt is None:
            self.__show_excess()
        elif self.rxpkt is None:
            self.__show_missing()
        else:
            self.__show_mismatch()

        return

# Packet Comparator for Scapy packets.
class PacketComparator:
    def __init__(self, lg = logger):
        self.eid    = 0
        self.rid    = 0
        self.pid    = 0
        self.expkts = {}
        self.rxpkts = {}
        self.pairs  = {}
        self.match  = True
        self.lg     = lg
        return

    def __get_best_rpkt(self, epkt):
        bdeg = 0
        brpkt = None
        for rpktid,rpkt in self.rxpkts.items():
            deg = epkt.GetMatchDegree(rpkt)
            if deg == 0: continue
            if deg > bdeg:
                bdeg = deg
                brpkt = rpkt
        return brpkt

    def __add_pair(self, e, r):
        self.pid += 1
        self.lg.verbose("Adding pair id = %s" % self.pid)
        pair = CrPacketPair(e, r, self.pid, self.lg)
        self.pairs[self.pid] = pair
        return

    def __build_pairs(self):
        for epktid,epkt in self.expkts.items():
            rpkt = self.__get_best_rpkt(epkt)
            self.__add_pair(epkt, rpkt)
            if rpkt is not None:
                del self.rxpkts[rpkt.pktid]

        # Check if any other packets are left in rcvs
        for rpktid,rpkt in self.rxpkts.items():
            self.__add_pair(None, rpkt)
        return

    def AddExpected(self, spkt, ports, pktid):
        self.eid += 1
        epgid = '%s::%d' % (pktid, self.eid)
        self.lg.info("EXPECTED RX Packet: %s" % epgid)
        epobj = CrPacket(pktid, spkt, ports)
        epobj.Show(self.lg)
        assert(epgid not in self.expkts)
        self.expkts[epgid] = epobj
        return

    def AddReceived(self, spkt, ports):
        self.rid += 1
        rpgid = 'RXPKT%d' % self.rid
        self.lg.info("ACTUAL RX Packet: %s" % rpgid)
        rpobj = CrPacket(rpgid, spkt, ports)
        rpobj.Show(self.lg)
        self.rxpkts[rpobj.pktid] = rpobj
        return

    def Compare(self):
        self.__build_pairs()
        for pid,pair in self.pairs.items():
            if pair.Match() == False:
                self.match = False
        return

    def ShowResults(self):
        for pid,pair in self.pairs.items():
            pair.ShowResults()
        return

    def IsMatch(self):
        return self.match

