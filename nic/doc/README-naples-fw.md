#############################
# NIC build - On DEV machine
#############################

# ENV for repo
export WS_DIR=/local/$USER
export WS_NAME=foo                  # <<<<<< Change the name to your liking
export GOPATH=$WS_DIR/$WS_NAME
export PATH=$GOPATH/bin:$PATH
mkdir -p $WS_DIR/$WS_NAME/src/github.com/pensando
cd $WS_DIR/$WS_NAME/src/github.com/pensando

# clone repo
git clone git@github.com:pensando/sw

cd sw

# submodule
git submodule update --init —recursive

# ENV for build
make ws-tools
make pull-assets

cd nic

# start nic build container
make docker/shell

# Inside nic build container
cd /sw/nic
make ARCH=aarch64 PLATFORM=hw clean
make ARCH=aarch64 PLATFORM=hw


#############################
# Naples FW (Buildroot) - On DEV machine
#############################

# Get buildroot container
docker pull registry.test.pensando.io:5000/pensando/buildroot/f3d8bfd731:20181019.1304
docker run -it  -v <path_to_pensando_sw>:/sw registry.test.pensando.io:5000/pensando/buildroot/f3d8bfd731:20181019.1304

# Build images
cd /buildroot
make -j `nproc` BR2_ROOTFS_OVERLAY="board/pensando/capri/rootfs-overlay /sw/fake_root_target/aarch64"

or just run

docker run --rm -v <path_to_pensando_sw>:/sw registry.test.pensando.io:5000/pensando/buildroot/dmichaels:20181026.2036 sh -c 'make BR2_ROOTFS_OVERLAY="board/pensando/capri/rootfs-overlay /sw/fake_root_target/aarch64" && cp /buildroot/output/images/naples_fw.tar  /sw/nic'

# Naples FW image
cp output/images/naples_fw.tar /sw/nic/


#############################
# INSTALL Naples FW
#############################

< Copy naples_fw.tar from <ws>/nic/naples_fw.tar to Naples >

# ON naples host
/home/haps/memtun/memtun 1.0.0.1 &  # Do this once after every server reboot

scp /home/neel/naples/stat root@1.0.0.2:/bin/
scp <path-to-naples_fw.tar> root@1.0.0.2:/tmp
ssh root@1.0.0.2
# Password: pen123

# Flash the image
/nic/tools/sysupdate.sh -p /tmp/naples_fw.tar

# reset Naples to boot new image
/nic/tools/sysreset.sh

# start processes in classic mode
# /nic/tools/sysinit.sh classic hw

# start processes in smart-nic mode
# /nic/tools/sysinit.sh hostpin hw
