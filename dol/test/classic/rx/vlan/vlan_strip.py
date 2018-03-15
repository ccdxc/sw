#! /usr/bin/python3

import test.callbacks.networking.modcbs as modcbs


def Setup(infra, module):
    modcbs.Setup(infra, module)


def Teardown(infra, module):
    modcbs.Teardown(infra, module)


def Verify(infra, module):
    return modcbs.Verify(infra, module)


def TestCaseSetup(tc):
    modcbs.TestCaseSetup(tc)
    lif = tc.config.dst.endpoint.intf.lif
    lif.vlan_strip_en = True
    lif.Update()


def TestCaseTeardown(tc):
    modcbs.TestCaseTeardown(tc)
    lif = tc.config.dst.endpoint.intf.lif
    lif.vlan_strip_en = False
    lif.Update()

def TestCaseVerify(tc):
    return modcbs.TestCaseVerify(tc)
