#!/bin/bash

set -ex

cd /sw/iota

echo "Processing input json file: /warmd.json"

cat /warmd.json | jq '.Infra.VCenter | {ID: 100, Name: .IP, Type: "vm", Tag: "vcenter", NodeMgmtIP: .IP, Resource, PvtNetworks, DataNetworks}' > tinstance.json
cat /warmd.json | jq '.Infra.VCenter | {Provision: {Vars: {ProdVCenter: true, VcenterUsername: .Username, VcenterPassword: .Password, VcenterLicense: .License, Datastore: .Datastore}}}' > tprov.json

jq --slurpfile vm_inst tinstance.json '.Instances += $vm_inst' /warmd.json > twarmd.json
jq -s '.[0] * .[1]' twarmd.json tprov.json > /sw/iota/warmd.json
cat /sw/iota/warmd.json

./iota.py --testbed /sw/iota/warmd.json $@
