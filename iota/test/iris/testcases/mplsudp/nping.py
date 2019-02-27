#! /usr/bin/python3
import pdb
import re
import time
import ipaddress
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.testcases.telemetry.utils as utils
import iota.test.iris.config.mplsudp.tunnel as tunnel

def IsClassEAddress(ip):
    if ip >= "240.0.0.0" and ip <= "255.255.255.255":
        return True
    else:
        return False

def GetIPOptionsStr(src_ip, dst_ip):
    sd = src_ip + "." + dst_ip
    sd_list = sd.split('.')
    string = r"--ip-options '\x07\x13\x20\x00\x00\x00\x00\xde\xad\xbe\xef\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x{:02x}\x00'".\
                format(*map(int, sd_list))
    return string

def Setup(tc):
    tc.tunnels = tunnel.GetTunnels()
    tc.Workloads = api.GetWorkloads()
    return api.types.status.SUCCESS

def Trigger(tc):
    #Run all commands in serial
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tcp_dump_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []
    proto = "tcp"
    count = 0 
    for tunnel in tc.tunnels:
        w1 = tunnel.ltep
        w2 = tunnel.rtep
        w1_class_e_addr = ""
        w2_class_e_addr = ""

        for sec_ipaddr in w1.sec_ip_addresses:
            if ipaddress.IPv4Address(sec_ipaddr).is_reserved:
                w1_class_e_addr = sec_ipaddr
                api.Logger.info("Class-E IP = %s" % sec_ipaddr)
                break
        for sec_ipaddr in w2.sec_ip_addresses:
            if ipaddress.IPv4Address(sec_ipaddr).is_reserved:
                w2_class_e_addr = sec_ipaddr
                api.Logger.info("Class-E IP = %s" % sec_ipaddr)
                break

        if w1_class_e_addr == "" or w2_class_e_addr == "":
            api.Logger.info("No Class-E Address found")
            return api.types.status.FAILURE

        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, "arp -s %s %s" % (w2_class_e_addr, w2.mac_address))
        api.Logger.info("Add arp entry for %s" % w2_class_e_addr)
        tc.cmd_cookies.append("Add arp entry for %s" % w2_class_e_addr)

        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name, "arp -s %s %s" % (w1_class_e_addr, w1.mac_address))
        api.Logger.info("Add arp entry for %s" % w1_class_e_addr)
        tc.cmd_cookies.append("Add arp entry for %s" % w1_class_e_addr)

        if count % 2  == 0:
            # Traffic to Verify packet is encapped correctly
            cmd_cookie = "%s(%s) --> %s(%s)" % (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
            nping_cmd = utils.GetNpingCmd(proto, w2.ip_address, 2000, w1.ip_address, 1)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, nping_cmd)
            api.Logger.info("Send " + proto + " traffic from %s" % cmd_cookie)
            tc.cmd_cookies.append(proto + " positive test")

            # Traffic to Verify packet is encapped and decapped correctly
            cmd_cookie1 = "%s(%s) --> %s(%s) IP Opts Dest IP: %s" % (w1.workload_name, w1_class_e_addr, w2.workload_name, w2.ip_address, w1.ip_address)
            nping_cmd = utils.GetNpingCmd(proto, w2.ip_address, 2000, w1_class_e_addr)
            nping_cmd += " " + GetIPOptionsStr(w1_class_e_addr, w1.ip_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, nping_cmd)
            api.Logger.info("Send " + proto + " traffic with valid IP options from %s" % cmd_cookie1)
            tc.cmd_cookies.append(proto + " encap decap test")

            # Verify error counter doesn't go up on w1 and w2
            penctl_cmd = r"execute halctl show system statistics drop | grep 'Drop Vf IP'"
            common.AddPenctlCommand(req, w1.node_name, penctl_cmd)
            tc.cmd_cookies.append("Encap decap test statistics")

            cmd_cookie2 = "%s(%s) --> %s(%s) IP Opts Dest IP: 14.15.16.17" % (w1.workload_name, w1.ip_address, w2.workload_name, w2_class_e_addr)
            # Class-E test with invalid IP options
            nping_cmd = utils.GetNpingCmd(proto, w2_class_e_addr, 2000, w1.ip_address)
            nping_cmd += " " + GetIPOptionsStr(w1_class_e_addr, "14.15.16.17")
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, nping_cmd)
            api.Logger.info("Send " + proto + " traffic with invalid IP options from %s" % cmd_cookie2)
            tc.cmd_cookies.append(proto + " encap test with invalid IP options")

            # Verify error counter goes up on w1
            penctl_cmd = r"execute halctl show system statistics drop | grep 'Drop Vf Bad'"
            common.AddPenctlCommand(req, w1.node_name, penctl_cmd)
            tc.cmd_cookies.append("Encap test with invalid IP options statistics")

        if count % 2 == 1: 
            cmd_cookie3 = "%s(%s) --> %s(%s) IP Opts Dest IP: 0.0.0.0" % (w1.workload_name, w1.ip_address, w2.workload_name, w2_class_e_addr)
            # Class-E test with zero as dest IP in IP options
            nping_cmd = utils.GetNpingCmd(proto, w2_class_e_addr, 2000, w1.ip_address)
            nping_cmd += " " + GetIPOptionsStr(w1_class_e_addr, "0.0.0.0")
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, nping_cmd)
            api.Logger.info("Send " + proto + " traffic with zero as dest IP in IP options from %s" % cmd_cookie3)
            tc.cmd_cookies.append(proto + " encap test with 0.0.0.0 IP options")

            # Verify error counter goes up on w1
            penctl_cmd = r"execute halctl show system statistics drop | grep 'Drop Vf Bad'"
            common.AddPenctlCommand(req, w1.node_name, penctl_cmd)
            tc.cmd_cookies.append("Encap test with 0.0.0.0 IP options statistics")

            cmd_cookie4 = "%s(%s) --> %s(%s) IP Opts Dest IP: 224.0.0.1" % (w1.workload_name, w1.ip_address, w2.workload_name, w2_class_e_addr)
            # Class-E test with multicast dest IP in IP options
            nping_cmd = utils.GetNpingCmd(proto, w2_class_e_addr, 2000, w1.ip_address)
            nping_cmd += " " + GetIPOptionsStr(w1_class_e_addr, "224.0.0.1")
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, nping_cmd)
            api.Logger.info("Send " + proto + " traffic with multicast dest IP in IP options from %s" % cmd_cookie4)
            tc.cmd_cookies.append(proto + " encap test with multicast IP options")

            # Verify error counter goes up on w1
            penctl_cmd = r"execute halctl show system statistics drop | grep 'Drop Vf Bad'"
            common.AddPenctlCommand(req, w1.node_name, penctl_cmd)
            tc.cmd_cookies.append("Encap test with multicast IP options statistics")

        count += 1
        if proto is "tcp":
            proto = "udp"
        else:
            proto = "tcp"

        if count == 16:
            break
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    w1_ip_label_mismatch_ctr = 0
    w1_bad_rr_dst_ip_ctr = 0
    w2_ip_label_mismatch_ctr = 0
    w2_bad_rr_dst_ip_ctr = 0

    for cmd in tc.resp.commands:
        matchObj = None
        match = []
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if "statistics" in tc.cmd_cookies[cookie_idx]:
            for line in cmd.stdout.split('\n'):
                if "Label" in line:
                    for s in line.split():    
                        if s.isdigit():
                            if int(s) != (w1_ip_label_mismatch_ctr + 1):
                                if int(s) != (w2_ip_label_mismatch_ctr + 1):
                                    result = api.types.status.FAILURE
                                    api.Logger.info("Test Failed")
                                else:
                                    w2_ip_label_mismatch_ctr = int(s)
                            else:
                                w1_ip_label_mismatch_ctr = int(s)
                if "RR" in line:
                    for s in line.split():
                        if s.isdigit():
                            if int(s) != (w1_bad_rr_dst_ip_ctr + 1):
                                if int(s) != (w2_bad_rr_dst_ip_ctr + 1):
                                    result = api.types.status.FAILURE
                                    api.Logger.info("Test Failed")
                                else:
                                    w2_bad_rr_dst_ip_ctr = int(s)
                            else:
                                w1_bad_rr_dst_ip_ctr = int(s)

            if result != api.types.status.FAILURE:
                api.Logger.info("Test Passed")
        elif "positive test" in tc.cmd_cookies[cookie_idx]:
            lines = cmd.stdout.split('\n')
            for line in lines:
                if "tcp" in tc.cmd_cookies[cookie_idx]:
                    matchObj = re.findall("RA", line)
                    if (len(matchObj) != 0):
                        break
                else:
                    matchObj = re.findall("Port unreachable", line)
                    if (len(matchObj) != 0):
                        break
            if len(matchObj) == 0:
                result = api.types.status.FAILURE
                api.Logger.info("Test Failed")
            else:
                api.Logger.info("Test Passed")
        else:
            api.Logger.info("Test Passed")
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
