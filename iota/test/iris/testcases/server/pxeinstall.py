import time
import subprocess
import socket
import iota.harness.api as api
from iota.harness.infra.exceptions import *
import iota.test.iris.config.netagent.hw_push_config as hw_config



def waitforssh(ipaddr, port=22):

    api.Logger.info("Waiting for IP:%s to be up." % ipaddr)
    for retry in range(180):
        if not ipaddr:
            api.Logger.error("No IP set")
            return False
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ret = sock.connect_ex(('%s' % ipaddr, port))
        sock.settimeout(10)
        if ret == 0:
            api.Logger.info("Host is up after PXE Install! {}".format(ipaddr))
            return True
        time.sleep(20)
        print(".")
    api.Logger.error("Host IP {} did not come up".format(ipaddr))
    return False


def Setup(tc):
    api.Logger.info("PXE Install")

    tc.test_node = api.GetNodes()[0]
    tc.test_node_name = tc.test_node.Name()

#    if len(tc.test_node) == 0:
#        api.Logger.error("No naples node found")
#        return api.types.status.FAILURE
#
#    if len(tc.test_node) > 1:
#        api.Logger.info(f"Expecting one node setup, this testbed has {len(tc.nodes)}")
#        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Trigger(tc):

    cimc_info = tc.test_node.GetCimcInfo()

    tc.cimc_ip_address = cimc_info.GetIp()
    tc.cimc_username = cimc_info.GetUsername()
    tc.cimc_password = cimc_info.GetPassword()

    host_ipaddr = api.GetMgmtIPAddress(tc.test_node_name)

    for install in range(tc.args.install_iterations):

        # save
        api.Logger.info(f"Saving node: {tc.test_node_name}")
        if api.SaveIotaAgentState([tc.test_node_name]) == api.types.status.FAILURE:
            raise OfflineTestbedException

        # touch the file on server to ensure this instance of OS is gone later

        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddHostCommand(req, tc.test_node_name, "touch /naples/oldfs")
        resp = api.Trigger(req)

        if api.Trigger_IsSuccess(resp) is not True:
            api.Logger.error(f"Failed to run command on host {tc.test_node_name}")
            return api.types.status.FAILURE

        # Boot from PXE to intall an OS
        api.Logger.info(f"Starting PXE Install Loop # {install} on {tc.test_node_name}")
        cmd = "ipmitool -I lanplus -H %s -U %s -P %s chassis bootdev pxe options=efiboot" %\
              (tc.cimc_ip_address, tc.cimc_username, tc.cimc_password)
        subprocess.check_call(cmd, shell=True)
        time.sleep(5)

        # reboot server
        cmd = "ipmitool -I lanplus -H %s -U %s -P %s chassis power cycle" %\
              (tc.cimc_ip_address, tc.cimc_username, tc.cimc_password)
        subprocess.check_call(cmd, shell=True)
        time.sleep(180)

        # wait for installation to finish and server to come back
        api.Logger.info(f"Waiting for host to come up: {host_ipaddr}")
        if not waitforssh(host_ipaddr):
            raise OfflineTestbedException

        # Boot from HDD to run the test
        api.Logger.info(f"Setting Boot Order to HDD and rebooting {tc.test_node_name}")
        cmd = "ipmitool -I lanplus -H %s -U %s -P %s chassis bootdev disk options=efiboot" %\
              (tc.cimc_ip_address, tc.cimc_username, tc.cimc_password)
        subprocess.check_call(cmd, shell=True)
        time.sleep(5)

        #reboot server
        cmd = "ipmitool -I lanplus -H %s -U %s -P %s chassis power cycle" %\
              (tc.cimc_ip_address, tc.cimc_username, tc.cimc_password)
        subprocess.check_call(cmd, shell=True)

        api.Logger.info(f"Waiting for host to come up: {host_ipaddr}")
        time.sleep(180)
        if not waitforssh(host_ipaddr):
            raise OfflineTestbedException

        # restore
        api.Logger.info(f"Restoring node: {tc.test_node_name}")
        resp = api.ReInstallImage(fw_version=None, dr_version="latest")
        if resp != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to install images on the testbed")
            raise OfflineTestbedException

        resp = api.RestoreIotaAgentState([tc.test_node_name])
        if resp != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to restore agent state after PXE install")
            raise OfflineTestbedException
        api.Logger.info(f"PXE install iteration #{install} - SUCCESS")
        hw_config.ReAddWorkloads(tc.test_node)

        # check touched file is not present, to ensure this is a new OS instance
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddHostCommand(req, tc.test_node_name, "ls /naples/oldfs")
        resp = api.Trigger(req)

        if api.IsApiResponseOk(resp) is not True:
            api.Logger.error(f"Failed to run command on host {tc.test_node_name}")
            return api.types.status.FAILURE

        cmd = resp.commands.pop()
        if cmd.exit_code == 0:
            api.Logger.error(f"Old file is present in FS after PXE install")
            return api.types.status.FAILURE

        api.Logger.info("PXE boot completed! Host is up.")

    return api.types.status.SUCCESS

def Verify(tc):

    return api.types.status.SUCCESS
