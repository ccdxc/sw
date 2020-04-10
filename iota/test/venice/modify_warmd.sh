#!/bin/bash

if [[ -z "$GOPATH" ]]; then
    echo "GOPATH: $GOPATH is not defined"
    exit 1
fi

cd $GOPATH/src/github.com/pensando/sw/iota
if [ -f /warmd.json ]; then
    echo "Modifying warmd.json with vcenter details"
    cat /warmd.json | jq '.Infra.VCenter | {ID: 100, Name: .IP, Type: "vm", Tag: "vcenter", NodeMgmtIP: .IP, Resource, PvtNetworks, DataNetworks}' > tinstance.json
    cat /warmd.json | jq '.Infra.VCenter | {Provision: {Vars: {ProdVCenter: "true", VcenterUsername: .Username, VcenterPassword: .Password, VcenterLicense: .License, Datastore: .Datastore}}}' > tprov.json

    jq --slurpfile vm_inst tinstance.json '.Instances += $vm_inst' /warmd.json > twarmd.json
    jq -s '.[0] * .[1]' twarmd.json tprov.json > $GOPATH/src/github.com/pensando/sw/iota/warmd.json
fi
