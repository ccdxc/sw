#!/bin/bash

NEW_OVTOOL=ovftool
VCSA_OVA=output/venice.ova

ESXI_HOST=192.168.68.204
ESXI_USERNAME=root
ESXI_PASSWORD='pen123!'

VCSA_VMNAME=venice-test
VCSA_HOSTNAME=$VCSA_VMNAME
VCSA_IP="192.168.71.127"
VCSA_NETMASK="255.255.252.0"
VCSA_GATEWAY="192.168.68.1"
VCSA_DNS="192.168.68.2"
VM_NETWORK="VM Network"
VM_DATASTORE='datastore1 (2)'
PASSWORD="pensando123"
VCSA_DOMAIN="pensando.io"

### DO NOT EDIT BEYOND HERE ###

"${NEW_OVTOOL}" --noSSLVerify --disableVerification --overwrite --acceptAllEulas --skipManifestCheck --X:injectOvfEnv --powerOn "--net:VM Network=${VM_NETWORK}" --datastore="${VM_DATASTORE}" --diskMode=thin --name=${VCSA_VMNAME} --prop:domain=${VCSA_DOMAIN} --prop:hostname=${VCSA_HOSTNAME} --prop:dns=${VCSA_DNS} --prop:gateway=${VCSA_GATEWAY} --prop:ipaddress=${VCSA_IP} --prop:netmask=${VCSA_NETMASK} --prop:password=${PASSWORD} "${VCSA_OVA}" "vi://${ESXI_USERNAME}:${ESXI_PASSWORD}@${ESXI_HOST}/"

exit

for i in {2..3}; do
    VM_NAME="venice$i"
    echo "VM_NAME=$VM_NAME"
    "${NEW_OVTOOL}" --overwrite --acceptAllEulas --skipManifestCheck --X:injectOvfEnv --powerOn "--net:VM Network=${VM_NETWORK}" --datastore=${VM_DATASTORE} --diskMode=thin --name=${VM_NAME} --prop:hostname=${VM_NAME} --prop:password=${PASSWORD} "${VCSA_OVA}" "vi://${ESXI_USERNAME}:${ESXI_PASSWORD}@${ESXI_HOST}/"
done

