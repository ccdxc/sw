import pdb
import re
import iota.harness.api as api
import iota.test.utils.naples_host as host

def run_command(command, node):
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, node, command)

    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error(f"Failed to run command on host: {node}, command: {command}")

    return resp

def log_lspci_err(node, device, std_err, std_out, command):

    api.Logger.error(f"lspci exited with error on {node}:{device}")
    api.Logger.info(f"{command}")
    api.Logger.info(f"stderr: {std_err}")
    api.Logger.info(f"stdout: {std_out}")

def uptime(node):
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, node, "uptime")
    resp = api.Trigger(req)

    if resp is None:
        api.Logger.error("uptime command failed")
        return

    cmd = resp.commands.pop()
    api.Logger.info(f"{node} uptime: {cmd.stdout}")

    return

def verify_errors_lspci(node, os_type):

    pci_eth_device = '1dd8:1002'
    pci_root_device = '1dd8:1000'
    result = api.types.status.SUCCESS
    uptime(node)

    if os_type == host.OS_TYPE_ESX:
        return api.types.status.SUCCESS

    if not api.IsNaplesNode(node):
        return api.types.status.SUCCESS

    x8_result =  verify_x(node, os_type, "8")
    x16_result =  verify_x(node, os_type, "16")
    if api.types.status.SUCCESS in [x8_result,x16_result]:
        return api.types.status.SUCCESS
    return api.types.status.CRITICAL

def verify_x(node, os_type, mode):

    pci_eth_device = '1dd8:1002'
    pci_root_device = '1dd8:1000'
    status_x = 0
    capability_x = 0
    uptime(node)

    # Verify errors on the bridge (1dd8:1000)
    pci_verif_cmd = f"lspci -nnvvv -d {pci_root_device} | grep Sta"
    resp = run_command(pci_verif_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0 or cmd.stdout == "":
            log_lspci_err(node, pci_root_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.CRITICAL
        api.Logger.info(f"     {pci_verif_cmd}")

        if re.search('UncorrErr\+|FatalErr\+|TrErr\+|Train\+|RxErr\+|SDES\+|ECRC\+', cmd.stdout, re.I):
            api.Logger.error(f'Errors on PCI Device {node}:{pci_root_device}')
            api.Logger.error(cmd.stdout)
            return api.types.status.CRITICAL

        api.Logger.info(f"     No Errors on {pci_root_device}")

    # get a second oppinion, use grep instead of python search
    pci_verif_cmd = f"lspci -nnvvv -d {pci_root_device} | grep Sta: | grep Err+ | grep -v CorrErr+"
    resp = run_command(pci_verif_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code == 0:
            log_lspci_err(node, pci_root_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.CRITICAL

        api.Logger.info(f"     {pci_verif_cmd}")
        api.Logger.info(f"     {cmd.stdout}")

    # Verify PCIe Width came up correctly
    # 1: how many x capable interfaces
    pci_verif_cmd = f"lspci -nnvvv -d {pci_eth_device} | grep LnkCap: | grep -c x{mode}"
    resp = run_command(pci_verif_cmd, node)

    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0 or cmd.stdout == "0":
            log_lspci_err(node, pci_eth_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.CRITICAL

        capability_x = int(cmd.stdout)
        api.Logger.info(f"     x{mode} Capable: {capability_x}")

    # 2: how many x status interface
    pci_verif_cmd = f"lspci -nnvvv -d {pci_eth_device} | grep LnkSta: | grep -c x{mode}"
    resp = run_command(pci_verif_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0 or cmd.stdout == "0":
            log_lspci_err(node, pci_eth_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.CRITICAL

        status_x = int(cmd.stdout)

    if capability_x != status_x:
        api.Logger.error(f"     ERROR: Less configured x{mode} than capable")
        #print detailed status
        pci_verif_cmd = f"lspci -nnvvv -d {pci_eth_device} | grep LnkSta:"
        if run_command(pci_verif_cmd, node):
            cmd = resp.commands.pop()
            if cmd.exit_code != 0 or cmd.stdout == "0":
                log_lspci_err(node, pci_eth_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
                return api.types.status.CRITICAL

        api.Logger.info(f"{cmd.std.out}")

    api.Logger.info(f"     x{mode} Status: {status_x}")

    return api.types.status.SUCCESS
