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
parser.add_argument('--coverage', dest='coverage',
                    action='store_true', help='Collect code coverage data.')
parser.add_argument('--svcport', dest='svcport', default=60000,
                    help='IOTA Service Port.')
parser.add_argument('--testbed-json', dest='testbed_json', default="/warmd.json",
                    help='Testbed JSON file')
parser.add_argument('--no-keep-going', dest='no_keep_going',
                    action='store_true', help='Stop on first error.')
parser.add_argument('--mode', dest='mode', default=None,
                    choices=["venice", "netagent"],
                    help='Test mode - Venice / Agent level tests.')
parser.add_argument('--pipeline', dest='pipeline', default="iris",
                    help='Filter tests by Pipeline')
parser.add_argument('--rerun', dest='rerun',
                    action='store_true', help='Re-run mode, Skips copy and few init steps.')
GlobalOptions = parser.parse_args()

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
