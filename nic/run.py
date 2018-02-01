#!/usr/bin/python

import os
import sys
import argparse
import socket
import re
import time
import signal
import atexit
import pdb
import glob

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
mbt_log = nic_dir + "/mbt.log"
storage_dol_log = nic_dir + "/storage_dol.log"
sample_client_log = nic_dir + "/sample_client.log"
bullseye_model_cov_file = nic_dir + "/coverage/bullseye_model.cov"
bullseye_hal_cov_file = nic_dir + "/../bazel-out/../../bullseye_hal.cov"

lock_file = nic_dir + "/.run.pid"

# Environment

os.environ["LD_LIBRARY_PATH"] = "/usr/local/lib:/usr/local/lib64:asic/capri/model/capsim-gen/lib:third-party/lkl/export/bin"
os.environ["PKG_CONFIG_PATH"] = "/usr/local/lib/pkgconfig"

#Path and executables
model_executable = nic_dir + "/../bazel-bin/nic/model_sim/cap_model"
model_core_path  = nic_dir + "/../bazel-out/local-fastbuild/bin/nic/model_sim"

hal_executable = nic_dir + "/../bazel-bin/nic/hal/hal"
hal_core_path = nic_dir + "/../bazel-out/local-fastbuild/bin/nic/hal"

def print_core(executable, core):
    cmd = ['gdb -batch -ex  "bt"']
    cmd.extend([executable, core])
    call(" ".join(cmd), universal_newlines=True, shell=True)

def get_latest_core_file(path):                                          
    list_of_files = glob.glob(path + "/core.*")                          
    if list_of_files:
        latest_file = max(list_of_files, key=os.path.getctime)           
        return latest_file

def remove_core_files(path):
    for filename in glob.glob(path + "/core.*"):
        os.remove(filename)

def remove_all_core_files():
    remove_core_files(hal_core_path) 
    remove_core_files(model_core_path) 

def process_core(executable, core_path):   
    core_file = get_latest_core_file(core_path)
    if core_file:
        print_core(executable, core_file)
        os.system("mv " + core_file + "  " + nic_dir + "/core." +  os.path.basename(executable))
   

def process_hal_core():
    process_core(hal_executable, hal_core_path)

def process_model_core():
    process_core(model_executable, model_core_path)

def process_cores():
    process_hal_core()
    process_model_core()
          
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

def run_rtl(args):
    global model_log
    if args.test_suf:
        model_log = nic_dir + "/logs_%s/model.log" % args.test_suf
    log = open(model_log, "w")
    os.environ["ASIC_SRC"] = os.getcwd() + "/asic"
    os.environ["LD_LIBRARY_PATH"] = ".:../libs:/home/asic/tools/src/0.25/x86_64/lib64:/usr/local/lib:/usr/local/lib64:" + os.getcwd() + "/asic/capri/model/capsim-gen/lib:/home/asic/bin/tools/lib64"
    os.environ["PATH"] = os.getcwd() + "/asic/common/tools/bin" + ":" + os.environ["PATH"]
    model_test = "core_basic_dol"
    one_pkt_mode = ""
    if args.model_test:
        model_test = args.model_test
    if not args.skipverify:
        one_pkt_mode = "+dol_one_pkt_mode=1"
    model_cmd = [ 'runtest', '-ngrid', '-test', model_test, '-run_args', ' %s +core.axi_master0.max_write_latency=1500 +core.axi_master0.avg_max_write_latency=1500 +dol_poll_time=5 +dump_axi +pcie_all_lif_valid=1 +UVM_VERBOSITY=UVM_HIGH +fill_pattern=0 +te_dbg +plog=info +mem_verbose +verbose +PLOG_MAX_QUIT_COUNT=100 +top_sb/initial_timeout_ns=60000 %s ' % (one_pkt_mode, args.runtest_runargs) ]
    if args.noverilog:
        model_cmd = model_cmd + ['-ro']
    if not args.no_asic_dump:
        model_cmd = model_cmd + [ '-ucli', 'ucli_core' ]
    if args.test_suf:
        model_cmd = model_cmd + ['-test_suf', args.test_suf ]
    print os.getcwd() + "/asic/capri/verif/top/env"
    p = Popen(model_cmd, stdout=log, stderr=log, cwd = os.getcwd() + "/asic/capri/verif/top/env")
    print "* Starting ASIC rtl: pid (" + str(p.pid) + ")"
    print "- Log file: " + model_log + "\n"

    global model_process
    model_process = p
    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()

