#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

# Test overview.
# Use driver to set various flow control settings which is send to Naples
# using devcommand and once the same config is applied to card, driver gets
# NQ link event with current link settings.
#

BSD_LINK_PARAM = ["rxpause", "txpause", "rxpause,txpause"]
#
# From ifconfig media line, find the current pause setting
# media: Ethernet autoselect full-duplex,rxpause,txpause> (100GBase-CR4 <full-duplex,rxpause,txpause>)
# Value in "()" is the current settings.
#
BSD_IFCONFIG_MEDIA_CMD = 'ifconfig %s | grep media | cut -f 2 -d "(" | cut -d "<" -f 2 | cut -d ">" -f 1 | cut -d "," -f 2 '

LINUX_LINK_PARAM = ["on", "off"]

def Setup(tc):
    api.Logger.info("Validate flow control")

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if tc.os != host.OS_TYPE_BSD and tc.os != host.OS_TYPE_LINUX:
        api.Logger.info("Not implemented for %s" % tc.os)
        return api.types.status.IGNORED

    return api.types.status.SUCCESS

def bsd_flow_ctrl(node, inf, fc_type, fc_val, pattern):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, node, 'sysctl dev.%s.flow_ctrl=%d' %
                               (host.GetNaplesSysctl(inf), fc_type))
    api.Trigger_AddHostCommand(req, node, 'sysctl dev.%s.link_pause=%d' %
                               (host.GetNaplesSysctl(inf), fc_val))
    api.Trigger_AddHostCommand(req, node, BSD_IFCONFIG_MEDIA_CMD  % inf)
    resp = api.Trigger(req)
    # We are interested in only last command response.
    cmd = resp.commands[2]
    api.Logger.info("Setting %s link type: %d value: %d pattern: %s" %
                    (inf, fc_type, fc_val, pattern))
    if cmd.exit_code != 0:
        api.Logger.error("Failed exit code: %d link type: %d value: %d, stderr: %s" %
                         (cmd.exit_code, fc_type, fc_val, cmd.stderr))
        api.PrintCommandResults(cmd)
        return -1

    if cmd.stdout.find("[\n\t]*" + pattern + "[\n\t]*") != -1:
        api.Logger.error("Failed link type: %d value: %d, stdout: %s" %
                         (cmd.exit_code, fc_type, fc_val, cmd.stdout))
        api.PrintCommandResults(cmd)
        return -1
    return 0

def linux_flow_ctrl(node, inf, rx_val, tx_val):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, node, 'ethtool -A %s rx %s tx %s' %
                               (inf, rx_val, tx_val))
    api.Trigger_AddHostCommand(req, node, 'ethtool -a %s | grep RX | cut -d : -f 2' %
                               (inf))
    api.Trigger_AddHostCommand(req, node, 'ethtool -a %s | grep TX | cut -d : -f 2' %
                               (inf))
    resp = api.Trigger(req)
    cmd = resp.commands[0]
    api.Logger.info("Setting %s link value rx: %s tx: %s" %
                    (inf, rx_val, tx_val))
    # Check Rx returned value which is of form 'Rx:    <>'
    cmd = resp.commands[1]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to get rx pause value, exit code: %d stderr: %s" %
                         (cmd.exit_code, cmd.stderr))
        api.PrintCommandResults(cmd)
        return -1
    if cmd.stdout.find("[\t]*" + rx_val + "[\t]*") != -1:
        api.Logger.error("rx pause value mismatch, expected: %s found: %s" %
                         (rx_val, cmd.stdout))
        api.PrintCommandResults(cmd)
        return -1
    # Check Tx returned value which is of form 'Tx:    <>'
    cmd = resp.commands[2]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to get tx pause value, exit code: %d stderr: %s" %
                         (cmd.exit_code, cmd.stderr))
        api.PrintCommandResults(cmd)
        return -1
    if cmd.stdout.find("[\t]*" + tx_val + "[\t]*") != -1:
        api.Logger.error("tx pause value mismatch, expected: %s found: %s" %
                         (tx_val, cmd.stdout))
        api.PrintCommandResults(cmd)

    return 0

def Trigger(tc):
    if tc.os != host.OS_TYPE_BSD and tc.os != host.OS_TYPE_LINUX:
        api.Logger.info("Not implemented for %s" % tc.os)
        return api.types.status.IGNORED

    for node in tc.nodes:
        for intf in api.GetNaplesHostInterfaces(node):
            if tc.os == host.OS_TYPE_BSD:
                for i, val in enumerate(BSD_LINK_PARAM):
                    # Valid values start at 1:
                    # 1 - rx, 2 - tx & 3 - rx,tx
                    if bsd_flow_ctrl(node, intf, 1, i+1, val) == -1:
                        return api.types.status.FAILURE
            else:
	        # Rx and Tx value is same which is 'on' & 'off'
                for i, rx_val in enumerate(LINUX_LINK_PARAM):
                    for j, tx_val in enumerate(LINUX_LINK_PARAM):
                        if linux_flow_ctrl(node, intf, rx_val, tx_val) == -1:
                            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):

    for node in tc.nodes:
        # this is required to bring the testbed into operation state
        # after driver unload interfaces need to be initialized
        hw_config.ReAddWorkloads(node)


    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
