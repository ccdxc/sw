#!/usr/bin/python

import os
import sys
import argparse
import socket
import re
import time
import signal

from subprocess import Popen, PIPE, call

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
config_log = nic_dir + "/config.log"
storage_dol_log = nic_dir + "/storage_dol.log"
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

    #model_dir = nic_dir + "/model_sim/build"
    model_dir = nic_dir + "/../bazel-bin/nic/model_sim"
    os.chdir(model_dir)

    log = open(model_log, "w")
#    p = Popen(["sh", "run_model"], stdout=log, stderr=log)
    model_cmd = [ "./cap_model", "+PLOG_MAX_QUIT_COUNT=0" ]
    if args.modellogs:
        model_cmd.append("+plog=info")
        model_cmd.append("+model_debug=" + nic_dir + "/gen/iris/dbg_out/model_debug.json")
    if args.coveragerun or args.asmcov:
        dump_file= nic_dir + "/coverage/asm_cov.dump"
        model_cmd.append("+mpu_cov_dump_file=" + dump_file)
        dump_dir= nic_dir + "/coverage/asm_out_all"
        model_cmd.append("+mpu_pc_cov_dump_dir=" + dump_dir)

    p = Popen(model_cmd, stdout=log, stderr=log)
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

    #while loop == 1:
    #    for line in log2.readlines():
    #        if "initialized" in line:
    #            loop = 0
    log2.close()

#    log.close()

# HAL


def run_hal(args):
    os.environ["HAL_CONFIG_PATH"] = nic_dir + "/conf"
    #os.environ["LD_LIBRARY_PATH"] = "./obj:/usr/local/lib:/usr/local/lib64:asic/capri/model/capsim-gen/lib:third-party/lkl/export/bin"

    hal_dir = nic_dir + "/../bazel-bin/nic/hal"
    os.chdir(hal_dir)

    log = open(hal_log, "w")
    jsonfile = 'hal.json'
    os.system("cp " + nic_dir + "/conf/hal_default.ini " + nic_dir + "/conf/hal.ini")
    if args.hostpin:
        #jsonfile = 'hal_hostpin.json'
        os.system("cp " + nic_dir + "/conf/hal_hostpin.ini " + nic_dir + "/conf/hal.ini")
    if args.classic:
        #jsonfile = 'hal_classic.json'
        os.system("cp " + nic_dir + "/conf/hal_classic.ini " + nic_dir + "/conf/hal.ini")
    p = Popen(["./hal", "--config", jsonfile], stdout=log, stderr=log)
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
    #return

    # Wait until gRPC is listening on port

    while loop == 1:
        for line in log2.readlines():
            if "listening on" in line:
                loop = 0
    log2.close()
    os.system("rm " + nic_dir + "/conf/hal.ini")
    return


#    log.close()

def dump_coverage_data():
    model_process.send_signal(signal.SIGINT)
    hal_process.send_signal(signal.SIGUSR1)
    time.sleep(5)


# Run Storage DOL
def run_storage_dol(port):
    bin_dir = nic_dir + "/../bazel-bin/dol/test/storage"
    os.chdir(bin_dir)
    cmd = ['./storage_test', str(port)]
    p = Popen(cmd)
    p.communicate()
    return p.returncode


# DOL


def run_dol(args):
    dol_dir = nic_dir + "/../dol"
    os.chdir(dol_dir)

    log = open(dol_log, "w")
    cmd = ['./main.py', '--topo', args.topology ]
    if args.modlist is not None:
        cmd.append('--modlist')
        cmd.append(args.modlist)
    if args.feature is not None:
        cmd.append('--feature')
        cmd.append(args.feature)
    if args.sub is not None:
        cmd.append('--sub')
        cmd.append(args.sub)
    if args.test is not None:
        cmd.append('--test')
        cmd.append(args.test)
    if args.testcase is not None:
        cmd.append('--testcase')
        cmd.append(args.testcase)
    if args.hostpin is True:
        cmd.append('--hostpin')
    if args.classic is True:
        cmd.append('--classic')
    if args.coveragerun is True:
        cmd.append('--coveragerun')
    if args.regression is True:
        cmd.append('--regression')
    if args.skipverify is True:
        cmd.append('--skipverify')
    if args.tcscale is not None:
        cmd.append('--tcscale')
        cmd.append(args.tcscale)
    if args.dryrun is True:
        cmd.append('--dryrun')
    if args.configonly is True:
        cmd.append('--config-only')

    p = Popen(cmd)
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

