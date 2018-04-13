#!/bin/bash

if [ "$1" = "" ]; then
    echo "usage: $0 <nic-vx.y.tgz>"
    exit;
fi

echo "Installing NAPLES simulation ..."
# install the software components
export NAPLES_INSTALL_DIR=$HOME/naples/v1
echo "Extracting NAPLES packages to $NAPLES_INSTALL_DIR"
mkdir -p $NAPLES_INSTALL_DIR
tar xmzf $1 --strip-components=1 -C $NAPLES_INSTALL_DIR
cd $NAPLES_INSTALL_DIR
docker build -t naples:v1 $NAPLES_INSTALL_DIR
echo "NAPLES docker image v1 created"
docker images
cd -

# do all the plubming needed


echo "NAPLES installation completed"
