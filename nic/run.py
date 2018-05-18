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
import shlex

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
stg_dol_process = None
nw_dol_process = None

build_log = nic_dir + "/build.log"
model_log = nic_dir + "/model.log"
hal_log = nic_dir + "/hal.log"
dol_log = nic_dir + "/dol.log"
stg_dol_log = nic_dir + "/stg_dol.log"
mbt_log = nic_dir + "/mbt.log"
storage_dol_log = nic_dir + "/storage_dol.log"
sample_client_log = nic_dir + "/sample_client.log"
bullseye_model_cov_file = nic_dir + "/coverage/bullseye_model.cov"
bullseye_hal_cov_file = nic_dir + "/../bazel-out/../../bullseye_hal.cov"

nic_container_image_dir = nic_dir + "/obj/images"
#This should be changed to pick up the current version automatically.
nic_container_image = "naples-release-v1.tgz"
nic_container_name = "naples-v1"

#Naples start script
nic_container_startup_script = nic_dir + "/sim/naples/start-naples-docker.sh"

lock_file = nic_dir + "/.run.pid"

# Environment

os.environ["LD_LIBRARY_PATH"] = "/usr/local/lib:/usr/local/lib64:asic/capri/model/capsim-gen/lib:third-party/lkl/export/bin"
os.environ["PKG_CONFIG_PATH"] = "/usr/local/lib/pkgconfig"

#Path and executables
model_executable = nic_dir + "/../bazel-bin/nic/model_sim/cap_model"
model_core_path  = nic_dir

hal_executable = nic_dir + "/../bazel-bin/nic/hal/hal"
hal_core_path = nic_dir

def print_core(executable, core):
    cmd = ['gdb', '-batch', '-ex', 'bt']
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

def process_core(executable, core_file):
    if os.path.isfile(core_file):
        #Wait for all std out to clear.
        time.sleep(5)
        print_core(executable, core_file)
        os.system("mv " + core_file + "  " + nic_dir + "/core." +  os.path.basename(executable))


def process_hal_core(hal_core_file):
    print ("**********HAL CORE BEGIN**********")
    process_core(hal_executable, hal_core_file)
    print ("**********HAL CORE END**********")

def process_model_core(model_core_file):
    print ("**********MODEL CORE BEGIN**********")
    process_core(model_executable, model_core_file)
    print ("**********MODEL CORE END**********")


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
    asic_src = os.getcwd() + "/asic"
    os.environ["ASIC_SRC"] = asic_src
    os.environ["LD_LIBRARY_PATH"] = ".:../libs:/usr/local/lib:/usr/local/lib64:" + os.getcwd() + "/asic/capri/model/capsim-gen/lib:/home/asic/bin/tools/lib64"
    os.environ["PATH"] = os.getcwd() + "/asic/common/tools/bin" + ":" + os.environ["PATH"]

    if args.port_mode == 'nomac':
        model_test = "core_basic_dol_2x100_ahbm"
    elif args.port_mode == '2x100':
        model_test = "core_basic_dol_2x100"
    elif args.port_mode == '8x25':
        model_test = "core_basic_dol_8x25"
    else:
        print "Unknown port_mode", args.port_mode
        sys.exit(1)

    coverage_opts = []
    if args.rtl_coverage:
        coverage_opts = [ '-cov', '-tcov' ]

    one_pkt_mode = ""
    if args.model_test:
        model_test = args.model_test
    flow_stat_tbl_base = 0x108faf800
    if args.storage:
        flow_stat_tbl_base += 0x4
    if not args.skipverify:
        one_pkt_mode = "+dol_one_pkt_mode=1 +save_rtl_pkts=1"
    model_cmd = [ 'runtest', '-ngrid', '-test', model_test, '-run_args', ' %s  +flow_stat_tbl_base=%s +dol_poll_time=5 +dump_axi +pcie_all_lif_valid=1 +UVM_VERBOSITY=UVM_MEDIUM +fill_pattern=0 +te_dbg +plog=info +mem_verbose +verbose +PLOG_MAX_QUIT_COUNT=100 +top_sb/initial_timeout_ns=60000 %s ' % (one_pkt_mode, flow_stat_tbl_base, args.runtest_runargs), '-cfg_args', 'core/axi_master/<0:3>/max_read_latency=3000 core/axi_master/<0:3>/avg_max_read_latency=3000 core/axi_master/<0:3>/max_write_latency=3000 core/axi_master/<0:3>/avg_max_write_latency=3000' ] + coverage_opts
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
    os.environ["LD_LIBRARY_PATH"] = ".:../libs:/usr/local/lib:/usr/local/lib64:/home/asic/bin/tools/lib64"

    #model_dir = nic_dir + "/model_sim/build"
    model_dir = nic_dir + "/../bazel-bin/nic/model_sim"
    os.chdir(model_dir)

    log = open(model_log, "w")
