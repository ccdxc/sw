#! /bin/bash

sudo rm -rf /home/vm/images
sudo mkdir -p /home/vm/images
sudo mv /var/iota/*.tgz /home/vm/images/
sudo /var/iota/INSTALL.sh --clean-only
sudo systemctl stop pen-cmd
sudo docker rm -fv $(docker ps -aq)
sudo docker system prune -f
sudo rm /etc/hosts
sudo pkill iota*
sudo rm -rf /var/iota*
sudo docker ps
sudo docker rmi -f $(docker images -aq)
sudo rm -rf /var/run/naples
sudo iptables -F
sudo systemctl restart docker

for i in `ip link show type vlan | grep UP | cut -d : -f 2 | cut -d @ -f 1`
do 
    ip link delete $i; echo $i; 
done

sudo mkdir -p /var/iota
sudo mv /home/vm/images/* /var/iota/
sudo chown -R vm /var/iota
sudo chgrp -R vm /var/iota
