#!/usr/bin/python
import os
import sys
import docker
import subprocess
import pathlib2
import argparse
import time
import struct
import socket
import json

NAPLES_VER = "v1"
NAPLES_IMAGE = "pensando/naples:" + NAPLES_VER
NAPLES_IMAGE_DIRECTORY = os.environ.get("NAPLES_IMAGE_DIR") or os.environ.get("NAPLES_HOME") or "/vagrant"
NAPLES_IMAGE_FILE = "naples-docker-" + NAPLES_VER + ".tgz"


_DOCKER_CLIENT = docker.from_env()
_DOCKER_API_CLIENT = docker.APIClient(base_url='unix://var/run/docker.sock')


AGENT_PORT = 9007
HAL_GRPC_PORT = 1234
REST_PORT = 8888
SIM_PORT =   777
NAPLES_DATA_DIR = os.environ.get("NAPLES_DATA_DIR") or "/var/run/naples/"
NAPLES_HOME = os.environ.get("NAPLES_HOME") or "/pensando/iota/"
NAPLES_LOGS = NAPLES_HOME + "/varlog"
NAPLES_SYSCONF = NAPLES_HOME + "/sysconfig"
NAPLES_OVERLAY_CONFIG_DIR = "/home/vagrant/configs/config_vxlan_overlay"
NAPLES_IPSEC_CONFIG_DIR = "/home/vagrant/configs/config_ipsec"
NAPLES_TCP_PROXY_CONFIG_DIR = "/home/vagrant/configs/config_tcp_proxy"

DEF_NAPLES_SIM_NAME = "naples-sim"
NAPLES_VOLUME_MOUNTS = {
    NAPLES_DATA_DIR: {'bind': '/naples/data', 'mode': 'rw'},
    NAPLES_LOGS: {'bind': '/var/log', 'mode': 'rw'},
    NAPLES_SYSCONF: {'bind': '/sysconfig/config0', 'mode': 'rw'}
}


NAPLES_CFG_DIR        = "/naples/nic/conf"
HNTAP_CFG_FILE        = "hntap-cfg.json"
HNTAP_QEMU_CFG_FILE   = "hntap-with-qemu-cfg.json"
HNTAP_RESTART_CMD     = "/naples/nic/tools/restart-hntap.sh"
HNTAP_TEMP_CFG_FILE   = "/tmp/hntap-cfg.json"

NAPLES_PORT_MAPS = {
    str(AGENT_PORT) + '/tcp': AGENT_PORT,
    str(SIM_PORT) + '/tcp':SIM_PORT,
    str(REST_PORT) + '/tcp':REST_PORT
}

NAPLES_ENV = {"SMART_NIC_MODE" : "1"}
QEMU_ENV   = {"WITH_QEMU": "1"}
NETAGENT_CTRL_INTF   = {"NETAGENT_CTRL_INTF": ""}
VENICE_IPS = {"VENICE_IPS" : ""}
NMD_HOSTNAME = {"NMD_HOSTNAME" : ""}
NAPLES_UUID = {"SYSUUID" : ""}

# Move it out to a configu
_APP_IMAGES = [
    #"networkstatic/iperf3"
]

_NAPLES_DIRECTORIES = [
    NAPLES_DATA_DIR,
    NAPLES_LOGS,
    NAPLES_SYSCONF,
    NAPLES_OVERLAY_CONFIG_DIR,
    NAPLES_IPSEC_CONFIG_DIR,
    NAPLES_TCP_PROXY_CONFIG_DIR
]

_SETUP_CMDS = [
    "sudo chmod a+rwx /var/run/docker.sock",
    "sudo chmod a+rwx /var/run/openvswitch/db.sock",
    #"sudo systemctl stop firewalld",
    #"sudo setenforce 0",
    "ip link set dev eth1 mtu 9216",
    "ip link set up dev eth2"
]


def preexec():  # Don't forward signals.
    os.setpgrp()

# Run shell command


def RunShellCmd(cmd, timeout=None, background=False, stdout=None):
    cmd = " ".join(cmd)
    print ("Running : ", cmd)
    if background:
        cmd = ['nohup'] + (cmd.split(" "))
        p = subprocess.Popen(cmd, stdout=stdout, preexec_fn=preexec)
    else:
        return_code = subprocess.call(cmd, shell=True)
        if return_code:
            print ("Command exit code : ", return_code)
            return False
        print ("Success : ", cmd)
    return True