#    p = Popen(["sh", "run_model"], stdout=log, stderr=log)
    model_cmd = [ "./cap_model", "+PLOG_MAX_QUIT_COUNT=0", "+plog_add_scope=axi_trace" ]
    if args.modellogs:
        model_cmd.append("+plog=info")
        if args.gft or args.gft_gtest:
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
    os.environ["LD_LIBRARY_PATH"] += ":" + nic_dir + "/../bazel-bin/nic/model_sim/" + ":" + snort_dir + "/x86_64/lib/"
    os.environ["LD_LIBRARY_PATH"] += ":" + nic_dir + "/sdk/obj/lib"
    os.environ["LD_LIBRARY_PATH"] += ":" + nic_dir + "/sdk/obj/lib/external"
    os.environ["LD_LIBRARY_PATH"] += ":" + nic_dir + "/hal/obj"
    os.environ["SNORT_LUA_PATH"] = snort_dir + "/lua/"
    os.environ["LUA_PATH"] = snort_dir + "/lua/?.lua;;"
    os.environ["SNORT_DAQ_PATH"] = snort_dir + "/x86_64/lib/"
    os.environ["COVFILE"] = os.path.realpath(bullseye_hal_cov_file)

    #hal_dir = nic_dir + "/../bazel-bin/nic/hal"
    os.chdir(nic_dir)

    global hal_log
    jsonfile = 'hal.json'
    if args.rtl:
        jsonfile = 'hal_rtl.json'
    if args.gft:
        jsonfile = 'hal_gft.json'

    hal_log_dir = nic_dir
    if args.test_suf is not None:
        hal_log_dir = nic_dir + "/logs_%s/" % args.test_suf
    
    os.environ["HAL_LOG_DIR"] = hal_log_dir
    hal_log = hal_log_dir + "/hal.log"
    os.system("rm -rf %s/hal.log.*" % hal_log_dir)
    os.system("cp " + nic_dir + "/conf/hal_switch.ini " + nic_dir + "/conf/hal.ini")
    if args.hostpin:
        #jsonfile = 'hal_hostpin.json'
        os.system("cp " + nic_dir + "/conf/hal_hostpin.ini " + nic_dir + "/conf/hal.ini")
    if args.classic:
        #jsonfile = 'hal_classic.json'
        os.system("cp " + nic_dir + "/conf/hal_classic.ini " + nic_dir + "/conf/hal.ini")

    FNULL = open(os.devnull, 'w')
    p = Popen(["../bazel-bin/nic/hal/hal", "--config", jsonfile], stdout=FNULL, stderr=FNULL)
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
            cmd = ['./storage_test', '--hal_port', str(port), '--hal_ip', str(args.hal_ip), '--test_group', args.storage_test, '--poll_interval', '3600', '--long_poll_interval', '3600']
        else:
            cmd = ['./storage_test', '--hal_port', str(port), '--hal_ip', str(args.hal_ip), '--poll_interval', '3600', '--long_poll_interval', '3600']
    else:
        if args.storage_test:
            cmd = ['./storage_test', '--hal_port', str(port), '--hal_ip', str(args.hal_ip), '--test_group', args.storage_test]
        else:
            cmd = ['./storage_test', '--hal_port', str(port), '--hal_ip', str(args.hal_ip)]
    if args.combined:
        cmd.append('--combined')
    #pass additional arguments to storage_test
    if args.storage_runargs:
        cmd.extend(shlex.split(args.storage_runargs))
        print 'Executing command [%s]' % ', '.join(map(str, cmd))

    global stg_dol_process
    if args.combined:
        stg_log = open(stg_dol_log, "w")
        stg_dol_process = Popen(cmd, stdout=stg_log, stderr=stg_log)
        return 0
    else:
        stg_dol_process = Popen(cmd)
        return check_for_completion(None, stg_dol_process, model_process, hal_process, args)

