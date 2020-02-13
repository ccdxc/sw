import iota.harness.api as api
import iota.test.utils.naples_host as host

VXLAN_SERVER_IP = "100.1.1.1"
VXLAN_CLIENT_IP = "100.1.1.2"
VXLAN_SERVER_IPV6 = "3000::1"
VXLAN_CLIENT_IPV6 = "3000::2"

def runHostCmd(node, hostCmd, failExp=False):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, node, hostCmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to run host cmd: %s on host: %s"
                         %(hostCmd, node))
        return api.types.status.FAILURE

    cmd = resp.commands[0]
    if cmd.exit_code != 0 and not failExp:
        api.Logger.error("HOST CMD: %s failed on host: %s,"
                         " exit code: %d  stdout: %s stderr: %s" %
                         (hostCmd, node, cmd.exit_code, cmd.stdout, cmd.stderr))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE
    # Command expected to fail.
    elif cmd.exit_code != 0:
        return api.types.status.FAILURE
    
   # api.Logger.info("HOST CMD: %s SUCCESS on host: %s,"
   #                 " exit code: %d  stdout: %s stderr: %s" %
   #                 (hostCmd, node, cmd.exit_code, cmd.stdout, cmd.stderr))
   # api.PrintCommandResults(cmd)
    
    return api.types.status.SUCCESS

def runNaplesCmd(node, napleCmd, failExp=False):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddNaplesCommand(req, node, napleCmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to run Naples cmd: %s on host: %s"
                         %(napleCmd, node))
        return api.types.status.FAILURE

    cmd = resp.commands[0]
    if cmd.exit_code != 0 and not failExp:
        api.Logger.error("NAPLES CMD: %s failed on host: %s,"
                         " exit code: %d  stdout: %s stderr: %s" %
                         (napleCmd, node, cmd.exit_code, cmd.stdout, cmd.stderr))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE
    elif cmd.exit_code != 0:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

# Look for ionic error string and collect some debug data
def checkForBsdIonicError(node):
    if api.GetNodeOs(node) != host.OS_TYPE_BSD:
        return api.types.status.FAILURE

    status = runHostCmd(node, 'sysctl dev.ionic.0.adq ')
    if status != api.types.status.SUCCESS:
        api.Logger.error("CHECK_ERR: Is ionic loaded on host %s??" %(node))

    status = runHostCmd(node, 'vmstat -i ')

    # Check for errors
    status = runHostCmd(node, 'dmesg | grep "adminq is hung"', True)
    if status == api.types.status.SUCCESS:
        api.Logger.error("CHECK_ERR: Nicmgr not responding on host: %s?" %(node))

    status = runHostCmd(node, 'dmesg | grep "fw heartbeat stuck"', True)
    if status == api.types.status.SUCCESS:
        api.Logger.error("CHECK_ERR: Nicmgr crashed for host: %s?" %(node))

    return api.types.status.SUCCESS

def checkNaplesForError(node):
    status = runNaplesCmd(node, '/nic/bin/halctl show port')
    if status != api.types.status.SUCCESS:
        api.Logger.error("CHECK_ERR: HAL running for host: %s??" %(node))

    return status

def checkForIonicError(node):
    status = runHostCmd(node, 'dmesg | tail -n 10')
    if status != api.types.status.SUCCESS:
        api.Logger.error("CHECK_ERR: Is host: %s UP??" %(node))

    if not api.IsNaplesNode(node):
        return api.types.status.SUCCESS

    if api.GetNodeOs(node) == host.OS_TYPE_BSD:
        status = checkForBsdIonicError(node)
        if status != api.types.status.SUCCESS:
            return status

    return checkNaplesForError(node)

def addBSDVxLAN(node, local_ip, remote_ip, vxlan_ip):
    if api.GetNodeOs(node) != host.OS_TYPE_BSD:
        return api.types.status.SUCCESS
    api.Logger.info("Creating VxLAN on %s with local: %s remote: %s VxLAN IP: %s"
                    % (node, local_ip, remote_ip, vxlan_ip))
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, node, 'sudo ifconfig vxlan0 destroy')
    resp = api.Trigger(req)

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(
        req, node, 'sudo ifconfig vxlan create vxlanid 100 vxlanlocal %s vxlanremote %s'
        % (local_ip, remote_ip))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to create vxlan0 on %s" % (node))
        return api.types.status.FAILURE

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to create vxlan0, stderr: %s"
                         % (cmd.stderr))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(
        req, node, 'sudo ifconfig vxlan0 inet6 accept_rtadv')
    api.Trigger_AddHostCommand(
        req, node, 'sudo ifconfig vxlan0 %s' % (vxlan_ip))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error(
            "Failed to  configure IP %s on vxlan0 on %s" % (vxlan_ip, node))
        return api.types.status.FAILURE

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to  configure IP %s on vxlan0 on %s, stderr: %s"
                         % (vxlan_ip, node, cmd.stderr))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def addLinuxVxLAN(node, local_ip, remote_ip, vxlan_ip):
    if api.GetNodeOs(node) != host.OS_TYPE_LINUX:
        return api.types.status.SUCCESS
    api.Logger.info("Creating VxLAN on %s with local: %s remote: %s VxLAN IP: %s"
                    % (node, local_ip, remote_ip, vxlan_ip))
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, node, 'sudo ip link delele vxlan0')
    resp = api.Trigger(req)

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(
        req, node, 'sudo ip link add vxlan0 type vxlan id 100 local %s remote %s dstport 4789'
        % (local_ip, remote_ip))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to create vxlan0 on %s" % (node))
        return api.types.status.FAILURE

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to create vxlan0 on %s" % (node))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(
        req, node, 'sudo ifconfig vxlan0 %s' % (vxlan_ip))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error(
            "Failed to  configure IP %s on vxlan0 on %s" % (vxlan_ip, node))
        return api.types.status.FAILURE

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to  configure IP %s on vxlan0 on %s, stderr: %s"
                         % (vxlan_ip, node, cmd.stderr))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

# This creates device vxlan0. If it already exist, it deletes and add
def addVxLAN(node, local_ip, remote_ip, vxlan_ip):
    if api.GetNodeOs(node) == host.OS_TYPE_BSD:
        return addBSDVxLAN(node, local_ip, remote_ip, vxlan_ip)
    elif api.GetNodeOs(node) == host.OS_TYPE_LINUX:
        return addLinuxVxLAN(node, local_ip, remote_ip, vxlan_ip)
    
    return api.types.status.FAILURE


def setupVxLAN(ipv4, srv, cli):

    if ipv4:
        srv_addr = srv.ip_address
        cli_addr = cli.ip_address
    else:
        srv_addr = srv.ipv6_address
        cli_addr = cli.ipv6_address

    if ipv4:
        vxlan_ip_str = VXLAN_SERVER_IP
    else:
        vxlan_ip_str = "inet6 " + VXLAN_SERVER_IPV6 + "/120"

    status = addVxLAN(srv.node_name, srv_addr,
                      cli_addr, vxlan_ip_str)
    if status != api.types.status.SUCCESS:
        api.Logger.error("Failed to setup server VxLAN")
        return api.types.status.FAILURE

    if ipv4:
        vxlan_ip_str = VXLAN_CLIENT_IP
    else:
        vxlan_ip_str = "inet6 " + VXLAN_CLIENT_IPV6 + "/120"
    status = addVxLAN(cli.node_name, cli_addr,
                      srv_addr, vxlan_ip_str)
    if status != api.types.status.SUCCESS:
        api.Logger.error("Failed to setup client VxLAN")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS