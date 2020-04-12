import Queue
import argparse
import json
import logging
import os
import pdb
import pprint
import re
import string
import subprocess
import sys
import threading
import time
import traceback

try: os.remove("DOCKCTL.LOG")
except: pass
logging.basicConfig(filename='DOCKCTL.LOG',level=logging.DEBUG)
log = logging.getLogger()

VALID_OPS = ["start", "stop", "restart", "check", "chkerr"]
VALID_TYPES = ["events", "fwlogs"]

BACKGROUND_MODE_DSHELL = "dshell"
BACKGROUND_MODE_ISHELL = "ishell"
BACKGROUND_MODE_NO_SHELL = "noshell"
VALID_BACKGROUND_MODES = [BACKGROUND_MODE_DSHELL, BACKGROUND_MODE_ISHELL, BACKGROUND_MODE_NO_SHELL]

class TimeoutException(Exception):
    pass

class SshClient(object):

    def __init__(self, ip, username, password):
        self.username = username
        self.password = password
        self.sshHost = "%s@%s" % (self.username, ip)
        self.scpPfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
        self.sshPfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password

    def sshCmd(self,cmd,verbose=False,timeout=30):
        fullCmd = "%s %s %s" % (self.sshPfx, self.sshHost, cmd)
        output = ''
        runtime = 0
        if verbose:
            print("running ssh command: {0}".format(fullCmd))
        try:
            proc = subprocess.Popen(fullCmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            while proc.poll() == None:
                time.sleep(1)
                runtime += 1
                if runtime > timeout:
                    proc.terminate()
                    raise TimeoutException()
            output = proc.communicate()[0]
        except subprocess.CalledProcessError as e:
            if re.search("grep ", cmd) and e.returncode == 1:
                pass
            else:
                raise
        except:
            output = traceback.format_exc()
        output = re.split('\n',output.decode("utf-8"))
        if verbose:
            print("output was: {0}".format(output))
        return output

    def scpGetFile(self,srcFile,dstFile):
        fullCmd = "%s %s:%s %s" % (self.scpPfx, self.sshHost, srcFile, dstFile)
        output = subprocess.check_output(re.split('\s', fullCmd), stderr=subprocess.PIPE)
        return re.split('\n',output.decode("utf-8"))


class DictToObj(dict):
    __getattr__ = dict.__getitem__
    __setattr__ = dict.__setitem__


def dockerCmd(cmd, background, ips, varStr=None, checkOutput=None, verbose=False, count=0, timeout=30, offset=0):
    if count == 0:
        raise ValueError("sim count cannot be 0")
    if background not in VALID_BACKGROUND_MODES:
        raise ValueError("background mode must be one of the following: {0}".format(VALID_BACKGROUND_MODES))
    if varStr not in ["", None]:
        varStr = " -e " + varStr
    else:
        varStr = ""
    output = []
    threads = []
    for ip in ips:
        queue = Queue.Queue()
        _thread = threading.Thread(target=runDockerCmds, args=(queue, ip, cmd, count, background, log, varStr, checkOutput, verbose, timeout, offset))
        _thread.queue = queue
        _thread.setDaemon = True
        _thread.start()
        threads.append(_thread)
    for _thread in threads:
        _thread.join()
        output.append(_thread.queue.get())
    return output

def runDockerCmds(queue, ip, cmd, count, background, log, varStr=None, checkOutput=None, verbose=None, timeout=30, offset=0):
    if background not in VALID_BACKGROUND_MODES:
        raise ValueError("background mode must be one of the following: {0}".format(VALID_BACKGROUND_MODES))
    returnData = {}
    name = "NA"
    log.debug("{0} {1}".format(ip, cmd))
    try:
        ssh = SshClient(ip,username="root",password="docker")
        if checkOutput:
            coRe = re.compile('('+checkOutput+')')
        finalOutput = "#"*40 + "\nhost: " + ip + "\nmax sim count: " + str(count) + "\n" + "#"*40 + "\n"
        docks = ssh.sshCmd('docker ps',verbose,timeout)
        docks = docks[1:-1]
        tc = len(docks)
        if offset > 0:
            if (offset + count) > tc:
                print("{0} + {1} will go past max len of container list {2}".format(offset, count, tc))
            docks = docks[offset:]
            tc = len(docks)
        fc = 0
        nfc = 0
        for dockNum, dock in enumerate(docks):
            if dockNum >= count:
                print("reached max sim count of {0} for host {1}, skipping remaining sims on this host".format(count, ip))
                break
            name = re.search('^([\S]+)',dock)
            if name and len(name.group(1)) > 3:
                name = name.group(1)
                if background == BACKGROUND_MODE_DSHELL:
                    fullcmd = "'docker exec -d " + varStr + " " + name + " sh -c \"" + cmd + "\"'"
                elif background == BACKGROUND_MODE_ISHELL:
                    fullcmd = "'docker exec -i " + varStr + " " + name + " sh -c \"" + cmd + "\"'"
                elif background == BACKGROUND_MODE_NO_SHELL:
                    fullcmd = "'docker exec -i " + varStr + " " + name + " \"" + cmd + "\"'"
                resp = ssh.sshCmd( fullcmd, verbose, timeout)
                key = ip + "_" + name
                if resp == [""]:
                    returnData[key] = None
                else:
                    returnData[key] = resp
                if verbose: 
                    print(resp)
                log.debug("finished command: {0} {1} {2}".format(ip, name, resp))
                if checkOutput:
                    found= coRe.search(str(resp))
                    if found:
                        fc += 1
                        if verbose:
                            print("{0}/{1} FOUND ON ip:{2}, dckr:{3}, out:{4}".format(dockNum+1,tc,ip,name,found.group(1)))
                    else:
                        nfc += 1
                        if verbose:
                            print("{0}/{1} FAILED TO FIND ON ip:{2}, dckr:{3}, search:{4}".format(dockNum+1,tc,ip,name,checkOutput))
        if checkOutput:
            finalOutput += "ip:{0}, found:{1}, missed:{2}".format(ip, fc, nfc)
        print(finalOutput)
    except:
        returnData[name] = traceback.format_exc()
        print(returnData[name])
    queue.put(returnData)

def startEvents(ips, args, restart=False):
    if restart:
        print("="*30 + "\nstopping all operations\n" + "="*30 + "\n")
        stopEvents(ips, args)
        print("="*30 + "\nsleeping 5 seconds after stopping operations\n" + "="*30)
        time.sleep(5)
    cmd = "\"/naples/nic/bin/gen_events -r {0} -t 2000000 -s scaletesting > /tmp/events.stdout &\"".format(args.rate)
    dockerCmd(cmd, BACKGROUND_MODE_DSHELL, ips, 
              varStr="LD_LIBRARY_PATH=/naples/nic/lib/:/naples/nic/lib64/", 
              checkOutput=None, verbose=args.verbose, count=args.simCount, offset=args.offset)
    print("="*30 + "\nchecking operations\n" + "="*30)
    checkEvents(ips, args)

def checkEvents(ips, args):
    cmd = "ps ax" #and set checkOutput to "gen_events"
    dockerCmd(cmd, BACKGROUND_MODE_NO_SHELL, ips, varStr=None, checkOutput="gen_events", verbose=args.verbose, count=args.simCount, offset=args.offset)

def stopEvents(ips, args):
    cmd = "pkill gen_events || true"
    dockerCmd(cmd, BACKGROUND_MODE_NO_SHELL, ips, varStr=None, checkOutput=None, verbose=args.verbose, count=args.simCount, offset=args.offset)

def startFwLogs(ips, args, restart=False):
    if restart:
        print("="*30 + "\nstopping all fwlogs\n" + "="*30 + "\n")
        stopFwLogs(ips, args)
        print("="*30 + "\nsleeping 5 seconds after stopping fwlogs\n" + "="*30)
        time.sleep(5)
    cmd = "\"/naples/nic/bin/fwloggen -rate {0} -num 5000000 > /tmp/fwlogs.stdout &\"".format(args.rate)
    dockerCmd(cmd, BACKGROUND_MODE_NO_SHELL, ips,
              varStr="LD_LIBRARY_PATH=/naples/nic/lib/:/naples/nic/lib64/", 
              checkOutput=None, verbose=args.verbose, count=args.simCount, offset=args.offset)
    print("="*30 + "\nchecking operations\n" + "="*30)
    checkFwLogs(ips, args)

def checkFwLogs(ips, args):
    cmd = "ps ax" #and set checkOutput to "gen_events"
    dockerCmd(cmd, BACKGROUND_MODE_ISHELL, ips, varStr=None, checkOutput="fwloggen", verbose=args.verbose, count=args.simCount, offset=args.offset)

def stopFwLogs(ips, args):
    cmd = "pkill fwloggen || true"
    dockerCmd(cmd, BACKGROUND_MODE_ISHELL, ips, varStr=None, checkOutput=None, verbose=args.verbose, count=args.simCount, offset=args.offset)

def runCmd(ips, args):
    return dockerCmd(args.cmd, BACKGROUND_MODE_NO_SHELL, ips, varStr=None, checkOutput=None, verbose=args.verbose, count=args.simCount, offset=args.offset)

def getHealth(ips, args):
    memRe = re.compile("Mem:[\s]+([\S]+)[\s]+[\S]+[\s]+([\S]+)[\s]+", re.M)
    returnData = {}
    for ip in ips:
        returnData[ip] = {}
        ssh = SshClient(ip,username="root",password="docker")
        mem = ssh.sshCmd("free",args.verbose)
        mem = string.join(mem)
        found = memRe.search(mem)
        if not found:
            print("failed to find free memory in output: {0}".format(mem))
            sys.exit(1)
        totalMem = int(found.group(1))
        freeMem = int(found.group(2))
        returnData[ip]['totalMem'] = totalMem
        returnData[ip]['freeMem'] = freeMem
        returnData[ip]['containerCount'] = ssh.sshCmd("docker ps --format '{{json .}}' | wc -l")[0]
    return returnData

def rebootHosts(ips, args):
    for ip in ips:
        ssh = SshClient(ip,username="root",password="docker")
        ssh.sshCmd("reboot || true &",args.verbose)

def checkFwLogsErrors(ips, args):
    cmd = 'grep -E -- "could not put object|dropping, bucket" /var/log/pensando/pen-tmagent.log'
    data = dockerCmd(cmd, BACKGROUND_MODE_NO_SHELL, ips, varStr=None, checkOutput=None, verbose=args.verbose, count=args.simCount, timeout=30, offset=args.offset)
    print("CHECKING /var/log/pensando/pen-tmagent.log")
    for docker in data:
        for cName,cValue, in docker.items():
            if cValue:
                print("container:{0}, error:{1}".format(cName, cValue))
    cmd = 'grep "failed to " /tmp/fwlogs.stdout'
    data = dockerCmd(cmd, BACKGROUND_MODE_NO_SHELL, ips, varStr=None, checkOutput=None, verbose=args.verbose, count=args.simCount, timeout=30, offset=args.offset)
    print("CHECKING /tmp/fwlogs.stdout")
    for docker in data:
        for cName,cValue, in docker.items():
            if cValue:
                print("container:{0}, error:{1}".format(cName, cValue))

def flapMgmtLink(ips, args, linkDown=True, linkUp=True):
    downCmd = "ifconfig eth0 down && ifconfig eth1 down"
    upCmd = "ifconfig eth0 up && ifconfig eth1 up && route add default gw 172.17.0.1"
    if linkDown:
        dockerCmd(downCmd, BACKGROUND_MODE_ISHELL, ips, varStr=None, checkOutput=None, verbose=args.verbose, count=args.simCount, offset=args.offset)
    time.sleep(args.mgmtFlapDelay)
    if linkUp:
        dockerCmd(upCmd, BACKGROUND_MODE_ISHELL, ips, varStr=None, checkOutput=None, verbose=args.verbose, count=args.simCount, offset=args.offset)


if __name__ == "__main__":
    defaultIps = []
    if not defaultIps:
        print("update defaultIps with the IPs of your sim servers")
        sys.exit(1)
    parser = argparse.ArgumentParser(description='sim control')
    parser.add_argument('--ips', dest='ips', default=defaultIps,
                        nargs='+', help='sim server ips')
    parser.add_argument('--sim-count', dest='simCount', default=0,
                        type=int, help='max sim count')
    parser.add_argument('--op', dest='op', default=None,
                        help='which op (start | stop | restart | chkerr | check)')
    parser.add_argument('--type', dest='type', default=None,
                        help='which type (fwlogs | events)')
    parser.add_argument('--cmd', dest='cmd', default=None,
                        help='run a command on all containers')
    parser.add_argument('--reboot', dest='reboot', action="store_true",
                        help='reboot sim servers')
    parser.add_argument('--health', dest='health', action="store_true",
                        help='prints health stats')
    parser.add_argument('--rate', dest='rate', default=None,
                        type=int, help='rate per second')
    parser.add_argument('--offset', dest='offset', default=0,
                        type=int, help='starting sim offset on each server')
    parser.add_argument('--mgmt-flap-delay', dest='mgmtFlapDelay', default=0,
                        type=int, help='link flap mgmt ports on sim')
    parser.add_argument('--mgmt-link-down', dest='mgmtLinkDown', action="store_true",
                        help='force mgmt link down')
    parser.add_argument('--mgmt-link-up', dest='mgmtLinkUp', action="store_true",
                        help='force mgmt link up')
    parser.add_argument('--verbose', dest='verbose', action="store_true",
                        help='print everything')
    _args = parser.parse_args()
    for ip in _args.ips:
        if ip not in defaultIps:
            print("invalid ip {0}. must be one of {1}".format(ip, defaultIps))
            sys.exit(1)
    if _args.op:
        if _args.op not in VALID_OPS:
            print("invalid op. must be one of {0}".format(VALID_OPS))
            sys.exit(1)
        if not _args.rate and _args.op == "start":
            print("must specify --rate when using --op start")
            sys.exit(1)
    if _args.type and _args.type not in VALID_TYPES:
        print("invalid type. must be one of {0}".format(VALID_TYPES))
        sys.exit(1)
    if _args.op and not _args.type:
        print("user must specify --type")
        sys.exit(1)

    if _args.cmd:
        print(runCmd(_args.ips, _args))
    elif _args.health:
        pprint.PrettyPrinter().pprint(getHealth(_args.ips, _args))
    elif _args.reboot:
        print(rebootHosts(_args.ips, _args))
    elif _args.mgmtFlapDelay:
        flapMgmtLink(_args.ips, _args)
    elif _args.mgmtLinkDown:
        flapMgmtLink(_args.ips, _args, linkUp=False)
    elif _args.mgmtLinkUp:
        flapMgmtLink(_args.ips, _args, linkDown=False)
    elif _args.op == "start":
        if _args.type == "events":
            startEvents(_args.ips, _args)
        elif _args.type == "fwlogs":
            startFwLogs(_args.ips, _args)
    elif _args.op == "stop":
        if _args.type == "events":
            stopEvents(_args.ips, _args)
        elif _args.type == "fwlogs":
            stopFwLogs(_args.ips, _args)
    elif _args.op == "check":
        if _args.type == "events":
            checkEvents(_args.ips, _args)
        elif _args.type == "fwlogs":
            checkFwLogs(_args.ips, _args)
    elif _args.op == "chkerr":
        #if _args.type == "events":
        #    checkEvents(_args.ips, _args)
        if _args.type == "fwlogs":
            checkFwLogsErrors(_args.ips, _args)
    elif _args.op == "restart":
        if _args.type == "events":
            startEvents(_args.ips, _args, restart=True)
        elif _args.type == "fwlogs":
            startFwLogs(_args.ips, _args, restart=True)
    else:
        parser.print_help()