# Run GFT tests
def run_gft_test(args):
    os.environ["HAL_CONFIG_PATH"] = nic_dir + "/conf"
    os.environ["LD_LIBRARY_PATH"] += ":" + nic_dir + "/../bazel-bin/nic/model_sim/"
    os.chdir(nic_dir)
    cmd = ['../bazel-bin/nic/hal/test/gtests/gft_test']
    p = Popen(cmd)
    #p.communicate()
    return check_for_completion(p, None, model_process, hal_process, args)


# DOL

def check_for_completion(nw_dol_process, stg_dol_process,  model_process, hal_process, args):
    nw_dol_done = False
    stg_dol_done = False
    if nw_dol_process is None:
        nw_dol_done = True
    if stg_dol_process is None:
        stg_dol_done = True
    while 1:
        if nw_dol_process and nw_dol_process.poll() is not None:
            nw_dol_done = True            
        if stg_dol_process and stg_dol_process.poll() is not None:
            stg_dol_done = True
        if nw_dol_done and stg_dol_done:
            break
        if model_process and model_process.poll() is not None:
            break
        if hal_process and hal_process.poll() is not None:
            break
        time.sleep(5)

    status = 0
    if nw_dol_process:
        status = nw_dol_process.returncode
    if status == 0 and stg_dol_process:
        status = stg_dol_process.returncode
    if args.rtl and status == 0:
        # Wait for runtest to finish only in case of DOL finishing
        # successfully.
        count = 0
        while model_process and model_process.poll() is None:
            count += 5
            time.sleep(5)
            if count % 300 == 0:
                print "RUNTEST/SIMV not exited after %d seconds" % count

            exit_timeout = 4
            if 'MODEL_EXIT_TIMEOUT' in os.environ:
                exit_timeout = os.environ['MODEL_EXIT_TIMEOUT']
            if count == exit_timeout * 60 * 60:
                print "%s" % '='*80
                print "             %d hour Exit Timeout Reached. Killing all processes." % exit_timeout
                print "%s" % '='*80
                return 1


    exitcode = 0
    check_for_cores = False;
    if model_process:
        print "* MODEL exit code " + str(model_process.returncode)
        if model_process.returncode:
            exitcode = model_process.returncode
            core_file = model_core_path + "/core." + str(model_process.pid)
            process_model_core(core_file)

    if hal_process:
        print "* HAL exit code " + str(hal_process.returncode)
        if hal_process.returncode:
            exitcode = hal_process.returncode
            core_file = hal_core_path + "/core." + str(hal_process.pid)
            process_hal_core(core_file)

    if nw_dol_process:
        print "* NW DOL exit code " + str(nw_dol_process.returncode)
        if nw_dol_process.returncode:
            exitcode = nw_dol_process.returncode

    if stg_dol_process:
        print "* Storage DOL exit code " + str(stg_dol_process.returncode)
        if stg_dol_process.returncode:
            exitcode = stg_dol_process.returncode

    return exitcode