def run_config_validation(args):
    dol_dir = nic_dir + "/../dol"
    os.chdir(dol_dir)

    log = open(config_log, "w")
    cmd = ['./config_validator/main.py']
    p = Popen(cmd)
    print "* Starting Config validator with  pid (" + str(p.pid) + ")"
    print "- Log file: " + config_log + "\n"

    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()
    p.communicate()
    log.close()

    print "* Validator exit code " + str(p.returncode)
    return p.returncode    

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

def run_e2e_tlsproxy_dol():
    os.chdir(nic_dir)
    cmd = ['./tools/run_e2e_tlsproxy_test.py']
    p = Popen(cmd)
    print "* Starting E2E TLS Proxy DOL, pid (" + str(p.pid) + ")"
    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()
    p.communicate()
    if p.returncode != 0:
        print "* FAIL: E2E TLS Proxy DOL, exit code ", p.returncode
    return p.returncode

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
                        default=None, help='Module List File')
    parser.add_argument('--feature', dest='feature',
                        default=None, help='Run all tests of a feature.')
    parser.add_argument('--sub', dest='sub',
                        default=None, help='Run all tests of a sub-feature.')
    parser.add_argument('--test', dest='test',
                        default=None, help='Module List File')
    parser.add_argument('--testcase', dest='testcase',
                        default=None, help='Module List File')
    parser.add_argument('--hostpin', dest='hostpin', action="store_true",
                        help='Run tests in Hostpinned mode.')
    parser.add_argument('--classic', dest='classic', action="store_true",
                        help='Run tests in Classic NIC mode.')
    parser.add_argument('--storage', dest='storage', action="store_true",
                        help='Run storage dol as well.')
    parser.add_argument("--asmcov", action="store_true",
                        help="Generate ASM coverage for this run")
    parser.add_argument('--regression', dest='regression',
                        action='store_true', help='Run tests in regression mode.')
    parser.add_argument('--skipverify', dest='skipverify',
                        action='store_true', help='Skip Verification all tests.')
    parser.add_argument('--tcscale', dest='tcscale', default=None,
                        help='Testcase Scale Factor.')
#    parser.add_argument('--cfgscale', dest='cfgscale', default=None,
#                        help='Configuration Scale Factor.')
    parser.add_argument('--dryrun', dest='dryrun', action='store_true',
                        help='Dry-Run mode. (No communication with HAL & Model)')
    parser.add_argument("--configtest", action="store_true",
                        help="Run Config tests.")    
    parser.add_argument("--config-only", dest='configonly', action="store_true",
                        help="Generate ASM coverage for this run")
    parser.add_argument("--e2e-tls-dol", dest='e2etls', action="store_true",
                        default=None, help="Run E2E TLS DOL")
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

        if args.dryrun is False:
            run_model(args)
            run_hal(args)

        if (args.storage):
          status = run_storage_dol(port)
          if status != 0:
            print "- Storage dol failed, status=", status
        elif args.configtest:
            status = run_config_validation(args)
            if status != 0:
                print "- Config validaton failed, status=", status
        else:
          status = run_dol(args)
          if status == 0:
             if (args.e2etls):
                status = run_e2e_tlsproxy_dol()
        if args.coveragerun:
            dump_coverage_data()
        cleanup(keep_logs=True)
        os.chdir(nic_dir)
        if args.asmcov:
            call(["tools/run-coverage -k -c coverage_asm.json"], shell=True)
        sys.exit(status)
    else:
        print "- build failed!"
        sys.exit(1)


if __name__ == "__main__":
    main()
