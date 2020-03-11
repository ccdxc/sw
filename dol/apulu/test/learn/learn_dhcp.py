#! /usr/bin/python3
import pdb
from infra.common.logging import logger
import apollo.test.callbacks.common.modcbs as modcbs
import apollo.test.utils.pdsctl as pdsctl
import apollo.config.utils as utils
import apollo.test.utils.learn as learn

def Setup(infra, module):
    modcbs.Setup(infra, module)
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
    # this module is specific to dhcp feature only
    if tc.module.feature != "learn::dhcp": return False
    if utils.IsDryRun(): return True
    if "LEARN_MAC_FROM_DHCP" in tc.module.name:
        ep_mac = learn.EpMac(tc.config.localmapping)
        return learn.ExistsOnDevice(ep_mac)
    elif "LEARN_IP_FROM_DHCP" in tc.module.name:
        ep_ip = learn.EpIp(tc.config.localmapping)
        return  learn.ExistsOnDevice(ep_ip)
    elif tc.module.name == "CLEAR_IP_THEN_MAC":
        ep_mac = learn.EpMac(tc.config.localmapping)
        ep_ip = learn.EpIp(tc.config.localmapping)
        return learn.ClearOnDevice(ep_ip) and \
                not learn.ExistsOnDevice(ep_ip) and \
                learn.ExistsOnDevice(ep_mac) and \
                learn.ClearOnDevice(ep_mac) and \
                not learn.ExistsOnDevice(ep_mac)
    elif tc.module.name == "CLEAR_IP_ALL":
        ep_ip = learn.EpIp(tc.config.localmapping)
        return learn.ClearAllLearntIps() and \
                not learn.ExistsOnDevice(ep_ip)
    elif tc.module.name == "CLEAR_MAC_ALL":
        ep_mac = learn.EpMac(tc.config.localmapping)
        ep_ip = learn.EpIp(tc.config.localmapping)
        return learn.ClearAllLearntMacs() and \
                not learn.ExistsOnDevice(ep_ip) and \
                not learn.ExistsOnDevice(ep_mac)
    else:
        return True

def TestCaseStepTeardown(tc, step):
    return True

def TestCaseVerify(tc):
    return True