def run_dol(args):
    dol_dir = nic_dir + "/../dol"
    os.chdir(dol_dir)

    #log = open(dol_log, "w")
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
    if args.cfgjson is not None:
        cmd.append('--cfgjson')
        cmd.append(args.cfgjson)
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
    if args.modscale is not None:
        cmd.append('--modscale')
        cmd.append(args.modscale)
    if args.dryrun is True:
        cmd.append('--dryrun')
    if args.configonly is True:
        cmd.append('--config-only')
    if args.shuffle:
        cmd.append('--shuffle')
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
    if args.l2l is True:
        cmd.append('--l2l')
    if args.gft is True:
        cmd.append('--gft')
    if args.perf is True:
        cmd.append('--perf')
    if args.e2e_mode == "dol-auto":
        cmd.append("--e2e")
    if args.debug:
        cmd.append("--debug")
    if args.verbose:
        cmd.append("--verbose")
    if args.agent:
        cmd.append("--agent")
    if args.niccontainer:
        cmd.append("--niccontainer")

    global nw_dol_process
    if args.coveragerun:
        #Increasing timeout for coverage runs only.
        dol_env = os.environ.copy()
        dol_env["MODEL_TIMEOUT"] = "300"
        nw_dol_process = Popen(cmd, env=dol_env)
    else:
        nw_dol_process = Popen(cmd)
    print "* Starting DOL pid (" + str(nw_dol_process.pid) + ")"
    print "- Log file: " + dol_log + "\n"

    lock = open(lock_file, "a+")
    lock.write(str(nw_dol_process.pid) + "\n")
    lock.close()
    #p.communicate()
    #log.close()

    if args.combined:
        return 0
    else:
        return check_for_completion(nw_dol_process, None, model_process, hal_process, args)

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
    print_nic_container_cores()
    bringdown_nic_container()
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

def run_v6_e2e_tlsproxy_dol():
    os.chdir(nic_dir)
    cmd = ['./tools/run_v6_e2e_tlsproxy_test.py']
    p = Popen(cmd)
    print "* Starting E2E TLS Proxy DOL, pid (" + str(p.pid) + ")"
    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()
    p.communicate()
    print("* FAIL:" if p.returncode != 0 else "* PASS:") + " E2E TLS Proxy DOL, exit code ", p.returncode
    return p.returncode

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

def run_e2e_infra_dol(mode, e2espec = None, niccontainer = None):
    os.chdir(nic_dir)
    cmd = ['./e2etests/main.py', '--e2e-mode', mode]
    if e2espec:
        cmd.extend(['--e2e-spec', e2espec])
    if niccontainer:
        cmd.extend(["--niccontainer-name", niccontainer])
    p = Popen(cmd)
    print "* Starting E2E , pid (" + str(p.pid) + ")"
    lock = open(lock_file, "a+")
    lock.write(str(p.pid) + "\n")
    lock.close()
    p.communicate()
    print("* FAIL:" if p.returncode != 0 else "* PASS:") + " E2E ,DOL, exit code ", p.returncode
    return p.returncode

def bringup_nic_container():
    bringdown_nic_container()
    hal_log_file = os.environ['HOME'] + "/naples/data/logs/hal.log"
    def get_hal_port(log_file):
        log2 = open(log_file, "r")
        loop = 1
        while loop == 1:
            for line in log2.readlines():
                if "listening on" in line:
                    return line.split(":")[-1].strip()  
        log2.close()
        return

    if os.path.isfile(hal_log_file):
        os.remove(hal_log_file)
    
    os.chdir(nic_container_image_dir)
    if not os.path.isfile(nic_container_image):
        print ("Nic Container image %s was not found", nic_container_image)
        sys.exit(1) 
    #Extract the container image
    print ("Extracting Container image")
    retcode = call(['tar', '-xzvf', nic_container_image])
    if retcode:
        print ("Extraction failed!")
        sys.exit(1)
    retcode = call(nic_container_startup_script)
    if retcode:
        print ("Bringing up Nic container failed")
        sys.exit(1)
    os.chdir(nic_dir)
    time.sleep(5)
    print ("Waiting for HAL to be up..")
    os.environ["HAL_GRPC_PORT"] = get_hal_port(hal_log_file)
    print ("Nic container bring up was successfull.")


