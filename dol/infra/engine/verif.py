#! /usr/bin/python3

import copy
import pdb

import infra.penscapy.penscapy as penscapy
import infra.common.defs as defs
import infra.common.objects as objects
import infra.common.utils as utils
import infra.engine.comparators as comparators

from infra.factory.store    import FactoryStore
from infra.common.glopts    import GlobalOptions
from infra.common.logging   import logger as logger
from infra.asic.model       import ModelConnector

class VerifEngineObject:
    def __init__(self):
        # Pending testcase database
        self.ptcdb = objects.ObjectDatabase(logger)
        # Completed testcase database 
        self.ctcdb = objects.ObjectDatabase(logger)
        return

    def __compare_pktbuffers(self, epktbuf, apktbuf, lgh):
        if GlobalOptions.dryrun:
            return True

        if epktbuf is None or apktbuf is None:
            lgh.error("Packet Compare: ExpType:%s, ActType:%s" %\
                      (type(epktbuf), type(apktbuf)))
            return defs.status.ERROR

        # Make sure we are not accidentally comparing the same object
        if id(apktbuf) == id(epktbuf):
            lgh.error("ExpBuf and ActBuf are same objects.")
            return defs.status.ERROR

        lgh.verbose("Comparing Packets")
        pcr = comparators.PacketComparator(lgh)
        pcr.AddExpected(epktbuf, None, 1)
        pcr.AddReceived(apktbuf, None)
        pcr.Compare()
        pcr.ShowResults()
        return pcr.IsMatch()


    def __verify_pktbuffers(self, epktbuf, apktbuf, lgh):
        match = self.__compare_pktbuffers(epktbuf, apktbuf, lgh)
        if match is False:
            lgh.error("Packet compare result = Mismatch")
            return defs.status.ERROR
        lgh.info("Packet compare result = Match")
        return defs.status.SUCCESS

    def __compare_buffers(self, ebuf, abuf, lgh):
        if GlobalOptions.dryrun:
            return True

        if ebuf is None or abuf is None:
            lgh.error("Buffer Compare: ExpType:%s, ActType:%s" %\
                      (type(ebuf), type(abuf)))
            return defs.status.ERROR

        lgh.info("Comparing Buffers: %s <--> %s " % (ebuf.GID(), abuf.GID()))
        # Make sure we are not accidentally comparing the same object
        if id(ebuf) == id(abuf):
            lgh.error("ExpBuf and ActBuf are same objects.")
            return defs.status.ERROR

        return ebuf == abuf

    def __get_pktbuffer(self, buf):
        if buf is not None:
            return buf.Read()

        if GlobalOptions.dryrun is False:
            lgh.error("Trying to GetBuffer() on a None descriptor.")
            assert(0)
        return None

    def __verify_buffers(self, ebuf, abuf, lgh):
        match = self.__compare_buffers(ebuf, abuf, lgh)
        if match is False:
            lgh.error("Buffer compare result = Mismatch")
            return defs.status.ERROR
        lgh.info("Buffer compare result = Match")

        # If this buffer is not a packet, then stop.
        if ebuf.IsPacket() is False:
            return defs.status.SUCCESS
       
        epktbuf = self.__get_pktbuffer(ebuf)
        apktbuf = self.__get_pktbuffer(abuf)
        return self.__verify_pktbuffers(epktbuf, apktbuf, lgh)

    def __get_buffer(self, descr, lgh):
        if descr is not None:
            return utils.SafeFnCall(None, lgh, descr.GetBuffer)

        if GlobalOptions.dryrun is False:
            lgh.error("Trying to GetBuffer() on a None descriptor.")
            assert(0)
        return None

    def __compare_descriptors(self, edescr, adescr, lgh):
        if GlobalOptions.dryrun:
            return True

        lgh.info("Comparing Descriptors: %s <--> %s " % (edescr.GID(), adescr.GID()))
        return edescr == adescr

    def __verify_one_descriptor(self, edescr, adescr, lgh):
        match = self.__compare_descriptors(edescr, adescr, lgh)
        if match is False:
            lgh.error("Descriptor compare result = MisMatch")
            return defs.status.ERROR
        lgh.info("Descriptor compare result = Match")
        ebuf = self.__get_buffer(edescr, lgh)
        abuf = self.__get_buffer(adescr, lgh)
        return self.__verify_buffers(ebuf, abuf, lgh)

    def __consume_descriptor(self, edescr, ring):
        if GlobalOptions.dryrun:
            return None
        else:
            adescr = copy.copy(edescr)
            ring.Consume(adescr)
        return adescr

    def __verify_descriptors(self, step, lgh):
        for dsp in step.expect.descriptors:
            edescr = dsp.descriptor.object
            adescr = self.__consume_descriptor(edescr, dsp.descriptor.ring)
            status = self.__verify_one_descriptor(edescr, adescr, lgh)
            if status == defs.status.ERROR:
                return status
        return defs.status.SUCCESS

    def __receive_packets(self, pcr, step, lgh):
        mpkts = ModelConnector.Receive()
        for mpkt in mpkts:
            pcr.AddReceived(mpkt.rawpkt, [ mpkt.port ])
        return

    def __add_expected(self, pcr, step, lgh):
        for psp in step.expect.packets:
            pkt = psp.packet
            ports = psp.ports
            pcr.AddExpected(pkt.rawbytes, ports, pkt.GID())
        return

    def __add_dummy_rx(self, pcr, step, lgh):
        for psp in step.expect.packets:
            pkt = psp.packet
            ports = psp.ports
            pcr.AddReceived(pkt.rawbytes, ports)
        return

    def __verify_packets(self, step, lgh):
        pcr = comparators.PacketComparator(lgh)
        # Add Expected
        self.__add_expected(pcr, step, lgh)
        # Add RX
        if GlobalOptions.dryrun:
            self.__add_dummy_rx(pcr, step, lgh)
        else:
            self.__receive_packets(pcr, step, lgh)
        
        pcr.Compare()
        pcr.ShowResults()
        if pcr.IsMatch() == False:
            return defs.status.ERROR
        return defs.status.SUCCESS

    def __verify(self, step, lgh):
        verify_pass = True
        pstatus = self.__verify_packets(step, lgh)
        if pstatus == defs.status.ERROR:
            verify_pass = False

        dstatus = self.__verify_descriptors(step, lgh)
        if dstatus == defs.status.ERROR:
            verify_pass = False

        if verify_pass == False:
            return defs.status.ERROR

        return defs.status.SUCCESS

    def Verify(self, step, lgh):
        return self.__verify(step, lgh)

VerifEngine = VerifEngineObject()
