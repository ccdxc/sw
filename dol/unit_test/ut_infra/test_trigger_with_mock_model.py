'''
Created on Jul 30, 2017

@author: sudhiaithal
'''
import os
import time
from copy import copy
import subprocess
from unit_test.ut_infra.test_trigger import DolTriggerTest, init_trigger_test
from infra.engine.trigger import Trigger
from infra.common import defs
from factory.objects.ut.buffer import InfraUtBufferObject
from factory.objects.ut.descriptor import InfraUtTxDescriptorObject, InfraUtRxDescriptorObject
from infra.common.objects import MacAddressBase
from factory.objects.ut.doorbell import InfraUtDoorbell
import model_sim.src.model_wrap as model_wrap
from config.objects.ring import RingObject
from infra.common.logging import logger
from infra.common.utils import convert_object_to_dict
from copy import copy
from macpath import split
from pickletools import read_uint1


class TestModel():

    def __init__(self):
        self.proc = None

    def start(self):
        mock_model = os.environ["WS_TOP"] + \
            defs.DOL_PATH + "mockmodel/mockmodel"
        self.proc = subprocess.Popen([mock_model], shell=False)

    def stop(self):
        print("Kill mock", self.proc.pid)
        subprocess.call(["kill", "-9", "%d" % self.proc.pid])