def run_model(args):
    global model_log
    if args.test_suf:
        model_log = nic_dir + "/logs_%s/model.log" % args.test_suf
    os.environ["LD_LIBRARY_PATH"] = ".:../libs:/home/asic/tools/src/0.25/x86_64/lib64:/usr/local/lib:/usr/local/lib64:/home/asic/bin/tools/lib64"

    #model_dir = nic_dir + "/model_sim/build"
    model_dir = nic_dir + "/../bazel-bin/nic/model_sim"
    os.chdir(model_dir)

    log = open(model_log, "w")
#    p = Popen(["sh", "run_model"], stdout=log, stderr=log)
    model_cmd = [ "./cap_model", "+PLOG_MAX_QUIT_COUNT=0", "+plog_add_scope=axi_trace" ]
    if args.modellogs:
        model_cmd.append("+plog=info")
        if args.gft:
            model_cmd.append("+model_debug=" + nic_dir + "/gen/gft/dbg_out/model_debug.json")
        else:
            model_cmd.append("+model_debug=" + nic_dir + "/gen/iris/dbg_out/model_debug.json")
    if args.coveragerun or args.asmcov:
        dump_file= nic_dir + "/coverage/asm_cov.dump"
        model_cmd.append("+mpu_cov_dump_file=" + dump_file)
        dump_dir= nic_dir + "/coverage/asm_out_all"
        model_cmd.append("+mpu_pc_cov_dump_dir=" + dump_dir)

    model_env = os.environ.copy()
    model_env["COVFILE"] = bullseye_model_cov_file
    p = Popen(model_cmd, stdout=log, stderr=log, env=model_env)
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

def wait_for_hal():
    # Wait until gRPC is listening on port
    log2 = open(hal_log, "r")
    loop = 1
    while loop == 1:
        for line in log2.readlines():
            if "listening on" in line:
                loop = 0
    log2.close()
    return


def run_hal(args):
    snort_dir = nic_dir + "/hal/third-party/snort3/export"
    os.environ["HAL_CONFIG_PATH"] = nic_dir + "/conf"
    os.environ["LD_LIBRARY_PATH"] = "/home/asic/bin/tools/lib64:" + os.environ["LD_LIBRARY_PATH"]
    os.environ["HAL_PLUGIN_PATH"] = nic_dir + "/../bazel-bin/nic/hal/plugins/"
    os.environ["LD_LIBRARY_PATH"] += ":" + nic_dir + "/../bazel-bin/nic/model_sim/" + ":" + snort_dir + "/bin/"
    os.environ["SNORT_LUA_PATH"] = snort_dir + "/lua/"
    os.environ["LUA_PATH"] = snort_dir + "/lua/?.lua;;"
    os.environ["SNORT_DAQ_PATH"] = snort_dir + "/daqs/"
    os.environ["COVFILE"] = os.path.realpath(bullseye_hal_cov_file)

    hal_dir = nic_dir + "/../bazel-bin/nic/hal"
    os.chdir(hal_dir)

    global hal_log
    jsonfile = 'hal.json'
    if args.rtl:
        jsonfile = 'hal_rtl.json'

    if args.test_suf is not None:
         hal_log = nic_dir + "/logs_%s/hal.log" % args.test_suf
    os.system("cp " + nic_dir + "/conf/hal_switch.ini " + nic_dir + "/conf/hal.ini")
    if args.hostpin:
        #jsonfile = 'hal_hostpin.json'
        os.system("cp " + nic_dir + "/conf/hal_hostpin.ini " + nic_dir + "/conf/hal.ini")
    if args.classic:
        #jsonfile = 'hal_classic.json'
        os.system("cp " + nic_dir + "/conf/hal_classic.ini " + nic_dir + "/conf/hal.ini")

    log = open(hal_log, "w")
    p = Popen(["./hal", "--config", jsonfile], stdout=log, stderr=log)
    global hal_process
    hal_process = p
    print "* Starting HAL pid (" + str(p.pid) + ")"
    print "- Log file: " + hal_log + "\n"

    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()

    time.sleep(10)
    #return

    os.system("rm " + nic_dir + "/conf/hal.ini")
    return