def __pull_image(image):
    print "Pulling image :", image
    images_info = _DOCKER_CLIENT.images.pull(image)
    print "Download following images as part of ", image
    for image_info in images_info:
        print "\tImage ID : %s \n" % (image_info.id)


def __pull_app_docker_images():
    for image in _APP_IMAGES:
        __pull_image(image)


def __create_directories():
    for dir in _NAPLES_DIRECTORIES:
        pathlib2.Path(dir).mkdir(parents=True, exist_ok=True)


def __run_setup_cmds():
    for cmd in _SETUP_CMDS:
        RunShellCmd(cmd.split(" "))


def __pull_naples_image():
    username = os.getenv("DOCKER_USERNAME")
    if not username:
        raise "Docker username not set"
    password = os.getenv("DOCKER_PASSWORD")
    if not password:
        raise "Docker password not set"
    _DOCKER_CLIENT.login(username=username, password=password)
    print "Docker login successfull\n"
    __pull_image(NAPLES_IMAGE)


def __load_naples_image():
    print "Trying to import naples image from : %s ...\n" % NAPLES_IMAGE_FILE
    load_cmd = ["docker", "load", "--input", NAPLES_IMAGE_DIRECTORY + "/" + NAPLES_IMAGE_FILE]
    ret = RunShellCmd(load_cmd)
    if not ret:
        raise
    # repo, tag = NAPLES_IMAGE.split(":")
    #_DOCKER_API_CLIENT.import_image(NAPLES_IMAGE_FILE,
    #                              repository=repo,
    #                              tag=tag)
    print "Importing successfull!\n"


def pull_docker_images():
    __pull_app_docker_images()


def bootstrap_naples():
    pass


def __initial_setup():
    __create_directories()
    __run_setup_cmds()



def moveInterfaceToNs(intf, ns):
    cmd = ["ip", "link", "set",  intf, "netns",  ns,  "name",  intf]
    if not RunShellCmd(cmd):
        print "Moving interface %s to naples-sim  failed" % intf
        sys.exit(1)
    cmd = ["ip", "netns", "exec", ns, "ifconfig", intf, "up"]
    RunShellCmd(cmd)
    print "Moving interface %s to naples-sim  success" % intf


def __move_data_intfs_to_naples_sim(container_obj, node_ip, data_intfs):
    pid = str(_DOCKER_API_CLIENT.inspect_container(container_obj.id)["State"]["Pid"])
    for intf in data_intfs:
        moveInterfaceToNs(intf, pid)
        cmd = None
        if node_ip:
            cmd = ["ip", "netns", "exec", pid, "ifconfig", data_intfs[0], node_ip, "up"]
        else:
            cmd = ["ip", "netns", "exec", pid, "ifconfig", data_intfs[0], "up"]

        if not RunShellCmd(cmd):
            print ("Failed to configure IP for intf : %s with  :%s" % (intf, node_ip))
            sys.exit(1)
        print ("Configure IP for intf : %s with  :%s" % (intf, node_ip))


def __move_control_intf_to_naples_sim(container_obj,control_intf, control_ip, control_intf_alias=None):
    pid = str(_DOCKER_API_CLIENT.inspect_container(container_obj.id)["State"]["Pid"])
    moveInterfaceToNs(control_intf, pid)
    cmd = ["ip", "netns", "exec", pid, "ifconfig", control_intf, "up"]
    if control_ip:
        cmd = ["ip", "netns", "exec", pid, "ifconfig", control_intf, control_ip, "up"]
    if not RunShellCmd(cmd):
        print ("Failed to configure IP for intf : %s with  :%s" % (control_intf, control_ip))
        sys.exit(1)

    if control_intf_alias:
        cmd = ["ip", "netns", "exec", pid, "ifconfig", control_intf, "down"]
        if not RunShellCmd(cmd):
            print ("Failed to bring down intf : %s " % (control_intf))
            sys.exit(1)
        cmd = ["ip", "netns", "exec", pid, "ip", "link", "set", control_intf, "name", control_intf_alias]
        if not RunShellCmd(cmd):
            print ("Failed to rename interface : %s " % (intf))
            sys.exit(1)
        if not control_ip:
            cmd = ["ip", "netns", "exec", pid, "dhclient", control_intf_alias]
            if not RunShellCmd(cmd):
                print ("Failed to run dhclient : %s " % (control_intf_alias))
                sys.exit(1)


