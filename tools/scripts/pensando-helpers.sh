# The config file should be copied to your home directory.  A sample config
# file is checked in.  Please follow the instructions in the config file.

# Change directory to your workspace
ws() {
    cd "$WS/$1"
}

# Reload helpers
ph_reload() {
    . "$WS/pensando/sw/tools/scripts/pensando-helpers.sh"
}

clean_tree() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from NIC directory"
        return -1
    fi

    make clean
    cd ..
    git submodule deinit -f .
    cd nic
}

update_tree() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from NIC directory"
        return -1
    fi

    if [[ $(parse_git_branch) == 'master' ]]; then
        echo "*************************************"
        echo "  YOU'RE ON MASTER, DUMMY"
        echo "*************************************"
        return -1
    fi

    git fetch upstream && git rebase upstream/master
    git submodule deinit -f .
    git submodule update --init --recursive
    cd ..
    make pull-assets
    cd nic
    make -j4
}

fix_tree() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from NIC directory"
        return -1
    fi

    if [[ $(parse_git_branch) == 'master' ]]; then
        echo "*************************************"
        echo "  YOU'RE ON MASTER, DUMMY"
        echo "*************************************"
        return -1
    fi

    git submodule deinit -f .
    git submodule init
    git submodule update --recursive
    make pull-assets
}

clone_tree() {
    USAGE="Usage: clone_tree <src_tree> <upstream_tree> <ws_dir_name> <branch>"

    SRC_TREE="$1"
    UPS_TREE="$2"
    WS_DIR="$3"
    BRANCH="$4"

    if [[ -z $WS_DIR ]]; then
        echo $USAGE
        return -1
    fi

    if [[ -e $WS_DIR ]]; then
        echo "Workspace directory already exists!"
        return -1
    fi

    if [[ -z $SRC_TREE || -z $UPS_TREE ]]; then
        echo $USAGE
        return -1
    fi

    if [[ -z $BRANCH ]]; then
        echo $USAGE
        return -1
    fi

    if [[ -z "$GOPATH" || -z "$GOROOT" ]]; then
        echo "GOPATH: $GOPATH or GOROOT: $GOROOT is not defined"
        return -1
    fi

    git clone --recursive -b $BRANCH git@github.com:$SRC_TREE $WS_DIR
    cd $WS_DIR
    git remote add upstream git@github.com:$UPS_TREE
    make ws-tools
    make pull-assets
    git submodule init
    git submodule update --recursive
    cd nic
}

make_tree()
{
    USAGE="Usage: make_tree [clean]"

    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from nic directory"
        return -1
    fi

    CLEAN=$1
    if [[ ! -z $CLEAN && $CLEAN != "clean" ]]; then
        echo $USAGE
        return -1
    fi

    make -C proto $CLEAN
    make $CLEAN
    make -C ../platform $CLEAN
    make -C ../platform/src/sim/qemu $CLEAN
    make -C ../platform/src/sim/model_server $CLEAN
}

rename_branch()
{
    USAGE="Usage: rename_branch <old_branch> <new_branch>"

    if [[ -z "$1" || -z "$2" ]]; then
        echo $USAGE
        return -1
    fi

    OLD_BRANCH=$1
    NEW_BRANCH=$2

    git branch -m $OLD_BRANCH $NEW_BRANCH
    git push origin :$OLD_BRANCH $NEW_BRANCH
    git checkout $NEW_BRANCH
    git push origin -u $NEW_BRANCH
}

build_simserver() {
    if [[ `basename $PWD` != 'platform' ]]; then
        echo "Run this command from Platform directory"
        return -1
    fi

    make && make -C src/sim/qemu && make -C src/sim/model_server
}

make_all() {
    make -C nic && \
        make -C platform && \
        make -C platform/src/sim/qemu && \
        make -C platform/src/sim/model_server
}

build_all() {
    git submodule update --recursive && \
        make ws-tools && make pull-assets && make_all
}

