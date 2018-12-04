#! /bin/bash

sudo rm -rf /pensando/images
sudo mkdir -p /pensando/images
sudo mv /pensando/iota/*.tgz /home/vm/images/
sudo /pensando/iota/INSTALL.sh --clean-only
sudo systemctl stop pen-cmd
sudo docker rm -fv $(docker ps -aq)
sudo docker system prune -f
sudo rm /etc/hosts
sudo pkill iota*
sudo rm -rf /pensando/iota*
sudo docker ps
sudo docker rmi -f $(docker images -aq)
sudo rm -rf /pensando/run/naples
sudo iptables -F
sudo systemctl restart docker

for i in `ip link show type vlan | grep UP | cut -d : -f 2 | cut -d @ -f 1`
do 
    ip link delete $i; echo $i; 
done

sudo mkdir -p /pensando/iota
sudo mv /pensando/images/* /home/vm/iota/
sudo chown -R vm /pensando/iota
sudo chgrp -R vm /pensando/iota
