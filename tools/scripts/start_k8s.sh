#!/bin/bash

nodes=(`vagrant status | grep running | cut -d " " -f 1`)

if [ ${#nodes[@]} -eq 0 ]; then
    echo No vagrant nodes to setup k8s
    exit 1
else
    echo Found ${#nodes[@]} nodes, setting up k8s
fi

master_ip=`vagrant ssh ${nodes[0]} -- 'cat /etc/hosts' | grep ${nodes[0]} | cut -d " " -f 1`

for i in "${!nodes[@]}"
do
    if [ $i == 0 ]; then
    	echo Setting up ${nodes[0]} as master
    	vagrant ssh ${nodes[0]} -- 'sudo kubeadm init --apiserver-advertise-address '$master_ip' --token f0c861.753c505740ecde4c --skip-preflight-checks=true'
    	vagrant ssh ${nodes[0]} -- 'sudo cp /etc/kubernetes/admin.conf $HOME/'
    	vagrant ssh ${nodes[0]} -- 'sudo chown $(id -u):$(id -g) $HOME/admin.conf'
    	# workaround for https://github.com/kubernetes/kubernetes/issues/34101
    	vagrant ssh ${nodes[0]} -- 'KUBECONFIG=$HOME/admin.conf kubectl -n kube-system get ds -l '\''k8s-app=kube-proxy'\'' -o json | /usr/local/bin/jq-linux64 '\''.items[0].spec.template.spec.containers[0].command |= .+ ["--proxy-mode=userspace"]'\'' |   kubectl apply -f - && kubectl -n kube-system delete pods -l '\''k8s-app=kube-proxy'\'''

        # Add weave networking
        vagrant ssh ${nodes[0]} -- 'KUBECONFIG=$HOME/admin.conf /usr/bin/kubectl apply -f https://github.com/weaveworks/weave/releases/download/v2.1.3/weave-daemonset-k8s-1.7.yaml'
    else
    	echo Adding ${nodes[$i]} to cluster
    	vagrant ssh ${nodes[$i]} -- 'sudo kubeadm join --token f0c861.753c505740ecde4c --skip-preflight-checks=true '$master_ip':6443'
    fi
done
