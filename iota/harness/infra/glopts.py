#! /usr/bin/python3
import argparse
import pdb
import sys
import os

__valid = False

parser = argparse.ArgumentParser(description='IOTA Harness')
parser.add_argument('--debug', dest='debug',
                    action='store_true', help='Enable Debug Mode')
parser.add_argument('--verbose', dest='verbose',
                    action='store_true', help='Enable Verbose Mode')
parser.add_argument('--dryrun', dest='dryrun', action='store_true',
                    help='Dry-Run mode. (No communication with HAL & Model)')
parser.add_argument('--testsuite', dest='testsuite', default=None,
                    help='Run single Testsuite.')
parser.add_argument('--testbundle', dest='testbundle', default=None,
                    help='Run single Testbundle.')
parser.add_argument('--testcase', dest='testcase', default=None,
                    help='Run single Testcase.')
parser.add_argument('--coverage', dest='coverage',
                    action='store_true', help='Collect code coverage data.')
parser.add_argument('--coverage-file', dest='coverage_file',
                    help='Naples Coverage file.')
parser.add_argument('--coverage-dir', dest='coverage_dir',
                    help='Directory to copy coverage file')

#parser.add_argument('--svcport', dest='svcport', default=60000,
#                    help='IOTA Service Port.')
parser.add_argument('--testbed', dest='testbed_json', default="warmd.json",
                    help='Testbed JSON file')
parser.add_argument('--no-keep-going', dest='no_keep_going',
                    action='store_true', help='Stop on first error.')
parser.add_argument('--stop-on-critical', dest='stop_on_critical',
                    action='store_true', default=True, help='Stop on critical errors')
parser.add_argument('--skip-host-intf-check', dest='skip_host_intf_check',
                    action='store_true', help='Skip Host Interface check')
#parser.add_argument('--mode', dest='mode', default='hardware',
#                    choices=["hardware", "simulation", "mixed"],
#                    help='Testbed Mode: Hardware / Simulation / Mixed.')
parser.add_argument('--pipeline', dest='pipeline', default="iris",
                    help='Filter tests by Pipeline')
parser.add_argument('--rerun', dest='rerun',
                    action='store_true', help='Re-run mode, Skips copy and few init steps.')
parser.add_argument('--skip-driver-install', dest='skip_driver_install',
                    action='store_true', help='Skips driver install.')
parser.add_argument('--skip-firmware-upgrade', dest='skip_firmware_upgrade',
                    action='store_true', help='Skip Firmware Upgrade and Start Tests.')
parser.add_argument('--only-firmware-upgrade', dest='only_firmware_upgrade',
                    action='store_true', help='Only Do Firmware Upgrade and Stop.')
parser.add_argument('--only-reboot', dest='only_reboot',
                    action='store_true', help='Only reboot the nodes and start tests.')
parser.add_argument('--precheckin', dest='precheckin',
                    action='store_true', help='Also run precheckin Testsuites.')
parser.add_argument('--regression', dest='regression',
                    action='store_true', help='Also run regression Testsuites.')
parser.add_argument('--extended', dest='extended',
                    action='store_true', help='Also run extended Testsuites.')
parser.add_argument('--skip-sanity', dest='skip_sanity',
                    action='store_true', help='Skip sanity testbundles.')
parser.add_argument('--skip-setup', dest='skip_setup',
                    action='store_true', help='Skip iota setup')
parser.add_argument('--skip-switch-init', dest='skip_switch_init',
                    action='store_true', help='Skip Switch Initialization')
parser.add_argument('--stress', dest='stress',
                    action='store_true', help='Enable stress test mode.')
parser.add_argument('--bundle-stress', dest='bundle_stress',type=int, default=1,
                    help='Number of iterations to run testbundle')
parser.add_argument('--stress-factor', dest='stress_factor', type=int, default=1,
                    help='Stress factor - Control # of iterations in stress test.')
parser.add_argument('--logdir', dest='logdir', default=None,
                    help='Save logs in a specific directory. (Relative to IOTA directory)')
parser.add_argument('--skip-logs', dest='skip_logs', help='Skip log collection.',
                    action='store_true')
parser.add_argument('--use-gold-firmware', dest='use_gold_firmware',
                    action='store_true', help='Use gold firmware for testing')
parser.add_argument('--fast-upgrade', dest='fast_upgrade',
                    action='store_true', help='install firmware only')
#netagent - iota posts the config to netagent on naples
parser.add_argument('--netagent', dest='netagent', action='store_true',
                    help='Use REST queries to netagent instead of GRPC',
                    default=False)

parser.add_argument('--naples-type', dest='naples_type', default="capri",
                    help='Naples type to run')

GlobalOptions = parser.parse_args()

GlobalOptions.default_username = "vm"
GlobalOptions.default_password = "vm"

GlobalOptions.testsuites = None
if GlobalOptions.testsuite != None:
    GlobalOptions.testsuites = GlobalOptions.testsuite.split(',')

GlobalOptions.testbundles = None
if GlobalOptions.testbundle != None:
    GlobalOptions.testbundles = GlobalOptions.testbundle.split(',')

GlobalOptions.beg_testcase = None
GlobalOptions.end_testcase = None
GlobalOptions.testcases = None
GlobalOptions.markers_present = False
GlobalOptions.inb_markers = False
#GlobalOptions.skip_firmware_upgrade = True
#GlobalOptions.skip_driver_install = True

if GlobalOptions.testcase != None:
    if "..." in GlobalOptions.testcase:
        markers = GlobalOptions.testcase.split("...")
        GlobalOptions.testcase_begin = markers[0]
        GlobalOptions.testcase_end = markers[1]
        GlobalOptions.markers_present = True
    else:
        GlobalOptions.testcases = GlobalOptions.testcase.split(',')

GlobalOptions.uid = os.environ.get("USER", None)
if GlobalOptions.uid == None:
    GlobalOptions.uid = os.environ.get("SUDO_USER", None)
    if GlobalOptions.uid == None:
        if os.environ.get("JOB_ID", None) == None:
            raise("Please run as no sudo user")
        else:
            GlobalOptions.uid = "SUPER_USER"


def __validate():
    if GlobalOptions.debug:
        print("========================================================")
        print("Global Command Line Options")
        for k,v in GlobalOptions.__dict__.items():
            print ("  %s = " % k, v)
        print("========================================================")

    global __valid
    __valid = True

if not __valid:
    __validate()
