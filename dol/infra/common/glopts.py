#! /usr/bin/python3
import argparse
import pdb
import sys
import os

import infra.common.utils as utils

parser = argparse.ArgumentParser(description='DOL Framework')
parser.add_argument('--nohal', dest='no_hal',
                    action='store_true', help='No Connection to HAL')
parser.add_argument('--debug', dest='debug',
                    action='store_true', help='Enable Debug Mode')
parser.add_argument('--verbose', dest='verbose',
                    action='store_true', help='Enable Verbose Mode')
parser.add_argument('--pkglist', dest='pkglist', default=None,
                    help='Run only the tests in package list')
parser.add_argument('--config-only', dest='cfgonly',
                    action='store_true', help='Test HAL Config Only')
parser.add_argument('--cfgjson', dest='cfgjson', default=None,
                    help='Dump configuration to json file')
parser.add_argument('--e2ecfg', dest='e2ecfg', default='conf/hntap.conf',
                    help='Dump E2E configuration to json file')
parser.add_argument('--dryrun', dest='dryrun', action='store_true',
                    help='Dry-Run mode. (No communication with HAL & Model)')
parser.add_argument('--topo', dest='topology', default=None,
                    help='Topology to generate Config Objects.')
parser.add_argument('--test', dest='test', default=None,
                    help='Run single TestSpec by name (from modules.list)')
parser.add_argument('--testcase', dest='tcid', default=None,
                    help='Run single Testcase by ID(E.g. For TC000001 use 1')
parser.add_argument('--module', dest='module', default=None,
                    help='Run all TestSpecs of a module (from modules.list)')
parser.add_argument('--pipeline', dest='pipeline', default="iris",
                    help='Run all Tests for a pipeline.')
parser.add_argument('--feature', dest='feature', default=None,
                    help='Run all Tests for a feature.')
parser.add_argument('--subfeature', dest='subfeature',
                    default=None, help='Run tests for a subfeature')
parser.add_argument('--hostpin', dest='hostpin',
                    action='store_true', help='Run tests in host pinned mode.')
parser.add_argument('--classic', dest='classic',
                    action='store_true', help='Run tests in classic nic mode.')
parser.add_argument('--coveragerun', dest='coveragerun',
                    action='store_true', help='Move asm data if coverage run enabled')
parser.add_argument('--regression', dest='regression',
                    action='store_true', help='Run tests in regression mode.')
parser.add_argument('--skipverify', dest='skipverify',
                    action='store_true', help='Skip Verification all tests.')
parser.add_argument('--eth_mode', dest='eth_mode', default="onepkt",
                    choices=["onepkt", "ionic"],
                    help='ETH driver mode selection.')
parser.add_argument('--tcscale', dest='tcscale', default=None,
                    help='Testcase Scale Factor.')
parser.add_argument('--modscale', dest='modscale', default=None,
                    help='Module Scale Factor.')
parser.add_argument('--shuffle', dest='shuffle', action='store_true',
                    help='Shuffle test order')
parser.add_argument('--mbt', dest='mbt',
                    action='store_true', help='Enable model based tester'
                    'for enabled features')
#parser.add_argument('--cfgscale', dest='cfgscale', default=None,
#                    help='Configuration Scale Factor.')
parser.add_argument('--rtl', dest='rtl',
                    action='store_true', help='Run tests in RTL mode.')
parser.add_argument('--nohostmem', dest='hostmem', default=True,
                    action='store_false', help='Disable Host Memory.')
parser.add_argument('--mbtrandomseed', dest='mbtrandomseed', default=None,
                    help='Random seem for Model based tests')
parser.add_argument('--latency', dest='latency', default=False,
                    action='store_true', help='Latency Test.')
parser.add_argument('--pps', dest='pps', default=False,
                    action='store_true', help='PPS Test.')
parser.add_argument('--lite', dest='lite', default=False,
                    action='store_true', help='Lite Sanity Test.')
parser.add_argument('--l2l', dest='l2l', default=False,
                    action='store_true', help='Local-2-Local Endpoint RDMA Sanity Run.')
