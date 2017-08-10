#! /usr/bin/python3

import argparse

parser = argparse.ArgumentParser(description='DOL Framework')
parser.add_argument('-n', '--no-hal', dest='no_hal',
                    action='store_true', help='No Connection to HAL')
parser.add_argument('-d', '--debug', dest='debug',
                    action='store_true', help='Enable Debug Mode')
parser.add_argument('-v', '--verbose', dest='verbose',
                    action='store_true', help='Enable Verbose Mode')
parser.add_argument('-m', '--modlist', dest='modlist',
                    default='modules.list', help='Module List File')
parser.add_argument('--config-only', dest='cfgonly',
                    action='store_true', help='Test HAL Config Only')
parser.add_argument('-s', '--standalone', dest='standalone',
                    action='store_true',
                    help='Run DOL Framework in Standalone mode.')
parser.add_argument('-t', '--topology', dest='topology',
                    default='base/base.spec',
                    help='Topology to generate Config Objects.')
parser.add_argument('-T', '--test', dest='test',
                    default=None, help='Run single TestSpec by name')
parser.add_argument('--tc', dest='tcid',
                    default=None, help='Run single Testcase by ID')

GlobalOptions = parser.parse_args()