#    log.close()

def dump_coverage_data():
    model_process.send_signal(signal.SIGINT)
    if hal_process:
        hal_process.send_signal(signal.SIGUSR1)
    time.sleep(5)


# Run Storage DOL
def run_storage_dol(port, args):
    wait_for_hal()
    bin_dir = nic_dir + "/../bazel-bin/dol/test/storage"
    os.chdir(bin_dir)
    if args.rtl:
        if args.storage_test:
            cmd = ['./storage_test', '--hal_port', str(port), '--test_group', args.storage_test, '--poll_interval', '900']
        else:
            cmd = ['./storage_test', '--hal_port', str(port), '--poll_interval', '900']
    else:
        if args.storage_test:
            cmd = ['./storage_test', '--hal_port', str(port), '--test_group', args.storage_test]
        else:
            cmd = ['./storage_test', '--hal_port', str(port)]
    p = Popen(cmd)
    p.communicate()
    return p.returncode

# Run GFT tests
def run_gft_test():
    os.environ["HAL_CONFIG_PATH"] = nic_dir + "/conf"
    os.environ["LD_LIBRARY_PATH"] += ":" + nic_dir + "/../bazel-bin/nic/model_sim/"
    os.chdir(nic_dir)
    cmd = ['../bazel-bin/nic/hal/test/gtests/gft_test']
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
    if args.rtl and args.skipverify:
        cmd.append('--eth_mode=ionic')
    if args.tcscale is not None:
        cmd.append('--tcscale')
        cmd.append(args.tcscale)
    if args.dryrun is True:
        cmd.append('--dryrun')
    if args.configonly is True:
        cmd.append('--config-only')
    if args.shuffle is not None:
        cmd.append('--shuffle')
        cmd.append(args.shuffle)
    if args.mbt is True:
        cmd.append('--mbt')
    if args.rtl is True:
        cmd.append('--rtl')
    if args.latency is True:
        cmd.append('--latency')
    if args.pps is True:
        cmd.append('--pps')
    if args.lite is True:
        cmd.append('--lite')

    p = Popen(cmd)
    print "* Starting DOL pid (" + str(p.pid) + ")"
    print "- Log file: " + dol_log + "\n"

    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()
    #p.communicate()
    log.close()

    while 1:
        if p and p.poll() is not None:
            break
        if model_process and model_process.poll() is not None:
            break
        if hal_process and hal_process.poll() is not None:
            break
        time.sleep(5)

    if args.rtl and p.returncode == 0:
        # Wait for runtest to finish only in case of DOL finishing
        # successfully.
        count = 0
        while model_process and model_process.poll() is None:
            count += 5
            time.sleep(5)
            if count % 300 == 0:
                print "RUNTEST/SIMV not exited after %d seconds" % count
            if count == 7200: # 2 hours
                print "%s" % '='*80
                print "             2 Hour Exit Timeout Reached. Killing all processes."
                print "%s" % '='*80
                return 1


    exitcode = 0
    check_for_cores = False;
    if model_process:
        print "* MODEL exit code " + str(model_process.returncode)
        if model_process.returncode:
            exitcode = model_process.returncode
            check_for_cores = True

    if hal_process:
        print "* HAL exit code " + str(hal_process.returncode)
        if hal_process.returncode:
            exitcode = hal_process.returncode
            check_for_cores = True

    if p:
        print "* DOL exit code " + str(p.returncode)
        if p.returncode:
            exitcode = p.returncode

    if check_for_cores:
        #Wait for all stdouts to clear out.
        time.sleep(5)
        process_cores()
    return exitcode

