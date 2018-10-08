#! /usr/bin/python3
import os
import pdb
import binascii
import atexit

import infra.factory.scapyfactory as scapyfactory
import infra.penscapy.penscapy as penscapy

from infra.common.logging import logger
from infra.common.glopts import GlobalOptions

class PacketCollectorObject:
    def __init__(self):
        self.pktsdb = {}
        self.pcapfiles = {}
        os.system("rm -rf *.pcap")
        return

    def __init_db_entry(self, key):
        if key not in self.pktsdb:
            self.pktsdb[key] = []
            if GlobalOptions.pcapdir:
                self.pcapfiles[key] = "%s/%s_packets.pcap" % (GlobalOptions.pcapdir, key)
            else:
                self.pcapfiles[key] = "%s_packets.pcap" % (key)
        return

    def Save(self, tcid, pkt, port):
        logger.info("Saving Input Packet of Length:%d on Port:%d to PCAP" %\
                    (len(pkt), port))
        key = "uplink%d" % port
        self.__init_db_entry(key)
        self.pktsdb[key].append(pkt)

        if GlobalOptions.save_tc_pcap:
            logger.info("Saving Input Packet of Length:%d on tcid:%d to PCAP" %\
                        (len(pkt), tcid))
            key = "TC%06d" % tcid
            self.__init_db_entry(key)
            self.pktsdb[key].append(pkt)
        return

    def SaveTx(self, rawpkt, lif):
        if rawpkt is None:
            return
        logger.info("Saving TX Packet of Length:%d on Lif:%s to PCAP" %\
                    (len(rawpkt), lif.GID()))
        key = lif.GID()
        self.__init_db_entry(key)
        spobj = scapyfactory.Parse(rawpkt)
        self.pktsdb[key].append(spobj.GetScapyPacket())
        return

    def Write(self):
        for k,v in self.pktsdb.items():
            penscapy.wrpcap(self.pcapfiles[k], v)
        return

PacketCollector = PacketCollectorObject()
