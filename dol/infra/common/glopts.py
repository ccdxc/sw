#! /usr/bin/python3
import argparse
import pdb

parser = argparse.ArgumentParser(description='DOL Framework')
parser.add_argument('--nohal', dest='no_hal',
                    action='store_true', help='No Connection to HAL')
parser.add_argument('--debug', dest='debug',
                    action='store_true', help='Enable Debug Mode')
parser.add_argument('--verbose', dest='verbose',
                    action='store_true', help='Enable Verbose Mode')
parser.add_argument('--modlist', dest='modlist',
                    default=None, help='Module List File')
parser.add_argument('--pkglist', dest='pkglist', default=None, 
                    help='Run only the tests in package list')
parser.add_argument('--config-only', dest='cfgonly',
                    action='store_true', help='Test HAL Config Only')
parser.add_argument('--dryrun', dest='dryrun', action='store_true',
                    help='Dry-Run mode. (No communication with HAL & Model)')
parser.add_argument('--topo', dest='topology', default='base',
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
GlobalOptions = parser.parse_args()