def run_mbt(args, standalone=True):
    mbt_dir = nic_dir + "/../mbt"
    os.chdir(mbt_dir)

    log = open(mbt_log, "a+")
    cmd = ['./main.py']
    # If topology is set, then it is being run in DOL mode.
    if not standalone:
        cmd.append('--mbt')
    if args.mbtrandomseed:
        cmd.append('--mbtrandomseed')
        cmd.append(args.mbtrandomseed)
    p = Popen(cmd)
    print "* Starting Model based testing with  pid (" + str(p.pid) + ")"
    print "- Log file: " + mbt_log + "\n"

    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()

    status = None
    if standalone:
        p.communicate()
        status = p.returncode
    return status

def find_port():
    s = socket.socket()
    s.bind(('', 0))
    port = s.getsockname()[1]
    s.close()

    return port

def is_running(pid):
    try:
        os.kill(pid, 0)
        return True
    except OSError:
        return False

def cleanup(keep_logs=True):
    print "* Killing running processes:"

    if not os.path.exists(lock_file):
        return
    lock = open(lock_file, "r")
    for pid in lock:
        if is_running(int(pid)):
            #print "Sending SIGTERM to process group %d" % int(pid)
            #try:
            #    os.killpg(int(pid), signal.SIGTERM)
            #except:
            #    print "No processes found with PGID %d" % int(pid)


            print "Sending SIGTERM to process %d" % int(pid)
            try:
                os.kill(int(pid), signal.SIGTERM)
            except:
                print "No processes found with PID %d" % int(pid)

    print "Sending SIGTERM to run.py PG %d" % os.getpid()
    #try:
    #    os.killpg(os.getpid(), signal.SIGTERM)
    #except:
    #    print "No processes found with PGID %d" % os.getpid()

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
    print("* FAIL:" if p.returncode != 0 else "* PASS:") + " E2E TLS Proxy DOL, exit code ", p.returncode
    return p.returncode

