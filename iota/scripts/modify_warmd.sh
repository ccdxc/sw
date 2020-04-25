#!/bin/bash
#set -x
#set -e

if [[ -z "${GOPATH}" ]]; then
    echo "GOPATH: ${GOPATH} is not defined"
    exit 1
fi

if [[ $# -eq 0 ]] ;
then
    WARMD_FILE=/warmd.json
else
    WARMD_FILE=$1
fi

cd $GOPATH/src/github.com/pensando/sw/iota
if [ -f ${WARMD_FILE} ]; then
    echo "Modifying warmd.json with vcenter details"
    cat ${WARMD_FILE} | jq '.Infra.VCenter | {ID: 100, Name: .IP, Type: "vm", Tag: "vcenter", NodeMgmtIP: .IP, Resource, PvtNetworks, DataNetworks}' > tinstance.json
    cat ${WARMD_FILE} | jq '.Infra.VCenter | {Provision: {Vars: {ProdVCenter: "true", VcenterUsername: .Username, VcenterPassword: .Password, VcenterLicense: .License, Datastore: .Datastore}}}' > tprov.json

    jq --slurpfile vm_inst tinstance.json '.Instances += $vm_inst' ${WARMD_FILE} > twarmd.json
    jq -s '.[0] * .[1]' twarmd.json tprov.json > $GOPATH/src/github.com/pensando/sw/iota/warmd_vcenter.json
    cat $GOPATH/src/github.com/pensando/sw/iota/warmd_vcenter.json
    echo ""
    echo "Generated: $GOPATH/src/github.com/pensando/sw/iota/warmd_vcenter.json"
    echo ""
    exit 0
else
    echo "Missing: ${WARMD_FILE}"
    exit 1
fi
