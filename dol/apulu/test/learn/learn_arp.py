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
    if not utils.IsDryRun():
        return learn.ClearLearnStatistics()
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
    if "LEARN_MAC_IP_WITH_ARP" in tc.module.name:
        # verify pdsctl show learn mac/ip produced correct output
        # TODO: verify show vnic and show mapping output is correct
        ep_mac = learn.EpMac(tc.config.localmapping)
        ep_ip = learn.EpIp(tc.config.localmapping)
        if learn.ExistsOnDevice(ep_mac) and learn.ExistsOnDevice(ep_ip):
            stats = learn.GetLearnStatistics()
            if not stats:
                return False
            return stats['pktsrcvd'] == 1 and \
                    stats['pktssent'] == 1 and \
                    stats['maclearnevents'][1]['count'] == 1 and \
                    stats['iplearnevents'][1]['count'] == 1
        return False
    elif tc.module.name == "RECV_ARP_PROBE_AND_REPLY":
        # verify age is reflected correctly
        if step.step_id == 1:
            ep_ip = learn.EpIp(tc.config.localmapping)
            return learn.VerifyIPAgeRefreshed(tc, ep_ip)
        return True
    elif tc.module.name == "RECV_ARP_PROBES_AND_TIMEOUT_MAC_IP":
        # verify IP is timed out in step 3 and mac in step 4
        if step.step_id == 3:
            ep_ip = learn.EpIp(tc.config.localmapping)
            return not learn.ExistsOnDevice(ep_ip)
        elif step.step_id == 4:
            ep_mac = learn.EpMac(tc.config.localmapping)
            return not learn.ExistsOnDevice(ep_mac)
        else:
            return True
    elif tc.module.name == "CLEAR_MAC":
        # clear the learnt MAC
        ep_mac = learn.EpMac(tc.config.localmapping)
        return learn.ClearOnDevice(ep_mac) and \
                not learn.ExistsOnDevice(ep_mac) and \
                learn.ClearLearnStatistics()
    elif tc.module.name == "LEARN_MAC_WITH_RARP":
        ep_mac = learn.EpMac(tc.config.localmapping)
        if learn.ExistsOnDevice(ep_mac):
            stats = learn.GetLearnStatistics()
            if not stats:
                return False
            return stats['pktsrcvd'] == 1 and \
                    stats['maclearnevents'][1]['count'] == 1
        return False
    return True

def TestCaseStepTeardown(tc, step):
    return True

def TestCaseVerify(tc):
    return True