def run_e2e_l7_dol():
    os.chdir(nic_dir)
    cmd = ['./tools/run_e2e_l7_test.py']
    p = Popen(cmd)
    print "* Starting E2E L7 DOL, pid (" + str(p.pid) + ")"
    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()
    p.communicate()
    print("* FAIL:" if p.returncode != 0 else "* PASS:") + " E2E L7 DOL, exit code ", p.returncode
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
    parser.add_argument('--rtl', dest='rtl', action="store_true",
                        help='Run RTL sim as well.')
    parser.add_argument('--noverilog', dest='noverilog', action="store_true",
                        help='Skip verilog compile dol as well.')
    parser.add_argument('--runtest_runargs', dest='runtest_runargs', default='',
                        help='any extra options that should be passed to runtest as run_args.')
    parser.add_argument('--model_test', dest='model_test', default=None,
                        help='Model test name')
    # This is not needed anymore. It will be automatically derived from skipverify flag.
    #parser.add_argument('--one_pkt_mode', dest='one_pkt_mode', action="store_true",
    #                    help='Enable DOL one pkt mode')
    parser.add_argument('--no_asic_dump', dest='no_asic_dump', action="store_true",
                        help='Disable rtl dump')
    parser.add_argument('--test_suf', dest='test_suf', default=None,
                        help='Model test suffix.')
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
    parser.add_argument("--e2e-l7-dol", dest='e2el7', action="store_true",
                        default=None, help="Run E2E L7 DOL")
    parser.add_argument("--gft", dest='gft', action="store_true",
                        default=False, help="GFT tests")
    parser.add_argument('--shuffle', dest='shuffle', default=None,
                        help='Shuffle tests and loop for X times.')
    parser.add_argument('--mbt', dest='mbt', default=None,
                        action='store_true',
                        help='Modify DOL config testre starting packet tests')
    parser.add_argument('--storage_test', dest='storage_test', default=None,
                        help='Run only a subtest of storage test suite')
    parser.add_argument('--no_error_check', dest='no_error_check', default=None,
                        action='store_true',
                        help='Disable model error checking')
    parser.add_argument('--mbtrandomseed', dest='mbtrandomseed', default=None,
                        help='Seed for random numbers used in model based tests')
    parser.add_argument('--latency', dest='latency', default=None,
                        action='store_true', help='Latency test.')
    parser.add_argument('--pps', dest='pps', default=None,
                        action='store_true', help='PPS Test.')
    parser.add_argument('--lite', dest='lite', default=None,
                        action='store_true', help='Lite Sanity Test.')
    args = parser.parse_args()

    if args.rtl == False and args.skipverify:
        print "ERROR: skipverify option cannot be used for non RTL runs."
        sys.exit(1)
    if not args.dryrun and args.rtl == False and (args.latency or args.pps):
        print "ERROR: latency or pps option cannot be used for non RTL runs."
        sys.exit(1)

    zmq_soc_dir = nic_dir
    if args.test_suf is not None:
        os.system("mkdir -p " + nic_dir + "/logs_%s" % args.test_suf)
        #os.environ['MODEL_SOCKET_NAME'] = "zmqsock_%s" % args.test_suf
        global lock_file
        lock_file = nic_dir + "/logs_%s/.run.pid" % args.test_suf
        zmq_soc_dir = nic_dir + "/logs_%s/" % args.test_suf
    os.environ['ZMQ_SOC_DIR'] = zmq_soc_dir
    os.environ['MODEL_SHMKEY_PATH'] = nic_dir
    os.environ['MODEL_SHMKEY_PROJID'] = '1'

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

    if r != 0:
        print "- build failed!"
        sys.exit(1)

    port = find_port()
    print "* Using port (" + str(port) + ") for HAL\n"
    os.environ["HAL_GRPC_PORT"] = str(port)

    if args.dryrun is False:
        if args.rtl:
            run_rtl(args)
        else:
            run_model(args)
        if args.gft is False:
            run_hal(args)

    if args.storage:
        status = run_storage_dol(port, args)
        if status != 0:
            print "- Storage dol failed, status=", status
    elif args.gft:
        status = run_gft_test()
        if status != 0:
            print "- GFT test failed, status=", status
    elif args.mbt and not args.feature:
        status = run_mbt(args)
        if status != 0:
            print "- MBT test failed, status=", status
    else:
        if args.mbt:
            mbt_port = find_port()
            print "* Using port (" + str(mbt_port) + ") for mbt\n"
            os.environ["MBT_GRPC_PORT"] = str(mbt_port)
            run_mbt(args, standalone=False)

        status = run_dol(args)
        if status == 0:
            if (args.e2etls):
                status = run_e2e_tlsproxy_dol()

            if (args.e2el7):
                status = run_e2e_l7_dol()

    if args.coveragerun:
        dump_coverage_data()
    os.chdir(nic_dir)
    if args.asmcov:
        call(["tools/run-coverage -k -c coverage_asm.json"], shell=True)

    cleanup()
    if not args.rtl and not args.no_error_check:
        ec = os.system("grep ERROR " + model_log)
        if not ec:
            print "ERRORs found in " + model_log
            sys.exit(1)
        print "PASS: No errors found in " + model_log
    print "Status = %d" % status
    os.system("echo %d > .run.status" % status)
    sys.exit(status)

def signal_handler(signal, frame):
    #print "cleanup from signal_handler"
    cleanup()
    sys.exit(1)

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler) # ctrl-c
    signal.signal(signal.SIGTERM, signal_handler) # kill
    signal.signal(signal.SIGQUIT, signal_handler) # ctrl-d
    signal.signal(signal.SIGABRT, signal_handler) # Assert or Abort
    signal.signal(signal.SIGSEGV, signal_handler) # Seg Fault
    remove_all_core_files()
    main()