def bringdown_nic_container():
    print ("Bringing down nic container")
    try: 
        os.chdir(nic_container_image_dir)
        retcode = call(["docker", "stop", nic_container_name])
    except:
        retcode = 0
    if retcode:
        print("Bringdown of Nic Container failed")
    os.chdir(nic_dir)
    
def print_nic_container_cores():
    print_core_script = "/naples/nic/tools/print-cores.sh"
    try: 
        retcode = call(["docker", "exec", "-it", nic_container_name, print_core_script])
    except:
        retcode = 0
    if retcode:
        print("Printing container cores failed.")

def run_dol_test(args):
    status = run_dol(args)
    if status == 0:
        if (args.e2etls):
            status = run_e2e_tlsproxy_dol()
        elif (args.e2el7):
            status = run_e2e_l7_dol()
        elif (args.v6e2etls):
                status = run_v6_e2e_tlsproxy_dol()
        elif (args.e2e_mode and args.e2e_mode != "dol-auto"):
            status = run_e2e_infra_dol(args.e2e_mode, args.e2e_spec,
                                        niccontainer = nic_container_name if args.niccontainer else None)
    return status
                
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", "--build", action="store_true", help="run build")
    parser.add_argument("-c", "--cleanup", action="store_true",
                        help="cleanup running process")
    parser.add_argument("--niccontainer", action="store_true",
                        help="run with Nic container image")
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
    parser.add_argument('--modscale', dest='modscale', default=None,
                        help='Module Scale Factor.')
