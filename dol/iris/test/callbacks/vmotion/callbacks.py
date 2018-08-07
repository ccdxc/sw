#! /usr/bin/python3
import pdb

def GetExpectedPacket(testcase):
    step = testcase.module.iterator.Get().step
    if step == 'FLOW_DROP':
        return None

    return testcase.packets.Get('EXPECTED')

