#!/usr/bin/python3
import os
import sys
import argparse
import json
import requests
import subprocess
import docker
import re

AGENT_URL = "http://localhost:9007/"
NaplesUplinkInterfaces = ["pen-intf1", "pen-intf2"]

DockerEnvClient = docker.from_env()
DockerApiClient = docker.APIClient(base_url='unix://var/run/docker.sock')


def run(cmd, timeout=None, background=False, stdout=None):
    print ("Running : ", cmd)
    if background:
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        return p
    else:
        try:
            p = subprocess.run(cmd, shell=True, timeout=timeout)
        except subprocess.TimeoutExpired:
            print ("timed out : ", cmd)
        if p.returncode:
            print ("Command exit code : ", p.returncode)
            return False

    print ("Success : ", cmd)
    return True


class Interface(object):

    def __init__(self, name, vlan=None, mac_address=None):
        self._name = name
        self._vlan = vlan
        self._mac_address = mac_address
        self._ip_address = "0.0.0.0"
        self._prefix_len = 0
        self._vlan_intfs = {}

    def AddVlanInterface(self, name, vlan):
        intf = Interface(name, vlan)
        self._vlan_intfs[vlan] = intf

    def GetVlanInterface(self, vlan):
        return self._vlan_intfs.get(vlan, None)

    def DeleteVlanInterface(self, vlan):
        if vlan in self._vlan_intfs:
            del self._vlan_intfs[vlan]

    def PrintInterfaceInformation(self):
        print("\t\t Intetrface name     : ", self._name)
        print("\t\t Mac address         : ", self._mac_address)
        print("\t\t Encap Vlan          : ", self._vlan)
        print("\t\t IP Address          : ", self._ip_address)
        for intf_name in self._vlan_intfs:
            self._vlan_intfs[intf_name].PrintInterfaceInformation()

    def SetIpAddress(self, address, prefix_len):
        self._ip_address = address
        self._prefix_len = prefix_len


class NS(object):

    def __init__(self, name, skip_init=False):
        self._name = name
        self._cmd_prefix = "ip netns exec %s" % self._name
        self._interfaces = {}
        if not skip_init:
            self.Init()

    def Init(self):
        # Create name space
        self.Reset()
        cmd = "ip netns add %s" % self._name
        run(cmd)

    def AddVlan(self, interface, vlan, name=None):
        intf = self._interfaces.get(interface)
        if not intf:
            raise Exception("Primary Interface not found!")
        vlan_intf = name if name else intf._name + "_" + str(vlan)
        intf.AddVlanInterface(vlan_intf, vlan)
        cmd = self._cmd_prefix + \
            " ip link add link %s name %s type vlan id %s" \
            % (intf._name, vlan_intf, str(vlan))
        run(cmd)
        cmd = self._cmd_prefix + " ifconfig %s up" % (vlan_intf)
        run(cmd)

    def DeleteVlan(self, interface, vlan):
        intf = self._interfaces.get(interface, None)
        if not intf:
            raise("Primary Interface not found")
        vlan_intf = intf.GetVlanInterface(vlan)
        if not vlan_intf:
            raise("Vlan Interface not found")
        cmd = self._cmd_prefix + \
            " ip link delete dev %s" \
            % (vlan_intf._name)
        run(cmd)
        intf.DeleteVlanInterface(vlan)

    def AttachInterface(self, interface):
        intf = Interface(interface)
        cmd = "ip link set dev %s netns %s" % (interface, self._name)
        run(cmd)
        cmd = self._cmd_prefix + " ifconfig %s up" % (interface)
        run(cmd)
        self._interfaces[interface] = intf

    def DetachInterface(self, interface):
        intf = self._interfaces.get(interface, None)
        if not intf:
            return
        run("ip netns exec " + self._name + " ip link set " + interface + " netns 1")
        run("ifconfig " + interface + " up")
        del self._interfaces[interface]

    def DetachInterfaces(self):
        for intf in list(self._interfaces.keys()):
            self.DetachInterface(intf)

    def SetMacAddress(self, interface, mac_address, vlan=None):
        intf = self._interfaces.get(interface, None)
        if not intf:
            raise("Primary Interface not found")
        if vlan:
            intf = intf.GetVlanInterface(vlan)
            if not intf:
                raise("Vlan Interface not found")

        cmd = self._cmd_prefix + " ifconfig %s hw ether %s" % (intf._name, mac_address)
        intf._mac_address = mac_address
        run(cmd)

    def SetIpAddress(self, interface, ipaddress, prefixlen, vlan=None):
        intf = self._interfaces.get(interface, None)
        if not intf:
            raise("Primary Interface not found")
        if vlan:
            intf = intf.GetVlanInterface(vlan)
            if not intf:
                raise("Vlan Interface not found")
        cmd = self._cmd_prefix + " ifconfig %s %s/%d" % (intf._name,
                                                         ipaddress, int(prefixlen))
        run(cmd)
        intf.SetIpAddress(ipaddress, prefixlen)

    def AddArpEntry(self, ipaddress, mac_addr):
        cmd = self._cmd_prefix + " arp -s %s  %s" % (ipaddress, mac_addr)
        run(cmd)

    def SetDefaultRoute(self, ipaddress):
        pass

    def Reset(self):
        try:
            self.Delete()
        except:
            pass

    def RunCommand(self, cmd, timeout=None, background=False):
        cmd = self._cmd_prefix + " " + cmd
        return run(cmd, timeout, background)

    def Delete(self):
        # Delete name space
        self.DetachInterfaces()
        cmd = "ip netns del %s" % self._name
        run(cmd)

    def GetInftName(self):
        return self._vlan_intf or self._intf

    def PrintAppInformation(self):
        print("\t Namespace           : ", self._name)
        print("\t Interfaces")
        for intf_name in self._interfaces:
            self._interfaces[intf_name].PrintInterfaceInformation()

    def AddArpEntry(self, ipaddress, mac_addr):
        cmd = self._cmd_prefix + " arp -s %s  %s" % (ipaddress, mac_addr)
        run(cmd)

    def MoveInterface(self, interface, ns):
        cmd = self._cmd_prefix + " ip link set " + interface + " netns " + str(ns)
        return run(cmd)


