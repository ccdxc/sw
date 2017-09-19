import mock
import unittest
import os
import sys
import subprocess
from infra.common import defs
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
    "proxy_pb2",
    "ipseccb_pb2",
    "tls_proxy_cb_pb2",
    "descriptor_aol_pb2",
    "wring_pb2",
    "cpucb_pb2",
    "tcp_proxy_cb_pb2_grpc",
    "proxy_pb2_grpc",
    "ipseccb_pb2_grpc",
    "tls_proxy_cb_pb2_grpc",
    "descriptor_aol_pb2_grpc",
    "wring_pb2_grpc",
    "cpucb_pb2_grpc",
    "GlobalOptions",
    "nwsec_pb2",
    "nwsec_pb2_grpc",
    "nw_pb2",
    "nw_pb2_grpc",
    "acl_pb2",
    "acl_pb2_grpc",
    "l4lb_pb2",
    "l4lb_pb2_grpc",
    "crypto_keys_pb2",
    "crypto_keys_pb2_grpc",
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
        glopts.no_hal = True
        glopts.modlist = "dol_test_modules.list"
        glopts.topology = "base/base.spec"
        glopts.cfgonly = False
        glopts.test = None
        glopts.tcid = None
        glopts.level = 0
        return glopts

    def show2_mock(*args, **kwargs):
        pass
    pb2_mock = mock.MagicMock()
    pb2_mock.return_value = True
    mock_map = {}
    for module in modules:
        mock_map[module] = pb2_mock

    module_patcher = mock.patch.dict('sys.modules', mock_map)
    module_patcher.start()

    import argparse
    mock_object_func_map = {
        argparse.ArgumentParser: {"parse_args": mock_parse_args},
    }

    for obj_class in mock_object_func_map:
        for func, mock_func in mock_object_func_map[obj_class].items():
            patcher = mock.patch.object(obj_class, func, mock_func)
            patcher.start()

    db_func_patcher = mock.patch(
        "infra.engine.engine.ProcessReport", show2_mock)
    db_func_patcher.start()

    from third_party.scapy.all import Packet as ScapyPacket
    from infra.factory.pktfactory import Packet
    mock_object_func_map = {
        ScapyPacket: {"show2": show2_mock},
        Packet: {"Show": show2_mock},

    }

    for obj_class in mock_object_func_map:
        for func, mock_func in mock_object_func_map[obj_class].items():
            patcher = mock.patch.object(obj_class, func, mock_func)
            patcher.start()


def mock_modules():
    _mock_module_common(mocked_modules)


def mock_model_modules():
    _mock_module_common(mocked_model_modules)


class DolMainTest(unittest.TestCase):

    def mock_model_start(self):
        mock_model = os.environ["WS_TOP"] + \
            defs.DOL_PATH + "mockmodel/mockmodel"
        self.proc = subprocess.Popen([mock_model], shell=False)

    def mock_model_stop(self):
        print("Kill mock", self.proc.pid)
        subprocess.call(["kill", "-9", "%d" % self.proc.pid])

    def setUp(self):
        mock_modules()

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
        self.mock_model_start()

    def module_result_matcher(self, module, result):

        from infra.engine.trigger import Trigger
        module_match = {"PKT_TO_PKT": lambda res:  len(
            res.session[0].step.result.packets.matched) == 1,
            "PKT_SESSION": lambda res:  len(
            res.session[0].step.result.packets.matched) == 1,
            "PKT_SESSION_FAIL": lambda res:  (len(
                res.session[0].step.result.packets.mismatch) == 1 and
            res.session[1].step.result == Trigger.TEST_CASE_NOT_RUN),
            "PKT_VXLAN": lambda res:  len(
            res.session[0].step.result.packets.matched) == 1,
            "PKT_TCP_OPTS": lambda res:  len(
            res.session[0].step.result.packets.matched) == 0,
            "PKT_TO_PKT_MATCH": lambda res:  len(
            res.session[0].step.result.packets.matched) == 1,
            "PKT_TO_PKT_MISMATCH": lambda res:  len(
            res.session[0].step.result.packets.mismatch) == 1,
            "PKT_TO_PKT_MISSING": lambda res:  len(
            res.session[0].step.result.packets.missing) == 1,
            "PKT_TO_PKT_EXTRA": lambda res:  len(
            res.session[0].step.result.packets.extra) == 1}

        if not module_match[module](result):
            print("Module failed:", module)
            self.assertTrue(0)

    def test_dol_main(self):
        from infra.engine.trigger import TrigExpEngine
        ws_top = os.getenv("WS_TOP")
        sys.argv[0] = ws_top + "/dol/main.py"
        # This run default main.
        import main
        main.Main()
        report = TrigExpEngine.get_run_report()
        self.assertTrue(report)

        from infra.engine.engine import TestCaseDB
        for tcid, tcreport in report.details.items():
            tc = TestCaseDB.Get(tcid)
            self.module_result_matcher(tc.module.name, tcreport)

    def tearDown(self):
        self.mock_model_stop()
        self.module_patcher.stop()


if __name__ == '__main__':
    import unittest
    suite = unittest.TestSuite()
    # logger.add_stdout()
    suite.addTest(DolMainTest(
        'test_dol_main'))
    unittest.TextTestRunner(verbosity=2).run(suite)
