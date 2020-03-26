# install any python libraries using "pip3 install <module> --user" to install
# without sudo privileges


export PYTESTAPPROOT=/home/vsampath/pytest
export PYTHONPATH=${PYTESTAPPROOT}/configobjects:${PYTESTAPPROOT}/proto

mkdir ${PYTESTAPPROOT}
mkdir ${PYTESTAPPROOT}/apps
mkdir ${PYTESTAPPROOT}/configobjects
mkdir ${PYTESTAPPROOT}/proto
mkdir ${PYTESTAPPROOT}/proto/meta

# in your workspace
cp build/aarch64/apulu/gen/proto/* ${PYTESTAPPROOT}/proto/
cp build/aarch64/apulu/gen/proto/meta/* ${PYTESTAPPROOT}/proto/meta/
cp apollo/tools/apulu/pytestapp/configobjects/* ${PYTESTAPPROOT}/configobjects/
cp apollo/tools/apulu/pytestapp/apps/* ${PYTESTAPPROOT}/apps/
 


# Load ionic driver on node1 and node2 host
sudo insmod /naples/drivers-linux-eth/drivers/eth/ionic/ionic.ko

cd ${PYTESTAPPROOT}

export OOB_NODE1=192.168.71.46
export OOB_NODE2=192.168.68.41

python3 apps/config_host1_naples.py $OOB_NODE1 --grpc_port 50054
python3 apps/config_host2_naples.py $OOB_NODE2 --grpc_port 50054


====================================================================


# node 1

sudo insmod /naples/drivers-linux-eth/drivers/eth/ionic/ionic.ko

sudo ip netns add A
sudo ip netns add B

sudo ip link set enp20s0 netns A
sudo ip link set enp21s0 netns B

sudo ip netns exec A ifconfig enp20s0 2.1.0.2/24 mtu 9000 up
sudo ip netns exec A ifconfig enp20s0 hw ether 00:ae:cd:00:08:11
sudo ip netns exec A ip route add 64.0.0.0/16 via 2.1.0.1
sudo ip netns exec A ip route add 3.1.0.0/16 via 2.1.0.1

sudo ip netns exec B ifconfig enp21s0 3.1.0.2/24 mtu 9000 up
sudo ip netns exec B ifconfig enp21s0 hw ether 00:ae:cd:00:08:12
sudo ip netns exec B ip route add 2.1.0.0/16 via 3.1.0.1
sudo ip netns exec B ip route add 3.1.0.0/16 via 3.1.0.1


# node 2

sudo insmod /naples/drivers-linux-eth/drivers/eth/ionic/ionic.ko
sudo ifconfig enp20s0 2.1.0.3/24 mtu 9000 up
sudo ifconfig enp20s0 hw ether 00:ae:cd:00:00:0a

sudo ifconfig enp22s0 hw ether 00:ae:cd:00:00:0c
sudo ifconfig enp22s0 64.0.0.2/24 mtu 9000 up
sudo ip route add 3.1.0.0/24 via 2.1.0.1
sudo ip route add 50.0.0.0/24 via 64.0.0.1