parser.add_argument('--gft', dest='gft', default=False,
                    action='store_true', help='GFT Test.')
parser.add_argument('--perf', dest='perf', default=False,
                    action='store_true', help='Run only Perf tests.')
parser.add_argument('--e2e', dest='e2e', action='store_true',
                    help='Start DOL with E2E Testing')
parser.add_argument('--zmqtcp', dest='zmqtcp', default=False,
                    action='store_true', help='Use ZMQ TCP instead of IPC')
parser.add_argument('--hal_ip', dest='hal_ip', default=None,
                    help='HAL IP Address.')
parser.add_argument('--model_ip', dest='model_ip', default=None,
                    help='Model IP Address.')
parser.add_argument('--agent', dest='agent', action='store_true',
                    help='Start DOL with Agent')
parser.add_argument('--savepcap', dest='savepcap', action='store_true',
                    help='Save all the packets to PCAP instead.')
parser.add_argument('--save-tc-pcap', dest='save_tc_pcap', action='store_true',
                    help='Save per testcase PCAP files.')
parser.add_argument('--pcapdir', dest='pcapdir',
                    help='PCAP Directory.')
parser.add_argument('--naplescontainer', dest='naples_container', action='store_true',
                    help='Nic running(hal,model) inside container.')
parser.add_argument('--netagent', dest='netagent', action='store_true',
                    help='Use REST queries to netagent instead of GRPC',
                    default=False)

GlobalOptions = None
def ParseArgs():
    global GlobalOptions
    GlobalOptions = parser.parse_args()

def ValidateGlopts():
    if GlobalOptions.topology is None or GlobalOptions.feature is None:
        print("Error: Manadatory args 'topo' and 'feature' are missing.")
        print("Usage: ./main.py --topo <topology-name> --feature <feature1,feature2...>")
        sys.exit(1)

    if GlobalOptions.tcid is not None:
        tcid_range_list = GlobalOptions.tcid.split(',')
        tcid_list = []
        for tcid_range in tcid_range_list:
            rl = tcid_range.split('-')
            low = None
            high = None
            if len(rl) > 2:
                print("Error: Invalid tcid range.")
                sys.exit(1)
            elif len(rl) == 2:
                high = utils.ParseInteger(rl[1])
            low = utils.ParseInteger(rl[0])
            if high:
                tcid_list += range(low, high+1)
            else:
                tcid_list.append(low)
        tcid_list.sort()

        GlobalOptions.tcid = tcid_list
        if len(GlobalOptions.tcid) == 1:
            GlobalOptions.tcid = GlobalOptions.tcid[0]
    GlobalOptions.alltc_done = False

    if GlobalOptions.rtl:
        GlobalOptions.lite = True

    if GlobalOptions.latency:
        if not GlobalOptions.dryrun:
            GlobalOptions.tcscale = 10
        GlobalOptions.lite = True

    if GlobalOptions.pps:
        if not GlobalOptions.dryrun:
            GlobalOptions.tcscale = 1000
        GlobalOptions.lite = True

    if GlobalOptions.perf:
        GlobalOptions.lite = True

    if GlobalOptions.modscale:
        GlobalOptions.modscale = utils.ParseInteger(GlobalOptions.modscale)

    GlobalOptions.feature_set = "iris"
    if GlobalOptions.gft:
        GlobalOptions.feature_set = "gft"

    if GlobalOptions.zmqtcp:
        os.environ['MODEL_ZMQ_TYPE_TCP'] = '1'
    if GlobalOptions.hal_ip:
        os.environ["HAL_GRPC_IP"] = GlobalOptions.hal_ip
    if GlobalOptions.model_ip:
        os.environ["MODEL_ZMQ_SERVER_IP"] = GlobalOptions.model_ip

    if GlobalOptions.debug:
        print("========================================================")
        print("Global Command Line Options")
        for k,v in GlobalOptions.__dict__.items():
            print ("  %s = " % k, v)
        print("========================================================")

    if GlobalOptions.savepcap:
        GlobalOptions.skipverify=True
