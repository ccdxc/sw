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

    uptime(node)

    if os_type == host.OS_TYPE_ESX:
        return api.types.status.SUCCESS

    # Verify errors on the bridge (1dd8:1000)
    pci_verif_cmd = f"lspci -nnkvvv -d {pci_root_device} | grep Sta"
    resp = run_command(pci_verif_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0 or cmd.stdout == "":
            log_lspci_err(node, pci_root_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.FAILURE
        api.Logger.info(f"     {pci_verif_cmd}")

        if re.search('UncorrErr\+|FatalErr\+|TrErr\+|Train\+|RxErr\+|SDES\+|ECRC\+', cmd.stdout, re.I):
            api.Logger.error(f'Errors on PCI Device {node}:{pci_root_device}')
            api.Logger.error(cmd.stdout)
            return api.types.status.FAILURE

        api.Logger.info(f"     No Errors on {pci_root_device}")

    # get a second oppinion, use grep instead of python search
    pci_verif_cmd = f"lspci -nnkvvv -d {pci_root_device} | grep Sta: | grep Err+ | grep -v CorrErr+"
    resp = run_command(pci_verif_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code == 0:
            log_lspci_err(node, pci_root_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.FAILURE

        api.Logger.info(f"     {pci_verif_cmd}")
        api.Logger.info(f"     {cmd.stdout}")

    # Verify PCIe Width came up correctly
    # 1: how many x8 capable interfaces
    pci_verif_cmd = f"lspci -nnkvvv -d {pci_eth_device} | grep LnkCap: | grep -c x8"
    resp = run_command(pci_verif_cmd, node)

    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0 or cmd.stdout == "0":
            log_lspci_err(node, pci_eth_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.FAILURE

        capability_x8 = int(cmd.stdout)
        api.Logger.info(f"     x8 Capable: {capability_x8}")

    # 2: how many x8 status interface
    pci_verif_cmd = f"lspci -nnkvvv -d {pci_eth_device} | grep LnkSta: | grep -c x8"
    resp = run_command(pci_verif_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0 or cmd.stdout == "0":
            log_lspci_err(node, pci_eth_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.FAILURE

        status_x8 = int(cmd.stdout)

    if capability_x8 != status_x8:
        api.Logger.error(f"     ERROR: Less configured x8 than capable")
        #print detailed status
        pci_verif_cmd = f"lspci -nnkvvv -d {pci_eth_device} | grep LnkSta:"
        if run_command(pci_verif_cmd, node):
            cmd = resp.commands.pop()
            if cmd.exit_code != 0 or cmd.stdout == "0":
                log_lspci_err(node, pci_eth_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
                return api.types.status.FAILURE

        api.Logger.info(f"{cmd.std.out}")
        return api.types.status.FAILURE

    api.Logger.info(f"     x8 Status: {status_x8}")

    return api.types.status.SUCCESS
