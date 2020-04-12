import Queue
import argparse
import ipaddress
import json
import pdb
import pprint
import re
import string
import subprocess
import sys
import threading
import time
import traceback

class SshClient(object):

    def __init__(self, ip, username, password):
        self.username = username
        self.password = password
        self.sshHost = "%s@%s" % (self.username, ip)
        self.scpPfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
        self.sshPfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password

    def sshCmd(self,cmd,verbose=False):
        fullCmd = "%s %s %s" % (self.sshPfx, self.sshHost, cmd)
        output = ''
        if verbose:
            print("running ssh command: {0}".format(fullCmd))
        try:
            output = subprocess.check_output(fullCmd, shell=True, stderr=subprocess.PIPE)
        except subprocess.CalledProcessError as e:
            if re.search("grep ", cmd) and e.returncode == 1:
                pass
            else:
                raise
        output = re.split('\n',output.decode("utf-8"))
        if verbose:
            print("output was: {0}".format(output))
        return output

    def scpGetFile(self,srcFile,dstFile,verbose=False):
        fullCmd = "%s %s:%s %s" % (self.scpPfx, self.sshHost, srcFile, dstFile)
        if verbose: 
            print("scp get cmd: {0}".format(fullCmd))
        output = subprocess.check_output(fullCmd, shell=True, stderr=subprocess.PIPE)
        return re.split('\n',output.decode("utf-8"))

    def scpPutFile(self,srcFile,dstFile,verbose=False):
        fullCmd = "%s %s %s:%s" % (self.scpPfx, srcFile, self.sshHost, dstFile)
        if verbose: 
            print("scp put cmd: {0}".format(fullCmd))
        output = subprocess.check_output(fullCmd, shell=True, stderr=subprocess.PIPE)
        return re.split('\n',output.decode("utf-8"))



class DictToObj(dict):
    __getattr__ = dict.__getitem__
    __setattr__ = dict.__setitem__

def runCmd(ips, args):
    return veniceCmd(ips, args.cmd, varStr=None, checkOutput=None, verbose=args.verbose)

def veniceCmd(ips, cmd, varStr=None, checkOutput=None, verbose=False):
    if varStr not in ["", None]:
        varStr = " -e " + varStr
    else:
        varStr = ""
    output = {}
    threads = []
    for ip in ips:
        queue = Queue.Queue()
        _thread = threading.Thread(target=runVeniceCmds, args=(queue, ip, cmd, varStr, checkOutput, verbose))
        _thread.queue = queue
        _thread.setDaemon = True
        _thread.start()
        _thread.ip = ip
        threads.append(_thread)
    for _thread in threads:
        _thread.join()
        output[_thread.ip] = _thread.queue.get()
    return output

def runVeniceCmds(queue, ip, cmd, varStr=None, checkOutput=None, verbose=None):
    returnData = None
    try:
        ssh = SshClient(ip, username="vm",password="vm")
        if checkOutput:
            coRe = re.compile('('+checkOutput+')')
        resp = ssh.sshCmd(cmd, verbose)
        if resp == [""]:
            returnData = None
        else:
            returnData = resp
        if verbose: 
            print(resp)
        if checkOutput:
            found= coRe.search(str(resp))
            if found:
                if verbose:
                    print("FOUND ON ip:{0}, out:{1}".format(ip,found.group(1)))
            else:
                if verbose:
                    print("FAILED TO FIND ON ip:{0}".format(ip))
    except:
        returnData = traceback.format_exc()
    queue.put(returnData)

def rebootVenice(ips, verbose=False):
    for ip in ips:
        ssh = SshClient(ip, username="vm",password="vm")
        ssh.sshCmd("sudo reboot || true &", verbose)

def findNpmNode(ips, verbose=False):
    cmd = "ps ax | grep \"pen-npm\" | grep -v \"ps ax\""
    output = veniceCmd(ips,cmd)
    for node,data in output.items():
        if data:
            return node
    return None

def findApiNode(ips, verbose=False):
    cmd = "ps ax | grep \"apiserver\" | grep -v \"ps ax\""
    output = veniceCmd(ips,cmd)
    for node,data in output.items():
        if data:
            return node
    return None

