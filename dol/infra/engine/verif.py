#! /usr/bin/python3

import copy
import pdb
import time

import infra.penscapy.penscapy as penscapy
import infra.common.defs as defs
import infra.common.objects as objects
import infra.common.utils as utils
import infra.engine.comparators as comparators

from infra.factory.store    import FactoryStore
from infra.common.glopts    import GlobalOptions
from infra.common.logging   import logger as logger
from infra.asic.model       import ModelConnector

RTL_RETRY_SCALE     = 20
MAX_RETRIES         = 10
MAX_DROP_RETRIES    = 1

class VerifEngineObject:
    def __init__(self):
        pass

    def _verify(self, step, tc):
        return defs.status.SUCCESS

    def Verify(self, step, tc):
        if GlobalOptions.skipverify:
            logger.info("Run with skipverify=True: SKIPPING VERIFICATION")
            return defs.status.SUCCESS

class DolVerifEngineObject(VerifEngineObject):
    def __init__(self):
        super().__init__()
        # Pending testcase database
        self.ptcdb = objects.ObjectDatabase()
        # Completed testcase database 
        self.ctcdb = objects.ObjectDatabase()
        return

    def __compare_pktbuffers(self, epktbuf, apktbuf, tc):
        if GlobalOptions.dryrun:
            return True

        if epktbuf is None:
            logger.info("Packet Compare: Expected Buffer is None. Skipping")
            return True

        if apktbuf is None:
            logger.error("Packet Compare: ExpType:%s, ActType:%s" %\
                      (type(epktbuf), type(apktbuf)))
            return False

        # Make sure we are not accidentally comparing the same object
        if id(apktbuf) == id(epktbuf):
            logger.error("ExpBuf and ActBuf are same objects.")
            return False

        logger.verbose("Comparing Packets")
        pcr = comparators.PacketComparator(tc.GetIgnorePacketFields())
        pcr.AddExpected(epktbuf, None, 1)
        pcr.AddReceived(apktbuf, None)
        pcr.Compare()
        pcr.ShowResults()
        return pcr.IsMatch()


    def __verify_pktbuffers(self, epktbuf, apktbuf, tc):
        match = self.__compare_pktbuffers(epktbuf, apktbuf, tc)
        if match is False:
            logger.error("PacketBuffer Compare Result = Mismatch")
            return defs.status.ERROR
        logger.info("PacketBuffer Compare Result = Match")
        return defs.status.SUCCESS

    def __compare_buffers(self, ebuf, abuf, tc):
        if GlobalOptions.dryrun:
            return True

        if ebuf is None or abuf is None:
            logger.error("Buffer Compare: ExpType:%s, ActType:%s" %\
                      (type(ebuf), type(abuf)))
            return False

        logger.info("Comparing Buffers: %s <--> %s " % (ebuf.GID(), abuf.GID()))
        # Make sure we are not accidentally comparing the same object
        if id(ebuf) == id(abuf):
            logger.error("ExpBuf and ActBuf are same objects.")
            return False

        return ebuf == abuf

    def __get_pktbuffer(self, buf, tc):
        if buf is not None:
            return buf.Read()

        if GlobalOptions.dryrun is False:
            logger.error("Trying to GetBuffer() on a None descriptor.")
            assert(0)
        return None

    def __verify_buffers(self, ebuf, abuf, tc):
        match = self.__compare_buffers(ebuf, abuf, tc)
        if match is False:
            logger.error("Buffer compare result = Mismatch")
            return defs.status.ERROR
        logger.info("Buffer compare result = Match")

        # If this buffer is not a packet, then stop.
        if ebuf.IsPacket() is False:
            logger.info("Buffer is not a packet. Skipping PktCompare.")
            return defs.status.SUCCESS

        epktbuf = self.__get_pktbuffer(ebuf, tc)
        apktbuf = self.__get_pktbuffer(abuf, tc)
        return self.__verify_pktbuffers(epktbuf, apktbuf, tc)

    def __get_buffer(self, descr, tc):
        if descr is not None:
            return utils.SafeFnCall(None, logger, descr.GetBuffer)

        if GlobalOptions.dryrun is False:
            logger.error("Trying to GetBuffer() on a None descriptor.")
            assert(0)
        return None

    def __compare_descriptors(self, edescr, adescr, tc):
        if GlobalOptions.dryrun:
            return True

        egid = None
        if edescr is not None:
            egid = edescr.GID()
        agid = None
        if adescr is not None:
            agid = adescr.GID()

        logger.info("Comparing Descriptors: %s <--> %s " % (egid, agid))
        return edescr == adescr

    def __verify_one_descriptor(self, edescr, adescr, negtest, tc):
        if negtest:
            edescr = None
        match = self.__compare_descriptors(edescr, adescr, tc)
        if match is False:
            logger.error("Descriptor compare result = MisMatch")
            return defs.status.ERROR
        logger.info("Descriptor compare result = Match")
        if negtest:
            return defs.status.SUCCESS
        ebuf = self.__get_buffer(edescr, tc)
        abuf = self.__get_buffer(adescr, tc)
        if ebuf is None:
            logger.info("Expected buffer is None = Buffer Verification skipped!")
            return defs.status.SUCCESS
        return self.__verify_buffers(ebuf, abuf, tc)

    def __retry_wait(self, tc):
        if GlobalOptions.dryrun:
            return
        logger.info("Retry wait.........")
        if GlobalOptions.rtl:
            time.sleep(1*RTL_RETRY_SCALE)
        else:
            time.sleep(1)
        return

    def __get_scaled_retry(self, value):
        if GlobalOptions.rtl:
            return value * RTL_RETRY_SCALE
        return value

    def __get_num_retries(self, tc, rxcount):
        if rxcount == 0:
            return self.__get_scaled_retry(MAX_DROP_RETRIES)
        return self.__get_scaled_retry(MAX_RETRIES)

    def __is_retry_enabled(self, tc):
        if tc.IsRetryEnabled() == False and GlobalOptions.rtl == False:
            return False
        return True

    def __consume_descriptor(self, edescr, negtest, ring, tc):
        if GlobalOptions.dryrun:
            return None

        adescr = copy.copy(edescr)

        max_retries = self.__get_num_retries(tc, None)
        if negtest and max_retries > 2:
            max_retries = 1

        status = ring.Consume(adescr)
        for r in range(max_retries):
            if status != defs.status.RETRY: break
            self.__retry_wait(tc)
            status = ring.Consume(adescr)

        # return None is treated as SUCCESS
        if status is not None and status != defs.status.SUCCESS:
            return None

        return adescr

    def __verify_descriptors(self, step, tc):
        final_status = defs.status.SUCCESS
        for dsp in step.expect.descriptors:
            negtest_obj = dsp.descriptor.negtest
            if objects.IsCallback(negtest_obj):
                negtest = negtest_obj.call(tc)
            else:
                negtest = negtest_obj
            edescr = dsp.descriptor.object
            adescr = self.__consume_descriptor(edescr, negtest,
                                               dsp.descriptor.ring,
                                               tc)
            status = self.__verify_one_descriptor(edescr, adescr, negtest, tc)
            if status == defs.status.ERROR:
                final_status = status
        return final_status

    def __verify_configs(self, step, tc):
        for config in step.expect.configs:
            if config.method is not None:
                method = getattr(config.actual_object, config.method)
                if not method:
                    assert 0
                if not method(config.spec):
                    logger.error("Config object verify result = Failed")
                    return defs.status.ERROR
            if not config.original_object.Equals(config.actual_object, tc):
                logger.error("Config object compare result = MisMatch")
                return defs.status.ERROR
        return defs.status.SUCCESS

    def __receive_packets(self, pcr, step, tc):
        max_retries = self.__get_num_retries(tc, pcr.GetExPacketCount())
        for r in range(max_retries):
            mpkts = ModelConnector.Receive()
            for mpkt in mpkts:
                pcr.AddReceived(mpkt.rawpkt, [ mpkt.port ])
            if pcr.GetExPacketCount() == 0:
                #logger.info("0 Packets expected: Waiting for Excess packets")
                break
            else:
                if pcr.GetRxPacketCount() >= pcr.GetExPacketCount():
                    break
                logger.info("RETRY required: ExPktCount:%d RxPktCount:%d" %\
                        (pcr.GetExPacketCount(), pcr.GetRxPacketCount()))

            if self.__is_retry_enabled(tc) == False:
                break
            self.__retry_wait(tc)
        return

    def __add_expected(self, pcr, step, tc):
        for psp in step.expect.packets:
            pkt = psp.packet
            ports = psp.ports
            pcr.AddExpected(pkt.rawbytes, ports, pkt.GID())
        return

    def __add_dummy_rx(self, pcr, step, tc):
        for psp in step.expect.packets:
            pkt = psp.packet
            ports = psp.ports
            pcr.AddReceived(pkt.rawbytes, ports)
        return

    def __verify_packets(self, step, tc):
        pcr = comparators.PacketComparator(tc.GetIgnorePacketFields())
        # Add Expected
        self.__add_expected(pcr, step, tc)
        # Add RX
        if GlobalOptions.dryrun:
            self.__add_dummy_rx(pcr, step, tc)
        else:
            self.__receive_packets(pcr, step, tc)
        tc.rxpkts = copy.deepcopy(pcr.rxpkts)
        pcr.Compare()
        pcr.ShowResults()
        if pcr.IsMatch() == False:
            #if getattr(step.expect, "ignore_excess_packets", False) and \
            #    pcr.HasOnlyExcessPackets():
            #    return defs.status.SUCCESS
            return defs.status.ERROR
        return defs.status.SUCCESS


    def _verify(self, step, tc):
        verify_pass = True

        pstatus = self.__verify_packets(step, tc)
        if pstatus == defs.status.ERROR:
            verify_pass = False

        dstatus = self.__verify_descriptors(step, tc)
        if dstatus == defs.status.ERROR:
            verify_pass = False

        if verify_pass == False:
            return defs.status.ERROR

        cstatus = self.__verify_configs(step, tc)
        if cstatus == defs.status.ERROR:
            verify_pass = False

        return defs.status.SUCCESS

    def __verify_delay(self, step, tc):
        if GlobalOptions.dryrun:
            return
        if step.expect.delay:
            final_delay = step.expect.delay
            if GlobalOptions.rtl:
                final_delay = step.expect.delay * RTL_RETRY_SCALE
            logger.info("Expectation Delay: %d" % final_delay)
            time.sleep(final_delay)
        return

    def Verify(self, step, tc):
        if GlobalOptions.skipverify:
            logger.info("Run with skipverify=True: SKIPPING VERIFICATION")
            return defs.status.SUCCESS
        self.__verify_delay(step, tc)
        return self._verify(step, tc)

class E2EVerifEngineObject(VerifEngineObject):

    def __init__(self):
        super().__init__()

    def __verify_commands(self, step, tc):
        all_status = []
        for cmd in step.expect.commands:
            cmd_status = defs.status.SUCCESS
            if not cmd.status:
                logger.error("Command Failed %s" % cmd.command)
                cmd_status = defs.status.SUCCESS if cmd.ignore_error else defs.status.ERROR 
            else:
                logger.info("Command Success %s" % cmd.command)
            all_status.append(cmd_status)

        if any(status == defs.status.ERROR for status in all_status):
            return defs.status.ERROR
        else:
             return defs.status.SUCCESS

    def _verify(self, step, tc):
        verify_pass = True

        cstatus = self.__verify_commands(step, tc)
        if cstatus == defs.status.ERROR:
           verify_pass = False

        if verify_pass == False:
            return defs.status.ERROR

        return defs.status.SUCCESS

    def Verify(self, step, tc):
        if GlobalOptions.skipverify:
            logger.info("Run with skipverify=True: SKIPPING VERIFICATION")
            return defs.status.SUCCESS
        return self._verify(step, tc)

DolVerifEngine = DolVerifEngineObject()
E2EVerifEngine = E2EVerifEngineObject()
