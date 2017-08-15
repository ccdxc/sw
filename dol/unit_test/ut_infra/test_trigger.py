import time
import pdb
import copy
import pickle
import mock
import os
from collections import defaultdict
from third_party.scapy.all import Dot1Q, IP, Ether
from unit_test.ut_infra.test_common import mock_modules, mock_model_modules
mock_modules()
from config.objects.ring import RingIdAllocator
from factory.objects.ut.buffer import BufferIdAllocator
from infra.common.utils import convert_object_to_dict


import infra.common.utils as utils
import infra.penscapy.penscapy as penscapy
from infra.common.logging import logger
from infra.common.thread import InfraThreadHandler
from unit_test.ut_infra.test_common import DolInfraTest
import infra.common.defs as defs
import infra.common.objects as objects
import infra.common.parser as parser
from infra.common.objects import MacAddressBase

factory_init_done = False


def init_trigger_test(steps=1):
    def inner_func_decorator(func):
        def inner_func_wrapper(self, *args, **kwargs):
            RingIdAllocator.reset()
            BufferIdAllocator.reset()
            self._tc_input_out_map = defaultdict(lambda: [])
            self._tc_input_out_obj_map = defaultdict(lambda: [])
            self._test_case_spec, self._packet_spec, self._desc_spec = self._get_test_obj_specs()
            tc_steps = self._get_steps(steps)
            self._test_case_spec.session = tc_steps
            setattr(self._test_case_spec, "info", self.info)
            setattr(self._test_case_spec, "debug", self.info)
            setattr(self._test_case_spec, "verbose", self.info)
            self._test_case_spec.logger = logger
            x = next(self._tc_iter)
            self._test_case_spec.ID(x)
            func(self, *args, **kwargs)
        return inner_func_wrapper
    return inner_func_decorator


