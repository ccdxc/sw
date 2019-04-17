#! /usr/bin/python3
import re
import iota.harness.api as api
import iota.test.iris.utils.debug as debug_utils
import iota.test.iris.utils.host as host_utils
# tc.desc = 'Extract Driver info, Firmware Info, and Card info'

def Setup(tc):
    api.Logger.info("Driver/Device Info: Tranciever Data")
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    
    return api.types.status.SUCCESS 

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True) 
    
    for n in tc.nodes:
        intfs = api.GetNaplesHostInterfaces(n)
        for i in intfs:
            api.Logger.info("Get EEPOM Data from %s" % i)
            if tc.os == 'linux':
                api.Trigger_AddHostCommand(req, n, "ethtool -m %s" % i)
            elif tc.os == 'freebsd':
                # iota passes interface name in a form of "ionic0". BSD sysctl wants "ionic.0".
                i =i[:len(i)-1] + '.' + i[len(i)-1:] 
                api.Trigger_AddHostCommand(req, n, "sysctl dev.%s.media_status" % i)
            else:
                return api.types.status.FAILURE

    tc.resp = api.Trigger(req)
    
    if tc.resp == None:
        return api.types.status.FAILURE
    
    for cmd in tc.resp.commands: 
        if cmd.exit_code != 0:
            api.Logger.error("Failed to get EEPROM data")
            api.Logger.print(cmd.stderr)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)

        # is EXIT code !0?
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
    
        # does the output have any non-ASCII characters?
        for c in cmd.stdout:
            if  ord (c) not in range (32,127) and ord (c) not in [9,10,13]:
                api.Logger.error("Non-ASCII character in the EEPROM strings: 0x", ord(c))
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
