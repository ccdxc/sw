#!/usr/bin/python

import os
import sys
import argparse
import re
import time
import subprocess
import threading

from threading import Thread

from subprocess import Popen, PIPE, call

nic_tools_dir = os.path.dirname(sys.argv[0])
nic_dir = nic_tools_dir + "/.."
nic_dir = os.path.abspath(nic_dir)

print "NIC_DIR = ", nic_dir
if nic_dir is None:
    print "NIC dir is not set!"
    sys.exit(1)

pid = os.getpid()
hntap_process = None
tls_svr_process = None
sock_stats = []

hntap_log = nic_dir + "/hntap.log"
tls_svr_log = nic_dir + "/tls_server.log"
tcp_clt_log = nic_dir + "/tcp_client.log"

def run_hntap():
    log = open(hntap_log, "w")
    cmd = ['../bazel-bin/nic/proxy-e2etest/nic_proxy-e2etest_hntap']
    p = Popen(cmd, stdout=log, stderr=log)
    global hntap_process
    hntap_process = p
    print "* Starting Host-Network tapper, pid (" + str(p.pid) + ")"
    print "    - Log file: " + hntap_log + "\n"


def run_tls_server():
    log = open(tls_svr_log, "w")
    cmd = ['../bazel-bin/nic/proxy-e2etest/nic_proxy-e2etest_tls-server', str(80)]
    p = Popen(cmd, stdout=log, stderr=log)
    global tls_svr_process
    tls_svr_process = p
    print "* Starting TLS Server on port 80, pid (" + str(p.pid) + ")"
    print "    - Log file: " + tls_svr_log + "\n"


def run_tcp_client():
    log = open(tcp_clt_log, "w")
    cmd = ['../bazel-bin/nic/proxy-e2etest/nic_proxy-e2etest_tcp-client', '-p', str(80), '-d', nic_dir + "/proxy-e2etest/hello-world", '-m', 'from-host']
    p = Popen(cmd, stdout=log, stderr=log)
    print "* Starting TCP Client on port 80, pid (" + str(p.pid) + ")"
    print "    - Log file: " + tcp_clt_log + "\n"
    p.communicate()
    p.wait()
    return p.returncode

def cleanup(keep_logs=True):
    os.kill(int(hntap_process.pid), 9)
    os.kill(int(tls_svr_process.pid), 9)

    if keep_logs:
        return

    for log in hntap_log, tls_svr_log, tcp_clt_log:
        if os.path.isfile(log):
            print "- " + log
            os.remove(log)
        else:
            print "- " + log + " - not found"

def check_sockets(id, stop):
   global sock_stats
   while True:
     cmd = "netstat -t | grep http"
     p = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
     num_lines = 0
     est_socks = []
     for line in iter(p.stdout.readline, ''):
         if "ESTABLISHED" in line:
             est_socks.append(line)
             num_lines += 1
     if (num_lines == 2):
         sock_stats = est_socks
         break
     if stop():
        break
     p.stdout.close()
     sys.stdout.write("\rRunning..")
     sys.stdout.flush()
     time.sleep(.300)
     sys.stdout.write("\rRunning....")
     sys.stdout.flush()
     #print('\r', end=" ")
   p.stdout.close()
   sys.stdout.write("\n")

def print_logs():
    log = open(tcp_clt_log, "r")
    print "TCP Client log:"
    for line in log.readlines():
       if "Client:" in line:
           print "    " + line
    log.close()

    print "TLS Server log:"
    log = open(tls_svr_log, "r")
    for line in log.readlines():
       if "Server:" in line:
           print "    " + line
    log.close()
    print "Socket states:"
    for line in sock_stats:
       print "    " + line


def main():
    run_hntap()
    run_tls_server()

    stop_thread = False
    t = Thread(target = check_sockets, args=(id, lambda: stop_thread))
    t.start()

    status = run_tcp_client()
    if status != 0:
        cleanup(keep_logs=True)
        stop_thread = True
        t.join()
        print_logs()
        print "FAIL - E2E TLS proxy test, status = ", status
        sys.exit(status)
    
    cleanup(keep_logs=True)

    print_logs()
    print "SUCCESS - E2E TLS proxy test, status = ", status

# cleanup(keep_logs=False)
    stop_thread = True
    t.join()
    sys.exit(0)

if __name__ == "__main__":
    main()