def findLeaderNode(isoIps, verbose=False):
    #how to find leader 
    raise Exception("not ready yet")

def isolateNpmNode(isoIps, complete=False, verbose=False):
    node = findNpmNode(isoIps, verbose)
    if not node:
        raise Exception("failed to find npm node in ip list {0}".format(isoIps))
    isolateVenice(node, isoIps, complete, verbose)

def isolateApiNode(isoIps, complete=False, verbose=False):
    node = findApiNode(isoIps, verbose)
    if not node:
        raise Exception("failed to find api node in ip list {0}".format(isoIps))
    isolateVenice(node, isoIps, complete, verbose)

def isolateLeaderNode(isoIps, complete=False, verbose=False):
    node = findLeaderNode(isoIps, verbose)
    if not node:
        raise Exception("failed to find leader node in ip list {0}".format(isoIps))
    isolateVenice(node, isoIps, complete, verbose)

def isolateVenice(ip, isoIps, complete=False, verbose=False):
    ssh = SshClient(ip, username="vm",password="vm")
    veniceIps.remove(ip)
    for isoIp in isoIps:
        cmd = "sudo iptables -A INPUT -s {0} -j DROP".format(isoIp)
        try:
            ssh.sshCmd(cmd, verbose)
        except:
            pass
    if complete:
        for isoIp in isoIps:
            ssh = SshClient(isoIp, username="vm",password="vm")
            cmd = "sudo iptables -A INPUT -s {0} -j DROP".format(ip)
            try:
                ssh.sshCmd(cmd, verbose)
            except:
                pass
    checkFirewall(ip, isoIps, False)
    print("isolated {0}".format(ip))

def reconnectVenice(ip, isoIps, complete=False, verbose=False):
    ssh = SshClient(ip, username="vm",password="vm")
    veniceIps.remove(ip)
    for isoIp in isoIps:
        cmd = "sudo iptables -D INPUT -s {0} -j DROP".format(isoIp)
        try:
            ssh.sshCmd(cmd, verbose)
        except:
            pass
    #if complete:  #always do this for reconnect.
    for isoIp in isoIps:
        ssh = SshClient(isoIp, username="vm",password="vm")
        cmd = "sudo iptables -D INPUT -s {0} -j DROP".format(ip)
        try:
            ssh.sshCmd(cmd, verbose)
        except:
            pass
    checkFirewall(ip, isoIps, True)
    print("reconnected {0}".format(ip))

def getFirewallRules(ip, knownIps, verbose=False):
    ssh = SshClient(ip, username="vm",password="vm")
    output = ssh.sshCmd("sudo iptables --list", verbose)
    ipRuleRe = re.compile("^([\S]+)[\s]+[\S]+[\s]+--[\s]+([\S]+)[\s]+[\S]+")
    ipRules=[]
    for line in output:
        found = ipRuleRe.search(line)
        if found and found.group(1) in ["ALLOW", "DROP", "REJECT"]:
            if found.group(2) in knownIps:
                ipRules.append(found.groups())
    return ipRules

def checkFirewall(ip, isoIps, allow=True, verbose=False):
    ssh = SshClient(ip, username="vm",password="vm")
    for isoIp in isoIps:
        if re.search("/",isoIp):
            isoIp = list(ipaddress.IPv4Network(unicode(isoIp)).hosts())[0].exploded
        try:
            print('pinging host {0}'.format(isoIp))
            ssh.sshCmd('ping {0} -c 5'.format(isoIp), verbose)
        except:
            if allow:
                raise Exception("venice {0} failed to ping target {1}".format(ip, isoIp))
        else:
            if not allow:
                raise Exception("isolated venice {0} was allowed to ping target {1}".format(ip, isoIp))

def killProc(ips, procName, verbose=False):
    cmd = "ps ax | grep \"{0}\" | grep -v \"ps ax\"".format(procName)
    print(cmd)
    output = veniceCmd(ips, cmd, verbose=verbose)
    for ip,data in output.items():
        if data:
            ssh = SshClient(ip, username="vm",password="vm")
            if verbose:
                print("killing proc {0} on node {1}".format(procName, ip))
            ssh.sshCmd('pkill {0}'.format(procName))

