#!/bin/bash

#
# Install software iWarp to get the basic verbs infrastructure going.
#

sudo apt install libibverbs1 libibverbs-dev librdmacm1 librdmacm-dev ibverbs-utils
sudo modprobe ib_uverbs
sudo rm -rf /tmp/siw
mkdir /tmp/siw
cd /tmp/siw
curl -H 'Authorization: token cea8eed42fc1f34ca333aed6162e5bdd05e036be' -H 'Accept: application/vnd.github.v3.raw' -O -L https://api.github.com/repos/pensando/bins/precomp_siw.tgz
tar xzf precomp_siw.tgz
cd precomp_siw
chmod +x install.sh
sudo ./install.sh
cd /tmp
/bin/rm -rf siw/
