#! /usr/bin/python3
import argparse
import sys
import os
top_dir = os.path.dirname(sys.argv[0])
sys.path.insert(0,'%s' % top_dir)
sys.path.insert(0,'../../')
import harness.infra.utils.parser as par
from nic import Naples

parser = argparse.ArgumentParser(description='Console')
# Mandatory parameters
parser.add_argument('--console-ip', dest='console_ip', required = True, default=None, help='Term Server IP/Hostname')
parser.add_argument('--console-port', dest='console_port', required = True, default=None, help='Term Server Port')
parser.add_argument('--enable-oob', dest='enable_oob', required = False, default=False, help='Enable OOB and get ip address')
parser.add_argument('--print-frequency', dest='print_frequency', required = False, default=False, help='print frequency file')
parser.add_argument('--check-frequency-file', dest='check_frequency_settings', required = False, default=True, help='print frequency file')


GlobalOptions = parser.parse_args()

def IsFrequencyFilePresent(naples):
    op = naples.RunCmdGetOp("ls -l /sysconfig/config0/ | grep frequency.json | wc -l")
    if int(op.split('\n')[1]) == 1:
        return "[%s] Frequency Settings Enabled" % str(naples)
    else:
        return "[%s] Frequency Settings Not Enabled" % str(naples)

def PrintFrequencyFile(naples):
    return naples.RunCmd("cat /sysconfig/config0/frequency.json")

def GetNaplesOobIp(naples):
    return naples.GetOobIp()

def Main():
    naples = Naples(GlobalOptions.console_ip, int(GlobalOptions.console_port))
    if GlobalOptions.enable_oob:
        print (GetNaplesOobIp(naples))
    
    if GlobalOptions.print_frequency:
        print (PrintFrequencyFile(naples))
    
    if GlobalOptions.check_frequency_settings:
        print (IsFrequencyFilePresent(naples))
    
    return True

if __name__ == '__main__':
    try: 
        Main()
    except Exception as ex:
        sys.stderr.write(str(ex))
        sys.exit(1)
    sys.exit(os.EX_OK)
