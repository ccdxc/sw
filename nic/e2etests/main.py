#!/usr/bin/python3
import os
import sys
import glob
import ruamel.yaml as yaml
import consts
import argparse
import time
import atexit
import signal
from infra.e2e_test import E2eTest

e2e_test = None

def get_test_files():
    return glob.glob(tests_dir + "/*.py")

def parse_yaml_file(file):
    with open(file, 'r') as stream:
        return yaml.load(stream)    

def get_test_specs():
    spec_files = glob.glob(consts.specs_dir + "/*.spec")
    config_specs = [parse_yaml_file(spec) for spec in spec_files]
    return config_specs

def run_tests_in_auto_mode(spec=None):
    test_specs = get_test_specs()
    global e2e_test
    for cfg in test_specs:
        e2e_test = E2eTest(cfg)
        e2e_test.BringUp()
        e2e_test.PrintEnvironmentSummary()
        ret = e2e_test.Run()
        e2e_test.Teardown()
        if not ret:
            print ("Test %s Failed" % str(e2e_test))
            sys.exit(1)
        print ("Test %s Passed" % str(e2e_test))

def bringup_test_spec_env(spec):
    test_specs = get_test_specs()
    global e2e_test
    for test_spec in test_specs:
        if test_spec["name"] == spec:
            e2e_test = E2eTest(test_spec)
            print ("Bring up E2E environment for testspec : ",  spec)
            e2e_test.BringUp()
            print ("E2E environment up for testspec : ",  spec)
            e2e_test.PrintEnvironmentSummary()
            while True:
                time.sleep(10)
            e2e_test.Teardown()    


def cleanup():
    #Clean up environment if process is cancelled.
    if e2e_test:
        e2e_test.Teardown()

def signal_handler(signal, frame):
    #print "cleanup from signal_handler"
    cleanup()
    sys.exit(1)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--e2e-mode', dest='test_mode', default="auto",
                    choices=["auto", "manual"],
                    help='E2E Test mode.')
    parser.add_argument('--e2e-spec', dest='e2e_spec', default=None,
                    help='E2E spec if running in manual mode.')    
    args = parser.parse_args()

    os.chdir(consts.nic_e2e_dir)

    if args.test_mode == "auto":
        run_tests_in_auto_mode()
    else:
        bringup_test_spec_env(args.e2e_spec)
    return 


if __name__ == "__main__":
    signal.signal(signal.SIGTERM, signal_handler) # kill
    main()
