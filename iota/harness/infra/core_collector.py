import argparse
import json
import logging
import os
import pdb
import re
import subprocess
import sys
import traceback

log = logging.getLogger()

winCoreDirs = [
    "c:\\windows\\",
    "c:\\windows\system32\\",
]
winCoreFileREs = [
    "MEMORY.DMP",
]

linuxCoreDirs = [
    "/var/crash/",
    "/var/core/",
]
linuxCoreFileREs = [
    "core.*",
]


class CoreCollector(object):

    def __init__(self,testbed, destDir, username, password, log=None):
        if not log:
            log = logging.getLogger()
        self.log = log
        self.excluded = ["minfree"]
        self.testbed = testbed
        self.username = username
        self.password = password
        self.scpPfx = "timeout 300 sshpass -p %s scp -o ConnectTimeout=60 -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
        self.sshPfx = "timeout 300 sshpass -p %s ssh -o ConnectTimeout=60 -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
        if destDir == "/":
            raise ValueError("destDir cannot be / directory")
        if destDir == "":
            destDir = "./"
        if not destDir.endswith("/"):
                destDir += "/"
        self.destDir = destDir
        if not os.path.exists(destDir):
            os.makedirs(destDir)
 
    def buildNodesFromTestbedFile(self):
        nodes=[]
        try:
            with open(self.testbed,"r") as topoFile:
                data=json.load(topoFile)
                for inst in data["Instances"]:
                    if inst.get("Type","") == "bm" and "NodeMgmtIP" in inst and "Name" in inst:
                        nodes.append(inst["NodeMgmtIP"])
        except:
            msg="failed to build nodes from testbed file. error was:"
            msg+=traceback.format_exc()
            print(msg)
        try:
            osType = data["Provision"]["Vars"]["BmOs"]
        except:
            osType = "linux"
        return nodes, osType

    def sshCmd(self,ip,cmd,verbose=False):
        sshHost = "%s@%s" % (self.username, ip)
        fullCmd = "%s %s \"%s\"" % (self.sshPfx, sshHost, cmd)
        if verbose:
            self.log.debug("sending ssh command: {0}".format(fullCmd))
        output = subprocess.check_output(fullCmd, stderr=subprocess.PIPE, shell=True)
        if verbose:
            self.log.debug("output of command was: {0}".format(output))
        return re.split('\n',output.decode("utf-8"))

    def scpGetFile(self,ip,srcFile,dstFile):
        sshHost = "%s@%s" % (self.username, ip)
        fullCmd = "%s %s:%s %s" % (self.scpPfx, sshHost, srcFile, dstFile)
        output = subprocess.check_output(fullCmd, stderr=subprocess.PIPE, shell=True)
        return re.split('\n',output.decode("utf-8"))

    def gatherCores(self):
        maxCores = 5
        filelist = ""
        nodes,osType = self.buildNodesFromTestbedFile()
        cfREs = []
        if osType == "windows":
            coreDirs = winCoreDirs
            for cfre in winCoreFileREs:
                cfREs.append(re.compile('(' + cfre + ')'))
        else:
            coreDirs = linuxCoreDirs
            for cfre in linuxCoreFileREs:
                cfREs.append(re.compile('(' + cfre + ')'))
        for _dir in coreDirs:
            for node in nodes:
                coreCount = 0
                self.log.debug("looking for cores in dir {0} on host {1}".format(_dir, node))
                try:
                    corefiles = self.sshCmd(node,"sudo find {0} -maxdepth 1 -type f".format(_dir),verbose=True)
                    corefiles = [re.sub(_dir,"",c) for c in corefiles]
                    self.log.debug("found raw corefile list: {0}".format(corefiles))
                except subprocess.CalledProcessError as e:
                    if e.returncode == 1:
                        self.log.debug("directory {0} does not exist on host {1}".format(_dir, node))
                        continue
                    else:
                        self.log.debug("failed to search directory {0} on node {1}. return code was: {2}".format(_dir, node, e.returncode))
                        continue
                except:
                    self.log.debug("failed to search directory {0} on node {1}. error was: {2}".format(_dir, node, traceback.format_exc()))
                    continue
                for core in corefiles:
                    if core in self.excluded:
                        self.log.debug("skipping file {0}. in excluded list".format(core))
                        continue
                    if coreCount >= maxCores:
                        self.log.debug("skipping file {0}. max core count of {1} reached for node {2} in directory {3}".format(core,maxCores,node,_dir)) 
                        continue
                    for cfre in cfREs:
                        if cfre.search(core):
                            self.log.debug("matched re {0} to corefile {1}".format(cfre, core))
                            break
                    else:
                        continue
                    coreCount += 1
                    destFile = self.destDir + core
                    try:
                        self.log.debug("processing core file {0}".format(core))
                        if osType == "windows":
                            self.scpGetFile(node,"{0}\{1}".format(_dir, core), destFile)
                            self.sshCmd(node,"sudo rm {0}\{1}".format(_dir, core))
                        else:
                            self.sshCmd(node,"sudo mv {0}/{1} /tmp".format(_dir,core))
                            self.sshCmd(node,"sudo chown vm:vm /tmp/{0}".format(core))
                            self.scpGetFile(node,"/tmp/{0}".format(core),destFile)
                            self.sshCmd(node,"sudo rm /tmp/{0}".format(core))
                        filelist += destFile + ' '
                    except:
                        self.log.debug("failed to scp core file. error was: {0}".format(traceback.format_exc()))
        return filelist


def CollectCores(testbed,destdir,username="vm",password="vm",log=None):
    cc = CoreCollector(testbed, destdir, username, password, log)
    filelist = cc.gatherCores()
    if filelist:
        tgzFile = destdir+".tgz"
        print("found cores[{0}]. creating {1}".format(filelist,tgzFile))
        subprocess.check_call("tar -czvf {0} {1}".format(tgzFile,filelist), shell=True)
        subprocess.check_call("rm -f {0}".format(filelist), shell=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='core collector')
    parser.add_argument('-t', '--testbed', dest='testbed',
                        required=True, help='json testbed')
    parser.add_argument('-d', '--destdir', dest='destdir',
                        required=True, help='local dest directory for cores')
    parser.add_argument('-u', '--username', dest='username',
                        default="vm", help='host username')
    parser.add_argument('-p', '--password', dest='password',
                        default="vm", help='host password')
    args = parser.parse_args()
    cc = CollectCores(args.testbed, args.destdir, args.username, args.password)


