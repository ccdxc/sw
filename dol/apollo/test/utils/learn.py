#! /usr/bin/python3
import pdb
import yaml
import uuid
import ipaddress
from infra.common.logging import logger
import apollo.test.utils.pdsctl as pdsctl
import apollo.config.utils as utils

# Active EP states
EP_STATE_CREATED = 2
EP_STATE_PROBING = 3

def Int2IPAddrStr(ip_int):
    return str(ipaddress.ip_address(ip_int))

def Int2MacStr(mac_int):
    mac_hex = "{:012x}".format(mac_int)
    return ':'.join(format(s, '02x') for s in bytes.fromhex(mac_hex))

def List2UuidStr(uuid_list):
    return str(uuid.UUID(bytes=bytes(uuid_list)))

# String representation of constructed UUIDs has certain portions in big
# endian format. This messes up with how pdsctl dsplays and expects UUID
# strings. Do the massaging to make UUID strings uniform
def PdsUuidToUuidStr(pdsuuid):
    return str(uuid.UUID(bytes=pdsuuid.Uuid))

class EpMacRuntimeInfo(object):
    def __init__(self, yaml_str):
        data = yaml.load(yaml_str, Loader=yaml.Loader)
        self.MacStr = Int2MacStr(data['key']['macaddr'])
        self.SubnetUuidStr = List2UuidStr(data['key']['subnetid'])
        self.VnicUuidStr = List2UuidStr(data['vnicid'])
        self.State = data['state']
        self.Ttl = data['ttl']

class EpMac:
    def __init__(self, localmapping):
        self.MacStr = str(localmapping.VNIC.MACAddr)
        self.SubnetUuidStr = PdsUuidToUuidStr(localmapping.VNIC.SUBNET.UUID)
        self.VnicUuidStr = PdsUuidToUuidStr(localmapping.VNIC.UUID)
        self.CliCmd = "learn mac"
        self.CliArgs = "--mac %s --subnet %s" % \
                (self.MacStr, self.SubnetUuidStr)
        self.RuntimeInfo = None

    def ValidateRuntimeInfo(self):
        if not self.RuntimeInfo:
            return False
        # cannot compare generated Id's deterministically
        #if self.VnicUuidStr != self.RuntimeInfo.VnicUuidStr:
        #    return False
        if self.RuntimeInfo.State != EP_STATE_CREATED:
            return False
        return True

class EpIpRuntimeInfo(object):
    def __init__(self, yaml_str):
        data = yaml.load(yaml_str, Loader=yaml.Loader)
        self.IpStr = Int2IPAddrStr(data['key']['ipaddr']['v4orv6']['v4addr'])
        self.VpcUuidStr = List2UuidStr(data['key']['vpcid'])
        self.MacStr = Int2MacStr(data['macinfo']['macaddr'])
        self.SubnetUuidStr = List2UuidStr(data['macinfo']['subnetid'])
        self.State = data['state']
        self.Ttl = data['ttl']

class EpIp:
    def __init__(self, localmapping):
        self.IpStr = str(localmapping.IP)
        self.VpcUuidStr = PdsUuidToUuidStr(localmapping.VNIC.SUBNET.VPC.UUID)
        self.MacStr = str(localmapping.VNIC.MACAddr)
        self.SubnetUuidStr = PdsUuidToUuidStr(localmapping.VNIC.SUBNET.UUID)
        self.CliCmd = "learn ip"
        self.CliArgs = "--ip %s --vpc %s" % \
                (self.IpStr, self.VpcUuidStr)
        self.RuntimeInfo = None

    def ValidateRuntimeInfo(self):
        if not self.RuntimeInfo:
            return False
        if self.MacStr != self.RuntimeInfo.MacStr:
            return False
        if self.SubnetUuidStr != self.RuntimeInfo.SubnetUuidStr:
            return False
        if self.RuntimeInfo.State != EP_STATE_CREATED:
            return False
        return True