class DolTriggerTest(DolInfraTest):

    TRIGGER_STEP_TIMEOUT = 2

    def _setup_db_patcher(self):
        mock_object_func_map = {
            # $CLASSTNAME: {"$FUNC_NAME": $MOCK_FUNC_NAME}
        }

        for obj_class in mock_object_func_map:
            for func, mock_func in mock_object_func_map[obj_class].iteritems():
                patcher = mock.patch.object(obj_class, func, mock_func)
                patcher.start()

        # db_func_patcher = mock.patch(
            #'$LIB_FUNC', $MOCK_FUNC
        # db_func_patcher.start()

    def _stop_all_patch(self):
        mock.patch.stopall()

    def _get_test_obj_specs(self):
        test_spec = '''
meta:
    id: TEST_CASE

config:
    #Config Info is Don't care.

snapshot:
    # List of Object entries
    - entry:
        object: None    # Configuration object for snapshot
        # E.g. config/rdma/lqpair/sq/qstate
        # This will take a snapshot of  the SendQ QPCB of the Local Qpair

current_step : None # Current step being executed by trigger.
GetLogPrefix : None

session:
    # List of steps in a session
    - step:
        trigger:
            packets:
                # List of packets
                # Instance of Template: testobjects/packet
                - packet: None

            descriptors:
                # List of Descriptors
                - descriptor:
                    object  : None
                    ring    : None
            doorbell:
                object: None # Doorbell object

        expect:
            timeout : None
            packets:
                # List of packets
                - packet: None

            descriptors:
                # List of descriptors
                - descriptor:
                    object  : None
                    ring    : None
                    # actual : None  # Framework provided empty actual
            callback: None #Custom callback

            snapshots:
                - entry:
                # List of
                    object  : None      # Expected Snapshot object
                    verify  : None
                    # orig  : None  # Framework provided empty actual
                    # actual : None  # Framework provided empty actual
                    # After the TC is verified, the 'actual' will be
                    # updated to 'original'.
                    fields  :
                       # A   : 100
                       # B   : 200
                       # C   : +5

            buffers:
                - buffer:
                    object      : None  # Expected BUFFER object
                    rw_verify   : None  # Verify READ_ONCE,WRITTEN_ONCE etc
                    # actual     : None  # Framework provided empty actualpass
        received:
            timeout : None
            packets:
                # List of packets
                - packet: None

            descriptors:
                # List of descriptors
                - object  : None
                  ring    : None
                    # actual : None  # Framework provided empty actual                           

    - step:
        trigger:
            packets:
                # List of packets
                # Instance of Template: testobjects/packet
                - packet: None

            descriptors:
                # List of Descriptors
                - descriptor:
                    object  : None
                    ring    : None
            
            doorbell:
                object: None # Doorbell object

        expect:
            timeout : None
            packets:
                # List of packets
                - packet: None

            descriptors:
                # List of descriptors
                - descriptor:
                    object  : None
                    ring    : None
                    # actual : None  # Framework provided empty actual
            callback: None #Custom callback
                

            snapshots:
                # List of
                - entry:
                    object  : None      # Expected Snapshot object
                    verify  : None
                    # orig  : None  # Framework provided empty actual
                    # actual : None  # Framework provided empty actual
                    # After the TC is verified, the 'actual' will be
                    # updated to 'original'.
                    fields  :
                       # A   : 100
                       # B   : 200
                       # C   : +5

            buffers:
                - buffer:
                    object      : None  # Expected BUFFER object
                    rw_verify   : None  # Verify READ_ONCE,WRITTEN_ONCE etc
                    # actual     : None  # Framework provided empty actualpass
        received:
            timeout : None
            packets:
                # List of packets
                - packet: None

            descriptors:
                # List of descriptors
                - object  : None
                  ring    : None
                    # actual : None  # Framework provided empty actual                    
'''
        with open(self._TMP_PKT_FILE, "w") as text_file:
            text_file.write(test_spec)
        prs = parser.ParserBase()
        tmp_path = defs.TEST_MODULE_PATH
        defs.TEST_MODULE_PATH = "/"
        tmp_env_ws_top = os.environ["WS_TOP"]
        tmp_dol_path = defs.DOL_PATH
        defs.DOL_PATH = "/"
        os.environ["WS_TOP"] = "/"
        objlist = prs.Parse("/tmp", self._TMP_PKT_FILE_NAME)
        test_spec = objlist[0]
        test_spec.GetLogPrefix = lambda : "test"
        pkt_yml = '''
meta:
    id: None

payloadsize : None  # Int
template    : None  # Packet Factory Template Object
headers     : None  # List of Headers
spkt        : None  # Scapy packet object
ports       : None  # List of Config Uplink objects
count    : None  # Int
'''
        with open(self._TMP_PKT_FILE, "w") as text_file:
            text_file.write(pkt_yml)

        objlist = prs.Parse("/tmp", self._TMP_PKT_FILE_NAME)

        pkt_spec = objlist[0]

        desc_yml = '''
meta:
    id: None

object     : None  # Descriptor object
ring        : None  #  Ring Object
'''
        with open(self._TMP_PKT_FILE, "w") as text_file:
            text_file.write(desc_yml)

        objlist = prs.Parse("/tmp",
                            self._TMP_PKT_FILE_NAME)

        desc_spec = objlist[0]

        os.environ["WS_TOP"] = tmp_env_ws_top
        defs.TEST_MODULE_PATH = tmp_path
        defs.DOL_PATH = tmp_dol_path
        return test_spec, pkt_spec, desc_spec

    def setUp(self, trigger, model):
        mock_model_modules()
        super(DolTriggerTest, self).setUp()
        global PEN_REF
        from infra.engine.trigger import PEN_REF
        logger.set_level(0)
        self.model = model
        self.model.start()
        time.sleep(0.25)
        self._trigger = trigger
        self._trigger.start()
        time.sleep(0.25)
        self.assertFalse(self._trigger._scheduler_running())
        self.assertFalse(self._trigger._receiver_running())
        self._tc_input_out_map = defaultdict(lambda: [])
        self._tc_input_out_obj_map = defaultdict(lambda: [])
        self._ring_queue = defaultdict(lambda: [])
        self._test_case_spec, self._packet_spec, self._desc_spec = self._get_test_obj_specs()
        self._tc_iter = iter(self.gen_test_case_id())
        # Create one door bell object
        import infra.factory.factory as factory
        global factory_init_done
        if not factory_init_done:
            factory.init()
            factory_init_done = True

    def tearDown(self):
        # Scheduler should have stopped when test case is completed.
        self.assertFalse(self._trigger._scheduler_running())
        self.assertFalse(self._trigger._receiver_running())
        super(DolTriggerTest, self).tearDown()
        self._trigger.stop()
        time.sleep(1)
        self.model.stop()
        self._stop_all_patch()

    def gen_test_case_id(self):
        for tc_id in range(1, 10000):
            yield tc_id

    @staticmethod
    def newinst(template):
        inst = objects.FrameworkObject()
        for key, data in template.__dict__.items():
            if isinstance(data, objects.FrameworkObject):
                inst.__dict__[key] = DolTriggerTest.newinst(data)
            else:
                inst.__dict__[key] = data
        return inst

    def _get_steps(self, count):
        step = self._test_case_spec.session[0]
        step.step.expect.timeout = self.TRIGGER_STEP_TIMEOUT
        return [DolTriggerTest.newinst(step) for _ in range(count)]

    def _wait_for_test_case_completion(self):
        time.sleep(len(self._test_case_spec.session)
                   * self.TRIGGER_STEP_TIMEOUT + 5)
        self.assertTrue(self._trigger.pending_test_case_count() == 0)

    def get_next_test_step(self):
        for i, step in enumerate(self._test_case_spec.session, 0):
            yield i, step

    @staticmethod
    def info(*args, **kwargs):
        # return
        logger.info(args, kwargs)

    def _add_trigger_expect_packet(self, step, input_output, add_pen_header=True):
        input_pkt = step.trigger.packets[0]
        input_descr = step.trigger.descriptors[0]

        step.trigger.packets = [DolTriggerTest.newinst(
            input_pkt) for data in (input_output["input"]) if "packet" in data]
        step.expect.packets = [DolTriggerTest.newinst(
            input_pkt) for data in (input_output["output"]) if "packet" in data]
        step.trigger.descriptors = [DolTriggerTest.newinst(
            input_descr) for data in (input_output["input"]) if "descriptor" in data]
        step.expect.descriptors = [DolTriggerTest.newinst(
            input_descr) for data in (input_output["output"]) if "descriptor" in data]
        i = j = 0
        for data in (input_output["input"]):
            if "packet" in data:
                input_pkt = self.newinst(self._packet_spec)
                input_pkt.spkt = self._construct_spkt(
                    data["packet"], 64, add_pen_hdr=add_pen_header,
                    port=data.get("encode_port"), opcode=data.get("encode_opcode"))
                if add_pen_header:
                    input_pkt.spkt[PEN_REF].id = self._test_case_spec.GID()
                input_pkt.count = data["count"]
                input_pkt.ID(self._test_case_spec.GID())
                step.trigger.packets[i].packet = input_pkt
                step.trigger.packets[i].ports = data["ports"][:]
                i += 1
            elif "descriptor" in data:
                input_descr = self.newinst(self._desc_spec)
                input_descr.object = data["descriptor"]["descr"]
                input_descr.ring = data["descriptor"]["ring"]
                input_descr.ID(self._test_case_spec.GID())
                step.trigger.descriptors[j].descriptor = copy.copy(
                    input_descr)
                j += 1
            else:
                assert 0

        i = j = 0
        for data in (input_output["output"]):
            if "packet" in data:
                input_pkt = self.newinst(self._packet_spec)
                input_pkt.spkt = self._construct_spkt(
                    data["packet"], 64, add_pen_hdr=add_pen_header, port=data.get("encode_port"))
                if add_pen_header:
                    input_pkt.spkt[PEN_REF].id = self._test_case_spec.GID()

                input_pkt.count = data["count"]
                input_pkt.ports = data["ports"][:]
                input_pkt.ID(self._test_case_spec.GID())
                step.expect.packets[i].packet = input_pkt
                step.expect.packets[i].ports = data["ports"][:]
                i += 1
            elif "descriptor" in data:
                output_descr = self.newinst(self._desc_spec)
                output_descr.ID(self._test_case_spec.GID())
                output_descr.object = copy.deepcopy(
                    data["descriptor"]["descr"])
                output_descr.ring = data["descriptor"]["ring"]
                step.expect.descriptors[j].descriptor = output_descr
                j += 1
            else:
                assert 0


