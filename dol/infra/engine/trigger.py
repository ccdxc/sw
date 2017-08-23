#! /usr/bin/python3

import threading
import time
import pdb
import copy
import sys
import ruamel.yaml as yaml

from collections import defaultdict, OrderedDict
import infra.common.utils as utils
import infra.penscapy.penscapy as penscapy
from infra.common.thread import InfraThreadHandler
from infra.common.connector import Connector, PacketContext, RingContext
from infra.common.logging import logger

from infra.factory.store import FactoryStore
from infra.common.glopts import GlobalOptions

import infra.common.objects as objects
import infra.common.defs as defs
import infra.common.parser as parser
from infra.common.objects import FrameworkObject
from infra.common.utils import convert_object_to_dict, log_exception
from infra.factory.testcase import TestCaseTrigExpPacketObject,\
    TestCaseTrigExpDescriptorObject
from infra.factory.pktfactory import get_factory_id_from_scapy_id

PEN_REF = getattr(penscapy, "PENDOL")

TC_ID_OFFSET_START_FROM_END = -12
TC_ID_OFFSET_END_FROM_END = -8

TC_STEP_ID_OFFSET_START_FROM_END = -7
TC_STEP_ID_OFFSET_END_FROM_END = -6


def get_test_case_id_from_packet(packet):
    ''' Assuming packet in bytes'''
    return int.from_bytes(
        packet[TC_ID_OFFSET_START_FROM_END:TC_ID_OFFSET_END_FROM_END], byteorder='big'), \
        int.from_bytes(
            packet[TC_STEP_ID_OFFSET_START_FROM_END:TC_STEP_ID_OFFSET_END_FROM_END], byteorder='big')


class TestCaseParser(parser.ParserBase):
    def __init__(self, path, filename):
        super().__init__()
        self.path = path
        self.filename = filename
        return

    def parse(self):
        objlist = super().Parse(self.path, self.filename)
        assert(len(objlist) == 1)
        return objlist[0]


TestCaseOutputTemplateObject = None

parser = TestCaseParser(defs.FACTORY_TEMPLATE_TESTOBJECTS_PATH,
                        "testcase_output.template")
TestCaseOutputTemplateObject = parser.parse()
assert(TestCaseOutputTemplateObject != None)


