#! /bin/bash

HOST_KEY_DISABLE="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
NAPLES_DIR="/home/vm/naples/"
REMOTE_HOME="/home/vm/"

function CopyToVm()
{
    set -e
    echo "CopyToVm: $1 to $2"  
    sshpass -p vm scp ${HOST_KEY_DISABLE} $1 vm@${CURRENT_VM_IP}:$2
    return $?
}

function CopyFromVm()
{
    set -e
    echo "CopyFromVm: $1 to $2"  
    sshpass -p vm scp ${HOST_KEY_DISABLE} vm@${CURRENT_VM_IP}:$1 $2
    return $?
}

function RunVmCmd()
{
    set -e
    sshpass -p vm ssh ${HOST_KEY_DISABLE} vm@${CURRENT_VM_IP} "$*"
    return $?
}

function BuildReleaseImage()
{
    set -e
    
#    cp /sw/nic/sim/warmd/naples-release-v1.tgz /sw/nic/obj/images/naples-release-v1.tgz
#    return
    
    cd /sw/
    make pull-assets
    cd /sw/nic
    make nic
    make sanity-tests
    make package
    make release
}

set -e
BuildReleaseImage
yum install -y sshpass

for vm in `/sw/nic/sim/warmd/parse_vm_ips.py`
do
    echo "Setting up VM: $vm"
    export CURRENT_VM_IP=$vm
    CopyToVm /sw/nic/obj/images/naples-release-v1.tgz ${REMOTE_HOME}/naples-release-v1.tgz
    CopyToVm /sw/nic/sim/warmd/setup_warmd_vm.sh ${REMOTE_HOME}/setup_warmd_vm.sh
    CopyToVm /sw/nic/sim/warmd/warmd_bootstrap.sh ${REMOTE_HOME}/warmd_bootstrap.sh
    CopyToVm /sw/nic/sim/warmd/start-naples-docker.sh ${REMOTE_HOME}/start-naples-docker.sh
    CopyToVm /sw/nic/sim/warmd/stop-naples-docker.sh ${REMOTE_HOME}/stop-naples-docker.sh
    RunVmCmd ${REMOTE_HOME}/setup_warmd_vm.sh
done

echo "Waiting for NAPLES container to bootup"
sleep 300
for vm in `/sw/nic/sim/warmd/parse_vm_ips.py`
do
    echo "Verify AGENT reachability to $vm"
    curl -f http://${CURRENT_VM_IP}:9007/api/tenants/
done
