#! /usr/bin/python3
import time
import pexpect
import re
import iota.harness.api as api
import iota.harness.infra.utils.parser as parser
from iota.harness.infra.glopts import GlobalOptions

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    tbspec = parser.JsonParse(GlobalOptions.testbed_json)
    for instance in tbspec.Instances:
        #Check if the node is HPE or not
        if hasattr(instance, 'NodeServer'):
            if instance.NodeServer != "hpe":
                api.Logger.info("Node is not an HPE server")
                result = api.types.status.SUCCESS
                continue
        else:
            api.Logger.info("CIMC ip is {} - ucs server".format(instance.NodeCimcIP))
            result = api.types.status.SUCCESS
            continue

        api.Logger.info("Node server ip is {}".format(instance.NodeCimcIP))
        child = pexpect.spawn("telnet %s 8" % instance.NodeCimcIP)
        try :
            child.expect(">")
            child.sendline("plat_inv dump tbl 0")
            child.expect(">\r\n>")
            plat_inv_dump = child.before
            plat_inv_dump_str = plat_inv_dump.decode("utf-8")
            plat_inv_dump_str = plat_inv_dump_str.replace(" ", "")
            newlist = plat_inv_dump_str.splitlines(True)
            for i in range(len(newlist)):
                if (newlist[i].find("pci_vendor_id:0x1dd8") != -1):
                    i2c_seg_str = newlist[i-5].split(":")[1]
                    break
            i2c_seg = int(i2c_seg_str, 16)
            child.sendline("i2c s %d" % i2c_seg)
            child.expect(">\r\n>")
            child.sendline("i2c s")
            child.expect(">\r\n>")
            child.sendline("i2c a 0x94 w 0x18 r 1")
            child.expect(">\r\n>")
            cpld_temp_board = child.before
            cpld_temp_board_data = cpld_temp_board.decode("utf-8")
            cpld_temp_board_str = cpld_temp_board_data.split('Response Data length 1:')[1]
            cpld_temp_board_str = cpld_temp_board_str.replace("\r\n", "")
            cpld_temp_board_str = cpld_temp_board_str.replace("\x1b[0m", "")
            cpld_temp_board_str = cpld_temp_board_str.replace("\t", "")
            cpld_temp_board_str = cpld_temp_board_str.strip()
            res = int(cpld_temp_board_str, 16)
            api.Logger.info("Board temperature is {}".format(res))
            if (res != 0):
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        except pexpect.TIMEOUT:
            api.Logger.error("Time out encountered exiting")
            result = api.types.status.FAILURE

        child.close()
    return result

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