def __setup_hntap(container_obj, args):
    def __ip2int(ipstr): return struct.unpack('!I', socket.inet_aton(ipstr))[0]

    def __int2ip(n): return socket.inet_ntoa(struct.pack('!I', n))

    if args.data_intfs == None or len(args.data_intfs) == 0:
        print "No data interfaces be specified, skipping hntap setup"
        return

    #TODO, logic has to be cleaned up.
    node_ip = None
    if args.hntap_mode == "tunnel":
        start_ip = __ip2int(args.tunnel_ip_start)
        node_ip = args.data_ips[0]
        if not node_ip:
            node_ip = __int2ip(start_ip + int(args.node_id) - 1)

    __move_data_intfs_to_naples_sim(container_obj, node_ip, args.data_intfs)

    hntapFile = None
    if args.with_qemu:
        hntapFile = NAPLES_CFG_DIR + "/" + HNTAP_QEMU_CFG_FILE
    else:
        hntapFile = NAPLES_CFG_DIR + "/" + HNTAP_CFG_FILE

    cpCmd = ["docker", "cp", args.sim_name + ":" + hntapFile, HNTAP_TEMP_CFG_FILE]

    if not RunShellCmd(cpCmd):
        print "Error in coping hntap config file to host"
        sys.exit(1)

    try:
        hntap_cfg = json.load(open(HNTAP_TEMP_CFG_FILE))
    except Exeception as ex:
        print "Error opening Json file :" + str(ex)
        sys.exit(1)


    if args.hntap_mode == "tunnel":
        #For Tunnel mode, just pick the first data interface.
        peerIPs = [ ip.split("/")[0] for ip in args.naples_ips if ip != args.data_intfs[0] ]
        hntap_cfg["switch"] = {
                                "tunnel-mode" :
                                    {
                                        "out-interface" : args.data_intfs[0],
                                        "peers" : peerIPs
                                    }
                                }
    else:
        #For Pass through mode, associate each uplink interface to data interface.
        hntap_cfg["switch"] = {
                                "passthrough-mode" :
                                    {
                                        "uplink-map": {},
                                        "allowed-macs":{}
                                    }
                               }
        i = 0
        for intf in hntap_cfg["devices"]:
            if not intf["local"]  and (i + 1 <= len(args.data_intfs)):
                hntap_cfg["switch"]["passthrough-mode"]["uplink-map"][intf["name"]] = args.data_intfs[i]
                i += 1

    with open(HNTAP_TEMP_CFG_FILE, "w") as fp:
        json.dump(hntap_cfg, fp)

    cpCmd = ["docker", "cp", HNTAP_TEMP_CFG_FILE,  args.sim_name + ":" + hntapFile]
    if not RunShellCmd(cpCmd):
        print "Error in coping hntap config to naples-sim"
        sys.exit(1)

    cmd_to_run = [HNTAP_RESTART_CMD]
    container_obj.exec_run(cmd_to_run, detach=True, stdout=True, tty=False)

def __setup_ns_for_container(container_obj):
    pid = str(_DOCKER_API_CLIENT.inspect_container(container_obj.id)["State"]["Pid"])
    cmd = ["mkdir", "-p", "/var/run/netns"]
    RunShellCmd(cmd)
    cmd = ["ln", "-s",  "/proc/" + pid + "/ns/net" ,  "/var/run/netns/" + pid]
    RunShellCmd(cmd)

def __setup_data_network(container_obj, args):
    __setup_hntap(container_obj, args)

def __setup_control_network(container_obj, args):
    if args.control_intf:
        __move_control_intf_to_naples_sim(container_obj, args.control_intf, args.control_ip, args.control_intf_alias)


def __wait_for_line_log(log_file, line_match):
    loop = 1
    while loop == 1:
        if os.path.exists(log_file):
            log2 = open(log_file, "r")
            for line in log2.readlines():
                if line_match in line:
                    log2.close()
                    return
        else:
            time.sleep(5)

