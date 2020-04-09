#!/usr/bin/python3

import os
import sys
import argparse
import re
import time
import subprocess
import threading
import signal

asic = os.environ.get('ASIC', 'capri')
paths = [
    '/nic/build/x86_64/iris/' + asic + '/gen/proto/',
    '/nic'
]
ws_top = os.path.dirname(sys.argv[0]) + '/../../'
ws_top = os.path.abspath(ws_top)
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

import grpc
import types_pb2
import proxy_pb2

from threading import Thread

from subprocess import Popen, PIPE, call

nic_tools_dir = os.path.dirname(sys.argv[0])
nic_dir = nic_tools_dir + "/.."
nic_dir = os.path.abspath(nic_dir)

print("NIC_DIR = ", nic_dir)
if nic_dir is None:
    print("NIC dir is not set!")
    sys.exit(1)

pid = os.getpid()
hntap_process = None
http_svr_process = None
sock_stats = []

hntap_log = nic_dir + "/hntap.log"
http_svr_log = nic_dir + "/http_server.log"
http_clt_log = nic_dir + "/http_client.log"
HalChannel = None

def hal_init():
    global HalChannel

    if 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    print("Creating GRPC channel to HAL")
    server = 'localhost:' + port
    HalChannel = grpc.insecure_channel(server)
    print("Waiting for HAL to be ready ...")
    grpc.channel_ready_future(HalChannel).result()
    print("Connected to HAL!")
    return

def run_hntap():
    log = open(hntap_log, "w")
    cmd = ['../nic/build/x86_64/iris/' + asic + '/bin/nic_proxy_e2etest_hntap', '-n', '5']
    p = Popen(cmd, stdout=log, stderr=log)
    global hntap_process
    hntap_process = p
    print("* Starting Host-Network tapper, pid (%s)" % str(p.pid))
    print("    - Log file: " + hntap_log + "\n")

    log2 = open(hntap_log, "r")
    loop = 1
    time.sleep(2)

    # Wait until tap devices setup is complete

    while loop == 1:
        for line in log2.readlines():
            if "listening on" in line:
                loop = 0
    log2.close()
    return

def run_http_server():
    log = open(http_svr_log, "w")
    cmd = ['go', 'run', nic_tools_dir + '/server/simple_http_server.go' ]
    p = Popen(cmd, stdout=log, stderr=log, preexec_fn=os.setsid)
    global http_svr_process
    http_svr_process = p
    print("* Starting HTTP Server on pid (%s)" % (str(p.pid)))
    print("    - Log file: " + http_svr_log + "\n")
    time.sleep(2)
    return


def run_http_client():
    log = open(http_clt_log, "a")
    cmd = ['go', 'run', nic_tools_dir + '/client/simple_http_client.go' ]
    p = Popen(cmd, stdout=log, stderr=log)
    print("* Starting HTTP Client on pid (%s)" % (str(p.pid)))
    print("    - Log file: " + http_clt_log + "\n")
    p.communicate()
    p.wait()
    return p.returncode

def cleanup(keep_logs=True):
    os.kill(int(hntap_process.pid), 9)
    if http_svr_process:
        #os.kill(int(http_svr_process.pid), 9)
        os.killpg(os.getpgid(http_svr_process.pid), signal.SIGTERM)

    if keep_logs:
        return

    for log in hntap_log, http_svr_log, http_clt_log:
        if os.path.isfile(log):
            print("- " + log)
            os.remove(log)
        else:
            print("- " + log + " - not found")
            print("not found")


def run_test(testnum, testname):
    print("Test %d: Running E2E %s L7 test\n" % (testnum, testname))
    start_time = time.time()

    run_http_server()
    status = run_http_client()
    if status == 0:
        FNULL = open(os.devnull, 'w')
        cmd = ['grep', '{state=\"APPID_STATE_IN_PROGRESS\",id=3}', nic_dir + '/hal.log']
        p = Popen(cmd, stdout=FNULL, stderr=FNULL)
        p.communicate()
        p.wait()
        status = p.returncode

    print("\n- Test %d: run time: %s seconds\n" % (testnum, round(time.time() - start_time, 1)))
    if status != 0:
        print("Test %d: E2E %s Http Status = FAIL\n" % (testnum, testname))
    else:
        print("Test %d: E2E %s Http Status = PASS\n" % (testnum, testname))
    return status

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config-only", dest='configonly', action="store_true",
                        help="Configuration only, no tests run")
    args = parser.parse_args()

    if args.configonly:
        run_hntap()
        print("E2E L7 DOL: config-only, stopping..\n")
        sys.exit(0)

    start_time = time.time()
    log = open(http_clt_log, "w")
    log.close()

    run_hntap()

    status = run_test(1, "HTTP")

    cleanup(keep_logs=True)

    print("\n- Total run time: %s seconds\n" % round(time.time() - start_time, 1))
    print("Final Status = %s\n" % ("PASS" if (status == 0) else "FAIL"))

    sys.exit(status)

if __name__ == "__main__":
    main()
