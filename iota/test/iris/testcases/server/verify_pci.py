import re
import iota.harness.api as api

def verify_errors_lspci (node):

    pcie_dev_list = [ '1dd8:1000' ]
    
    #                  '1dd8:1001', \
    #                  '1dd8:1002', \
    #                  '1dd8:1004'  \
    #                  '1dd8:1007'

    for pci_device in pcie_dev_list:
        lspci_cmd = f"lspci -nnkvvv -d {pci_device} | grep Sta"
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddHostCommand(req, node, lspci_cmd)
        resp = api.Trigger(req)

        if resp is None:
            api.Logger.error(f"Failed to run lspci on {node}:{pci_device}" )
            return api.types.status.FAILURE

        cmd = resp.commands.pop()

        if cmd.exit_code != 0 or cmd.stdout == "": 
            api.Logger.error(f"lspci exited with error on {node}:{pci_device}" )
            api.Logger.info (cmd.stderr)
            return api.types.status.FAILURE

        lspci_out = cmd.stdout

        #if re.search( 'CorrErr\+|UncorrErr\+|FatalErr\+|TrErr\+|Train\+|RxErr\+|SDES\+|ECRC\+', lspci_out, re.I ):
        if re.search( 'UncorrErr\+|FatalErr\+|TrErr\+|Train\+|RxErr\+|SDES\+|ECRC\+', lspci_out, re.I ):
            api.Logger.error(f'Errors on PCI Device {node}:{pci_device}')
            api.Logger.error(lspci_out)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS


