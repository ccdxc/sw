#!/bin/sh

set -x

cd /home/vagrant/venice
rm -fr  venice
mkdir -p venice

cd venice
tar zxvf /home/vagrant/venice/venice.tgz

cd  -
# ./dind/do.py -configFile tb_config_2.json -nettype macvlan -venice_image_dir /home/vagrant/venice/venice
./dind/do.py -configFile /vagrant/testbed.json -custom_config_file /vagrant/venice-conf.json -venice_image_dir /home/vagrant/venice/venice
