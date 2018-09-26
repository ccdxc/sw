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
parser.add_argument('--svcport', dest='svcport', default=55555,
                    help='IOTA Service Port.')
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