class TestModel(InfraThreadHandler):

    def __init__(self, infra,
                 connector='SocketDgram', addr=utils.FakeModelSockAddr()):
        global Connector
        from infra.common.connector import Connector
        global Trigger, TriggerTestCase, TriggerTestCaseStep
        from infra.engine.trigger import Trigger, TriggerTestCase, TriggerTestCaseStep
        global PacketContext
        from infra.engine.trigger import PacketContext
        super(TestModel, self).__init__(name="TestModel",
                                        max_idle_interval=0, max_time_per_run=200)
        self._connector = Connector.factory(connector)(addr)
        self._connector.bind()
        self._infra = infra

    def execute(self, event):
        try:
            addr, msg, port = self._connector.recv()
        except Connector.Timeout:
            return
        except:
            return
        if port:
            pkt = msg
            tc_id = pkt[penscapy.PENDOL].id
            if not self._infra._tc_input_out_map[tc_id]:
                # Not step input output for this test case.
                return
            current_step_input_output = self._infra._tc_input_out_map[tc_id][0]
            input_spkt = penscapy.Parse(
                bytes(current_step_input_output["input"][0]))
            if tc_id in self._infra._tc_input_out_map:
                match, _ = TriggerTestCaseStep._packets_match(
                    input_spkt, pkt, None)
                if match:
                    current_step_input_output["input"].pop(0)
                    # All packets matched, send everything out.
                    if not current_step_input_output["input"]:
                        for pkt_ctx in current_step_input_output["output"]:
                            pkt_ctx_cp = PacketContext(
                                addr, pkt_ctx.packet, pkt_ctx.port)
                            pkt_ctx_cp.packet[penscapy.PENDOL].id = tc_id
                            self.process_one_packet(pkt_ctx_cp)
                        # Remove this as everything associated with this step
                        # input done
                        self._infra._tc_input_out_map[tc_id].pop(0)
        else:
            # Door bell
            for descriptor in self._infra._ring_queue[msg["ring"]]:
                model_ip_op = self._infra._tc_input_out_obj_map[descriptor].pop(
                    0)
                for op_obj in model_ip_op["output"]:
                    if not isinstance(op_obj, PacketContext):
                        ring, desc = op_obj
                        self._infra._ring_queue[ring._id].append(desc)
                    else:
                        # Sending Packet..
                        pkt_ctx_cp = PacketContext(
                            addr, op_obj.packet, op_obj.port)
                        self.process_one_packet(pkt_ctx_cp)
            del self._ring_queue[msg["ring"]][:]

    def send(self, pkt_ctx):
        pkt = pkt_ctx.packet
        sdata = pickle.dumps({'port': pkt_ctx.port, "data": bytes(pkt)})
        self._connector.send_to_client(pkt_ctx.sender_info, sdata)
        return

    def start(self):
        self.startThread()

    def stop(self):
        self._connector.close()
        self.stopThread()

    def process_one_packet(self, pkt):
        self.send(pkt)

    def process_one_l2uc_packet(self, pkt):
        self.send(pkt)

    def process_one_l3uc_packet(self, pkt):
        self.send(pkt)

    def process_one_unk_packet(self, pkt):
        self.send(pkt)


