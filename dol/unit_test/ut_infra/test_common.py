import mock
import unittest
import time
import datetime
from collections import OrderedDict
mocked_model_modules = ["model_sim", "model_sim.src.model_wrap"]

mocked_modules = [
    "l2segment_pb2", 
    "types_pb2", 
    "tenant_pb2", 
    "interface_pb2", 
    "endpoint_pb2", 
    "session_pb2", 
    "endpoint_pb2_grpc",
    "l2segment_pb2_grpc", 
    "tenant_pb2_grpc", 
    "interface_pb2_grpc", 
    "session_pb2_grpc", 
    "telemetry_pb2_grpc",
    "telemetry_pb2", 
    "tcp_proxy_cb_pb2", 
    "tls_proxy_cb_pb2", 
    "descriptor_aol_pb2", 
    "wring_pb2", 
    "tcp_proxy_cb_pb2_grpc", 
    "tls_proxy_cb_pb2_grpc",
    "descriptor_aol_pb2_grpc",
    "wring_pb2_grpc",
    "GlobalOptions",
    "nwsec_pb2", 
    "nwsec_pb2_grpc", 
    "nw_pb2", 
    "nw_pb2_grpc",
    "acl_pb2", 
    "acl_pb2_grpc",
    "third_party.scapy.arch.windows"
]


def _mock_module_common(modules):
    def mock_parse_args(arg1=None):
        class Object(object):
            pass
        glopts = Object()
        glopts.debug = False
        glopts.verbose = False
        glopts.dryrun = False
        glopts.no_hal = False
        glopts.level = 0
        return glopts

    pb2_mock = mock.MagicMock()
    pb2_mock.return_value = True
    mock_map = {}
    for module in modules:
        mock_map[module] = pb2_mock

    module_patcher = mock.patch.dict('sys.modules', mock_map)
    module_patcher.start()

    import argparse
    from third_party.scapy.all import Packet as ScapyPacket
    mock_object_func_map = {
        argparse.ArgumentParser: {"parse_args": mock_parse_args},
        ScapyPacket: {"show2": mock.MagicMock()},
    }

    for obj_class in mock_object_func_map:
        for func, mock_func in mock_object_func_map[obj_class].items():
            patcher = mock.patch.object(obj_class, func, mock_func)
            patcher.start()


def mock_modules():
    _mock_module_common(mocked_modules)


def mock_model_modules():
    _mock_module_common(mocked_model_modules)


factory_init_done = False


class TestFakedatetime(datetime.datetime):

    def __init__(self):
        pass

    FAKE_TIME = time.time()

    @classmethod
    def time(cls):
        return TestFakedatetime.FAKE_TIME

    @classmethod
    def set_time(cls, new_time):
        TestFakedatetime.FAKE_TIME = new_time


class DolInfraTest(unittest.TestCase):

    _TMP_FILE_DIRECTORY = "/tmp"
    _TMP_PKT_FILE_NAME = "test.pin"
    _TMP_MOD_FILE_NAME = "modules.list"
    _TMP_PKT_FILE = _TMP_FILE_DIRECTORY + "/" + _TMP_PKT_FILE_NAME
    _TMP_MOD_FILE = _TMP_FILE_DIRECTORY + "/" + _TMP_MOD_FILE_NAME

    def setUp(self):
        def isHalDisabled():
            return True
        from infra.common.logging import logger, cfglogger, memlogger, pktlogger
        logger.set_level(0)
        cfglogger.set_level(0)
        memlogger.set_level(0)
        pktlogger.set_level(0)

        self.pb2_mock = mock.MagicMock()
        self.pb2_mock.return_value = True
        mock_map = {}
        for module in mocked_modules:
            mock_map[module] = self.pb2_mock

        self.module_patcher = mock.patch.dict('sys.modules', mock_map)
        self.module_patcher.start()
        db_func_patcher = mock.patch(
            'config.hal.api.IsHalDisabled', isHalDisabled)
        db_func_patcher.start()

    def tearDown(self):
        self.module_patcher.stop()

    def _construct_spkt(self, pkt_data, data_len=64, add_pen_hdr=True, port=None, opcode=None, tc_id=None):
        from infra.penscapy import penscapy
        spkt = None
        order = {"Ether": 0, "Dot1Q": 1, "IP": 2, "TCP": 3, "UDP": 3}
        pkt_data = OrderedDict(
            sorted(pkt_data.items(),  key=lambda t: order[t[0]]))
        for header, hdr_data in pkt_data.items():
            scapyhdl = getattr(penscapy, header)
            shdr = scapyhdl()
            for field, value in hdr_data.items():
                shdr.__setattr__(field, value)
            if spkt != None:
                spkt = spkt / shdr
            else:
                spkt = shdr

        scapyhdl = getattr(penscapy, "PAYLOAD")
        shdr = scapyhdl()
        shdr.data = [0] * data_len
        spkt = spkt / shdr
        if add_pen_hdr:
            scapyhdl = getattr(penscapy, "PENDOL")
            shdr = scapyhdl()
            spkt = spkt / shdr
            scapyhdl = getattr(penscapy, "CRC")
            shdr = scapyhdl()
            if not port and not opcode:
                shdr.crc = 0xBEEF
            else:
                crc_bytes = bytearray(2)
                crc_bytes[0] = port if port else 0
                crc_bytes[1] = opcode if opcode else 0
                shdr.crc = int.from_bytes(bytes(crc_bytes), byteorder='big')
            spkt = spkt / shdr

        # print(convert_scapy_out_to_dict(spkt))
        # spkt.show()
        return spkt
