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


def TestCaseTeardown(tc):
    modcbs.TestCaseTeardown(tc)


def TestCaseVerify(tc):
    return modcbs.TestCaseVerify(tc)