class DolTriggerTestWithMockModel(DolTriggerTest):

    def setUp(self):
        model = TestModel()
        trig_obj = Trigger()
        super(DolTriggerTestWithMockModel, self).setUp(trig_obj, model)
        global PEN_REF
        from infra.engine.trigger import PEN_REF

    def tearDown(self):
        super(DolTriggerTestWithMockModel, self).tearDown()

    def test_init_buffer(self):
        buff = InfraUtBufferObject(64)
        buff.Init()
        data = bytearray(map(ord, "THIS IS A TEST INIT FOR BUFFER"))
        buff.data[0:len(data)] = data
        buff.Write()

        read_buff = InfraUtBufferObject(64)
        read_buff.Init(buff.GID())
        read_buff.Read()

        self.assertTrue(buff.data == read_buff.data)

        data = bytearray(map(ord, "OVERWRTING MY OWN DATA AGAIN"))
        buff.data[0:len(data)] = data
        buff.Write()

        read_buff.Read()

        self.assertTrue(buff.data == read_buff.data)

    @init_trigger_test(steps=1)
    def test_single_desc_to_packet(self):
        logger.set_level(6)
        # Create a ring object
        ring = RingObject(4, 256)
        doorbell = InfraUtDoorbell()
        desc = InfraUtTxDescriptorObject()
        desc.Init()

        input_output = {"input": [{"descriptor": {"descr": desc, "ring": ring, "id": 1}}],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [4],
                                    "encode_port": 4,
                                    "count": 1,
                                    "id": 1},

                                   #{"descriptor": {"descr": output_descr,
                                   #               "ring": output_ring, "id": 1}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()

        data = bytes(
            self._test_case_spec.session[0].step.expect.packets[0].packet.spkt)
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        desc.add_buff(buff_1)

        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched[0].packet.port == 4)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])

    @init_trigger_test(steps=1)
    def test_mult_desc_to_packet(self):
        logger.set_level(6)
        # Create a ring object
        ring = RingObject(4, 256)
        doorbell = InfraUtDoorbell()
        desc_1 = InfraUtTxDescriptorObject()
        desc_1.Init()
        desc_2 = InfraUtTxDescriptorObject()
        desc_2.Init()

        input_output = {"input": [{"descriptor": {"descr": desc_1, "ring": ring, "id": 1}},
                                  {"descriptor": {"descr": desc_2, "ring": ring, "id": 2}}],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [4],
                                    "encode_port": 4,
                                    "count": 1,
                                    "id": 1},
                                   {"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [6],
                                    "encode_port": 6,
                                    "count": 1,
                                    "id": 1},

                                   #{"descriptor": {"descr": output_descr,
                                   #               "ring": output_ring, "id": 1}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()

        data = bytes(
            self._test_case_spec.session[0].step.expect.packets[0].packet.spkt)
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        desc_1.add_buff(buff_1)

        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()

        data = bytes(
            self._test_case_spec.session[0].step.expect.packets[1].packet.spkt)
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        desc_2.add_buff(buff_1)

        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched[0].packet.port == 4)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched[1].packet.port == 6)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])

    @init_trigger_test(steps=1)
    def test_packet_to_desc(self):
        logger.set_level(6)
        # Create a ring object
        output_ring = RingObject(4, 256)
        doorbell = InfraUtDoorbell()
        output_desc = InfraUtTxDescriptorObject()
        output_desc.Init()

        input_output = {"input": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                        "dst": MacAddressBase("0000.9999.0002").get(),
                                                        "type": 0x8100
                                                        },
                                              "Dot1Q": {"vlan": 12},
                                              "IP": {"src": "10.0.0.64",
                                                     "dst": "10.0.0.65",
                                                     "proto": 6},
                                              "TCP": {"sport": 100, "dport": 200}},
                                   "ports": [1],
                                   "encode_port": 9,  # 9 is Ring 1.
                                   "encode_opcode":5,  # 5 is packet to descriptor
                                   "count": 1,
                                   "id": 1}],
                        "output": [{"descriptor": {"descr": output_desc,
                                                   "ring": output_ring, "id": 1}},

                                   ],
                        }

        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()

        data = bytes(
            self._test_case_spec.session[0].step.trigger.packets[0].packet.spkt)
        buff_1.data = data
        buff_1.size = len(data)
        output_desc.add_buff(buff_1)

        output_ring.GID(1)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == 1)

    @init_trigger_test(steps=1)
    def test_mult_packet_to_desc(self):
        logger.set_level(6)
        # Create a ring object
        output_ring = RingObject(4, 256)
        doorbell = InfraUtDoorbell()
        output_desc_1 = InfraUtTxDescriptorObject()
        output_desc_1.Init()
        output_desc_2 = InfraUtTxDescriptorObject()
        output_desc_2.Init()

        input_output = {"input": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                        "dst": MacAddressBase("0000.9999.0002").get(),
                                                        "type": 0x8100
                                                        },
                                              "Dot1Q": {"vlan": 12},
                                              "IP": {"src": "10.0.0.64",
                                                     "dst": "10.0.0.65",
                                                     "proto": 6},
                                              "TCP": {"sport": 100, "dport": 200}},
                                   "ports": [1],
                                   "encode_port": 9,  # 9 is Ring 1.
                                   "encode_opcode":5,  # 5 is packet to descriptor
                                   "count": 1,
                                   "id": 1},
                                  {"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                        "dst": MacAddressBase("0000.9999.0002").get(),
                                                        "type": 0x8100
                                                        },
                                              "Dot1Q": {"vlan": 12},
                                              "IP": {"src": "10.0.0.64",
                                                     "dst": "10.0.0.65",
                                                     "proto": 6},
                                              "TCP": {"sport": 100, "dport": 200}},
                                   "ports": [1],
                                   "encode_port": 9,  # 9 is Ring 1.
                                   "encode_opcode":5,  # 5 is packet to descriptor
                                   "count": 1,
                                   "id": 1}],
                        "output": [{"descriptor": {"descr": output_desc_1,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_desc_2,
                                                   "ring": output_ring, "id": 2}},
                                   ],
                        }

        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()

        data = bytes(
            self._test_case_spec.session[0].step.trigger.packets[0].packet.spkt)
        buff_1.data = data
        buff_1.size = len(data)
        output_desc_1.add_buff(buff_1)

        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()

        data = bytes(
            self._test_case_spec.session[0].step.trigger.packets[1].packet.spkt)
        buff_1.data = data
        buff_1.size = len(data)
        output_desc_2.add_buff(buff_1)

        output_ring.GID(1)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == 2)

    @init_trigger_test(steps=1)
    def test_desc_to_desc(self):
        logger.set_level(6)
        # Create a ring object
        doorbell = InfraUtDoorbell()
        input_ring = RingObject(4, 256)
        input_desc = InfraUtTxDescriptorObject()
        input_desc.Init()

        output_ring = RingObject(4, 256)
        output_desc = InfraUtTxDescriptorObject()
        output_desc.Init()

        input_output = {"input": [{"descriptor": {"descr": input_desc, "ring": input_ring, "id": 1}}],
                        "output": [{"descriptor": {"descr": output_desc, "ring": output_ring, "id": 1}},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()
        # Port 10 is nothing but ring 2 , 10 - MAX_PORTS(8)
        data = bytes(self._construct_spkt(packet, 64, port=10))
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        input_desc.add_buff(buff_1)

        output_ring.GID(2)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == 1)

    @init_trigger_test(steps=1)
    def test_mult_desc_to_desc(self):
        logger.set_level(6)
        # Create a ring object
        doorbell = InfraUtDoorbell()
        input_ring = RingObject(4, 256)
        input_desc = InfraUtTxDescriptorObject()
        input_desc.Init()
        input_desc_1 = InfraUtTxDescriptorObject()
        input_desc_1.Init()

        output_ring = RingObject(4, 256)
        output_desc = InfraUtTxDescriptorObject()
        output_desc.Init()
        output_desc_1 = InfraUtTxDescriptorObject()
        output_desc_1.Init()

        input_output = {"input": [{"descriptor": {"descr": input_desc, "ring": input_ring, "id": 1}},
                                  {"descriptor": {"descr": input_desc_1, "ring": input_ring, "id": 1}}],
                        "output": [{"descriptor": {"descr": output_desc, "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_desc_1,
                                                   "ring": output_ring, "id": 1}}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()
        # Port 10 is nothing but ring 2 , 10 - MAX_PORTS(8)
        data = bytes(self._construct_spkt(packet, 64, port=10))
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        input_desc.add_buff(buff_1)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()
        # Port 10 is nothing but ring 2 , 10 - MAX_PORTS(8)
        data = bytes(self._construct_spkt(packet, 64, port=10))
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        input_desc_1.add_buff(buff_1)

        output_ring.GID(2)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == 2)

    @init_trigger_test(steps=1)
    def test_desc_to_desc_mismatch(self):
        logger.set_level(6)
        # Create a ring object
        doorbell = InfraUtDoorbell()
        input_ring = RingObject(4, 256)
        input_desc = InfraUtTxDescriptorObject()
        input_desc.Init()

        output_ring = RingObject(4, 256)
        output_desc = InfraUtTxDescriptorObject()
        output_desc.Init()

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()
        # Port 10 is nothing but ring 3 , 10 - MAX_PORTS(8),
        # Ring 3 is associated to Buffer mismatch.
        data = bytes(self._construct_spkt(packet, 64, port=10, opcode=3))
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        input_desc.add_buff(buff_1)

        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()
        buff_1.data = data
        buff_1.size = len(data)
        # Don't write.
        output_desc.add_buff(buff_1)

        input_output = {"input": [{"descriptor": {"descr": input_desc, "ring": input_ring, "id": 1}}],
                        "output": [{"descriptor": {"descr": output_desc, "ring": output_ring, "id": 1}},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        output_ring.GID(2)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self._trigger.get_dict_report()
        self.assertTrue(report and report.passed_count == 0 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch) == 1)

    @init_trigger_test(steps=1)
    def test_descriptor_to_descriptor_mult_buffers(self):
        logger.set_level(6)
        # Create a ring object
        doorbell = InfraUtDoorbell()
        input_ring = RingObject(4, 256)
        input_desc = InfraUtTxDescriptorObject()
        input_desc.Init()

        output_ring = RingObject(4, 256)
        output_desc = InfraUtTxDescriptorObject()
        output_desc.Init()

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()
        # Port 10 is nothing but ring 2 , 10 - MAX_PORTS(8)
        # TODO: Opcode, split up the buffers into 2 75 bytes
        data = bytes(self._construct_spkt(packet, 200, port=10, opcode=6))
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        input_desc.add_buff(buff_1)

        data = copy(data)
        rem_size = len(data)
        split_size = 75
        while rem_size:
            buff_1 = InfraUtBufferObject(512)
            buff_1.Init()
            copy_size = min(rem_size, split_size)
            buff_1.data = data[:copy_size]
            buff_1.size = len(buff_1.data)
            output_desc.add_buff(buff_1)
            data = data[copy_size:]
            rem_size -= copy_size

        input_output = {"input": [{"descriptor": {"descr": input_desc, "ring": input_ring, "id": 1}}],
                        "output": [{"descriptor": {"descr": output_desc, "ring": output_ring, "id": 1}}, ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        output_ring.GID(2)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self._trigger.get_dict_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == 1)

    @init_trigger_test(steps=1)
    def test_descriptor_to_descriptor_mult_buffers_corrupt_1(self):
        logger.set_level(6)
        # Create a ring object
        doorbell = InfraUtDoorbell()
        input_ring = RingObject(4, 256)
        input_desc = InfraUtTxDescriptorObject()
        input_desc.Init()

        output_ring = RingObject(4, 256)
        output_desc = InfraUtTxDescriptorObject()
        output_desc.Init()

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()
        # Port 10 is nothing but ring 2 , 10 - MAX_PORTS(8)
        # TODO: Opcode, split up the buffers into 2 75 bytes
        data = bytes(self._construct_spkt(packet, 200, port=10, opcode=7))
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        input_desc.add_buff(buff_1)

        data = copy(data)
        rem_size = len(data)
        split_size = 75
        while rem_size:
            buff_1 = InfraUtBufferObject(512)
            buff_1.Init()
            copy_size = min(rem_size, split_size)
            buff_1.data = data[:copy_size]
            buff_1.size = len(buff_1.data)
            output_desc.add_buff(buff_1)
            data = data[copy_size:]
            rem_size -= copy_size

        input_output = {"input": [{"descriptor": {"descr": input_desc, "ring": input_ring, "id": 1}}],
                        "output": [{"descriptor": {"descr": output_desc, "ring": output_ring, "id": 1}}, ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        output_ring.GID(2)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self._trigger.get_dict_report()
        self.assertTrue(report and report.passed_count == 0 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch) == 1)

    @init_trigger_test(steps=1)
    def test_descriptor_to_mult_descriptors(self):
        logger.set_level(6)
        # Create a ring object
        doorbell = InfraUtDoorbell()
        input_ring = RingObject(4, 256)
        input_desc = InfraUtTxDescriptorObject()
        input_desc.Init()

        output_ring = RingObject(4, 256)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()
        # Port 10 is nothing but ring 2 , 10 - MAX_PORTS(8)
        # TODO: Opcode, split up the buffers into 2 75 bytes and each with 1
        # descriptor.
        data = bytes(self._construct_spkt(packet, 200, port=10, opcode=8))
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        input_desc.add_buff(buff_1)

        data = copy(data)
        rem_size = len(data)
        split_size = 75
        num_buff_per_desc = 1
        output_descrs = []

        while rem_size:
            buf_cnt = 0
            output_desc = InfraUtTxDescriptorObject()
            output_desc.Init()
            while rem_size and buf_cnt < num_buff_per_desc:
                buff_1 = InfraUtBufferObject(512)
                buff_1.Init()
                copy_size = min(rem_size, split_size)
                buff_1.data = data[:copy_size]
                buff_1.size = len(buff_1.data)
                output_desc.add_buff(buff_1)
                data = data[copy_size:]
                rem_size -= copy_size
                buf_cnt += 1
            output_descrs.append(
                {"descriptor": {"descr": output_desc, "ring": output_ring, "id": 1}})

        input_output = {"input": [{"descriptor": {"descr": input_desc, "ring": input_ring, "id": 1}}],
                        "output": output_descrs,
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        output_ring.GID(2)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self._trigger.get_dict_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == len(output_descrs))

    @init_trigger_test(steps=1)
    def test_descriptor_to_mult_descriptors_corrupt(self):
        logger.set_level(6)
        # Create a ring object
        doorbell = InfraUtDoorbell()
        input_ring = RingObject(4, 256)
        input_desc = InfraUtTxDescriptorObject()
        input_desc.Init()

        output_ring = RingObject(4, 256)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        buff_1 = InfraUtBufferObject(512)
        buff_1.Init()
        # Port 10 is nothing but ring 2 , 10 - MAX_PORTS(8)
        # TODO: Opcode, split up the buffers into 2 75 bytes and each with 1
        # descriptor.
        data = bytes(self._construct_spkt(packet, 200, port=10, opcode=9))
        buff_1.data = data
        buff_1.size = len(data)
        buff_1.Write()
        input_desc.add_buff(buff_1)

        data = copy(data)
        rem_size = len(data)
        split_size = 75
        num_buff_per_desc = 1
        output_descrs = []

        while rem_size:
            buf_cnt = 0
            output_desc = InfraUtTxDescriptorObject()
            output_desc.Init()
            while rem_size and buf_cnt < num_buff_per_desc:
                buff_1 = InfraUtBufferObject(512)
                buff_1.Init()
                copy_size = min(rem_size, split_size)
                buff_1.data = data[:copy_size]
                buff_1.size = len(buff_1.data)
                output_desc.add_buff(buff_1)
                data = data[copy_size:]
                rem_size -= copy_size
                buf_cnt += 1
            output_descrs.append(
                {"descriptor": {"descr": output_desc, "ring": output_ring, "id": 1}})

        input_output = {"input": [{"descriptor": {"descr": input_desc, "ring": input_ring, "id": 1}}],
                        "output": output_descrs,
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._test_case_spec.session[0].step.trigger.doorbell.object = doorbell

        output_ring.GID(2)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self._trigger.get_dict_report()
        self.assertTrue(report and report.passed_count == 0 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch) == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == len(output_descrs) - 1)


if __name__ == '__main__':
    import unittest
    suite = unittest.TestSuite()
    # logger.add_stdout()
    suite.addTest(DolTriggerTestWithMockModel(
        'test_init_buffer'))
    unittest.TextTestRunner(verbosity=2).run(suite)
