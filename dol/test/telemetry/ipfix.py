#! /usr/bin/python3
# Test Module

import pdb
import model_sim.src.model_wrap as model_wrap
from infra.common.glopts import GlobalOptions

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    return

def TestCaseTrigger(tc):
    if (GlobalOptions.dryrun):
        return
    upd = 3
    lif_id = 1005
    queue_type = 0
    pid = 0
    queue_id = 0
    ring_id = 0
    p_index = 0

    address = 0x800000 + (upd << 17) + (lif_id << 6) + (queue_type << 3)
    data = (pid << 48) | (queue_id << 24) | (ring_id << 16) | p_index
    model_wrap.step_doorbell(address, data)
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    return