def getImageInfo(ips, imgName, verbose=False):
    if not isinstance(ips, list):
        raise ValueError("ips must be list")
    data={}
    cmd1 = "docker images | grep {0}".format(imgName)
    for ip in ips:
        data[ip] = {}
        if ip == "local":
            try:
                output1 = subprocess.check_output(cmd1, shell=True)
            except:
                if verbose:
                    print("failed to find image name {0} in local docker images".format(imgName))
                continue
        else:
            ssh = SshClient(ip, username="vm",password="vm")
            output1 = ssh.sshCmd(cmd1,verbose)
            output1 = output1[0]
        found = re.search("[\S]+[\s]+([\S]+)[\s]+([\S]+)[\s]+([\d]+ [\w]+) ago",output1)
        if not found:
            if verbose:
                print("failed to find image tag / id for {0} in output:\n\"{1}\"".format(imgName,output1))
            continue
        imgTag = found.group(1)
        imgId = found.group(2)
        imgTs = found.group(3)
        cmd2 = "docker ps | grep {0}".format(imgId)
        if ip == "local":
            data[ip]['imgTag'] = imgTag
            data[ip]['imgId'] = imgId
            return data
        output2 = ssh.sshCmd(cmd2,verbose)
        output2 = output2[0]
        found = re.search("([\S]+)[\s]+"+imgId,output2)
        if not found:
            if verbose:
                print("failed to find {0} image on {1}".format(imgName,ip))
            continue
        data[ip]['imgTag'] = imgTag
        data[ip]['imgId'] = imgId
        data[ip]['imgTs'] = imgTs
        data[ip]['dockId'] = found.group(1)
    return data

def patchImage(ips, imgName, verbose=False, containerCheck=False):
    if not isinstance(ips, list):
        raise ValueError("ips must be list")
    if containerCheck:
        #verify running in container
        output = subprocess.check_output("cat /proc/1/cgroup", shell=True)
        if re.search("^1:name=systemd:/\n",output,re.M):
            raise Exception("this operation must be run from inside container")
    #get local imgId and tag.
    localData = getImageInfo(["local"], imgName)
    if localData["local"] == {}:
        raise ValueError("failed to find local docker image info for {0}".format(imgName))
    localImgId = localData["local"]["imgId"]
    if verbose:
        print("local docker info: {0}".format(localData))
    remoteData = getImageInfo(ips, imgName, verbose)
    if verbose:
        print("remote docker info: {0}".format(remoteData))
    imgTag = ""
    dockerId = ""
    remoteImageId = ""
    for ip,data in remoteData.items():
        if not data == {}:
            imgTag = data["imgTag"]
            dockerId = data["dockId"]
            remoteImgId = data["imgId"]
            break
    else:
        raise ValueError("failed to find docker image data")
    for ip in ips:
        ssh = SshClient(ip, username="vm",password="vm")
        #local tag image and save tar
        cmd = "docker tag {0} {1}:{2}".format(localImgId, imgName, imgTag)
        if verbose:
            print("tagging image with command: {0}".format(cmd))
        subprocess.check_call(cmd, shell=True)
        imgTar = "{0}_new.tar".format(imgName)
        cmd = "docker save -o {0} {1}:{2}".format(imgTar, imgName, imgTag)
        if verbose:
            print("saving image with command: {0}".format(cmd))
        subprocess.check_call(cmd, shell=True)
        #upload image to venice
        ssh.scpPutFile(imgTar,imgTar,verbose)
        if remoteData[ip] != {}:
            #go to venice and rmi imgId, kill docker, load tar
            ssh.sshCmd("docker kill {0}; docker rmi -f {1}; docker load -i {2}".format(dockerId, remoteImgId, imgTar), verbose)
            #toggle imagePullPolicy:IfNotPresent->Always->IfNotPresent
            ssh.sshCmd("\"sudo /usr/pensando/bin/kubectl --server=https://" + ip + ":6443 --client-certificate=/var/lib/pensando/pki/kubernetes/apiserver-client/cert.pem --client-key=/var/lib/pensando/pki/kubernetes/apiserver-client/key.pem --insecure-skip-tls-verify=true get cronjob pen-elastic-curator -o yaml > out.yaml\"",verbose)
            #update out.yaml, imagePullPolicy:IfNotPresent to imagePullPolicy:Always
            ssh.sshCmd("\"sed -i -e 's/imagePullPolicy: IfNotPresent/imagePullPolicy: Always/g' out.yaml\"",verbose)
            ssh.sshCmd("\"sudo /usr/pensando/bin/kubectl --server=https://" + ip + ":6443 --client-certificate=/var/lib/pensando/pki/kubernetes/apiserver-client/cert.pem --client-key=/var/lib/pensando/pki/kubernetes/apiserver-client/key.pem --insecure-skip-tls-verify=true apply -f out.yaml\"",verbose)
            ssh.sshCmd("\"sudo /usr/pensando/bin/kubectl --server=https://" + ip + ":6443 --client-certificate=/var/lib/pensando/pki/kubernetes/apiserver-client/cert.pem --client-key=/var/lib/pensando/pki/kubernetes/apiserver-client/key.pem --insecure-skip-tls-verify=true get cronjob pen-elastic-curator -o yaml > out.yaml\"",verbose)
            #update out.yaml, imagePullPolicy:Always to imagePullPolicy:IfNotPresent
            ssh.sshCmd("\"sed -i -e 's/imagePullPolicy: Always/imagePullPolicy: IfNotPresent/g' out.yaml\"",verbose)
            ssh.sshCmd("\"sudo /usr/pensando/bin/kubectl --server=https://" + ip + ":6443 --client-certificate=/var/lib/pensando/pki/kubernetes/apiserver-client/cert.pem --client-key=/var/lib/pensando/pki/kubernetes/apiserver-client/key.pem --insecure-skip-tls-verify=true apply -f out.yaml\"",verbose)


