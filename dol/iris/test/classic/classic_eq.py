#! /usr/bin/python3

import iris.test.classic.classic as classic
from iris.test.classic.classic import *

from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger

def TestCaseSetup(tc):
    classic.TestCaseSetup(tc)
    tc.pvtdata.eq = tc.testspec.eq.Get(tc)
    tc.pvtdata.queue = tc.testspec.queue.Get(tc)
    tc.pvtdata.queue.EnableEQ(tc.pvtdata.eq)

def TestCaseTeardown(tc):
    tc.pvtdata.queue.DisableEQ()
    classic.TestCaseTeardown(tc)

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True

    spec = tc.testspec.session[step.step_id].step.expect
    valid = True

    if hasattr(spec, 'armed'):
        actual = tc.pvtdata.queue.IsQstateArmed()
        logger.info("Verify 'armed' actual: %s expected: %s"%(actual, spec.armed))
        if actual != spec.armed:
            valid = False

    return valid
