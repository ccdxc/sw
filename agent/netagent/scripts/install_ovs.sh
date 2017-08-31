#!/bin/bash

wget http://cbs.centos.org/kojifiles/packages/openvswitch/2.5.0/2.el7/x86_64/openvswitch-2.5.0-2.el7.x86_64.rpm -O /tmp/openvswitch-2.5.0-2.el7.x86_64.rpm
sudo yum install -y /tmp/openvswitch-2.5.0-2.el7.x86_64.rpm
sudo systemctl start openvswitch