class TriggerReport(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.passed_count = 0
        self.failed_count = 0
        self.pending_count = 0
        self.junk_recvd = False
        self.details = {}


class TriggerTestCaseReport(objects.FrameworkObject):
    def __init__(self, test_case):
        super().__init__()
        self.test_case = test_case
        self.result = None
        self.details = None
        self.id = None


class TestStepResult():

    def __init__(self):
        self.result = None
        self.packets = None
        self.descriptors = None
        self.verify_callback = None


class VerifyCallbackStepResult(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.status = None
        self.message = None


class ObjectsTestStepResult(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.extra = []
        self.missing = []
        self.mismatch = []
        self.matched = []
        self.status = None

    def _set_status(self):
        self.status = Trigger.TEST_CASE_FAILED  \
            if any(self.extra or self.mismatch or self.missing) else \
            Trigger.TEST_CASE_PASSED


class PacketsTestStepResult(ObjectsTestStepResult):
    def __init__(self):
        ObjectsTestStepResult.__init__(self)


class DescriptorsTestStepResult(ObjectsTestStepResult):
    def __init__(self):
        super().__init__()


class ObjectMismatchResult(objects.FrameworkObject):
    def __init__(self, result_type):
        super().__init__()
        self.id = None
        self.expected = None
        self.actual = None
        self.mismatch = None
        self._type = result_type


class PacketMismatchResult(ObjectMismatchResult):
    def __init__(self):
        ObjectMismatchResult.__init__(self, "packet")


class DescriptorMismatchResult(ObjectMismatchResult):
    def __init__(self):
        ObjectMismatchResult.__init__(self, "descriptor")


class ObjectMissingResult(objects.FrameworkObject):
    def __init__(self, missing_type):
        super().__init__()
        self._type = missing_type


class PacketMissingResult(ObjectMissingResult):

    def __init__(self):
        ObjectMissingResult.__init__(self, "packet")
        self.spkt = None
        self.ports = None


class DescriptorMissingResult(ObjectMissingResult):

    def __init__(self):
        ObjectMissingResult.__init__(self, "packet")


class ObjectResult(objects.FrameworkObject):
    def __init__(self, result_type):
        super().__init__()
        self._type = result_type


class PacketResult(ObjectResult):

    def __init__(self):
        ObjectResult.__init__(self, "packet")
        self.spkt = None
        self.port = None


class DescriptorResult(ObjectResult):

    def __init__(self):
        ObjectResult.__init__(self, "descriptor")
        self.object = None


class PacketCompareResult(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.extra_hdrs = []
        self.missing_hdrs = []
        self.headers = OrderedDict()

    def matched(self):
        return not self.extra_hdrs and not self.missing_hdrs and not self.headers


class TriggerObjectCompareResult(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.extra_fields = []
        self.missing_fields = []
        self.mismatch_fields = {}

    def not_empty(self):
        return self.extra_fields or self.mismatch_fields or self.mismatch_fields


class TriggerFieldCompareResult(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.expected = None
        self.actual = None

    def not_empty(self):
        return self.expected or self.actual


def PacketCompare(expected, actual, partial_match=None):

    def _get_layers(spkt):
        hdrs = []
        p = spkt
        while p.name != 'NoPayload':
            hdrs.append(p.__class__.__name__)
            p = p.payload
        return hdrs
        # return [header.strip().split(" ")[0] for header in
        # spkt.summary().split("/")]

    result = PacketCompareResult()

    act_layers = _get_layers(actual)
    exp_layers = _get_layers(expected)
    if not partial_match:
        result.extra_hdrs = set(act_layers) - set(exp_layers)
    else:
        exp_layers = set(exp_layers) - \
            set([hdr for hdr, value in partial_match.ignore_hdrs.items() if not value])
    result.missing_hdrs = set(exp_layers) - set(act_layers)
    match_hdrs = set(exp_layers) & set(act_layers)
    # Matched headers.
    match_hdrs = [x for x in exp_layers if x in match_hdrs]
    result.headers = defaultdict(lambda: {})
    for header in match_hdrs:
        hdr_result = TriggerObjectCompareResult()
        header_info = FactoryStore.headers.Get(
            get_factory_id_from_scapy_id(header))
        if not header_info:
            assert 0
        scapy_ref = getattr(penscapy, header)
        if not scapy_ref:
            assert 0
        scapy_fields = [
            field for field in header_info.fields.__dict__.keys()]
        actual_fields = [field.name for field in actual[scapy_ref].fields_desc]
        #actual_fields = [field.name for field in fields]
        expected_fields = [
            field.name for field in expected[scapy_ref].fields_desc]
        if not partial_match or not partial_match.ignore_hdrs.get(header):
            match_fields = scapy_fields
            extra_field_present = set(actual_fields) - set(expected_fields)
            if extra_field_present:
                hdr_result.extra_fields.extend(extra_field_present)
        else:
            match_fields = set(
                expected_fields) - set(partial_match.ignore_hdrs[header].ignore_fields)

        for field in match_fields:
            try:
                expected_val = getattr(expected[scapy_ref], field)
            except AttributeError:
                try:
                    actual_val = getattr(actual[scapy_ref], field)
                except AttributeError:
                    # Even the actual does not have the field, skip it.
                    continue
                hdr_result.extra_fields.append(field)
                continue
            try:
                actual_val = getattr(actual[scapy_ref], field)
            except AttributeError:
                hdr_result.missing_fields.append(field)
                continue
            if actual_val != expected_val:
                field_compare = TriggerFieldCompareResult()
                field_compare.actual, field_compare.expected = actual_val, expected_val
                hdr_result.mismatch_fields[field] = field_compare

        result.headers[header] = hdr_result

    # If all the headers matched, just set the result to empty.
    if all(not result.not_empty() for result in result.headers.values()):
        result.headers = {}

    return result


def ObjectCompare(actual, expected, partial_match=None):

    # Compare python objects and return result
    # For now ignore functions, assume no extra or missing fields checks.

    if not hasattr(actual, "__dict__"):
        field_result = TriggerFieldCompareResult()
        if expected != actual:
            field_result.expected, field_result.actual = expected, actual
        return field_result

    result = TriggerObjectCompareResult()
    for field, expected_val in expected.__dict__.items():
        if field.startswith("_"):
            continue
        actual_val = actual.__dict__[field]
        if isinstance(actual_val, list):
            list_result = []
            for actual_li, expected_li in zip(actual_val, expected_val):
                field_result = ObjectCompare(
                    actual_li, expected_li, partial_match)
                if field_result.not_empty():
                    list_result.append(field_result)
                else:
                    list_result.append(None)
            if any(list_result):
                result.mismatch_fields[field] = list_result
        elif not hasattr(actual_val, "__dict__"):
            if expected_val != actual_val:
                field_result = TriggerFieldCompareResult()
                field_result.expected, field_result.actual = expected_val, actual_val
                result.mismatch_fields[field] = field_result
        else:
            field_result = ObjectCompare(
                actual_val, expected_val, partial_match)
            if field_result.not_empty():
                result.mismatch_fields[field] = field_result

    return result


class TriggerTestCaseStep(objects.FrameworkObject):

    STATUS_QUEUED = 0
    STATUS_COMPLETED = 1
    STATUS_RUNNING = 2
    TEST_STEP_TIMEOUT = 5

    class ExpectedReceivedData(object):

        def __init__(self):
            self.expected = []
            self.received = []

    def __init__(self, test_case, test_step_spec, step_count, set_timeout, logger):
        super().__init__()
        self._tc = test_case
        self._tc_step = test_step_spec
        self._tc_step_status = self.STATUS_QUEUED
        self._connector = self._tc._connector
        self._start_ts = None
        self._logger = logger
        self._tc_status = None
        # Set timeout only if asked.
        # Typically last step or the only step will not be set
        # any timeout and will rely on trigger report call to cancel this.
        # This will be useful for the cases where there is only 1 test steps
        # and all of them have to be stopped based on global timeout called
        # by trigger.
        self._step_timeout = None if not set_timeout else (getattr(
            test_step_spec.expect, "timeout", None) or self.TEST_STEP_TIMEOUT)
        self._exp_rcv_data = defaultdict(
            lambda: TriggerTestCaseStep.ExpectedReceivedData())
        self._exp_rcv_descr = defaultdict(
            lambda: TriggerTestCaseStep.ExpectedReceivedData())

        for exp_pkt in self._tc_step.expect.packets[:]:
            # print(exp_pkt.__dict__)
            assert(exp_pkt.ports)
            for port in exp_pkt.ports:
                self._exp_rcv_data[port].expected.append(exp_pkt)
        for descriptor in self._tc_step.expect.descriptors:
            self._exp_rcv_descr[descriptor.descriptor.ring].expected.append(
                descriptor.descriptor)
        self._step_count = step_count

    def run_test_step(self):
        if self._tc_step is not self.STATUS_RUNNING:
            self._start_ts = time.time()
            self._timeout = self._start_ts + \
                (self._step_timeout or sys.maxsize)
            self._logger.info("Running test case step : %s" % self._step_count)
            # Set in the test case object which step is being executed.
            self._tc._test_spec.current_step = self._tc_step
            self._tc._test_spec.current_step.received.packets = []
            self._tc._test_spec.current_step.received.descriptors = []

            for exp_pkt in self._tc_step.expect.packets:
                assert exp_pkt.packet
                exp_pkt.packet.spkt = penscapy.Parse(
                    bytes(exp_pkt.packet.spkt))
            for packet in self._tc_step.trigger.packets:
                assert packet.packet
                sdata = bytes(packet.packet.spkt)
                # TODO: Retransmit count should be attribute of ports.
                for _ in range(1):
                    assert(packet.ports)
                    for port in packet.ports:
                        self._logger.info(
                            "Sending Input Packet (id=%s) of len:%d, Port = %d" % (packet.packet.GID(), len(sdata), port))
                        self._connector.send(sdata, port)
            ring_set = set()
            for descriptor in self._tc_step.trigger.descriptors:
                assert descriptor.descriptor and descriptor.descriptor.object and descriptor.descriptor.ring
                self._logger.info("Posting to ring = %s, descriptor(id=%s) = %s" % (
                    descriptor.descriptor.ring, descriptor.descriptor.GID(), descriptor.descriptor.object))
                descriptor.descriptor.ring.post(descriptor.descriptor.object)
                ring_set.add(descriptor.descriptor.ring)

            if (hasattr(self._tc_step.trigger, "doorbell") and
                self._tc_step.trigger.doorbell and
                    self._tc_step.trigger.doorbell.object):
                for ring in ring_set:
                    self._logger.info("Posting doorbell %s" %
                                      self._tc_step.trigger.doorbell.object)
                    self._connector.doorbell(
                        self._tc_step.trigger.doorbell, ring)
            if self._exp_rcv_descr:
                self._connector.consume_rings(self._exp_rcv_descr.keys())

            self._tc_status = self.STATUS_RUNNING
        return

    def recv_packet(self, packet_ctx):
        self._logger.info("Received packet on port %s, len:%d" %
                          (packet_ctx.port, len(packet_ctx.packet)))
        self._exp_rcv_data[packet_ctx.port].received.append(packet_ctx)
        # Add to test step received section.
        tc_pkt = TestCaseTrigExpPacketObject()
        tc_pkt.packet = packet_ctx.packet
        tc_pkt.ports = [packet_ctx.port]
        self._tc._test_spec.current_step.received.packets.append(tc_pkt)

    def recv_descriptors(self, ring, descriptors):
        for descr_obj in descriptors:
            self._logger.info(
                "Received descriptor on ring = %s, descriptor = %s" % (ring, descr_obj))
            self._exp_rcv_descr[ring].received.append(descr_obj)
            # Add to test step received section.
            tc_desc = TestCaseTrigExpDescriptorObject()
            tc_desc.object = descr_obj
            tc_desc.ring = ring
            self._tc._test_spec.current_step.received.descriptors.append(
                tc_desc)

    @classmethod
    def _packets_match(cls, expected, actual, partial_match):
        match_result = PacketCompare(expected, actual, partial_match)
        if not match_result.matched():
            return False, match_result
        return True, match_result

    @staticmethod
    def __min_mismatch_result(mismatch_res1, mismatch_res2):

        # All headers are orderred.
        for (h1, r1), (h2, r2) in zip(mismatch_res1.headers.items(), mismatch_res2.headers.items()):
            assert h1 == h2
            if not r1.not_empty():
                if not r2.not_empty():
                    # Both headers match.
                    continue
                else:
                    # R2 has some mistmatch, hence res1 is better.
                    return mismatch_res1
            elif not r2.not_empty():
                # R2 is empty but R1 is not, hence mismatch_res2 is better.
                return mismatch_res2
            else:
                # Both are not empty.
                if len(r1.mismatch_fields) != len(r2.mismatch_fields):
                    return mismatch_res2 if len(r1.mismatch_fields) > len(r2.mismatch_fields) else mismatch_res1

        # If both are same then return the first one, caller is expected to
        # check if same.
        return mismatch_res1

    @staticmethod
    def _convert_mismatch_result(mismatch_result):
        class Struct:
            def __init__(self, entries):
                self.__dict__.update(entries)
                for k in list(self.__dict__.keys()):
                    v = self.__dict__[k]
                    if isinstance(v, dict):
                        setattr(self, k, Struct(v))
                    elif "Framework" in k:
                        delattr(self, k)
                    if isinstance(v, FrameworkObject):
                        setattr(self, k, Struct(v.__dict__))

        return Struct(mismatch_result)

    def __packets_process_result(self):
        # Hack for now, ignore the IPV4 checksum!
        ignore_pkt_cmp = None
        if True or hasattr(self._tc._test_spec, "ignore_ipv4_checksum") and self._tc._test_spec.ignore_ipv4_checksum:
            ignore_pkt_cmp = objects.PacketComparePartial()
            hdr_ignore_result = objects.HeaderComparePartial()
            hdr_ignore_result.ignore_fields = ["chksum"]
            ignore_pkt_cmp.ignore_hdrs["IP"] = hdr_ignore_result

        self._logger.verbose("Processing test step packets result")
        result = PacketsTestStepResult()
        tmp_mismatch_ref = {}
        for port in self._exp_rcv_data.keys():
            recvd_packets = []
            for packet_ctx in self._exp_rcv_data[port].received:
                try:
                    new_packet_ctx = PacketContext(
                        None, penscapy.Parse(packet_ctx.packet), packet_ctx.port)
                    recvd_packets.append(new_packet_ctx)
                    new_packet_ctx.packet.show2(
                        indent=0, label_lvl=self._logger.GetLogPrefix())
                except:
                    extra_pkt = PacketResult()
                    # TODO : Decide how to store this.
                    extra_pkt.spkt, extra_pkt.port = None, packet_ctx.port
                    result.extra.append(extra_pkt)
                    self._logger.info("Received unparsable excess packet on port = %s of len:%s" % (
                        packet_ctx.port, len(packet_ctx.packet)))
                    continue
            while recvd_packets:
                mismatch_result, closest_packet_match = None, None
                packet_ctx = recvd_packets.pop()
                packet = packet_ctx.packet
                for exp_pkt in self._exp_rcv_data[port].expected[:]:
                    # TODO :Add Partial Match.
                    match, match_result = self._packets_match(
                        exp_pkt.packet.spkt, packet, ignore_pkt_cmp)
                    if not match:
                        if match_result.missing_hdrs or match_result.extra_hdrs:
                                # If packet has  missing or extra headers
                                # ignore.
                            continue
                        if mismatch_result:
                            new_mismatch_result = self.__min_mismatch_result(
                                mismatch_result, match_result)
                            if new_mismatch_result == mismatch_result:
                                # Mismatch is the same, don't change anything
                                continue
                            mismatch_result = new_mismatch_result
                        else:
                            mismatch_result = match_result
                            # Found a better mismatch with respected to expected
                            # packet.
                        if exp_pkt in tmp_mismatch_ref:
                            # Expected packet is already has a mismatched
                            # packet.
                            cur_exp_pkt_mismatch_result = tmp_mismatch_ref[exp_pkt][1]
                            new_mismatch_result = self.__min_mismatch_result(cur_exp_pkt_mismatch_result,
                                                                             mismatch_result)
                            if new_mismatch_result == cur_exp_pkt_mismatch_result:
                                # Have to invalidate this mismatch so that we
                                # can try to match with some other packet.
                                mismatch_result = None
                                continue
                            # Better match, move the current packet to be checked again.
                            # And delete as we found a better mismatch.
                            mismatch_result = new_mismatch_result
                            recvd_packets.append(
                                tmp_mismatch_ref[exp_pkt][0])
                            del tmp_mismatch_ref[exp_pkt]
                        # Found a best match for the packet, will be added
                        # after
                        mismatch_result, closest_packet_match = mismatch_result, exp_pkt
                    else:
                        if exp_pkt in tmp_mismatch_ref:
                            # Fully matched, move the current packet to be checked again.
                            # And delete as we found a better mismatch.
                            recvd_packets.append(
                                tmp_mismatch_ref[exp_pkt][0])
                            del tmp_mismatch_ref[exp_pkt]
                        self._logger.info("Matched packet (id=%s) on port = %s of len:%s" % (exp_pkt.packet.GID(),
                                                                                             packet_ctx.port, len(packet_ctx.packet)))
                        self._exp_rcv_data[port].expected.remove(exp_pkt)
                        matched_pkt = PacketResult()
                        matched_pkt.spkt, matched_pkt.port = packet_ctx.packet, packet_ctx.port
                        result.matched.append(matched_pkt)
                        # Remove this packet for port channel case.
                        for other_port in exp_pkt.ports:
                            if port != other_port:
                                self._exp_rcv_data[other_port].expected.remove(
                                    exp_pkt)
                        break
                else:
                    if closest_packet_match:
                        # Packet did not match anything,
                        # associate with closest packet which matched as
                        # mismatched.
                        tmp_mismatch_ref[closest_packet_match] = (
                            packet_ctx, mismatch_result)
                    else:
                        # If did not find the closest match, then its an excess
                        # packet.
                        extra_pkt = PacketResult()
                        extra_pkt.spkt, extra_pkt.port = packet_ctx.packet, packet_ctx.port
                        result.extra.append(extra_pkt)
                        self._logger.info("Received excess packet on port = %s of len:%s" % (
                            extra_pkt.port, len(packet_ctx.packet)))

        for key, res in tmp_mismatch_ref.items():
            actual_packet = PacketResult()
            expected_packet = PacketMissingResult()
            actual_packet.spkt, actual_packet.port = res[0].packet, res[0].port
            expected_packet.spkt, expected_packet.ports = key.packet.spkt, key.ports
            mismatch_result = PacketMismatchResult()
            mismatch_result.id, mismatch_result.expected, mismatch_result.actual = key.packet.GID(
            ), expected_packet, actual_packet

            # Remove header mismatches which are empty.
            for hdr in list(res[1].headers.keys()):
                hdr_result = res[1].headers[hdr]
                if not hdr_result.not_empty():
                    del res[1].headers[hdr]
            mismatch_result.mismatch = self._convert_mismatch_result(
                res[1].__dict__)
            result.mismatch.append(mismatch_result)
            self._logger.info("Mismatch packet id = %s, expected ports:%s len:%s, actual port:%s len: %s" % (key.packet.GID(),
                                                                                                             expected_packet.ports,
                                                                                                             len(
                                                                                                                 expected_packet.spkt),
                                                                                                             actual_packet.port,
                                                                                                             len(actual_packet.spkt)))
            for header in res[1].headers:
                self._logger.info("Header mismatch : %s" % header)
                for field, mis_result in res[1].headers[header].mismatch_fields.items():
                    self._logger.info("\tMismatch field : %s,  expected : %s, actual : %s" %
                                      (field, mis_result.expected, mis_result.actual))

        tmp_missing_ref = defaultdict(lambda: [])
        for port in self._exp_rcv_data.keys():
            expected_packets = self._exp_rcv_data[port].expected
            for packet in expected_packets:
                if packet not in tmp_mismatch_ref:
                    tmp_missing_ref[packet].append(port)

        for packet_ctx in tmp_missing_ref:
            missing_pkt = PacketMissingResult()
            missing_pkt.spkt = packet_ctx.packet.spkt
            missing_pkt.ports = tmp_missing_ref[packet_ctx]
            result.missing.append(missing_pkt)
            self._logger.info("Missing packet id = %s, expected on ports = %s of len:%s" % (packet_ctx.packet.GID(),
                                                                                            missing_pkt.ports, len(packet_ctx.packet.spkt)))

        result._set_status()
        return result

    def __descriptors_process_result(self):
        result = DescriptorsTestStepResult()
        for ring in self._exp_rcv_descr.keys():
            for expected, received in zip(self._exp_rcv_descr[ring].expected,
                                          self._exp_rcv_descr[ring].received):

                obj_compare_result = ObjectCompare(
                    received, expected.object, partial_match=None)
                if obj_compare_result.not_empty():
                    mismatch_result = DescriptorMismatchResult()
                    mismatch_result.id, mismatch_result.expected, mismatch_result.actual = expected.GID(
                    ), expected.object, received
                    mismatch_result.mismatch = obj_compare_result
                    result.mismatch.append(mismatch_result)
                else:
                    matched = DescriptorResult()
                    matched.object = expected.object
                    result.matched.append(matched)

            for expected in self._exp_rcv_descr[ring].expected[len(self._exp_rcv_descr[ring].received):]:
                missing = DescriptorResult()
                missing.object = expected.object
                result.missing.append(missing)

            for expected in self._exp_rcv_descr[ring].received[len(self._exp_rcv_descr[ring].expected):]:
                extra = DescriptorResult()
                extra.object = expected
                result.extra.append(extra)

        result._set_status()
        return result

    def __verify_callback_process_result(self):
        pass

    def __process_result(self):
        self._logger.verbose("Processing test step result")
        result = TestStepResult()
        result.packets = self.__packets_process_result()
        result.descriptors = self.__descriptors_process_result()
        result.verify_callback = VerifyCallbackStepResult()
        if hasattr(self._tc._test_spec, "verify_callback") and self._tc._test_spec.verify_callback:
            cb_result = self._tc._test_spec.verify_callback()
            if cb_result == defs.status.SUCCESS:
                self._logger.info(
                    "TestCaseVerify(): Success.")
                result.verify_callback.status = Trigger.TEST_CASE_PASSED
            else:
                self._logger.info(
                    "TestCaseVerify(): Failure.")
                result.verify_callback.status = Trigger.TEST_CASE_FAILED

        if result.packets.status == Trigger.TEST_CASE_FAILED or \
                result.descriptors.status == Trigger.TEST_CASE_FAILED or \
                result.verify_callback.status == Trigger.TEST_CASE_FAILED:
            result.result = Trigger.TEST_CASE_FAILED
            return result
        else:
            result.result = Trigger.TEST_CASE_PASSED
            return result

    def timed_out(self):
        return self._timeout and self._timeout < time.time()

    def get_result(self, force=False):
        result = TestStepResult()
        if self._tc_status != self.STATUS_RUNNING:
            result.result = Trigger.TEST_CASE_NOT_RUN
        elif force or self.timed_out():
            # Test case timed out or user forcing to read the current status.
            try:
                result = self.__process_result()
            except Exception as ex:
                log_exception(self._logger)
                print(ex)
                self._logger.info("Error proceessing result", ex)
        else:
            result.result = Trigger.TEST_CASE_PENDING
        return result


class TriggerTestCase(objects.FrameworkObject):

    STATUS_QUEUED = 0
    STATUS_COMPLETED = 1
    STATUS_RUNNING = 2

    def __init__(self, connector, test_case_spec, logger, run_test_steps_serially=False):
        super().__init__()
        self._test_spec = test_case_spec
        self._connector = connector
        self._test_steps = []
        self._start_time = None
        self._logger = logger
        for i, step in enumerate(test_case_spec.session, 1):
            self._test_steps.append(
                TriggerTestCaseStep(self, step.step, i, i != len(test_case_spec.session),
                                    logger))
        self._tc_status = self.STATUS_QUEUED
        self._current_tc_step = None
        self._step_results = []
        self._run_test_steps_serially = run_test_steps_serially

    def run_test_case(self):
        if self._tc_status is not self.STATUS_RUNNING:
            self._start_ts = time.time()
            self._logger.verbose("Started running.")
            self._tc_status = self.STATUS_RUNNING
        else:
            self._logger.info("Continuing to next step")
        if not self._run_test_steps_serially:
            for tc_step in self._test_steps:
                tc_step.run_test_step()
        else:
            tc_step = self._test_steps[0]
            self._current_tc_step = tc_step
            tc_step.run_test_step()
        return

    def needs_serial_run(self):
        for step in self._test_spec.session:
            if step.step.trigger.descriptors or step.step.expect.descriptors:
                return True

        return False

    def _all_steps_triggered(self):
        return not self._test_steps

    def recv_packet(self, packet_ctx, step_id):
        if self._run_test_steps_serially and self._current_tc_step:
            self._current_tc_step.recv_packet(packet_ctx)
        else:
            self._test_steps[step_id].recv_packet(packet_ctx)

    def recv_descriptors(self, ring, descriptors):
        if self._current_tc_step:
            self._current_tc_step.recv_descriptors(ring, descriptors)
        else:
            assert len(self._test_steps) == 1
            self._test_steps[0].recv_descriptors(ring, descriptors)

    def get_current_step_result(self):
        return self._current_tc_step.get_result()

    def get_result(self, force=False):
        # All test steps are run in parallel or all steps triggered.
        # Force should be set to true if we are collecting all the results.
        final_result = Trigger.TEST_CASE_PASSED
        step_results = []
        for tc_step in self._test_steps:
            result = tc_step.get_result(force)
            if result.result in [Trigger.TEST_CASE_FAILED, Trigger.TEST_CASE_NOT_RUN]:
                final_result = Trigger.TEST_CASE_FAILED
            step_results.append(result)

        return final_result, step_results


class Scheduler(InfraThreadHandler):

    def __init__(self, endp_connector):
        super(Scheduler, self).__init__(name="Trigger Scheduler",
                                        max_idle_interval=5, max_time_per_run=200)
        self.endp_connector = endp_connector


class Receiver(InfraThreadHandler):

    def __init__(self, trigger, endp_connector):
        super(Receiver, self).__init__(name="Trigger Receiver",
                                       max_idle_interval=0, max_time_per_run=200)
        self._endp_connector = endp_connector
        self._trigger = trigger

    def execute(self, event):
        try:
            data = self._endp_connector.recv()
        except Connector.Timeout:
            return
        self._trigger.eventEnqueue(data)


class Trigger(InfraThreadHandler):

    TEST_CASE_PASSED = "Passed"
    TEST_CASE_FAILED = "Failed"
    TEST_CASE_PENDING = "Pending"
    TEST_CASE_NOT_RUN = "NotRun"

    def __init__(self, connector='ModelConnector', addr=utils.FakeModelSockAddr(), run_test_steps_serially=True):
        super(Trigger, self).__init__(name="Trigger MAIN",
                                      max_idle_interval=1, max_time_per_run=200)
        if GlobalOptions.dryrun:
            logger.info("Connecting trigger to dumb connector....")
            self._connector = Connector.factory('DumbConnector')(addr)
        else:
            self._connector = Connector.factory(connector)(addr)
        self._scheduler = Scheduler(self._connector)
        self._receiver = Receiver(self, self._connector)
        self.__db_inits()
        self._descriptor_test_case_queue = []
        self._current_descriptor_test_case = None
        self._connector_started = False
        self._run_test_steps_serially = run_test_steps_serially

    def __db_inits(self):
        self._tc_status_dict = {}
        self._tc_done_db = {}
        self._tc_db_lock = threading.Lock()
        self._tc_db = dict()
        self._junk_packets = []

    def run_test_case(self, test_case):
        if GlobalOptions.dryrun:
            test_case.status = defs.status.SUCCESS
            return

        # tc.input.show(self.logger)
        trigger_tc = TriggerTestCase(
            self._connector, test_case, test_case, self._run_test_steps_serially)
        with self._tc_db_lock:
            if test_case.GID() in self._tc_db:
                pdb.set_trace()
                assert 0
            #print (test_case.input.spkt.show2())
            if not len(test_case.session):
                assert 0
            if trigger_tc.needs_serial_run():
                test_case.logger.debug("Test case need a serial run")
                if self._current_descriptor_test_case or self.pending_test_case_count():
                    # Current serial test case running or there are pending
                    # test cases still.
                    self._descriptor_test_case_queue.append(trigger_tc)
                    return
                self._current_descriptor_test_case = trigger_tc
            self._tc_db[test_case.GID()] = trigger_tc
            logger.debug("Test case ID ", test_case.GID())
            self._connector_start()
            trigger_tc.run_test_case()
        return

    def execute(self, event):

        def __send_to_current_step(data):
            if self._current_descriptor_test_case:
                self._current_descriptor_test_case.recv_packet(pkt_ctx)

        if event:
            if isinstance(event, PacketContext):
                pkt_ctx = event
                try:
                    tc_id, step_id = get_test_case_id_from_packet(
                        pkt_ctx.packet)
                    with self._tc_db_lock:
                        if tc_id in self._tc_db:
                            trigger_test_case = self._tc_db[tc_id]
                            trigger_test_case._logger.debug(
                                "Got a packet to process.")
                            trigger_test_case.recv_packet(pkt_ctx, step_id)
                        else:
                            logger.critical(
                                "Received packet with unknown test case ID :", tc_id)
                            if self._current_descriptor_test_case:
                                self._current_descriptor_test_case.recv_packet(
                                    pkt_ctx)
                            else:
                                self._junk_packets.append(pkt_ctx)
                except:
                    if self._current_descriptor_test_case:
                        self._current_descriptor_test_case.recv_packet(
                            pkt_ctx, 0)
                    else:
                        logger.critical(
                            "No Pensando header found in the packet")
                        self._junk_packets.append(pkt_ctx)
            elif isinstance(event, RingContext):
                if self._current_descriptor_test_case:
                    ring_ctx = event
                    self._current_descriptor_test_case.recv_descriptors(
                        ring_ctx.ring, ring_ctx.descriptors)
            else:
                assert 0

            return

    def _scheduler_running(self):
        return self._scheduler.started

    def _receiver_running(self):
        return self._receiver.started

    def pending_test_case_count(self):
        return len(self._tc_db)

    def test_case_running(self, test_case_id):
        with self._tc_db_lock:
            return test_case_id in self._tc_db

    def start(self):
        super(Trigger, self).startThread()
        logger.info("Trigger Started.")
        parser = TestCaseParser(defs.FACTORY_TEMPLATE_TESTOBJECTS_PATH,
                                "testcase_output.template")
        TestCaseOutputTemplateObject = parser.parse()
        assert(TestCaseOutputTemplateObject != None)

    def _connector_start(self):
        if not self._connector_started:
            self._scheduler.startThread()
            self._receiver.startThread()
            # First time start, give it some time.
            # TODO: May be check in loop for MAX interval and assert if not
            # started.
            time.sleep(0.5)
            self._connector_started = True

    def _connector_stop(self):
        if self._connector_started:
            self._scheduler.stopThread()
            self._receiver.stopThread()
            self._connector_started = False

    def stop(self):
        self._connector_stop()
        super(Trigger, self).stopThread()
        logger.info("Trigger Stopped.")

    def eventTimeout(self):
        if GlobalOptions.dryrun:
            return

        # If not serial run, get out.
        if not self._run_test_steps_serially:
            return
        with self._tc_db_lock:
            for tc_id in list(self._tc_db.keys()):
                test_case = self._tc_db[tc_id]
                result = self._tc_db[tc_id].get_current_step_result()
                if result.result is Trigger.TEST_CASE_PENDING:
                    # Current Test step still pending.
                    continue
                elif not test_case._all_steps_triggered() and result.result == Trigger.TEST_CASE_PASSED:
                    # Move forward the test case if current test step passed.
                    test_case._test_steps.pop(0)
                    test_case.run_test_case()
                else:
                    if self._tc_db[tc_id] == self._current_descriptor_test_case:
                        self._current_descriptor_test_case = None

                    # Test case failed or all steps done, move it complete.
                    self._tc_done_db[tc_id] = test_case
                    del self._tc_db[tc_id]
                    if self.pending_test_case_count() == 0:
                        if not self._current_descriptor_test_case and self._descriptor_test_case_queue:
                            self._current_descriptor_test_case = self._descriptor_test_case_queue.pop(
                                0)
                            self._tc_db[self._current_descriptor_test_case.meta.id] = self._current_descriptor_test_case
                            self._current_descriptor_test_case.run_test_case()
                        else:
                            # All test cases completed, stop the connector.
                            self._connector_stop()

    def get_result(self, tc_id):
        if tc_id in self._tc_db:
            raise Exception("Test case still running!")
        if tc_id not in self._tc_done_db:
            raise Exception("Test id not present.!")
        test_case = self._tc_done_db.get(tc_id)
        if test_case:
            return test_case.get_result()
        return None, None

    def reset(self):
        logger.info("Resetting Trigger..")
        # Forgetting about all testcase is the only reset for now.
        self.__db_inits()

    def __compile_result_section(self, dst_step, src_step, details):

        def __gen_copy_packet_result_section_(result, obj_type, section):
            obj_type_to_attribute = {"packets": "packet",
                                     "descriptors": "descriptor"}
            dst_matched_obj = getattr(
                getattr(dst_step.step.result, obj_type), section)[0]
            dst_matched_objs = [copy.deepcopy(
                dst_matched_obj) for _ in getattr(result, section)]
            setattr(getattr(dst_step.step.result, obj_type),
                    section, dst_matched_objs)
            for dst_matched_packet, src_matched_packet in zip(dst_matched_objs, getattr(result, section)):
                dst_obj = getattr(dst_matched_packet,
                                  obj_type_to_attribute[obj_type])
                dst_obj.__dict__.update(src_matched_packet.__dict__)

        for dst_packet, src_packet in zip(dst_step.step.trigger.packets,  src_step.step.trigger.packets):
            #dst_packet.packet.id = src_packet.packet.GID()
            dst_packet.packet.spkt = src_packet.packet.spkt
            dst_packet.ports = src_packet.ports
            #dst_packet.count = src_packet.count

        if details.result != Trigger.TEST_CASE_NOT_RUN:
            for obj_type in ["packets", "descriptors"]:
                result = getattr(details, obj_type)
                for section in["matched", "missing", "mismatch", "extra"]:
                    __gen_copy_packet_result_section_(
                        result,  obj_type, section)
            dst_step.step.result.callback.status = details.verify_callback.status
            dst_step.step.result.callback.message = details.verify_callback.message
        else:
            dst_step.step.result = Trigger.TEST_CASE_NOT_RUN

    def __gen_test_report(self, tc_id):
        test_case_out = copy.deepcopy(TestCaseOutputTemplateObject)
        test_case_out.id = tc_id
        test_case_out.status = self._tc_status_dict[tc_id].result
        test_case_out.steps_passed = sum(1 for step_result in self._tc_status_dict[tc_id].details
                                         if step_result.result == Trigger.TEST_CASE_PASSED)

        step = TestCaseOutputTemplateObject.session[0]
        steps = [copy.deepcopy(
            step) for _ in self._tc_status_dict[tc_id].test_case._test_spec.session]
        test_case_out.session = steps
        # TODO
        index = 0
        for dst_step, src_step in zip(steps,  self._tc_status_dict[tc_id].test_case._test_spec.session):
            for packet in src_step.step.trigger.packets:
                packet.packet.spkt = packet.packet.spkt
            self.__compile_result_section(
                dst_step, src_step,  self._tc_status_dict[tc_id].details[index])
            index += 1
            if len(self._tc_status_dict[tc_id].details) <= index:
                break

        return test_case_out

    def get_run_report(self, max_wait_time=1):
        end_wait_time = time.time() + max_wait_time
        logger.info("Trying to get Trigger Report..")
        while self.pending_test_case_count() and end_wait_time > time.time():
            time.sleep(0.5)

        # Stop the connector as we forced every test case to complete.
        self._connector_stop()
        time.sleep(1)
        with self._tc_db_lock:
            for tc_id in self._tc_db:
                self._tc_done_db[tc_id] = self._tc_db[tc_id]
            self._tc_db = {}

        report = TriggerReport()
        for tc_id in self._tc_done_db:
            result, details = self._tc_done_db[tc_id].get_result(force=True)
            tc_report = TriggerTestCaseReport(self._tc_done_db[tc_id])
            tc_report.id, tc_report.result, tc_report.details = tc_id, result, details
            self._tc_status_dict[tc_id] = tc_report

            if self._tc_status_dict[tc_id].result == Trigger.TEST_CASE_PASSED:
                report.passed_count += 1
            else:
                report.failed_count += 1
            # pdb.set_trace()
            report.details[tc_id] = self.__gen_test_report(tc_id)

        if self._junk_packets:
            logger.info("Received junk packets in this run.")
            report.junk_recvd = True
        logger.info("Returning Trigger Report in testcase output format")
        return report

    def get_dict_report(self):
        report = self.get_run_report()
        dict_report = convert_object_to_dict(report, ignore_keys=[])
        with open('/tmp/data.yml', 'w') as outfile:
            yaml.dump(dict_report, outfile, default_flow_style=False)
        logger.info("Returning Trigger Report in dictionary format")
        return dict_report


TrigExpEngine = Trigger()


def start():
    TrigExpEngine.start()