def getElasticSearchStats(ips, verbose=False):
    vIp = None
    imgName = "pen-spyglass"
    data = getImageInfo(ips, imgName)
    for ip,params in data.items():
        if params != {}:
            vIp = ip
            break
    if not vIp:
        raise Exception("failed to find venice node with {0}".format(imgName))
    if verbose:
        print("found spyglass on node {0}".format(vIp))
    ssh = SshClient(vIp, username="vm",password="vm")
    output = ssh.sshCmd("curl -v http://127.0.0.1:9021/debug/vars")
    output = string.join(output)
    data = json.loads(output)
    ret = {}
    if "elasticFwlogDrops" not in data:
        print('*'*40)
        print("error: elasticFwlogDrops not found in data")
        print('*'*40)
        return None
    ret["elasticFwlogDrops"] = data["elasticFwlogDrops"]
    ret["elasticFwlogRetries"] = data["elasticFwlogRetries"]
    ret["watchq"] = {}
    for ip in ips:
        ssh = SshClient(ip, username="vm",password="vm")
        ret["watchq"][ip] = {}
        output = ssh.sshCmd("curl -v http://127.0.0.1:9052/debug/vars")
        output = string.join(output)
        if output == "Client sent an HTTP request to an HTTPS server. ":
            continue
        data = json.loads(output)
        if verbose:
            print(data)
        if "api.cache.watchq[default.fwlogs].ageEvictions" in data:
            ret["watchq"][ip]["ageEvictions"] = data["api.cache.watchq[default.fwlogs].ageEvictions"]
            ret["watchq"][ip]["clientEvictions"] = data["api.cache.watchq[default.fwlogs].clientEvictions"]
            ret["watchq"][ip]["depthEvictions"] = data["api.cache.watchq[default.fwlogs].depthEvictions"]
            ret["watchq"][ip]["dequeue"] = data["api.cache.watchq[default.fwlogs].dequeue"]
            ret["watchq"][ip]["enqueue"] = data["api.cache.watchq[default.fwlogs].enqueue"]
    return ret