#    parser.add_argument('--cfgscale', dest='cfgscale', default=None,
#                        help='Configuration Scale Factor.')
    parser.add_argument('--dryrun', dest='dryrun', action='store_true',
                        help='Dry-Run mode. (No communication with HAL & Model)')
    parser.add_argument("--configtest", action="store_true",
                        help="Run Config tests.")
    parser.add_argument("--config-only", dest='configonly', action="store_true",
                        help="Generate ASM coverage for this run")
    parser.add_argument("--v6-e2e-tls-dol", dest='v6e2etls', action="store_true",
                        default=None, help="Run V6 E2E TLS DOL")
    parser.add_argument("--e2e-tls-dol", dest='e2etls', action="store_true",
                        default=None, help="Run E2E TLS DOL")
    parser.add_argument("--e2e-l7-dol", dest='e2el7', action="store_true",
                        default=None, help="Run E2E L7 DOL")
    parser.add_argument('--e2e-mode', dest='e2e_mode', default=None,
                    choices=["dol-auto", "auto", "manual"], help='E2E Test mode.')
    parser.add_argument('--e2e-spec', dest='e2e_spec', default=None,
                        help='E2E Spec to bring up if manual mode.')
    parser.add_argument("--gft", dest='gft', action="store_true",
                        default=False, help="GFT tests")
    parser.add_argument("--gft_gtest", dest='gft_gtest', action="store_true",
                        default=False, help="Run GFT gtests")
    parser.add_argument('--shuffle', dest='shuffle', action="store_true",
                        help='Shuffle tests and loop for X times.')
    parser.add_argument('--mbt', dest='mbt', default=None,
                        action='store_true',
                        help='Modify DOL config testre starting packet tests')
    parser.add_argument('--mbt_test', dest='mbt_test', default=None,
                        action='store_true',
                        help='Modify DOL config testre starting packet tests')
    parser.add_argument('--storage_test', dest='storage_test', default=None,
                        help='Run only a subtest of storage test suite')
    parser.add_argument('--storage_runargs', dest='storage_runargs', default='',
                        help='any extra options that should be passed to storage as run_args.')
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
    parser.add_argument('--l2l', dest='l2l', default=None,
                        action='store_true', help='Local-2-Local Endpoint RDMA Sanity Run.')
    parser.add_argument('--port_mode', dest='port_mode', default='nomac',
                        help='In RTL mode choose how many ports are active: (nomac/8x25/2x100)')
    parser.add_argument('--perf', dest='perf', default=None,
                        action='store_true', help='Run Perf tests.')
    parser.add_argument('--cfgjson', dest='cfgjson', default=None,
                        help='Dump DOL configuration to json file')
    parser.add_argument('--zmqtcp', dest='zmqtcp', default=None,
                        action='store_true', help='Run for ZEBU environment.')
    parser.add_argument('--hal_ip', dest='hal_ip', default='localhost',
                        help='HAL IP Address.')
    parser.add_argument('--model_ip', dest='model_ip', default=None,
                        help='Model IP Address.')
    parser.add_argument('--debug', dest='debug', default=None,
                        action='store_true', help='Set loglevel to DEBUG.')
    parser.add_argument('--verbose', dest='verbose', default=None,
                        action='store_true', help='Set loglevel to VERBOSE.')
    parser.add_argument('--rtl_coverage', dest='rtl_coverage', default=None, action='store_true',
                        help='Dump RTL toggle coverage')
    parser.add_argument('--agent', dest='agent', default=None, action='store_true',
                        help='Run with agent.')
    parser.add_argument('--combined', dest='combined', default=False, action='store_true',
                        help='Run storage and network tests combined')

    args = parser.parse_args()

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
    if args.rtl:
        os.environ['HAL_PLATFORM_MODE_RTL'] = '1'

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
    if args.mbt_test:
        port = int(os.environ["HAL_GRPC_PORT"])
    print "* Using port (" + str(port) + ") for HAL\n"
    os.environ["HAL_GRPC_PORT"] = str(port)

    if args.zmqtcp:
        os.environ['MODEL_ZMQ_TYPE_TCP'] = '1'
        zmq_tcp_port = find_port()
        os.environ['MODEL_ZMQ_TCP_PORT'] = str(zmq_tcp_port)

    if args.dryrun is False:
        if args.niccontainer:
            bringup_nic_container()
        else:
            if args.rtl:
                run_rtl(args)
            else:
                run_model(args)
            if args.gft_gtest is False:
                run_hal(args)

    if args.storage and args.feature is not None and args.combined is False:
        print "ERROR: Use --combined to run storage and networking tests together\n"
        sys.exit(1)

    if args.storage and args.combined is False:
        status = run_storage_dol(port, args)
        if status != 0:
            print "- Storage dol failed, status=", status
    elif args.gft_gtest:
        status = run_gft_test(args)
        if status != 0:
            print "- GFT test failed, status=", status
    elif args.mbt and not args.feature:
        status = run_mbt(args)
        if status != 0:
            print "- MBT test failed, status=", status
    elif args.mbt_test:
        mbt_port = int(os.environ["MBT_GRPC_PORT"])
        print "* Using port (" + str(mbt_port) + ") for mbt\n"
        status = run_dol_test(args)
    else:
        if args.mbt:
            mbt_port = find_port()
            print "* Using port (" + str(mbt_port) + ") for mbt\n"
            os.environ["MBT_GRPC_PORT"] = str(mbt_port)
            run_mbt(args, standalone=False)
        status = run_dol_test(args)

    if args.combined:
        if args.storage:
            status = run_storage_dol(port, args)
            if status != 0:
                print "- Storage dol failed, status=", status
        status = check_for_completion(nw_dol_process, stg_dol_process, model_process, hal_process, args)
        if status != 0:
            print "- Dol failed, status=", status

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
