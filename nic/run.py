#!/usr/bin/python

import os
import sys
import argparse
import socket
import re
import time
import signal

from subprocess import Popen, PIPE

# git clone https://github.com/pensando/sw
# cd sw && git submodule update --init

# Globals

#nic_dir = os.environ.get('WS_TOP')
nic_dir = os.path.dirname(sys.argv[0])
nic_dir = os.path.abspath(nic_dir)
print "WS_TOP = ", nic_dir
if nic_dir is None:
    print "WS_TOP is not set!"
    sys.exit(1)

pid = os.getpid()
hal_process = None
model_process = None

build_log = nic_dir + "/build.log"
model_log = nic_dir + "/model.log"
hal_log = nic_dir + "/hal.log"
dol_log = nic_dir + "/dol.log"
sample_client_log = nic_dir + "/sample_client.log"

lock_file = nic_dir + "/.run.pid"

# Environment

os.environ["LD_LIBRARY_PATH"] = "/usr/local/lib:/usr/local/lib64:asic/capri/model/capsim-gen/lib:third-party/lkl/export/bin"
os.environ["PKG_CONFIG_PATH"] = "/usr/local/lib/pkgconfig"

# build


def build():
    print "* Starting build"
    print "- Log file: " + build_log
    start_time = time.time()

    nic_dir = nic_dir

    os.environ["LD_LIBRARY_PATH"] = "/usr/local/lib:/usr/local/lib64"
    os.environ["PKG_CONFIG_PATH"] = "/usr/local/lib/pkgconfig"
    os.chdir(nic_dir)

    log = open(build_log, "w")
    p = Popen(["make", "-j", "8"], stdout=log, stderr=log)
    p.communicate()
    log.close()

    print("- Build time: %s seconds\n" % round(time.time() - start_time, 2))

    return p.returncode

# ASIC model


def run_model(args):
    os.environ["LD_LIBRARY_PATH"] = ".:../libs:/home/asic/tools/src/0.25/x86_64/lib64:/usr/local/lib:/usr/local/lib64"

    model_dir = nic_dir + "/model_sim/build"
    os.chdir(model_dir)

    log = open(model_log, "w")
#    p = Popen(["sh", "run_model"], stdout=log, stderr=log)
    if args.modellogs:
        p = Popen(["./cap_model", "+plog=info",
                   "+model_debug=../../gen/iris/dbg_out/model_debug.json"], stdout=log, stderr=log)
    else:
        p = Popen(["./cap_model"], stdout=log, stderr=log)
    print "* Starting ASIC model pid (" + str(p.pid) + ")"
    print "- Log file: " + model_log + "\n"

    global model_process
    model_process = p
    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()

    log2 = open(model_log, "r")
    loop = 1

    # Wait until model is up and running

    while loop == 1:
        for line in log2.readlines():
            if "initialized" in line:
                loop = 0
    log2.close()

#    log.close()

# HAL


def run_hal():
    os.environ["HAL_CONFIG_PATH"] = nic_dir + "/conf"
    os.environ["LD_LIBRARY_PATH"] = "./obj:/usr/local/lib:/usr/local/lib64:asic/capri/model/capsim-gen/lib:third-party/lkl/export/bin"

    hal_dir = nic_dir
    os.chdir(hal_dir)

    log = open(hal_log, "w")
    p = Popen(["./obj/hal", "--config", "hal.json"], stdout=log, stderr=log)
    global hal_process
    hal_process = p
    print "* Starting HAL pid (" + str(p.pid) + ")"
    print "- Log file: " + hal_log + "\n"

    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()

    log2 = open(hal_log, "r")
    loop = 1
    time.sleep(10)
    return

    # Wait until gRPC is listening on port

    while loop == 1:
        for line in log2.readlines():
            if "listening on" in line:
                loop = 0
    log2.close()


#    log.close()

def dump_coverage_data():
    model_process.send_signal(signal.SIGINT)
    hal_process.send_signal(signal.SIGUSR1)
    time.sleep(5)

# DOL


def run_dol(args):
    dol_dir = nic_dir + "/../dol"
    os.chdir(dol_dir)

    log = open(dol_log, "w")
    p = Popen(["./main.py", "--topo", args.topology, "--modlist", args.modlist])
    print "* Starting DOL pid (" + str(p.pid) + ")"
    print "- Log file: " + dol_log + "\n"

    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()
    p.communicate()
    log.close()

    print "* DOL exit code " + str(p.returncode)
    return p.returncode

#    log.close()


# Sample Client
'''
def run_sample_client():
    sample_client_dir = nic_dir + "/model_sim/build"
    os.chdir(sample_client_dir)

    log = open(sample_client_log, "w")
    p = Popen(["./sample_client"], stdout=log, stderr=log)
    print "* Starting Sample Client pid (" + str(p.pid) + ")"
    print "- Log file: " + sample_client_log + "\n"

    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()
    p.communicate()
    log.close()

    print "* Sample Client exit code " + str(p.returncode)
    return p.returncode 

#    log.close()
'''

# find_port()


def find_port():
    s = socket.socket()
    s.bind(('', 0))
    port = s.getsockname()[1]
    s.close()

    return port

# is_running()


def is_running(pid):
    try:
        os.kill(pid, 0)
        return True
    except OSError:
        return False

# cleanup()


def cleanup(keep_logs=True):
    # print "* Killing running processes:"

    lock = open(lock_file, "r")
    for pid in lock:
        if is_running(int(pid)):
            # print "- pid (" + pid.rstrip() + ")"
            os.kill(int(pid), 9)
        # else:
            # print "- pid (" + pid.rstrip() + ") - process isn't running"
    lock.close()
    os.remove(lock_file)

    # print "\n* Removing log files:"

    if keep_logs:
        return

    for log in build_log, model_log, hal_log, dol_log:
        if os.path.isfile(log):
            print "- " + log
            os.remove(log)
        else:
            print "- " + log + " - not found"

# main()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", "--build", action="store_true", help="run build")
    parser.add_argument("-c", "--cleanup", action="store_true",
                        help="cleanup running process")
    parser.add_argument("--modellogs", action="store_true",
                        help="run with model logs enabled")
    parser.add_argument("--coveragerun", action="store_true",
                        help="run with model logs enabled")
    parser.add_argument('--topo', dest='topology', default='base',
                        help='Run for a specific topology')
    parser.add_argument('--modlist', dest='modlist',
                        default='modules.list', help='Module List File')
    args = parser.parse_args()

    if args.cleanup:
        if os.path.isfile(lock_file):
            cleanup(keep_logs=False)
        else:
            print "Nothing to cleanup"

        sys.exit(0)

    if args.build:
        r = build()
    else:
        r = 0

    if r == 0:
        port = find_port()
        print "* Using port (" + str(port) + ") for HAL\n"
        os.environ["HAL_GRPC_PORT"] = str(port)

        run_model(args)
        run_hal()
        status = run_dol(args)
        if args.coveragerun:
            dump_coverage_data()
        cleanup(keep_logs=True)
        sys.exit(status)
    else:
        print "- build failed!"
        sys.exit(1)


if __name__ == "__main__":
    main()