def wait_for_line_log(log_file, line_match):
    log2 = open(log_file, "r")
    loop = 1
    while loop == 1:
        for line in log2.readlines():
            if line_match in line:
                log2.close()
                return


def wait_for_agent_to_be_up():
    agent_log_file = os.environ['HOME'] + "/naples/data/logs/agent.log"
    if not os.path.isfile(agent_log_file):
        open(file, 'w').close()

    wait_for_line_log(agent_log_file, "Starting server at")


def configure_naples_container(naples_container, cfg):
    wait_for_agent_to_be_up()
    for config_name, config_data in cfg.items():
        print ("Configuring : ", config_name)
        url = AGENT_URL + config_data["api"]
        headers = {"Content-Type": "application/json"}
        for cfg_item in config_data[config_name.lower()]:
            payload = json.dumps(cfg_item)
            r = requests.post(url, data=payload, headers=headers)
            text = json.loads(r.text)
            if text["status-code"] != 200:
                print ("Config failed ", cfg_item)
                print ("Error : ", text["error"])
                sys.exit(1)


def _is_ep_remote(endpoint):
    return endpoint["spec"]["interface-type"] != "lif"


def _get_network_from_config(nw_name, full_cfg):
    for network in full_cfg["Networks"]["networks"]:
        if network["meta"]["name"] == nw_name:
            return network


def _get_id_from_string(string):
    patterns = [r'\d+']
    for p in patterns:
        match = re.findall(p, string)
        return int(match[0])


def init_name_space(naples_container_ns, ep_cfg, full_cfg):
    intf_name = NaplesUplinkInterfaces[
        int(_get_id_from_string(ep_cfg["spec"]["interface"])) - 1]
    app = NS(ep_cfg["meta"]["name"])
    naples_container_ns.MoveInterface(intf_name, 1)
    app.AttachInterface(intf_name)
    app.SetMacAddress(intf_name, ep_cfg["spec"]["mac-address"])
    network = _get_network_from_config(ep_cfg["spec"]["network-name"], full_cfg)
    app.AddVlan(intf_name, network["spec"].get("vlan-id"))
    app.SetMacAddress(intf_name, ep_cfg["spec"]["mac-address"], network["spec"].get("vlan-id"))
    app.SetIpAddress(intf_name, ep_cfg["spec"]["ipv4-address"].split("/")[0],
                     24, network["spec"].get("vlan-id"))
    return app


def setup_uplink_namespaces(naples_container, e2e_cfg):
    print ("Configuring uplink namespaces for remote endpoints")
    ns_id = str(DockerApiClient.inspect_container(naples_container.id)["State"]["Pid"])
    naples_container_ns = NS(ns_id, skip_init=True)
    cmd = "ln -s " + "/proc/" + ns_id + "/ns/net /var/run/netns/" + ns_id
    run(cmd)

    remote_eps = [entry for entry in e2e_cfg["Endpoints"]["endpoints"] if _is_ep_remote(entry)]

    app_namespaces = []
    for remote_ep in remote_eps:
        print ("Configuring Remote endpoint : ", remote_ep["meta"]["name"])
        app = init_name_space(naples_container_ns, remote_ep, e2e_cfg)
        app_namespaces.append(app)

    for app in app_namespaces:
        app.PrintAppInformation()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--config-file', dest='config_file',
                        required=True, help='Naples Agent configuration file')
    parser.add_argument('--naplescontainer', dest='naples_container', required=True,
                        help='Naples Container image name')
    args = parser.parse_args()

    naples_container = DockerEnvClient.containers.get(args.naples_container)
    if not naples_container:
        print ("Naples container not found!")
        sys.exit(1)

    e2e_cfg = json.load(open(args.config_file))
    configure_naples_container(naples_container, e2e_cfg)
    setup_uplink_namespaces(naples_container, e2e_cfg)


if __name__ == "__main__":
    main()
