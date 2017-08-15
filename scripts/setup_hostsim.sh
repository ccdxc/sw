#!/bin/bash

host_vms=$(vagrant status | grep host |  awk '{print $1}')
for vm in ${host_vms} ; do
  echo "Setting up hostsim on ${vm}..."
  vagrant scp bin/hostsim ${vm}:/home/vagrant
  vagrant ssh ${vm} -- 'sudo docker load -i alpine.tar; sudo nohup ./hostsim >& /tmp/hostsim.log &'
done