if __name__ == "__main__":
    veniceIps = []
    if not veniceIps:
        print("update veniceIps with the IPs of your venice nodes")
        sys.exit(1)
    simIps = []
    if not simIps:
        print("update simIps with the IPs of your sim servers")
        sys.exit(1)
    parser = argparse.ArgumentParser(description='sim control')
    parser.add_argument('--ips', dest='ips', default=veniceIps,
                        nargs='+', help='venice ips')
    parser.add_argument('--reboot', dest='reboot', action="store_true",
                        help='reboot venice nodes')
    parser.add_argument('--isolate', dest='isolate', default=None,
                        help='isolate venice node')
    parser.add_argument('--reconnect', dest='reconnect', default=None,
                        help='reconnect venice node')
    parser.add_argument('--getfw', dest='getfw', default=None,
                        help='get firewall rules')
    parser.add_argument('--vercon', dest='vercon', default=None,
                        help='verify connected firewall rules')
    parser.add_argument('--verdis', dest='verdis', default=None,
                        help='verify disconnected firewall rules')
    parser.add_argument('--cmd', dest='cmd', default=None,
                        help='run a command on all venice nodes')
    parser.add_argument('--patchimage', dest='patchimage', default=None,
                        help='patch docker image')
    parser.add_argument('--getimage', dest='getimage', default=None,
                        help='get docker image info')
    parser.add_argument('--killproc', dest='killproc', default=None,
                        help='kill proc, auto searches all venice nodes')
    #set this always true for now
    parser.add_argument('--complete', dest='complete', default=True,
                        help='isolate from both sides')
    parser.add_argument('--iso-leader', dest='iso_leader', action="store_true",
                        help='isolate venice leader node')
    parser.add_argument('--iso-npm', dest='iso_npm', action="store_true",
                        help='isolate venice npm node')
    parser.add_argument('--iso-api', dest='iso_api', action="store_true",
                        help='isolate venice api server node')
    parser.add_argument('--estats', dest='estats', action="store_true",
                        help='gather elasticsearch stats')
    parser.add_argument('--verbose', dest='verbose', action="store_true",
                        help='print everything')
    _args = parser.parse_args()
    for ip in _args.ips:
        if ip not in veniceIps:
            print("invalid ip {0}. must be one of {1}".format(ip, veniceIps))
            sys.exit(1)
    if _args.isolate and _args.isolate not in veniceIps:
        print("isolate ip must be one of the following: {0}".format(veniceIps))
        sys.exit(1)
    if _args.reconnect and _args.reconnect not in veniceIps:
        print("reconnect ip must be one of the following: {0}".format(veniceIps))
        sys.exit(1)

    if _args.reboot:
        rebootVenice(_args.ips, _args.verbose)
    elif _args.isolate:
        isolateVenice(_args.isolate, veniceIps + simIps, _args.complete, _args.verbose)
    elif _args.reconnect:
        reconnectVenice(_args.reconnect, veniceIps + simIps, _args.complete, _args.verbose)
    elif _args.iso_npm:
        isolateNpmNode(veniceIps + simIps, _args.complete, _args.verbose)
    elif _args.iso_api:
        isolateApiNode(veniceIps + simIps, _args.complete, _args.verbose)
    elif _args.iso_leader:
        isolateLeaderNode(veniceIps + simIps, _args.complete, _args.verbose)
    elif _args.getfw:
        getFirewallRules(_args.getfw, ["venice1", "venice2", "venice3"] + simIps, _args.verbose)
    elif _args.vercon:
        checkFirewall(_args.vercon, veniceIps + simIps, True, _args.verbose)
    elif _args.verdis:
        checkFirewall(_args.verdis, veniceIps + simIps, False, _args.verbose)
    elif _args.cmd:
        print(runCmd(_args.ips, _args))
    elif _args.patchimage:
        patchImage(_args.ips,_args.patchimage,_args.verbose)
    elif _args.getimage:
        img = getImageInfo(_args.ips,_args.getimage,_args.verbose)
        if img: print(img)
    elif _args.killproc:
        killProc(_args.ips,_args.killproc,_args.verbose)
    elif _args.estats:
        output = []
        with open('ESTATS.DATA','w') as outfile:
            while True:
                data = getElasticSearchStats(_args.ips,_args.verbose)
                print(data)
                output.append(data)
                json.dump(output,outfile)
                time.sleep(180)
    else:
        parser.print_help()
