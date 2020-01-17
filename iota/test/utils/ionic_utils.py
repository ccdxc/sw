import iota.harness.api as api
import iota.test.utils.naples_host as host

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

    status = runHostCmd(node, 'sysctl dev.ionic ')
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