# common functions
def PopulateRuntimeInfo(ep_mac_ip):
    ep_mac_ip.RuntimeInfo = None
    status_ok, output = pdsctl.ExecutePdsctlShowCommand(ep_mac_ip.CliCmd, \
                                                        ep_mac_ip.CliArgs, True)
    if not status_ok:
        logger.error(" - ERROR: pdstcl show failed for cmd %s %s" % \
                     (ep_mac_ip.CliCmd, ep_mac_ip.CliArgs))
        return False
    if 'API_STATUS_NOT_FOUND' in output:
        logger.info(" - INFO: entry not found %s" % ep_mac_ip.CliArgs)
        return False
    if isinstance(ep_mac_ip, EpMac):
        ep_mac_ip.RuntimeInfo = EpMacRuntimeInfo(output.split('---')[0])
    else:
        ep_mac_ip.RuntimeInfo = EpIpRuntimeInfo(output.split('---')[0])
    return True

def ExistsOnDevice(ep_mac_ip):
    if (PopulateRuntimeInfo(ep_mac_ip)):
        return ep_mac_ip.ValidateRuntimeInfo()
    else:
        return False

def VerifyIPAgeRefreshed(tc, ep_ip):
    if not PopulateRuntimeInfo(ep_ip):
        return False
    age = tc.config.devicecfg.LearnAgeTimeout
    # allow 5 second worst case test delay
    return ep_ip.RuntimeInfo.Ttl > (age - 5)

def ClearOnDevice(ep_mac_ip):
    status_ok, output = pdsctl.ExecutePdsctlClearCommand(ep_mac_ip.CliCmd, \
                                                         ep_mac_ip.CliArgs)
    if not status_ok:
        logger.error(" - ERROR: pdstcl clear failed for cmd %s %s" % \
                     (ep_mac_ip.CliCmd, ep_mac_ip.CliArgs))
        return False
    return True

def GetAllLearntEpInfo(ep_mac_or_ip):
    if ep_mac_or_ip == "mac":
        cmd = "learn mac"
        runtimeInfo = EpMacRuntimeInfo
    else:
        cmd = "learn ip"
        runtimeInfo = EPIpRuntimeInfo
        status_ok, output = pdsctl.ExecutePdsctlShowCommand(cmd)
        if not status_ok:
            logger.error(" - ERROR: pdstcl show failed for cmd %s" % (cmd))
            return None
        epinfos = output.split('---')
        infos = []
        for epi in epinfos:
            if (epi):
                infos.push(runtimeInfo(epi))
        return infos

def GetAllLearntMacs():
    return GetAllLearntEpInfo("mac")

def GetAllLearntIps():
    return GetAllLearntEpInfo("ip")

def GetLearnStatistics():
    status_ok, output = pdsctl.ExecutePdsctlShowCommand("learn statistics")
    if not status_ok:
        logger.error(" - ERROR: pdstcl show failed for learn statistics")
        return None
    yaml_str = output.split('---')[0]
    data = yaml.load(yaml_str, Loader=yaml.Loader)
    return data

def ClearAllLearntMacs():
    status_ok, output = pdsctl.ExecutePdsctlClearCommand("learn mac")
    if not status_ok:
        logger.error(" - ERROR: pdstcl clearn failed for learn mac")
    return status_ok and "succeeded" in output

def ClearAllLearntIps():
    status_ok, output = pdsctl.ExecutePdsctlClearCommand("learn ip")
    if not status_ok:
        logger.error(" - ERROR: pdstcl clearn failed for learn ip")
    return status_ok and "succeeded" in output

def ClearLearnStatistics():
    status_ok, output = pdsctl.ExecutePdsctlClearCommand("learn statistics")
    if not status_ok:
        logger.error(" - ERROR: pdstcl clearn failed for learn statistics")
    return status_ok and "succeeded" in output
