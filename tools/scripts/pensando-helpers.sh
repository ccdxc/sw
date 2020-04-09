# The config file should be copied to your home directory.  A sample config
# file is checked in.  Please follow the instructions in the config file.

export ASIC="${ASIC:-capri}"

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
    git submodule update --init --recursive && \
        make ws-tools && make pull-assets && make_all
}

clean_all() {
    rm -rf ~/.cache/bazel
    git clean -fdX
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

    # export GDB="gdb -ex run -e ~/.gdbinit --args"
    # IRIS_DEBUG=1 LOG_LEVEL=info

    LD_LIBRARY_PATH="$PWD/../nic/build/x86_64/iris/${ASIC}/lib" \
    ZMQ_SOC_DIR="$PWD/../nic" \
    src/sim/model_server/start-model -d "$@"
}

start_model_accel() {
    start_model_how type=accel,bdf=03:00.0,lif=66,intr_base=0,devcmd_pa=0x118748000,devcmddb_pa=0x118749000
}

start_model_eth() {
    start_model_how type=eth,bdf=03:00.0,lif=4,intr_base=0,devcmd_pa=0x13809f000,devcmddb_pa=0x1380a0000
}

start_hal_hostpin() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from nic directory"
        return -1
    fi

    killall hal

    cp conf/hal_hostpin.ini conf/hal.ini
    # export GDB="gdb -ex run -e ~/.gdbinit --args"
    ZMQ_SOC_DIR="$PWD" ./tools/start-hal.sh
}

start_hal_classic() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from nic directory"
        return -1
    fi

    killall hal

    export QEMU=1
    cp conf/hal_classic.ini conf/hal.ini
    # export GDB="gdb -ex run -e ~/.gdbinit --args"
    ZMQ_SOC_DIR="$PWD" ./tools/start-hal.sh
}

start_relay() {
    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from nic directory"
        return -1
    fi

    sleep 10 && setup_tap &
    SETUP_PID=$!
    sudo ZMQ_SOC_DIR=$PWD ./sdk/model_sim/src/relay.py bidi "$@" --mac $RELAY_MAC_ADDR -tname $RELAY_TAP
    kill $SETUP_PID
    wait $SETUP_PID
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

start_nicmgr_eth() {
    if [[ `basename $PWD` != 'platform' ]]; then
        echo "Run this command from platform directory"
        return -1
    fi

    HAL_CONFIG_PATH="$PWD/../nic/conf" \
    LD_LIBRARY_PATH="../nic/build/x86_64/iris/${ASIC}/lib/:./gen/x86_64/lib/:../nic/conf/sdk" \
        ZMQ_SOC_DIR=$PWD/../nic \
        $GDB ./gen/x86_64/bin/nicmgrd -c ./src/app/nicmgrd/etc/eth.json
}

start_nicmgr_eth_smart() {
    if [[ `basename $PWD` != 'platform' ]]; then
        echo "Run this command from platform directory"
        return -1
    fi

    HAL_CONFIG_PATH="../nic/conf" \
    LD_LIBRARY_PATH="../nic/build/x86_64/iris/${ASIC}/lib/:./gen/x86_64/lib/:../nic/conf/sdk" \
        ZMQ_SOC_DIR=$PWD/../nic \
        $GDB ./gen/x86_64/bin/nicmgrd -s -c ./src/app/nicmgrd/etc/eth-smart.json
}

start_nicmgr_accel() {
    if [[ `basename $PWD` != 'platform' ]]; then
        echo "Run this command from platform directory"
        return -1
    fi

    HAL_CONFIG_PATH="../nic/conf" \
    LD_LIBRARY_PATH="../nic/build/x86_64/iris/${ASIC}/lib/:./gen/x86_64/lib/:../nic/conf/sdk" \
        ZMQ_SOC_DIR=$PWD/../nic \
        $GDB ./gen/x86_64/bin/nicmgrd -p sim -c ./src/app/nicmgrd/etc/accel.json
}

start_qemu() {
    if [[ `basename $PWD` != 'platform' ]]; then
        echo "Run this command from platform directory"
        return -1
    fi

    setup_pcie

    #: ${GDB:=gdb -ex run --args}

    sudo GDB="$GDB" \
        QEMU_MACHINE="$QEMU_MACHINE" \
        QEMU_SERIAL_PORT="$QEMU_SERIAL_PORT" \
        QEMU_MONITOR_PORT="$QEMU_MONITOR_PORT" \
        QEMU_SSH_PORT="$QEMU_SSH_PORT" \
        QEMU_MEMORY_MB="$QEMU_MEMORY_MB" \
        QEMU_EXTRA="$QEMU_EXTRA" \
        src/sim/qemu/qemu-run --img $QEMU_IMAGE
}

start_bsd() {
    QEMU_EXTRA="$QEMU_EXTRA_BSD" QEMU_IMAGE="$QEMU_IMAGE_BSD" start_qemu
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

ssh_bsd() {
    QEMU_USER=root ssh_qemu "$@"
}

start_qemu_rxe() {
    if [ -z "$RELAY_BRIDGE" ] ; then
        echo "Please use a bridge device"
    fi

    # sudo needed for -netdev bridge (or we need to make qemu-bridge-helper work)
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

start_agent() {

    if [[ `basename $PWD` != 'nic' ]]; then
        echo "Run this command from nic directory"
        return -1
    fi

    ./obj/x86_64/agent/netagent -datapath hal -logtofile agent.log -hostif lo -agentdb $PWD/agent.db
}

start_heimdall() {

    if [[ `basename $PWD` != 'sw' ]]; then
        echo "Run this command from sw directory"
        return -1
    fi

    nic/obj/agent/heimdall  run \
        --device-file  platform/src/app/nicmgrd/etc/eth-smart.json \
        --config-file  nic/e2etests/go/agent/configs/golden/golden_cfg.yml
}

# source config file at end, so user can override any command
. ~/pensando-helpers-config.sh

# vi: set sw=4 et :
