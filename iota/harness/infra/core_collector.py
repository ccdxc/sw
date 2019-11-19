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

class CoreCollector(object):

    def __init__(self,testbed, destDir, username, password):
        self.excluded = ["minfree"]
        self.testbed = testbed
        self.username = username
        self.password = password
        self.scpPfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
        self.sshPfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
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
        return nodes

    def sshCmd(self,ip,cmd):
        sshHost = "%s@%s" % (self.username, ip)
        fullCmd = "%s %s \"%s\"" % (self.sshPfx, sshHost, cmd)
        output = subprocess.check_output(fullCmd, stderr=subprocess.PIPE, shell=True)
        return re.split('\n',output.decode("utf-8"))

    def scpGetFile(self,ip,srcFile,dstFile):
        sshHost = "%s@%s" % (self.username, ip)
        fullCmd = "%s %s:%s %s" % (self.scpPfx, sshHost, srcFile, dstFile)
        output = subprocess.check_output(fullCmd, stderr=subprocess.PIPE, shell=True)
        return re.split('\n',output.decode("utf-8"))

    def gatherCores(self):
        filelist = ""
        nodes = self.buildNodesFromTestbedFile()
        for node in nodes:
            log.debug("looking for cores on host {0}".format(node))
            corefiles = self.sshCmd(node,"sudo ls /var/crash")
            for core in corefiles:
                if len(core) < 4:
                    continue
                if core in self.excluded:
                    log.debug("skipping file {0}. in excluded list".format(core))
                    continue
                destFile = self.destDir + core
                self.sshCmd(node,"sudo mv /var/crash/{0} /var/".format(core))
                self.scpGetFile(node,"/var/{0}".format(core),destFile)
                self.sshCmd(node,"sudo rm /var/{0}".format(core))
                filelist += destFile + ' '
        return filelist


def CollectCores(testbed,destdir,username="vm",password="vm"):
    cc = CoreCollector(testbed, destdir, username, password)
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


