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
import pdb
import json
import subprocess
import requests
from infra.e2e_test import E2eTest, E2eEnv

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

def run_tests_in_auto_mode(spec=None, naples_container=None):
    test_specs = get_test_specs()
    global e2e_test
    for cfg in test_specs:
        if not cfg["enabled"]:
            continue
        e2e_test = E2eTest(cfg, naples_container)
        ret = e2e_test.Run()
        if not ret:
            print ("Test %s Failed" % str(e2e_test))
            sys.exit(1)
        print ("Test %s Passed" % str(e2e_test))

def bringup_test_spec_env(spec, nomodel, naples_container=None):
    test_specs = get_test_specs()
    global e2e_test
    for test_spec in test_specs:
        if test_spec["name"] == spec:
            e2e_test = E2eTest(test_spec, naples_container)
            print ("Bring up E2E environment for testspec : ",  spec)
            e2e_test.BringUp(nomodel)
            print ("E2E environment up for testspec : ",  spec)
            e2e_test.PrintEnvironmentSummary()
            try:
                while True:
                    time.sleep(10)
            except KeyboardInterrupt:
                pass
            e2e_test.Teardown()    

def setup_cfg_env(e2e_cfg, nomodel, naples_container=None):
    print ("Setting up E2E Environment for config : ",  e2e_cfg)
    env = E2eEnv(e2e_cfg)
    env.BringUp(nomodel)
    env.PrintEnvironmentSummary()
    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        pass
    env.Teardown()    
    
def cleanup():
    #Clean up environment if process is cancelled.
    if e2e_test:
        e2e_test.Teardown()

def signal_handler(signal, frame):
    print ("cleanup from signal_handler")
    cleanup()
    sys.exit(1)

def clean_up_app_containers(config_file=consts.E2E_APP_CONFIG_FILE):
    data = json.load(open(config_file))
    image = data["App"]["registry"] + data["App"]["image"]
    try:
        print ("Removing image ", image)
        retcode = subprocess.call(["docker", "rmi", image])
    except:
        pass    

def configure_naples_container():
    cfg = json.load(open(consts.E2E_CFG_FILE))
    for config_name, config_data in cfg.items():
        print ("Configuring : ", config_name)
        url = consts.AGENT_URL + config_data["api"]
        headers = {"Content-Type" : "application/json"}
        for cfg_item in config_data[config_name.lower()]:
            payload = json.dumps(cfg_item)
            r = requests.post(url, data=payload, headers=headers)
            text = json.loads(r.text)
            if (text["status-code"] != 200):
                print ("Config failed ", cfg_item) 
                print ("Error : ", text["error"])
                sys.exit(1)
        
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--e2e-mode', dest='test_mode', default="auto",
                    choices=["auto", "manual", "setup"],
                    help='E2E Test mode.')
    parser.add_argument('--e2e-spec', dest='e2e_spec', default=None,
                    help='E2E spec if running in manual mode.')    
    parser.add_argument('--e2e-cfg', dest='e2e_cfg', default=None,
                    help='E2E Configuration file if running in setup mode.')    
    parser.add_argument('--nomodel', dest='nomodel', action="store_true",
                        help='No Model mode, connect each other.')  
    parser.add_argument('--naplescontainer', dest='naples_container', default=None,
                    help='Naples Container image name')    
    args = parser.parse_args()

    os.chdir(consts.nic_e2e_dir)

    configure_naples_container()
    if args.test_mode == "auto":
        run_tests_in_auto_mode(naples_container=args.naples_container)
    elif args.test_mode == "manual":
        bringup_test_spec_env(args.e2e_spec, args.nomodel,
                               naples_container=args.naples_container)
    elif args.test_mode == "setup":
        setup_cfg_env(args.e2e_cfg, args.nomodel)
        
    clean_up_app_containers()
    return 


if __name__ == "__main__":
    signal.signal(signal.SIGTERM, signal_handler) # kill
    main()
