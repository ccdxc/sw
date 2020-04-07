import argparse
import os
import sys
import time

parser =  argparse.ArgumentParser(description="NVMeoF Initiator Script")
parser.add_argument('--intf', dest='intf', required = True, help='ethernet interface')
parser.add_argument('--ip', dest='ip', required = True, help='ip address')
parser.add_argument('--r_ip', dest='r_ip', required = True, help='remote ip address')
parser.add_argument('--prefixlen', dest='prefixlen', required = True, help='ip address/<prefixlen>')
parser.add_argument('--mtu', dest='mtu', required = True, help='packet mtu')
parser.add_argument('--nqn', dest='nqn', required = True, help='nqn')
args = parser.parse_args()

ret = os.system("ifconfig {} {}/{} up".format(args.intf, args.ip, args.prefixlen))
if ret:
    print("Error: Failed to configure IP for ethernet interface")
    sys.exit(ret)

ret = os.system("ifconfig {} mtu {}".format(args.intf, args.mtu))
if ret:
    print("Error: Failed to configure mtu for ethernet interface")
    sys.exit(ret)
print("Configured ethernet interface")

ret = os.system("modprobe nvme_tcp")
if ret:
    print("Error: Failed to insert nvme_tcp module")
    sys.exit(ret)
print("Inserted nvme_tcp module")

ret = os.system("nvme discover -t tcp -a {} -s 4420".format(args.r_ip))
if ret:
    print("Error: Failed to discover nvmf_tgt")
    sys.exit(ret)
print("Discovered nvmf_tgt")

ret = os.system("nvme connect -t tcp -n {} -a {} -s 4420".format(args.nqn, args.r_ip))
if ret:
    print("Error: Failed to connected to nvmf_tgt")
    sys.exit(ret)
print("Connect to nvmf_tgt is success")
time.sleep(2)

os.system("nvme list")
