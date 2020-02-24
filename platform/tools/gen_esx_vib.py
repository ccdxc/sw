#!/usr/bin/python3

import argparse
import paramiko
import subprocess
import os
import sys
import getpass
import socket

parser = argparse.ArgumentParser(description='Naples Boot Script')
parser.add_argument('--esx-bld-vm-username', dest='esx_bld_vm_username',
                    default="root", help='esx build vm username')
parser.add_argument('--esx-bld-vm-password', dest='esx_bld_vm_password',
                    default="vmware", help='esx build vm password')
parser.add_argument('--esx-bld-vm', dest='esx_bld_vm',
                    default="esx-6.7-vib.pensando.io", help='esx build vm')

parser.add_argument('--drivers-pkg', dest='drivers_pkg',
                    default=None, help='Esx Source Driver Package.')

parser.add_argument('--vib-version', dest='vib_version',
                    default=None, help='Which version of ESXi vib to be generated')

GlobalOptions = parser.parse_args()

if GlobalOptions.vib_version == '67':
    # Will be changed to -67 once we have IOTA change for ESXi version
    vib_ver_str = ""
    GlobalOptions.esx_bld_vm = "esx-6.7-vib.pensando.io"
elif GlobalOptions.vib_version == '65':
    GlobalOptions.esx_bld_vm = "esx-6.5-vib.pensando.io"
    vib_ver_str = "-65"
else:
    print ("Unknown version of vib that requested to be generated %s" % GlobalOptions.vib_version)
    sys.exit(1)

ssh=paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect(GlobalOptions.esx_bld_vm,  '22', GlobalOptions.esx_bld_vm_username,GlobalOptions.esx_bld_vm_password)
__bld_vm_ssh_handle = ssh
__bld_vm_ssh_host = "%s@%s" % (GlobalOptions.esx_bld_vm_username, GlobalOptions.esx_bld_vm)
__bld_vm_scp_pfx = "sshpass -p %s scp -o StrictHostKeyChecking=no " % GlobalOptions.esx_bld_vm_password
__bld_vm_ssh_pfx = "sshpass -p %s ssh -o StrictHostKeyChecking=no " % GlobalOptions.esx_bld_vm_password

def bld_vm_run(command):
    _, out, err  = __bld_vm_ssh_handle.exec_command(command)
    return out.readlines(), err.readlines(), out.channel.recv_exit_status()

def bld_vm_copyin(src_filename, dest_dir):
    dest_filename = dest_dir + "/" + os.path.basename(src_filename)
    cmd = "%s %s %s:%s" % (__bld_vm_scp_pfx, src_filename,
                        __bld_vm_ssh_host, dest_filename)
    print(cmd)
    ret = os.system(cmd)
    assert(ret == 0)

    bld_vm_run("sync")
    _, _, ret = bld_vm_run("ls -l %s" % dest_filename)
    assert(ret == 0)
    return 0

def bld_vm_copyout(src_filename, dest_dir):
    cmd = "%s %s:%s %s" % (__bld_vm_scp_pfx, __bld_vm_ssh_host, src_filename,
                        dest_dir)
    print(cmd)
    ret = os.system(cmd)
    assert(ret == 0)
    return 0

pkg_name = os.path.basename(GlobalOptions.drivers_pkg)
dst_pkg_base_name = "_".join([pkg_name, socket.gethostname(), getpass.getuser()])
stdout, _, exit_status  = bld_vm_run("find  /opt/vmware/  -type d  -name   nativeddk-6.*")
print ("Native DDK dir out ", stdout)
if len(stdout) == 0:
    print ("Invalid output when discovering native ddk", stdout)
    sys.exit(1)
#Pick the first one.
dst_dir = stdout[0].strip("\n") + "/src/" + dst_pkg_base_name + "_dir"

bld_vm_run("rm -rf " + dst_dir + " && mkdir -p " + dst_dir + " && sync ")
# Copy the driver package
bld_vm_copyin(GlobalOptions.drivers_pkg, dest_dir = dst_dir)
_, stderr, exit_status  = bld_vm_run("cd " + dst_dir + " && tar -xvf " + pkg_name)
if exit_status != 0:
    print ("Failed to extract drivers", stderr)
    bld_vm_run("rm -rf " + dst_dir)
    sys.exit(1)

_, stderr, exit_status  = bld_vm_run("cd " + dst_dir + "/drivers-esx-eth" + vib_ver_str + " && ./build.sh " + GlobalOptions.vib_version)
if exit_status != 0:
    print ("Driver build failed ", ''.join(stderr))
    bld_vm_run("rm -rf " + dst_dir)
    sys.exit(1)

_, stderr, exit_status  = bld_vm_run("cd " + dst_dir + " && tar -cJf " + pkg_name + " drivers-esx-eth" + vib_ver_str)
if exit_status != 0:
    print ("Failed to create tar file ", stderr)
    bld_vm_run("rm -rf " + dst_dir)
    sys.exit(1)

dst_file = dst_dir + "/" + pkg_name
bld_vm_copyout(dst_file, dest_dir = os.path.dirname(GlobalOptions.drivers_pkg))
bld_vm_run("rm -rf " + dst_dir)
