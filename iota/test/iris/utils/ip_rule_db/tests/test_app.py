import unittest
import sys

from app.udp import *
from app.tcp import *
from app.icmp import *
from app.app import *
from util.proto import *

class TestApp(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestApp, self).__init__(*args, **kwargs)
        self.portList = ["any", "1000 - 20000", "6456", "4789", "0"]

    def test_tcp(self):
        appName = "tcp"
        for sp in self.portList:
            for dp in self.portList:
                kwargs = {"sp":sp, "dp":dp}
                tcp  = Tcp(**kwargs)

                app = getAppInstance(appName, Proto(appName), **kwargs)
                self.assertEqual(app, tcp, "should be same")

                app = getAppInstance(None, Proto(appName), **kwargs)
                self.assertEqual(app, tcp, "should be same")

    def test_udp(self):
        appName = "udp"
        for sp in self.portList:
            for dp in self.portList:
                kwargs = {"sp":sp, "dp":dp}
                udp  = Udp(**kwargs)

                app = getAppInstance(appName, Proto(appName), **kwargs)
                self.assertEqual(app, udp, "should be same")

                app = getAppInstance(None, Proto(appName), **kwargs)
                self.assertEqual(app, udp, "should be same")

    def test_icmp(self):
        appName = "icmp"
        kwargs = {}
        icmp = Icmp(**kwargs)
        app = getAppInstance(appName, Proto(appName), **kwargs)
        self.assertEqual(app, icmp, "should be same")

        app = getAppInstance(None, Proto(appName), **kwargs)
        self.assertEqual(app, icmp, "should be same")
