import argparse
import os
import time
import sys

parser =  argparse.ArgumentParser(description="NVMeoF Target Script")
parser.add_argument('--intf', dest='intf', required = True, help='ethernet interface')
parser.add_argument('--ip', dest='ip', required = True, help='ip address')
parser.add_argument('--prefixlen', dest='prefixlen', required = True, help='ip address/<prefixlen>')
parser.add_argument('--mtu', dest='mtu', required = True, help='packet mtu')
parser.add_argument('--hmem', dest='hmem', required = True, help='HUGEMEM')
parser.add_argument('--spdk_dir', dest='spdk_dir', required = True, help='path of spdk dir')
parser.add_argument('--cpu_mask', dest='cpu_mask', required = True, help='cpu mask')
parser.add_argument('--mdev_name', dest='mdev_name', required = True, help='mdev_name')
parser.add_argument('--block_size', dest='block_size', required = True, help='block size')
parser.add_argument('--num_block', dest='num_block', required = True, help='number of blocks')
parser.add_argument('--ctrl_name', dest='ctrl_name', required = True, help='Nmve Controller Name')
parser.add_argument('--nqn', dest='nqn', required = True, help='nqn')
args = parser.parse_args()

ret = os.system("ifconfig {} {}/{} up".format(args.intf, args.ip, args.prefixlen))
if ret:
    print("Error: Failed to configure ethernet interface for ip")
    sys.exit(ret)

ret = os.system("ifconfig {} mtu {}".format(args.intf, args.mtu))
if ret:
    print("Error: Failed to configure ethernet interface for mtu")
    sys.exit(ret)
print("Configured ethernet interface")

ret = os.system("HUGEMEM={} {}/scripts/setup.sh".format(args.hmem, args.spdk_dir))
if ret:
    print("Error: Failed hugemem setup")
    sys.exit(ret)
print("hugemem setup success")

ret = os.system("{}/app/nvmf_tgt/nvmf_tgt -m {} &".format(args.spdk_dir, args.cpu_mask))
if ret:
    print("Error: Failed to configure cpu_mask nvmf_tgt")
    sys.exit(ret)
print("Configured cpu_mask on nvme target")
time.sleep(2)

ret = os.system("{}/scripts/rpc.py nvmf_create_transport -t TCP -u 16384 -p 128 -c 8192 -n 2048".format(args.spdk_dir))
if ret:
    print("Error: Failed to create tcp transport")
    sys.exit(ret)
print("Created TCP transport")

ret = os.system("{}/scripts/rpc.py bdev_malloc_create -b {} {} {}".format(args.spdk_dir, args.mdev_name, args.block_size, args.num_block))
if ret:
    print("Error: Failed bdev_malloc")
    sys.exit(ret)
print("bdev_malloc success")

ret = os.system("{}/scripts/rpc.py nvmf_create_subsystem {} -a -s SPDK00000000000001 -d {}".format(args.spdk_dir, args.nqn, args.ctrl_name))
if ret:
    print("Error: Failed to create nvmf_subsystem")
    sys.exit(ret)
print("nvmf_subsystem created")

ret = os.system("{}/scripts/rpc.py nvmf_subsystem_add_ns {} {}".format(args.spdk_dir, args.nqn, args.mdev_name))
if ret:
    print("Error: add_ns failed on nvmf_subsystem")
    sys.exit(ret)
print("nvmf_subsystem add_ns success")

ret = os.system("{}/scripts/rpc.py nvmf_subsystem_add_listener -t tcp -a {} -s 4420 -f ipv4 {}".format(args.spdk_dir, args.ip, args.nqn))
if ret:
    print("Error: Failed to added listerner to nvmf_subsys")
    sys.exit(ret)
print("nvmf_subsystem listening")
