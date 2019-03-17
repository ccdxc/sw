#! /usr/bin/python3
import argparse
import sys
import os
top_dir = os.path.dirname(sys.argv[0])
sys.path.insert(0,'%s' % top_dir)
sys.path.insert(0,'../../')
import harness.infra.utils.parser as par
from server import Server

parser = argparse.ArgumentParser(description='Console')
# Mandatory parameters
parser.add_argument('--hostname', dest='hostname', required = True, default=None, help='Term Server IP/Hostname')
parser.add_argument('--command', dest='command', required = True, default=None, help='command to execute')


GlobalOptions = parser.parse_args()

def Main():
    host = Server(GlobalOptions.hostname)
    host.RunSshCmd(GlobalOptions.command)
    return True

if __name__ == '__main__':
    try: 
        Main()
    except Exception as ex:
        sys.stderr.write(str(ex))
        sys.exit(1)
    sys.exit(os.EX_OK)