clean_all() {
    rm -rf ~/.cache/bazel
    git clean -fdx
}

clean_build_all() {
    clean_all
    build_all
}

setup_pcie() {
    if [[ `basename $PWD` != 'platform' ]]; then
        echo "Run this command from platform directory"
        return -1 
    fi

    LD_LIBRARY_PATH=gen/x86_64/lib gen/x86_64/bin/pciemgrd <<-EOF
	add eth
	finalize
	exit
	EOF
    LD_LIBRARY_PATH=gen/x86_64/lib gen/x86_64/bin/setpci -s 01:00.0 secondary_bus=3
    LD_LIBRARY_PATH=gen/x86_64/lib gen/x86_64/bin/setpci -s 01:00.0 subordinate_bus=3
    LD_LIBRARY_PATH=gen/x86_64/lib gen/x86_64/bin/lspci
}

kill_model() {
    if [ $UID == 0 ] ; then
        echo 'kill as root user disabled'
        return;
    fi

    killall model_server dol hal gdb
}

start_model_how() {
    if [[ `basename $PWD` != 'platform' ]]; then
        echo "Run this command from Platform directory"
        return -1 
    fi

    kill_model

    setup_pcie

    export GDB="gdb -ex run -e ~/.gdbinit --args"
    # IRIS_DEBUG=1 LOG_LEVEL=“info”

    LD_LIBRARY_PATH="$PWD/../nic/gen/x86_64/lib" \
    ZMQ_SOC_DIR="$PWD/../nic" \
    src/sim/model_server/start-model -d "$@"
}

start_model_old() {
    start_model_how type=eth,bdf=03:00.0,lif=5,rxq_type=0,rxq_count=1,txq_type=1,txq_count=1,intr_count=4,qstate_addr=0xc0003000,qidbase=0:1,mac=00:ee:00:00:00:02
}

start_model_rdma() {
    start_model_how type=eth,bdf=03:00.0,lif=5,rxq_type=0,rxq_count=1,txq_type=1,txq_count=1,intr_count=4,qstate_addr=0xc0003000:0xc0003400:0xc0003800:0xc0004000:0xc000c000:0xc0014000:0xc0014400,qstate_size=64:64:64:1024:1024:32:32,mac=00:ee:00:00:00:02
}

start_model() {
    start_model_how type=eth,bdf=03:00.0,lif=5,rxq_type=0,rxq_count=1,txq_type=1,txq_count=1,intr_count=4,qstate_addr=0xc0003000:0xc0003040:0xc0003080,qstate_size=64:64:64,mac=00:ee:00:00:00:02
}

start_hal() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from nic directory"
        return -1 
    fi

    killall hal

    export GDB="gdb -ex run -e ~/.gdbinit --args"
    ZMQ_SOC_DIR="$PWD" ./tools/start-hal.sh
}

start_hal_classic() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from nic directory"
        return -1 
    fi

    killall hal

    cp conf/hal_classic.ini conf/hal.ini
    export GDB="gdb -ex run -e ~/.gdbinit --args"
    ZMQ_SOC_DIR="$PWD" ./tools/start-hal.sh
}

start_dol() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from nic directory"
        return -1 
    fi
    ZMQ_SOC_DIR="$PWD" ../dol/main.py --topo classic --feature classic --classic --config-only --nohostmem
}

start_dol_rdma() {
    cp ../dol/config/specs/lif/{eth_rdma_drv.txt,eth.spec}
    start_dol
}

start_relay() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from nic directory"
        return -1 
    fi

    sudo ZMQ_SOC_DIR=$PWD ./model_sim/src/relay.py bidi -v -x --mac $RELAY_MAC_ADDR
}

find_tap() {
    ip link | grep -B1 $RELAY_MAC_ADDR | sed -ne 's/^.* \(tap[0-9]*\): .*$/\1/p'
}

