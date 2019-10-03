#! /usr/bin/python2
import argparse
import sys
import os
top_dir = os.path.dirname(sys.argv[0])
sys.path.insert(0,'%s' % top_dir)
sys.path.insert(0,'../../')
import harness.infra.utils.parser as par
from console import Console


parser = argparse.ArgumentParser(description='Clear console')
# Mandatory parameters
parser.add_argument('--console-ip', dest='console_ip', required = True, default=None, help='Term Server IP/Hostname')
parser.add_argument('--console-port', dest='console_port', required = True, default=None, help='Term Server Port')


GlobalOptions = parser.parse_args()

def Main():
    Console(GlobalOptions.console_ip, int(GlobalOptions.console_port))
    return True

if __name__ == '__main__':
    try:
        Main()
    except Exception as ex:
        sys.stderr.write(str(ex))
        sys.exit(1)
    sys.exit(os.EX_OK)
