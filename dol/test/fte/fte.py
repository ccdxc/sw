#! /usr/bin/python3

import test.callbacks.networking.modcbs as modcbs

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseTeardown(tc):
    session = tc.config.flow..GetSession()
    session.SetLabel("FTE_DONE")
    return
