#!/bin/bash

host_vms=$(vagrant status | grep host |  awk '{print $1}')
for vm in ${host_vms} ; do
  echo "Setting up hostsim on ${vm}..."
  vagrant ssh ${vm} -- 'sudo mkdir -p /import/bin; sudo chown -R vagrant /import'
  vagrant scp bin/cbin/hostsim ${vm}:/import/bin/
  vagrant scp bin/cbin/vsimctl ${vm}:/import/bin/
  vagrant scp nic/agent/netagent/scripts/nsenterShell.sh ${vm}:/import/bin/
  vagrant ssh ${vm} -- 'sudo docker load -i alpine.tar'
done