def __bringdown_naples_container(args):
    try:
        print "Bringing down Naples container : %s " % (args.sim_name)
        _DOCKER_API_CLIENT.stop(args.sim_name)
        time.sleep(5)
    except:
        print "Bringing down Naples container : %s failed " % (args.sim_name)
    if not args.skip_load:
        try:
            print "Removing Naples image : %s" % NAPLES_IMAGE
            _DOCKER_API_CLIENT.remove_image(NAPLES_IMAGE, force=True)
        except:
            print "Removing Naples image : %s failed" % NAPLES_IMAGE


def __bringup_naples_container(args):
    __bringdown_naples_container(args)
    if not args.skip_load:
        try:
            __load_naples_image()
        except:
            print "Loading naples image failed!"
            print "Trying to pull naples image..."
            __pull_naples_image()
    naples_sim_log_file = NAPLES_DATA_DIR + "/logs/start-naples.log"
    agent_log_file = NAPLES_LOGS + "/pensando/pen-netagent.log"
    nmd_log_file = NAPLES_LOGS + "/pensando/pen-nmd.log"

    def __wait_for_naples_sim_to_be_up():
        __wait_for_line_log(naples_sim_log_file, "NAPLES services/processes up and running")

    def __wait_for_agent_to_be_up():
        __wait_for_line_log(agent_log_file, "Agent up and running")

    def __wait_for_nmd_to_be_up():
        __wait_for_line_log(nmd_log_file, "Started NMD Rest server")

    print "Bringing up naples container...\n"
    if args.with_qemu:
        NAPLES_ENV.update(QEMU_ENV)
    if args.venice_ips:
        VENICE_IPS = {"VENICE_IPS" : args.venice_ips}
        NAPLES_ENV.update(VENICE_IPS)
    if args.nmd_hostname:
        NMD_HOSTNAME = {"NMD_HOSTNAME" : args.nmd_hostname}
        NAPLES_ENV.update(NMD_HOSTNAME)
    if args.control_intf:
        NETAGENT_CTRL_INTF = {"NETAGENT_CTRL_INTF" : args.control_intf}
        NAPLES_ENV.update(NETAGENT_CTRL_INTF)
    global NAPLES_PORT_MAPS
    if args.disable_portmap:
        NAPLES_PORT_MAPS = {}
    if args.disable_datapath:
        DISABLE_DATAPATH = {"NO_DATAPATH" : "1"}
        NAPLES_ENV.update(DISABLE_DATAPATH)
    if args.sys_uuid:
        NAPLES_UUID = {"SYSUUID" : args.sys_uuid}
    elif args.control_intf:
        with open("/sys/class/net/" + args.control_intf + "/address", 'r') as myfile:
            NAPLES_UUID = {"SYSUUID" : myfile.read().replace('\n', '')}
    else:
        NAPLES_UUID = {"SYSUUID" : ""}


    NAPLES_ENV.update(NAPLES_UUID)
    
    GRPC_PORT = {"HAL_GRPC_PORT" : str(HAL_GRPC_PORT)}
    NAPLES_ENV.update(GRPC_PORT)

    naples_obj = _DOCKER_CLIENT.containers.run(NAPLES_IMAGE,
                                               name=args.sim_name,
                                               privileged=True,
                                               detach=True,
                                               auto_remove=True,
                                               environment=NAPLES_ENV,
                                               ports=NAPLES_PORT_MAPS,
                                               volumes=NAPLES_VOLUME_MOUNTS)
    __setup_ns_for_container(naples_obj)
    print "Setting control network for  %s " % (NAPLES_IMAGE)

    __setup_control_network(naples_obj, args)
    print "Setting control network   : %s  success" % (NAPLES_IMAGE)


    print "Wating for naples sim to be up"
    time.sleep(5)
    __wait_for_naples_sim_to_be_up()
    print "Wating for agent server to be up"
    time.sleep(20)
    #__wait_for_agent_to_be_up()
    #__wait_for_nmd_to_be_up()
    print "Nic container bring up was successfull"


    if not args.disable_datapath:
        print "Setting uplink data network for  %s " % (NAPLES_IMAGE)
        __setup_data_network(naples_obj, args)
        print "Setting uplink data network  : %s  success" % (NAPLES_IMAGE)


