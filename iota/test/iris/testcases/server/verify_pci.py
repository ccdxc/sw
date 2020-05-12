import pdb
import re
import iota.harness.api as api
import iota.test.utils.naples_host as host

pci_partnum_list = {
    "68-0005" : {
        "subsystem_id" : "4002",
        "width" : "8"
    },
    "P18669-001" : {
        "subsystem_id" : "4002",
        "width" : "8"
    },
    "P26968-001": {
        "subsystem_id" : "4008",
        "width" : "8"
    },
    "68-0008" : {
        "subsystem_id" : "4005",
        "width" : "8"
    },
    "P18671-001" : {
        "subsystem_id" : "4007",
        "width" : "8"
    },
    "68-0003" : {
        "subsystem_id" : "4001",
        "width" : "16"
    },
    "68-0004" : {
        "subsystem_id" : "4000",
        "width" : "8"
    },
    "68-0013" : {
        "subsystem_id" : "400A",
        "width" : "16"
    },
    "68-0009" : {
        "subsystem_id" : "4006",
        "width" : "16"
    },
    "68-0011" : {
        "subsystem_id" : "4009",
        "width" : "16"
    }
}

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
    if os_type == host.OS_TYPE_ESX or os_type == host.OS_TYPE_WINDOWS:
        return api.types.status.SUCCESS

    if not api.IsNaplesNode(node):
        return api.types.status.SUCCESS

    uptime(node)
    if ((checkrootporterrors(node) != api.types.status.SUCCESS) and
        (checkpcirootbridge(node) != api.types.status.SUCCESS)):
        api.Logger.error("Errors on PCIe root port/bridge")
        return api.types.status.CRITICAL

    api.Logger.info("No errors on PCIe root port/bridge")
    return  api.types.status.SUCCESS

def checkpcirootbridge(node):
    pci_root_device = '1dd8:1000'
    # Verify errors on the bridge (1dd8:1000)
    pci_verif_cmd = f"lspci -nnvvv -d {pci_root_device} | grep Sta"
    resp = run_command(pci_verif_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0 or cmd.stdout == "":
            log_lspci_err(node, pci_root_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.CRITICAL

        if re.search('UncorrErr\+|FatalErr\+|TrErr\+|Train\+|RxErr\+|SDES\+|ECRC\+', cmd.stdout, re.I):
            api.Logger.error(f'Errors on PCI Device {node}:{pci_root_device}')
            api.Logger.error(cmd.stdout)
            return api.types.status.CRITICAL

    # get a second oppinion, use grep instead of python search
    pci_verif_cmd = f"lspci -nnvvv -d {pci_root_device} | grep Sta: | grep Err+ | grep -v CorrErr+"
    resp = run_command(pci_verif_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code == 0:
            log_lspci_err(node, pci_root_device, cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.CRITICAL
    return api.types.status.SUCCESS

def checkrootporterrors(node):
    pci_verif_cmd = f"lspci -Dnd 1dd8:1000 | awk '{{print $1;}}'"
    resp = run_command(pci_verif_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0:
            log_lspci_err(node, "1dd8:1000", cmd.stderr, cmd.stdout, pci_verif_cmd)
            return api.types.status.CRITICAL
    device = cmd.stdout.rstrip()
    root_port_cmd = f"readlink /sys/bus/pci/devices/{device}"
    resp = run_command(root_port_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0:
            log_lspci_err(node, device, cmd.stderr, cmd.stdout, root_port_cmd)
            return api.types.status.CRITICAL
    port = cmd.stdout
    port = port.split('/')[-2]
    root_port_uerr_cmd = f"lspci -vvv -s {port} | grep UESta"
    resp = run_command(root_port_uerr_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0:
            log_lspci_err(node, port, cmd.stderr, cmd.stdout, root_port_uerr_cmd)
            return api.types.status.CRITICAL

    return api.types.status.SUCCESS
