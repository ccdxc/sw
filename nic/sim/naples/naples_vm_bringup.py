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

NAPLES_VER = "v1"
NAPLES_IMAGE = "pensando/naples:" + NAPLES_VER
NAPLES_IMAGE_DIRECTORY = os.environ.get("NAPLES_HOME") or "/vagrant"
NAPLES_IMAGE_FILE = "naples-docker-" + NAPLES_VER + ".tgz"


_DOCKER_CLIENT = docker.from_env()
_DOCKER_API_CLIENT = docker.APIClient(base_url='unix://var/run/docker.sock')


AGENT_PORT = 9007
SIM_PORT =   777
NAPLES_DATA_DIR = "/var/run/naples/"
NAPLES_OVERLAY_CONFIG_DIR = "/home/vagrant/configs/config_vxlan_overlay"
NAPLES_IPSEC_CONFIG_DIR = "/home/vagrant/configs/config_ipsec"
NAPLES_TCP_PROXY_CONFIG_DIR = "/home/vagrant/configs/config_tcp_proxy"

NAPLES_SIM_NAME = "naples-sim"
NAPLES_VOLUME_MOUNTS = {
    NAPLES_DATA_DIR: {'bind': '/naples/data', 'mode': 'rw'}
}

NAPLES_PORT_MAPS = {
    str(AGENT_PORT) + '/tcp': AGENT_PORT,
    str(SIM_PORT) + '/tcp':SIM_PORT
}

NAPLES_ENV = {"SMART_NIC_MODE" : "1"}
QEMU_ENV   = {"WITH_QEMU": "1"}

# Move it out to a configu
_APP_IMAGES = [
    "networkstatic/iperf3"
]