def __run_bootstrap_naples(args):
    cp_cmd = ["cp", NAPLES_DATA_DIR + "/" + "bootstrap-naples.sh", "/usr/bin/bootstrap-naples.sh"]
    RunShellCmd(cp_cmd)
    chmod_cmd = ["chmod", "+x", "/usr/bin/bootstrap-naples.sh"]
    RunShellCmd(chmod_cmd)
    #bootstrap_cmd = ["/usr/bin/bootstrap-naples.sh", str(args.node_id)]
    bootstrap_cmd = ["/usr/bin/bootstrap-naples.sh", "--naples-sim-name", args.sim_name]
    if args.with_qemu:
        bootstrap_cmd.append("--qemu")
    RunShellCmd(bootstrap_cmd, background=True)


def __stop_bootstrap_naples():
    kill_cmd = ["pkill", "-9", "-f", "bootstrap-nap"]
    RunShellCmd(kill_cmd)


def __reset(args):
    __stop_bootstrap_naples()

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--name', dest='sim_name', default=DEF_NAPLES_SIM_NAME,
                        help='Naples sim name to be used.')
    parser.add_argument('--sysuuid', dest='sys_uuid', default=None,
                        help='System uuid to be used for naples')
    parser.add_argument('--network-driver', dest='nw_driver', default="macvlan",
                        choices=["macvlan"], help='Network Driver for docker network')
    parser.add_argument('--control-intf', dest='control_intf', default=None,
                        help='Control interface to be used.')
    parser.add_argument('--control-intf-alias', dest='control_intf_alias', default=None,
                        help='Control interface alias to be used.')
    parser.add_argument('--control-ip', dest='control_ip', default=None,
                        help='Control Network IP to be used.')
    parser.add_argument('--venice-ips', dest='venice_ips', default=None,
                        help='Venice IPs for agent to connect to.')
    parser.add_argument('--naples-ips', dest='naples_ips', default=None,
                        help='Naples IPs for for tunneled mode interaction')
    parser.add_argument('--data-intfs', dest='data_intfs', default=None,
                        help='List of data interfaces to be used.')
    parser.add_argument('--data-ips', dest='data_ips', default=None,
                        help='List of data IPs to be used.')
    parser.add_argument('--network-parent', dest='nw_parent', default="eth2",
                        help='Parent network for docker network')
    parser.add_argument('--tunnel-ip-start', dest='tunnel_ip_start', default="192.168.10.11",
                        help='Start address of tunnel IPs')
    parser.add_argument('--node-id', dest='node_id', default="1",
                        help="Node ID")
    parser.add_argument('--node-count', dest='node_cnt', default=2,
                        help="Node count")
    parser.add_argument('--tunnel-port-prefix', dest='tun_port_prefix', default="vxtun-to-node",
                        help="Tunnel Port prefix to use")
    parser.add_argument('--qemu', dest='with_qemu',
                        action='store_true', help='Setup naples sim for qemu env')
    parser.add_argument('--tunnel', dest='tun_port_prefix', default="vxtun-to-node",
                        help="Tunnel Port prefix to use")
    parser.add_argument('--hntap-mode', dest='hntap_mode', default="tunnel",
                        choices=["tunnel", "passthrough"], help='Hntap Mode to run in naples sim')
    parser.add_argument('--nmd-hostname', dest='nmd_hostname', default=None,
                        help="Nmd hostname")
    parser.add_argument('--disable-portmap', dest='disable_portmap',
                        action='store_true', help='Disable port mapping')
    parser.add_argument('--disable-datapath', dest='disable_datapath',
                        action='store_true', help='Disable datapath')
    parser.add_argument('--skip-image-load', dest='skip_load',
                        action='store_true', help='Skip loading of image')


    args = parser.parse_args()

    if args.data_ips:
        args.data_ips = args.data_ips.split(',')
    if args.data_ips:
        args.naples_ips = args.naples_ips.split(',')
    if args.data_intfs:
        args.data_intfs = args.data_intfs.split(',')
    __reset(args)
    __initial_setup()
    __pull_app_docker_images()
    __bringup_naples_container(args)
    __run_bootstrap_naples(args)


if __name__ == '__main__':
    main()
