#! /usr/bin/python3

import test.callbacks.networking.modcbs as modcbs
import test.callbacks.eth.toeplitz as toeplitz


def Setup(infra, module):
    modcbs.Setup(infra, module)


def Teardown(infra, module):
    modcbs.Teardown(infra, module)


def Verify(infra, module):
    return modcbs.Verify(infra, module)


def TestCaseSetup(tc):
    modcbs.TestCaseSetup(tc)
    lif = tc.config.dst.endpoint.intf.lif
    lif.rss_enable = True
    # TODO: Pass the key as args to test
    lif.rss_key = toeplitz.toeplitz_msft_key
    lif.Update()


def TestCaseTeardown(tc):
    modcbs.TestCaseTeardown(tc)
    lif = tc.config.dst.endpoint.intf.lif
    lif.rss_enable = True
    lif.rss_key = toeplitz.toeplitz_default_key
    lif.Update()


def TestCaseVerify(tc):
    return modcbs.TestCaseVerify(tc)
