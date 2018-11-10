#! /usr/bin/python3
import os
import socket
import pexpect
import sys
import iota.harness.infra.testbed as testbed
import iota.harness.api as api

HOST_NAPLES_DIR         = "/naples"
HOST_NAPLES_DRIVERS_DIR = "%s/drivers" % HOST_NAPLES_DIR
HOST_NAPLES_DRIVERS_LINUX_DIR = "%s/drivers-linux/" % HOST_NAPLES_DRIVERS_DIR

class HostManagement:
    def __init__(self, host_ip):
        self.host_username = 'root'
        self.host_password = 'docker'
        self.host_ip = host_ip
        self.drivers_ionic_pkg = api.GetTopDir() + '/platform/gen/drivers-linux.tar.xz'
        self.__ssh_host = "%s@%s" % (self.host_username, host_ip)
        self.__scp_pfx = "sshpass -p %s scp -o StrictHostKeyChecking=no " % self.host_password
        self.__ssh_pfx = "sshpass -p %s ssh -o StrictHostKeyChecking=no " % self.host_password
        self.__wait_for_ssh()
        self.__connect()
        return

    def __connect(self):
        self.hdl = pexpect.spawn("ssh -o StrictHostKeyChecking=no %s" % self.__ssh_host)
        self.hdl.timeout = 60
        self.hdl.logfile = sys.stdout.buffer
        self.hdl.expect_exact("Password:")
        self.hdl.sendline(self.host_password)
        self.hdl.expect_exact("#")
        return


    def __wait_for_ssh(self):
        print("Waiting for host to be up.")
        for retry in range(60):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            ret = sock.connect_ex(('%s' % self.host_ip, 22))
            sock.settimeout(1)
            if ret == 0:
                return
            else:
                time.sleep(5)

        print("Host not up. Ret:%d" % ret)
        sys.exit(1)
        return

    def run(self, command, background = False, ignore_result = False):
        if background:
            cmd = "%s -f %s \"%s\"" % (self.__ssh_pfx, self.__ssh_host, command)
        else:
            cmd = "%s %s \"%s\"" % (self.__ssh_pfx, self.__ssh_host, command)
        print(cmd)
        retcode = os.system(cmd)
        if not ignore_result:
            assert(retcode == 0)
        return retcode

    def copyin(self, src_filename, host_dir, naples_dir = None):
        dest_filename = host_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.__scp_pfx, src_filename,
                               self.__ssh_host, dest_filename)
        print(cmd)
        ret = os.system(cmd)
        assert(ret == 0)

        self.run("sync")
        ret = self.run("ls -l %s" % dest_filename)
        assert(ret == 0)

        if naples_dir:
            naples_dest_filename = naples_dir + "/" + os.path.basename(src_filename)
            ret = self.run("sshpass -p %s scp -o StrictHostKeyChecking=no %s root@1.0.0.2:%s" %\
                           (self.password, dest_filename, naples_dest_filename))
            assert(ret == 0)

        return 0

    def install_drivers(self):
        # Install IONIC driver package.
        self.copyin(self.drivers_ionic_pkg, HOST_NAPLES_DRIVERS_DIR)
        self.run("cd %s && tar xaf %s" %\
                 (HOST_NAPLES_DRIVERS_DIR, os.path.basename(self.drivers_ionic_pkg)))
        self.run("cd %s/drivers-linux/ && ./setup_apt.sh" % HOST_NAPLES_DRIVERS_DIR)
        self.run("cd %s/drivers-linux/ && ./build.sh" % HOST_NAPLES_DRIVERS_DIR)
        self.run("rmmod ionic_rdma", ignore_result = True)
        self.run("rmmod ionic", ignore_result = True)
        self.run("cd %s/drivers-linux/ && insmod drivers/eth/ionic/ionic.ko" % HOST_NAPLES_DRIVERS_DIR)
        self.run("modprobe ib_uverbs")
        self.run("cd %s/drivers-linux/ && insmod drivers/rdma/drv/ionic/ionic_rdma.ko xxx_haps=1" % HOST_NAPLES_DRIVERS_DIR)
        return

def Main(step):
    __testbed = testbed._Testbed()
    for instance in __testbed.tbspec.Instances:

        host = HostManagement(instance.NodeMgmtIP)
                          
        host.run("rm -rf %s" % HOST_NAPLES_DRIVERS_LINUX_DIR)
        host.run("mkdir -p %s" % HOST_NAPLES_DRIVERS_DIR)
        host.install_drivers()
        #set the path for testcases in this testsuite to use
        api.SetTestsuiteAttr("driver_path", HOST_NAPLES_DRIVERS_LINUX_DIR)

    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