setup_tap() {
    TAPIF=$RELAY_TAP
    if [ -z "$TAPIF" ] ; then
        TAPIF=$(find_tap)
        if [ -z "$TAPIF" ] ; then
            echo "Did not find the tap device"
            return -1
        fi
    fi

    if [ -z "$RELAY_BRIDGE" ] ; then
        #tap-only
        sudo ip addr add $RELAY_IP_ADDR dev $TAPIF
        if [ -z "$RELAY_IPV6_ADDR" ] ; then
            # disable ipv6 on tap interface
            sudo sysctl net.ipv6.conf.$TAPIF.disable_ipv6=1
        else
            # enable ipv6 on tap interface and assign IPv6 addr
            sudo sysctl net.ipv6.conf.$TAPIF.disable_ipv6=0
            sudo ip addr add $RELAY_IPV6_ADDR dev $TAPIF
        fi
        sudo ip link set $TAPIF up

        sudo ip addr show dev $TAPIF
    else
        #with bridge
        sudo brctl addif $RELAY_BRIDGE $TAPIF
        sudo ip addr add $RELAY_IP_ADDR dev $RELAY_BRIDGE
        if [ -z "$RELAY_IPV6_ADDR" ] ; then
            # disable ipv6 on bridge interface
            sudo sysctl net.ipv6.conf.$RELAY_BRIDGE.disable_ipv6=1
        else
            # enable ipv6 on bridge interface and assign IPv6 addr
            sudo sysctl net.ipv6.conf.$RELAY_BRIDGE.disable_ipv6=0
            sudo ip addr add $RELAY_IPV6_ADDR dev $RELAY_BRIDGE
        fi
        sudo ip link set $RELAY_BRIDGE up

        sudo ip addr show dev $TAPIF
        sudo ip addr show dev $RELAY_BRIDGE
    fi
}

start_qemu() {
    if [[ `basename $PWD` != 'platform' ]]; then
        echo "Run this command from platform directory"
        return -1 
    fi

    setup_tap

    sudo GDB="gdb -ex run --args" \
        QEMU_MACHINE="$QEMU_MACHINE" \
        QEMU_SERIAL_PORT="$QEMU_SERIAL_PORT" \
        QEMU_MONITOR_PORT="$QEMU_MONITOR_PORT" \
        QEMU_SSH_PORT="$QEMU_SSH_PORT" \
        QEMU_MEMORY_MB="$QEMU_MEMORY_MB" \
        QEMU_EXTRA="$QEMU_EXTRA" \
        src/sim/qemu/qemu-run $QEMU_IMAGE
}

monitor_qemu() {
    telnet 127.0.0.1 $QEMU_MONITOR_PORT
}

telnet_qemu() {
    telnet 127.0.0.1 $QEMU_SERIAL_PORT
}

ssh_qemu() {
    ssh -p $QEMU_SSH_PORT $QEMU_USER@127.0.0.1 "$@"
}

start_qemu_rxe() {
    if [ -z "$RELAY_BRIDGE" ] ; then
        echo "Please use a bridge device"
    fi

    sudo qemu-system-x86_64 \
        -machine q35,accel=kvm -smp cpus=4,sockets=2,cores=2 -nographic -m $QEMU_MEMORY_MB \
        -netdev user,id=net0,hostfwd=tcp::$QEMU_SSH_PORT_RXE-:22 \
        -device e1000,netdev=net0 \
        -netdev bridge,id=net1,br=$RELAY_BRIDGE \
        -device e1000,netdev=net1 \
        -drive file=$QEMU_IMAGE_RXE,if=virtio $QEMU_EXTRA
}

ssh_qemu_rxe() {
    ssh -p $QEMU_SSH_PORT_RXE $QEMU_USER@127.0.0.1 "$@"
}

# source config file at end, so user can override any command
. ~/pensando-helpers-config.sh

# vi: set sw=4 et :
