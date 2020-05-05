#!/bin/bash
#set -x
#set -e

if [[ -z "${GOPATH}" ]]; then
    echo "GOPATH: ${GOPATH} is not defined"
    exit 1
fi

VERSION="6.7" # Default vcenter version
WARMD_FILE=/warmd.json # Default warmd.json path

while (( "$#" )); do 
    case "$1" in 
        -t|--testbed-json)
            WARMD_FILE=$2 
            shift 2 
            ;; 
        -v|--vcenter-version)
            VERSION=$2
            shift 2
            ;;
        -h|--help)
            echo ""
            echo "$0 <options>"
            echo "  -t|--testbed-json <tb_json_file>"
            echo "  -v|--vcenter-version [6.7|7.0]"
            echo ""
            shift
            exit 0
            ;;
        --) # end argument parsing 
            shift 
            break 
            ;; 
        -*|--*=) # unsupported flags 
            echo "Error: Unsupported flag $1" >&2 
            exit 1 
            ;; 
    esac 
done

function process_vcenter_list() {
    cat ${WARMD_FILE} | jq ".Infra.VCenters[] | select(.Version==\"${VERSION}\") | {ID: 100, Name: .IP, Type: \"vm\", Tag: \"vcenter\", NodeMgmtIP: .IP, Resource, PvtNetworks, DataNetworks}" > /tmp/tinstance.json
    vcenter_ip=`jq ".Infra.VCenters[] | select(.Version==\"${VERSION}\") | .IP" ${WARMD_FILE}`
    if [[ $vcenter_ip =~ "dev" ]] ; 
    then
        echo "Found a dev-vcenter"
        cat ${WARMD_FILE} | jq ".Infra.VCenters[] | select(.Version==\"${VERSION}\") | {Provision: {Vars: {VcenterUsername: .Username, VcenterPassword: .Password, VcenterLicense: .License, Datastore: .Datastore}}}" > /tmp/tprov.json
    else
        echo "Found a prod-vcenter"
        cat ${WARMD_FILE} | jq ".Infra.VCenters[] | select(.Version==\"${VERSION}\") | {Provision: {Vars: {ProdVCenter: \"true\", VcenterUsername: .Username, VcenterPassword: .Password, VcenterLicense: .License, Datastore: .Datastore}}}" > /tmp/tprov.json
    fi

    jq --slurpfile vm_inst /tmp/tinstance.json '.Instances += $vm_inst' ${WARMD_FILE} > /tmp/twarmd.json
    jq -s '.[0] * .[1]' /tmp/twarmd.json /tmp/tprov.json > $GOPATH/src/github.com/pensando/sw/iota/warmd_vcenter.json
    cat $GOPATH/src/github.com/pensando/sw/iota/warmd_vcenter.json
    echo ""
    echo "Generated: $GOPATH/src/github.com/pensando/sw/iota/warmd_vcenter.json"
    echo ""
    exit 0
}

function process_legacy_warmd() {
    cat ${WARMD_FILE} | jq '.Infra.VCenter | {ID: 100, Name: .IP, Type: "vm", Tag: "vcenter", NodeMgmtIP: .IP, Resource, PvtNetworks, DataNetworks}' > /tmp/tinstance.json
    cat ${WARMD_FILE} | jq '.Infra.VCenter | {Provision: {Vars: {ProdVCenter: "true", VcenterUsername: .Username, VcenterPassword: .Password, VcenterLicense: .License, Datastore: .Datastore}}}' > /tmp/tprov.json 
    
    jq --slurpfile vm_inst /tmp/tinstance.json '.Instances += $vm_inst' ${WARMD_FILE} > /tmp/twarmd.json 
    jq -s '.[0] * .[1]' /tmp/twarmd.json /tmp/tprov.json > $GOPATH/src/github.com/pensando/sw/iota/warmd_vcenter.json 

    cat $GOPATH/src/github.com/pensando/sw/iota/warmd_vcenter.json 
    echo "" 
    echo "Generated: $GOPATH/src/github.com/pensando/sw/iota/warmd_vcenter.json" 
    echo "" 
    exit 0
}

cd $GOPATH/src/github.com/pensando/sw/iota
if [ -f ${WARMD_FILE} ]; then
    echo "Modifying ${WARMD_FILE} with vcenter details"
    # Check if we are dealing with legacy version
    legacy_vcenter_info=`jq ".Infra.VCenter | type" ${WARMD_FILE}`
    if [[ $legacy_vcenter_info = '"object"' ]] ;
    then
        process_legacy_warmd
        exit 0
    fi

    latest_vcenter_array=`jq ".Infra.VCenters | type" ${WARMD_FILE}`
    if [[ $latest_vcenter_array = '"array"' ]] ;
    then
        process_vcenter_list
    fi
else
    echo "Missing: ${WARMD_FILE}"
    exit 1
fi
