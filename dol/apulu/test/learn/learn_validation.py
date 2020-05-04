#! /usr/bin/python3
import pdb
from infra.common.logging import logger
import apollo.test.callbacks.common.modcbs as modcbs
import apollo.test.utils.pdsctl as pdsctl
import apollo.config.utils as utils
import apollo.test.utils.learn as learn
import learn_pb2 as learn_pb2

def VerifyCounter(group, index, val):
    stats = learn.GetLearnStatistics()
    if not group in stats.keys(): return False
    ctr_group = stats[group]
    counters = {}
    if group == "validationerrors":
        for verr in ctr_group:
            counters[verr['validationtype']] = verr['count']
    elif group == "dropstats":
        for dstat in ctr_group:
            counters[dstat['reason']] = dstat['numdrops']
    return index in counters.keys() and counters[index] == val

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return True

def Teardown(infra, module):
    if utils.IsDryRun(): return True
    if module.name == "LEARN_NEG_IP_NOT_IN_SUBNET":
        return learn.ClearAllLearntMacs() and learn.ClearLearnStatistics()
    return True

def TestCaseSetup(tc):
    tc.AddIgnorePacketField('UDP', 'sport')
    tc.AddIgnorePacketField('UDP', 'chksum')
    tc.AddIgnorePacketField('TCP', 'chksum')
    return True

def TestCaseTeardown(tc):
    return True

def TestCasePreTrigger(tc):
    return True

def TestCaseStepSetup(tc, step):
    return True

def TestCaseStepTrigger(tc, step):
    return True

def TestCaseStepVerify(tc, step):
    if utils.IsDryRun(): return True
    if "LEARN_MULTIPLE_IP_SAME_VNIC" in tc.module.name:
        ep_mac = learn.EpMac(tc.config.localmapping)
        ep_ip = learn.EpIp(tc.config.localmapping)
        return learn.ExistsOnDevice(ep_mac) and learn.ExistsOnDevice(ep_ip)
    elif tc.module.name == "LEARN_NEG_MULT_UNTAG_VNIC_SAME_SUBNET":
        ep_mac = learn.EpMac(tc.config.localmapping)
        val_type = learn_pb2.LEARN_CHECK_UNTAGGED_MAC_LIMIT
        drop_reason = learn_pb2.LEARN_PKTDROP_REASON_LEARNING_FAIL
        return VerifyCounter("validationerrors", val_type, 1) and \
               VerifyCounter("dropstats", drop_reason, 1)
    elif tc.module.name == "LEARN_NEG_IP_NOT_IN_SUBNET":
        ep_ip = learn.EpIp(tc.config.localmapping)
        val_type = learn_pb2.LEARN_CHECK_IP_IN_SUBNET
        drop_reason = learn.learn_pb2.LEARN_PKTDROP_REASON_LEARNING_FAIL
        return VerifyCounter("validationerrors", val_type, 1) and \
               VerifyCounter("dropstats", drop_reason, 2)
    return True

def TestCaseStepTeardown(tc, step):
    return True

def TestCaseVerify(tc):
    return True
