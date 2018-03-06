
# Copy this file to your home directory, and change all the paths, addresse,
# and ports so they don't collide with any other users on the same system.
#
# Also, add to your .bashrc:
#
# . /local/you/ws/src/github.com/pensando/sw/tools/scripts/pensando-helpers.sh
#
# The configuration file in your home directory will be loaded from there.
#
# If either pensando-helpers.sh or the config file is changed after your bash
# session is started, update the loaded helpers by running `ph_reload`.

# Path to your workspace, two levels up from cloned repos.
WS=/local/you/ws/src/github.com

# Path to your kernel sources, if working on device drivers
#export KSRC=/local/you/linux

# Please use your own grpc port.
export HAL_GRPC_PORT=9990

# Please use your own relay mac and ip subnet.
RELAY_MAC_ADDR=ba:aa:aa:aa:aa:aa
RELAY_IP_ADDR=172.0.0.1/24
# Enable this if wanting IPv6 address on relay tap...pick your own address
#RELAY_IPV6_ADDR=2001:172::1/64

# If using a bridge device, also edit /etc/qemu/bridge.conf
RELAY_BRIDGE=you_br

# Qemu configuration options.
QEMU_IMAGE='/local/you/sim.img'
#extra parameters: eg for ubuntu cloud, give the seed image
#QEMU_EXTRA='-cdrom /local/you/sim-seed.img'
#QEMU_MEMORY_MB=512
QEMU_USER=ubuntu
# Please use your own qemu ports
QEMU_SERIAL_PORT=9991
QEMU_MONITOR_PORT=9992
QEMU_SSH_PORT=9993

# Second qemu configuration, for soft-roce host.
QEMU_IMAGE_RXE='/local/you/sim-rxe.img'
QEMU_SSH_PORT_RXE=9994