_NAPLES_DIRECTORIES = [
    NAPLES_DATA_DIR,
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
    try:
        print "Deleting naples images."
        _DOCKER_API_CLIENT.remove_image(NAPLES_IMAGE, force=True)
    except:
        print "No old naples image found.."
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


def __setup_docker_network(args):
    out = """Setting up docker network
              Name : %s
              Driver : %s
              iprange : %s
              subnet : %s
              gateway : %s
              parent : %s\n""" % (
        args.nw_name, args.nw_driver, args.nw_ip_range,
        args.nw_subnet, args.nw_gateway, args.nw_parent
    )
    print out
    for nw in _DOCKER_CLIENT.networks.list():
        if nw.name == args.nw_name:
            print "Docker network %s already created" % args.nw_name
            return nw
    ipam_pool = docker.types.IPAMPool(subnet=args.nw_subnet,
                                      gateway=args.nw_gateway,
                                      iprange=args.nw_ip_range)
    ipam_config = docker.types.IPAMConfig(pool_configs=[ipam_pool])
    nw = _DOCKER_CLIENT.networks.create(name=args.nw_name, driver=args.nw_driver,
                                        options={"parent": args.nw_parent},
                                        ipam=ipam_config)
    print "Docker network created successfully"
    return nw


def __setup_data_network(args):
    def __ip2int(ipstr): return struct.unpack('!I', socket.inet_aton(ipstr))[0]

    def __int2ip(n): return socket.inet_ntoa(struct.pack('!I', n))

    def __add_tunnel(port_name, peer_ip):
        cmd = ["ovs-vsctl", "add-port", args.ovs_br_name, port_name]
        cmd.extend(["--", "set", "interface", port_name, "type=vxlan"])
        cmd.extend(["options:key=11111", "options:remote_ip=" + peer_ip])
        RunShellCmd(cmd)

    cmd = ["ovs-vsctl", "br-exists", args.ovs_br_name]
    if not RunShellCmd(cmd):
        print ("Setting up OVS bridge : ", args.ovs_br_name)
        cmd = ["ovs-vsctl", "add-br", args.ovs_br_name]
        RunShellCmd(cmd)
    start_ip = __ip2int(args.tunnel_ip_start)
    if int(args.node_id) == 1:
        # If you are the master node (1), connect to all other nodes.
        for peer in range(1, args.node_cnt):
            port_name = args.tun_port_prefix + str(peer + 1)
            peer_ip = __int2ip(start_ip + peer)
            __add_tunnel(port_name, peer_ip)
    else:
        # Else Just connnect to the master node (1).
        __add_tunnel(args.tun_port_prefix + "1", args.tunnel_ip_start)


def __wait_for_line_log(log_file, line_match):
    log2 = open(log_file, "r")
    loop = 1
    while loop == 1:
        for line in log2.readlines():
            if line_match in line:
                log2.close()
                return


def __bringdown_naples_container():
    try:
        print "Bringing down Naples container : %s " % (NAPLES_SIM_NAME)
        _DOCKER_API_CLIENT.stop(NAPLES_SIM_NAME)
        time.sleep(5)
    except:
        print "Bringing down Naples container : %s failed " % (NAPLES_SIM_NAME)
    try:
        print "Removing Naples image : %s" % NAPLES_IMAGE
        _DOCKER_API_CLIENT.remove_image(NAPLES_IMAGE, force=True)
    except:
        print "Removing Naples image : %s failed" % NAPLES_IMAGE


def __bringup_naples_container(args):
    __bringdown_naples_container()
    try:
        __load_naples_image()
    except:
        print "Loading naples image failed!"
        print "Trying to pull naples image..."
        __pull_naples_image()
    naples_sim_log_file = NAPLES_DATA_DIR + "/logs/start-naples.log"
    agent_log_file = NAPLES_DATA_DIR + "/logs/agent.log"

    def __wait_for_naples_sim_to_be_up():
        __wait_for_line_log(naples_sim_log_file, "NAPLES services/processes up and running")

    def __wait_for_agent_to_be_up():
        __wait_for_line_log(agent_log_file, "Starting server at")
    print "Bringing up naples container...\n"
    if args.with_qemu:
        NAPLES_ENV.update(QEMU_ENV)
    naples_obj = _DOCKER_CLIENT.containers.run(NAPLES_IMAGE,
                                               name=NAPLES_SIM_NAME,
                                               privileged=True,
                                               detach=True,
                                               auto_remove=True,
                                               environment=NAPLES_ENV,
                                               ports=NAPLES_PORT_MAPS,
                                               volumes=NAPLES_VOLUME_MOUNTS)
    print "Wating for naples sim to be up"
    time.sleep(5)
    __wait_for_naples_sim_to_be_up()
    print "Wating for agent server to be up"
    time.sleep(5)
    __wait_for_agent_to_be_up()
    print "Nic container bring up was successfull"
    nw = __setup_docker_network(args)
    print "Connecting %s to docker network : %s" % (NAPLES_IMAGE, args.nw_name)
    nw.connect(NAPLES_SIM_NAME)
    print "Connected %s to docker network : %s" % (NAPLES_IMAGE, args.nw_name)


def __run_bootstrap_naples(args):
    cp_cmd = ["cp", NAPLES_DATA_DIR + "/" + "bootstrap-naples.sh", "/usr/bin/bootstrap-naples.sh"]
    RunShellCmd(cp_cmd)
    chmod_cmd = ["chmod", "+x", "/usr/bin/bootstrap-naples.sh"]
    RunShellCmd(chmod_cmd)
    #bootstrap_cmd = ["/usr/bin/bootstrap-naples.sh", str(args.node_id)]
    bootstrap_cmd = ["/usr/bin/bootstrap-naples.sh"]
    if args.with_qemu:
        bootstrap_cmd.append("--qemu")
    RunShellCmd(bootstrap_cmd, background=True)


def __stop_bootstrap_naples():
    kill_cmd = ["pkill", "-9", "-f", "bootstrap-nap"]
    RunShellCmd(kill_cmd)


def __delete_ovs_bridge(args):
    print ("Deleting OVS bridge : ", args.ovs_br_name)
    cmd = ["ovs-vsctl", "del-br", args.ovs_br_name]
    RunShellCmd(cmd)


def __reset(args):
    __stop_bootstrap_naples()
    __delete_ovs_bridge(args)


def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--network-driver', dest='nw_driver', default="macvlan",
                        choices=["macvlan"], help='Network Driver for docker network')
    parser.add_argument('--network-name', dest='nw_name', default="control-net",
                        help='Network Driver for docker network')
    parser.add_argument('--network-subnet', dest='nw_subnet', default="11.1.1.0/24",
                        help='Network subnet for docker network')
    parser.add_argument('--network-ip-range', dest='nw_ip_range', default="11.1.1.32/27",
                        help='Network ip range for docker network')
    parser.add_argument('--network-gateway', dest='nw_gateway', default="11.1.1.254",
                        help='Network gateway for docker network')
    parser.add_argument('--network-parent', dest='nw_parent', default="eth2",
                        help='Parent network for docker network')
    parser.add_argument('--ovs-br-name', dest='ovs_br_name', default="data-net",
                        help='Ovs bridge name')
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
    args = parser.parse_args()

    __reset(args)
    __initial_setup()
    __pull_app_docker_images()
    __bringup_naples_container(args)
    __setup_data_network(args)
    __run_bootstrap_naples(args)


if __name__ == '__main__':
    main()