class DolTriggerTestWithUTModel(DolTriggerTest):

    def setUp(self):
        model = TestModel(self)
        trig_obj = Trigger(connector='SocketDgram')
        super(DolTriggerTestWithUTModel, self).setUp(trig_obj, model)
        self._tc_doorbell = self.Doorbell()

    def tearDown(self):
        super(DolTriggerTestWithUTModel, self).tearDown()

    class Descriptor():

        def __init__(self, desc_id):
            self._id = desc_id
            self.field_a = 1
            self.field_b = 1
            self.field_c = 1
            '''self.field_d = 1
            self.field_e = 1
            self.field_f = 1'''
            self.address = None

        def init(self):
            pass

        def read(self):
            pass

        def write(self):
            pass

    class Buffer:
        def __init__(self, buf_id, data):
            self.id = buf_id
            self.size = len(data)
            self.data = data

    class Ring(object):

        def __init__(self, infra, ring_id, size):
            self._id = ring_id
            self._size = size
            self._infra = infra

        def post(self, desc):
            self._infra.post_descriptor_ring(self._id, desc)

        def consume(self):
            for descr in self._infra.consume_descriptor_ring(self._id):
                yield descr

    class Doorbell():

        def __init__(self):
            self.ring = None
            self.field_a = 1
            self.field_b = 2

        def serialize(self):
            return self

    def post_descriptor_ring(self, ring_id, descriptor):
        self._ring_queue[ring_id].append(descriptor)

    def consume_descriptor_ring(self, ring_id):
        return_descrs = self._ring_queue[ring_id][:]
        del self._ring_queue[ring_id][:]
        return return_descrs

    def __add_model_input_output(self, input_output, add_pen_header=True):
        model_ip_op = {"input": [], "output": []}
        first_descr_ref = None
        for _, data in enumerate(input_output["input"], 0):
            if "packet" in data:
                spkt = self._construct_spkt(
                    data["packet"], 64, add_pen_hdr=add_pen_header)
                spkt[PEN_REF].id = self._test_case_spec.GID()
                model_ip_op["input"].append(spkt)
            elif "descriptor" in data:
                # Input matched based on first descriptor only.
                if not first_descr_ref:
                    first_descr_ref = data["descriptor"]["descr"]

        for _, data in enumerate(input_output["output"], 0):
            if "packet" in data:
                spkt = self._construct_spkt(
                    data["packet"], 64, add_pen_hdr=add_pen_header)
                for port in data["ports"]:
                    model_ip_op["output"].append(
                        PacketContext(None, spkt, port))
            elif "descriptor" in data:
                input_descr = self.newinst(self._desc_spec)
                input_descr.object = data["descriptor"]["descr"]
                input_descr.ring = data["descriptor"]["ring"]
                # Add as a tuple to diffrentiate packet and descriptor for now.
                # model_ip_op["output"].append(
                #    (data["descriptor"]["ring"], data["descriptor"]["descr"]))
                data["descriptor"]["ring"].post(
                    copy.deepcopy(data["descriptor"]["descr"]))

            else:
                assert 0
        if first_descr_ref:
            self._tc_input_out_obj_map[first_descr_ref].append(model_ip_op)
        else:
            self._tc_input_out_map[self._test_case_spec.GID()].append(
                model_ip_op)

    @init_trigger_test(steps=1)
    def test_single_packet_flow(self):
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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched[0].packet.port == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])

    @init_trigger_test(steps=1)
    def test_single_packet_flow_with_custom_callback(self):

        def custom_callback():
            # Make sure step is set
            self.assertTrue(self._test_case_spec.current_step ==
                            self._test_case_spec.session[0].step)
            return defs.status.ERROR

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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        self._test_case_spec.verify_callback = custom_callback
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 0 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched[0].packet.port == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(report.details[self._test_case_spec.GID(
        )].session[0].step.result.callback.status == Trigger.TEST_CASE_FAILED)
        self.assertTrue(
            len(self._test_case_spec.session[0].step.received.packets) == 1)

    @init_trigger_test(steps=1)
    def test_single_packet_2_outputs(self):

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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [3],
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
                                               "TCP": {"sport": 100, "dport": 200},
                                               },
                                    "ports": [2],
                                    "count": 1,
                                    "id": 2
                                    }
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, _ = self._trigger.get_result(self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_PASSED)

    @init_trigger_test(steps=1)
    def test_single_packet_flow_mismatch(self):
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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.mismatch) == 1)
        self.assertTrue(len(details.missing) == 0)
        self.assertTrue(
            details.mismatch[0].mismatch.headers.Dot1Q.mismatch_fields.vlan)
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.failed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            len(report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch) == 1)
        self.assertTrue(report.details[self._test_case_spec.GID()].session[0]
                        .step.result.packets.mismatch[0].packet.actual.port == 1)
        self.assertTrue(report.details[self._test_case_spec.GID()].session[0]
                        .step.result.packets.mismatch[0].packet.expected.ports == [1])

    @init_trigger_test(steps=1)
    def test_single_packet_flow_extra_packet(self):
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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1, 3],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"].append(copy.deepcopy(input_output["output"][0]))
        input_output["output"][1]["packet"]["Dot1Q"]["vlan"] = 13
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        details = details[0].packets
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        self.assertTrue(len(details.extra) == 3)
        self.assertTrue(details.extra[0].spkt[Dot1Q].vlan == 13)
        self.assertTrue(details.extra[2].spkt[Dot1Q].vlan == 12)
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.failed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched[0].packet.port == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            len(report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra) == 3)
        self.assertTrue(
            set([1, 3]) == set([packet.packet.port for packet in report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra]))

    @init_trigger_test(steps=1)
    def test_single_packet_flow_extra_and_mismatch_packets(self):
        input_output = {"input": [{"packet": {"Ether": {"src": MacAddressBase("0000.7777.0001").get(),
                                                        "dst": MacAddressBase("0000.7777.0002").get(),
                                                        "type": 0x8100
                                                        },
                                              "Dot1Q": {"vlan": 12},
                                              "IP": {"src": "10.0.0.64",
                                                     "dst": "10.0.0.65",
                                                     "proto": 6},
                                              "TCP": {"sport": 100, "dport": 200}},
                                   "ports": [1],
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1}]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"].append(copy.deepcopy(input_output["output"][0]))
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        input_output["output"][1]["packet"]["Dot1Q"]["vlan"] = 14
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.extra) == 1)
        self.assertTrue(len(details.mismatch) == 1)
        self.assertTrue(details.extra[0].spkt[Dot1Q].vlan == 13)
        self.assertTrue(
            details.mismatch[0].mismatch.headers.Dot1Q.mismatch_fields.vlan)

    @init_trigger_test(steps=1)
    def test_single_packet_flow_extra_mismatch_and_missing_packets(self):
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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0005").get(),
                                                         "dst": MacAddressBase("0000.9999.0006").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200},
                                               },
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   {"packet": {"Ether": {"src": MacAddressBase("0000.9999.0005").get(),
                                                         "dst": MacAddressBase("0000.9999.0006").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 11},
                                               "IP": {"src": "10.0.0.66",
                                                      "dst": "10.0.0.67",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200},
                                               },
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   {"packet": {"Ether": {"src": MacAddressBase("0000.9999.0005").get(),
                                                         "dst": MacAddressBase("0000.9999.0006").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 10},
                                               "IP": {"src": "10.0.0.68",
                                                      "dst": "10.0.0.69",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200},
                                               },
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1}]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        # Mismatch Packet
        input_output["output"][1]["packet"]["Dot1Q"]["vlan"] = 13
        # Unwanted Extra Packet
        input_output["output"][2]["packet"]["Ether"]["type"] = 0x800
        del input_output["output"][2]["packet"]["Dot1Q"]
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.extra) == 1)
        self.assertTrue(len(details.missing) == 1)
        self.assertTrue(len(details.mismatch) == 1)
        self.assertTrue(
            details.extra[0].spkt[Ether].type == 0x800)
        self.assertTrue(
            details.extra[0].port == 1)
        self.assertTrue(
            details.mismatch[0].mismatch.headers.Dot1Q.mismatch_fields.vlan)

    @init_trigger_test(steps=1)
    def test_single_packet_flow_timed_out(self):

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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1, 2],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.missing) == 1)
        self.assertTrue(
            details.missing[0].spkt[Dot1Q].vlan == 12)
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.failed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])
        self.assertTrue(
            len(report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing) == 1)
        self.assertTrue(
            [[1, 2]] == [packet.packet.ports for packet in report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing])

    @init_trigger_test(steps=1)
    def test_single_packet_2_outputs_with_missing(self):
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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0005").get(),
                                                         "dst": MacAddressBase("0000.9999.0006").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   {"packet": {"Ether": {"src": MacAddressBase("0000.9999.0005").get(),
                                                         "dst": MacAddressBase("0000.9999.0006").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 11},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   {"packet": {"Ether": {"src": MacAddressBase("0000.9999.0005").get(),
                                                         "dst": MacAddressBase("0000.9999.0006").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 10},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        # Remove the last expected.
        input_output["output"].pop(2)
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        details = details[0].packets
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        self.assertTrue(len(details.missing) == 1)
        self.assertTrue(details.missing[0].spkt[Dot1Q].vlan == 10)

    @init_trigger_test(steps=1)
    def test_single_packet_mismatch_twice(self):
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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0005").get(),
                                                         "dst": MacAddressBase("0000.9999.0006").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   {"packet": {"Ether": {"src": MacAddressBase("0000.9999.0005").get(),
                                                         "dst": MacAddressBase("0000.9999.0006").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 11},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        # Mismatch Packet
        input_output["output"][1]["packet"]["Dot1Q"]["vlan"] = 13
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 14
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.mismatch) == 2)
        self.assertTrue(
            details.mismatch[0].mismatch.headers.Dot1Q.mismatch_fields.vlan.expected == 12)
        self.assertTrue(
            details.mismatch[1].mismatch.headers.Dot1Q.mismatch_fields.vlan.expected == 11)
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.failed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            len(report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch) == 2)
        self.assertTrue(report.details[self._test_case_spec.GID()].session[0]
                        .step.result.packets.mismatch[0].packet.actual.port == 1)
        self.assertTrue(report.details[self._test_case_spec.GID()].session[0]
                        .step.result.packets.mismatch[0].packet.expected.ports == [1])
        self.assertTrue(report.details[self._test_case_spec.GID()].session[0]
                        .step.result.packets.mismatch[0].packet.mismatch.headers.Dot1Q.mismatch_fields.vlan.expected == 12)

    @init_trigger_test(steps=1)
    def test_single_packet_flow_with_extra_and_better_mismatch_packets(self):
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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"].append(copy.deepcopy(input_output["output"][0]))
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        input_output["output"][0]["packet"]["IP"]["src"] = "1.1.1.1"
        input_output["output"][1]["packet"]["Dot1Q"]["vlan"] = 15
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self.assertTrue(self._trigger.pending_test_case_count() == 1)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.extra) == 1)
        self.assertTrue(len(details.mismatch) == 1)
        self.assertTrue(details.extra[0].spkt[Dot1Q].vlan == 13)
        self.assertTrue(
            details.mismatch[0].mismatch.headers.Dot1Q.mismatch_fields.vlan)
        report = self._trigger.get_run_report()
        self.assertTrue(
            report and report.failed_count == 1 and report.details[self._test_case_spec.GID()])

    @init_trigger_test(steps=1)
    def test_single_packet_flow_better_mismatch_packets_headers(self):
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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"].append(copy.deepcopy(input_output["output"][0]))
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        # 2nd packet has better mismatch as vlan header would have matched.
        input_output["output"][1]["packet"]["IP"]["src"] = "1.1.1.1"
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self.assertTrue(self._trigger.pending_test_case_count() == 1)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.extra) == 1)
        self.assertTrue(len(details.mismatch) == 1)
        self.assertTrue(details.extra[0].spkt[Dot1Q].vlan == 13)
        self.assertTrue(
            details.mismatch[0].mismatch.headers.IP.mismatch_fields.src)
        report = self._trigger.get_run_report()
        self.assertTrue(
            report and report.failed_count == 1 and report.details[self._test_case_spec.GID()])

    @init_trigger_test(steps=1)
    def test_single_packet_flow_better_mismatch_packets_headers_2(self):
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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"].append(copy.deepcopy(input_output["output"][0]))
        input_output["output"][0]["packet"]["IP"]["src"] = "1.1.1.1"
        # 2nd packet has worst mismatch as vlan header would have matched.
        input_output["output"][1]["packet"]["Dot1Q"]["vlan"] = 13
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self.assertTrue(self._trigger.pending_test_case_count() == 1)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.extra) == 1)
        self.assertTrue(len(details.mismatch) == 1)
        self.assertTrue(details.extra[0].spkt[Dot1Q].vlan == 13)
        self.assertTrue(
            details.mismatch[0].mismatch.headers.IP.mismatch_fields.src)
        report = self._trigger.get_run_report()
        self.assertTrue(
            report and report.failed_count == 1 and report.details[self._test_case_spec.GID()])

    @init_trigger_test(steps=1)
    def test_single_packet_flow_better_mismatch_packets_headers_1(self):
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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"].append(copy.deepcopy(input_output["output"][0]))
        input_output["output"][0]["packet"]["IP"]["src"] = "1.1.1.1"
        input_output["output"][0]["packet"]["IP"]["dst"] = "2.2.2.2"
        # 2nd packet has better mismatch as just 1 field mismatched.
        input_output["output"][1]["packet"]["IP"]["src"] = "3.3.3.3"
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self.assertTrue(self._trigger.pending_test_case_count() == 1)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.extra) == 1)
        self.assertTrue(len(details.mismatch) == 1)
        self.assertTrue(details.extra[0].spkt[IP].src == "1.1.1.1")
        self.assertTrue(
            details.mismatch[0].mismatch.headers.IP.mismatch_fields.src)
        report = self._trigger.get_run_report()
        self.assertTrue(
            report and report.failed_count == 1 and report.details[self._test_case_spec.GID()])

    def test_report(self):
        self.test_single_packet_mismatch_twice()
        self.test_single_packet_flow_extra_and_mismatch_packets()
        self.test_single_packet_2_outputs()
        self.test_single_packet_flow_extra_mismatch_and_missing_packets()
        report = self._trigger.get_run_report()
        self.assertTrue(
            report and report.failed_count == 3 and report.passed_count)
        self._trigger.get_dict_report()
        # logger.set_level(6)

    @init_trigger_test(steps=2)
    def test_single_packet_flow_2_steps(self):

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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        self._add_trigger_expect_packet(
            self._test_case_spec.session[1].step, input_output)
        self.__add_model_input_output(input_output)
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, _ = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_PASSED)

    @init_trigger_test(steps=5)
    def test_single_packet_flow_5_steps(self):

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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        for i in range(5):
            self._add_trigger_expect_packet(
                self._test_case_spec.session[i].step, input_output)
            self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, _ = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_PASSED)

    @init_trigger_test(steps=2)
    def test_single_packet_flow_2_steps_last_fail(self):

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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        self.__add_model_input_output(input_output)
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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[1].step, input_output)
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[1].packets
        self.assertTrue(len(details.mismatch) == 1)
        self.assertTrue(len(details.missing) == 0)
        self.assertTrue(
            details.mismatch[0].mismatch.headers.Dot1Q.mismatch_fields.vlan)

    @init_trigger_test(steps=3)
    def test_single_packet_flow_3_steps_last_fail(self):

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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        self.__add_model_input_output(input_output)
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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[1].step, input_output)
        self.__add_model_input_output(input_output)
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
                                   "count": 1,
                                   "id": 1},
                                  ],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[2].step, input_output)
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[2].packets
        self.assertTrue(len(details.mismatch) == 1)
        self.assertTrue(len(details.missing) == 0)
        self.assertTrue(
            details.mismatch[0].mismatch.headers.Dot1Q.mismatch_fields.vlan)

    @init_trigger_test(steps=1)
    def test_single_packet_wrong_ports_1(self):

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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [2, 3],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"][0]["ports"] = [4, 5]
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.extra) == 2 and set(
            [4, 5]) == set([pkt.port for pkt in details.extra]))

        input_output["output"][0]["ports"] = [2, 3, 5, 6]
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"][0]["ports"] = [4]
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        details = details[0].packets
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        details = details[0].packets
        self.assertTrue(len(details.missing) == 1 and set(
            [2, 3, 5, 6]) == set(details.missing[0].ports))

    @init_trigger_test(steps=1)
    def test_single_packet_wrong_ports_2(self):

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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [2, 3],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }

        input_output["output"][0]["ports"] = [2, 3, 5, 6]
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"][0]["ports"] = [4]
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        details = details[0].packets
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        self.assertTrue(len(details.missing) == 1 and set(
            [2, 3, 5, 6]) == set(details.missing[0].ports))

    @init_trigger_test(steps=1)
    def test_single_packet_received_on_one_port_channel(self):

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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [2, 3],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        # Receive the packet only one port and still should be pass.
        input_output["output"][0]["ports"] = [2]
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, _ = self._trigger.get_result(
            self._test_case_spec.GID())
        self.assertTrue(result == Trigger.TEST_CASE_PASSED)

    @init_trigger_test(steps=1)
    def test_single_packet_received_on_after_partial_mismatch_packet_on_same_port(self):

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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [2, 3],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"].append(copy.deepcopy(input_output["output"][0]))
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        # Receive the packet only one port and still should be pass.
        input_output["output"][0]["ports"] = [2]
        input_output["output"][1]["ports"] = [2]
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        details = details[0].packets
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        self.assertTrue(len(details.extra) == 1 and set(
            [2]) == set([pkt.port for pkt in details.extra]))

    @init_trigger_test(steps=1)
    def test_single_packet_received_on_after_partial_mismatch_packet_on_different_port(self):

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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [2, 3],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"].append(copy.deepcopy(input_output["output"][0]))
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        # Receive the packet only one port and still should be pass.
        input_output["output"][0]["ports"] = [7]
        input_output["output"][1]["ports"] = [2]
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        details = details[0].packets
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        self.assertTrue(len(details.extra) == 1 and set(
            [7]) == set([pkt.port for pkt in details.extra]))

    @init_trigger_test(steps=1)
    def test_single_packet_received_on_after_partial_mismatch_packet_on_different_port_channel(self):

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
                                   "count": 1,
                                   "id": 1
                                   }],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [2, 3, 4
                                              ],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output)
        input_output["output"].append(copy.deepcopy(input_output["output"][0]))
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        # Receive the packet only one port and still should be pass.
        input_output["output"][0]["ports"] = [2]
        input_output["output"][1]["ports"] = [3]
        self.__add_model_input_output(input_output)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        details = details[0].packets
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        self.assertTrue(len(details.missing) == 0)
        self.assertTrue(len(details.mismatch) == 0)
        self.assertTrue(len(details.extra) == 1 and set(
            [2]) == set([pkt.port for pkt in details.extra]))

    @init_trigger_test(steps=1)
    def test_single_desc_to_packet_flow(self):

        input_descr = self.Descriptor(1)
        input_ring = self.Ring(self, 1, 100)
        output_descr = self.Descriptor(2)
        output_ring = self.Ring(self, 2, 100)
        input_output = {"input": [{"descriptor": {"descr": input_descr, "ring": input_ring, "id": 1}}],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   {"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=False)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        self.__add_model_input_output(input_output, add_pen_header=False)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched[0].packet.port == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch == [])

    @init_trigger_test(steps=1)
    def test_single_desc_to_packet_desc_mismatch(self):

        input_descr = self.Descriptor(1)
        input_ring = self.Ring(self, 1, 100)
        output_descr = self.Descriptor(2)
        output_ring = self.Ring(self, 2, 100)
        input_output = {"input": [{"descriptor": {"descr": input_descr, "ring": input_ring, "id": 1}}],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1},
                                   {"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=False)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        output_descr.field_a = 11
        output_descr.field_b = 11
        output_descr.field_c = 11
        self.__add_model_input_output(input_output, add_pen_header=False)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.passed_count == 0 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched[0].packet.port == 1)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch[0])
        self._trigger.get_dict_report()

    @init_trigger_test(steps=1)
    def test_single_desc_to_packet_flow_mismatch(self):
        input_descr = self.Descriptor(1)
        input_ring = self.Ring(self, 1, 100)
        input_output = {"input": [{"descriptor": {"descr": input_descr, "ring": input_ring, "id": 1}}],
                        "output": [{"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               "Dot1Q": {"vlan": 12},
                                               "IP": {"src": "10.0.0.64",
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1}
                                   ]
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=False)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        input_output["output"][0]["packet"]["Dot1Q"]["vlan"] = 13
        self.__add_model_input_output(input_output, add_pen_header=False)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        result, details = self._trigger.get_result(
            self._test_case_spec.GID())
        pkt_step_detail = details[0].packets
        self.assertTrue(result == Trigger.TEST_CASE_FAILED)
        self.assertTrue(len(pkt_step_detail.mismatch) == 1)
        self.assertTrue(len(pkt_step_detail.missing) == 0)
        self.assertTrue(
            pkt_step_detail.mismatch[0].mismatch.headers.Dot1Q.mismatch_fields.vlan)
        report = self._trigger.get_run_report()
        self.assertTrue(report and report.failed_count == 1 and
                        report.details[self._test_case_spec.GID()].id == self._test_case_spec.GID() and
                        report.details[self._test_case_spec.GID()].steps_passed == 0)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.extra == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.missing == [])
        self.assertTrue(
            len(report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch) == 1)
        self.assertTrue(report.details[self._test_case_spec.GID()].session[0]
                        .step.result.packets.mismatch[0].packet.actual.port == 1)
        self.assertTrue(report.details[self._test_case_spec.GID()].session[0]
                        .step.result.packets.mismatch[0].packet.expected.ports == [1])

    @init_trigger_test(steps=1)
    def test_single_desc_to_desc_flow(self):

        input_descr = self.Descriptor(1)
        input_ring = self.Ring(self, 1, 100)
        output_descr = self.Descriptor(2)
        output_descr_1 = self.Descriptor(3)
        output_ring = self.Ring(self, 2, 100)
        input_output = {"input": [{"descriptor": {"descr": input_descr, "ring": input_ring, "id": 1}}],
                        "output": [{"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 2}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 3}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=False)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        self.__add_model_input_output(input_output, add_pen_header=False)
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
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == 3)
        self._trigger.get_dict_report()

    @init_trigger_test(steps=1)
    def test_single_desc_to_desc_missing_one(self):

        input_descr = self.Descriptor(1)
        input_ring = self.Ring(self, 1, 100)
        output_descr = self.Descriptor(2)
        output_descr_1 = self.Descriptor(3)
        output_descr_2 = self.Descriptor(4)
        output_ring = self.Ring(self, 2, 100)
        input_output = {"input": [{"descriptor": {"descr": input_descr, "ring": input_ring, "id": 1}}],
                        "output": [{"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 2}},
                                   {"descriptor": {"descr": output_descr_2,
                                                   "ring": output_ring, "id": 3}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=False)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        input_output["output"].pop()
        self.__add_model_input_output(input_output, add_pen_header=False)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
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
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == 2)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.missing and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.missing) == 1)
        self._trigger.get_dict_report()

    @init_trigger_test(steps=1)
    def test_single_desc_to_desc_extra_one(self):

        input_descr = self.Descriptor(1)
        input_ring = self.Ring(self, 1, 100)
        output_descr = self.Descriptor(2)
        output_descr_1 = self.Descriptor(3)
        output_descr_2 = self.Descriptor(4)
        output_ring = self.Ring(self, 2, 100)
        input_output = {"input": [{"descriptor": {"descr": input_descr, "ring": input_ring, "id": 1}}],
                        "output": [{"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 2}},
                                   {"descriptor": {"descr": output_descr_2,
                                                   "ring": output_ring, "id": 3}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=False)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        input_output["output"].append(input_output["output"][0])
        self.__add_model_input_output(input_output, add_pen_header=False)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
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
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched) == 3)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.extra and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.extra) == 1)
        self._trigger.get_dict_report()

    @init_trigger_test(steps=1)
    def test_single_packet_to_desc_flow(self):

        output_descr = self.Descriptor(1)
        input_ring = self.Ring(self, 1, 100)
        output_descr_1 = self.Descriptor(2)
        output_ring = self.Ring(self, 2, 100)
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
                                   "count": 1,
                                   "id": 1}],
                        "output": [{"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 1}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=False)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        self.__add_model_input_output(input_output, add_pen_header=True)
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
    def test_single_packet_to_desc_with_buffer(self):

        output_descr = self.Descriptor(1)
        output_descr.address = self.Buffer(1, bytearray(10))
        input_ring = self.Ring(self, 1, 100)
        output_descr_1 = self.Descriptor(2)
        output_descr.address = self.Buffer(2, bytearray(10))
        output_ring = self.Ring(self, 2, 100)
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
                                   "count": 1,
                                   "id": 1}],
                        "output": [{"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 1}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=False)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        self.__add_model_input_output(input_output, add_pen_header=True)
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
    def test_single_packet_to_desc_with_buffer_mismatch(self):

        output_descr = self.Descriptor(1)
        output_descr.address = self.Buffer(1, bytearray(10))
        input_ring = self.Ring(self, 1, 100)
        output_descr_1 = self.Descriptor(2)
        output_descr_1.address = self.Buffer(2, bytearray(10))
        output_ring = self.Ring(self, 2, 100)
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
                                   "count": 1,
                                   "id": 1}],
                        "output": [{"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 1}}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=False)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        output_descr.address = self.Buffer(1, bytearray(10))
        # Corrupt data
        output_descr.address.data[5] = 7
        input_ring = self.Ring(self, 1, 100)
        output_descr_1 = self.Descriptor(2)
        output_descr_1.address = self.Buffer(2, bytearray(10))
        # Corrupt data
        output_descr_1.address.data[5] = 7
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
                                   "count": 1,
                                   "id": 1}],
                        "output": [{"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 1}}
                                   ],
                        }
        self.__add_model_input_output(input_output, add_pen_header=True)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
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
        # print(convert_object_to_dict(
        # report.details[self._test_case_spec.GID()].session[0].step.result.descriptors,
        # ignore_keys=[]))
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch) == 2)
        self._trigger.get_dict_report()

    @init_trigger_test(steps=1)
    def test_single_packet_to_desc_with_buffer_data_packet_mismatch(self):

        output_descr = self.Descriptor(1)
        output_descr.address = self.Buffer(1, bytearray(10))
        input_ring = self.Ring(self, 1, 100)
        output_descr_1 = self.Descriptor(2)
        output_descr_1.address = self.Buffer(2, bytearray(10))
        output_ring = self.Ring(self, 2, 100)
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
                                   "count": 1,
                                   "id": 1}],
                        "output": [{"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 1}},
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
                                    "count": 1,
                                    "id": 1}
                                   ],
                        }
        self._add_trigger_expect_packet(
            self._test_case_spec.session[0].step, input_output, add_pen_header=True)
        self._tc_doorbell.ring = input_ring._id
        self._test_case_spec.session[0].step.trigger.doorbell.object = self._tc_doorbell
        output_descr.address = self.Buffer(1, bytearray(10))
        # Corrupt data
        output_descr.field_a = 8
        for index in range(3, 8):
            output_descr.address.data[index] = 7
        input_ring = self.Ring(self, 1, 100)
        output_descr_1 = self.Descriptor(2)
        output_descr_1.address = self.Buffer(2, bytearray(10))
        # Corrupt data
        for index in range(3, 8):
            output_descr_1.address.data[index] = 7
        output_descr_1.field_a = 5
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
                                   "count": 1,
                                   "id": 1}],
                        "output": [{"descriptor": {"descr": output_descr,
                                                   "ring": output_ring, "id": 1}},
                                   {"descriptor": {"descr": output_descr_1,
                                                   "ring": output_ring, "id": 1}},
                                   {"packet": {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                                                         "dst": MacAddressBase("0000.9999.0002").get(),
                                                         "type": 0x8100
                                                         },
                                               # Corrupt
                                               "Dot1Q": {"vlan": 33},
                                               "IP": {"src": "10.0.0.99",  # Corrupt
                                                      "dst": "10.0.0.65",
                                                      "proto": 6},
                                               "TCP": {"sport": 100, "dport": 200}},
                                    "ports": [1],
                                    "count": 1,
                                    "id": 1}
                                   ],
                        }
        self.__add_model_input_output(input_output, add_pen_header=True)
        self._trigger.run_test_case(self._test_case_spec)
        self._wait_for_test_case_completion()
        report = self._trigger.get_run_report()
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
            report.details[self._test_case_spec.GID()].session[0].step.result.packets.mismatch)
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.matched == [])
        self.assertTrue(
            report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch and
            len(report.details[self._test_case_spec.GID()].session[0].step.result.descriptors.mismatch) == 2)
        self._trigger.get_dict_report()


if __name__ == '__main__':
    import unittest
    suite = unittest.TestSuite()
    # logger.add_stdout()
    suite.addTest(DolTriggerTestWithUTModel(
        'test_single_packet_flow_with_custom_callback'))
    unittest.TextTestRunner(verbosity=2).run(suite)
