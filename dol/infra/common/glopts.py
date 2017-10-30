#! /usr/bin/python3
import argparse
import pdb
import sys

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
GlobalOptions = parser.parse_args()

if GlobalOptions.topology is None or GlobalOptions.feature is None:
    print("Error: Manadatory args 'topo' and 'feature' are missing.")
    print("Usage: ./main.py --topo <topology-name> --feature <feature1,feature2...>")
    sys.exit(1)
