# IONIC FreeBSD Device Drivers

## Build and Install (Kernel and Drivers)

### FreeBSD 11

A freebsd image is prepared at `srv14:/local/allenbh/freebsd-11.1.img.bak`.

The following has already been completed in the above image:

```sh
rsync_to root@bsd-host
ssh root@bsd-host
cd /usr/src
# clone bsd repo here, and then build
make -j4 buildworld NOCLEAN=yes
make installworld
make buildkernel KERNCONF=PENSANDO NOCLEAN=yes 
make installkernel KERNONF=PENSANDO
reboot
```

This image was prepared from `FreeBSD-11.1-RELEASE-amd64.qcow2.xz`, downloaded
from freebsd.org.  Serial console was enabled in the boot configuration.  The
FreeBSD 11 stable branch was checked out in /usr/src, configured with ofed
libraries, built and installed.  The kernel is configured with ofed, linuxkpi,
and Mellanox drivers for refernce.  The build artifacts are left in /usr/obj,
to speed up incremental builds.

### Ionic Drivers

Copy the drivers to the bsd host, and then build and install.  Normally, the
drivers would be built and installed according to the above prodecure.  To
speed up development, individual sources can be compiled in place.

I am not sure how or if the following procedure respects the build
configuration such as KERNCONF.  The build artifacts are produced in the
directory where make is invoked, instead of /usr/obj.

```sh
make -C /usr/src/sys/modules/ionic_eth
make -C /usr/src/sys/modules/ionic_rdma
make -C /usr/src/contrib/ofed/libionic
```

Load the eth driver.  Fix the addresses for your workspace.

```sh
kldload /usr/src/sys/modules/ionic/ionic.ko

# fix the ip address below for your workspace
ifconfig ionic0 172.22.0.2 netmask 255.255.255.0

# if it was not up by default, bring up
ifconfig ionic0 up

# recommended to set arp entries for now, for each peer (fix for your ws)
arp -s 172.22.0.1 0c:c4:7a:93:23:c7
arp -s 172.22.0.3 52:54:00:03:34:57
arp -s 172.22.0.8 52:54:00:08:34:57

# check connectivity to one of the peers
ping -c2 -W10 172.22.0.3
```

Load the rdma driver.  Fix the addresses for your workspace.

```sh
kldload /usr/src/sys/modules/ionic_rdma/ionic_rdma.ko
```

Try some user space rdma.
```sh
LIB='/usr/src/contrib/ofed/libionic/libionic.so'

LD_PRELOAD="$LIB" ibv_devinfo -v

# on peer: ibv_rc_pingpong -g1 -r3 -n2
LD_PRELOAD="$LIB" ibv_rc_pingpong -g1 -r3 -n2 172.22.0.3
```

### Development helper scripts

Scripts are provided to copy driver changes between the workspace and bsd host.

It is recommended to add a host entry in your .ssh/config for the bsd host, and
optionally, to set up ssh keys for authentication.  Change the port to match
your workspace configuration.

```txt
Host bsd
        HostName 127.0.0.1
	Port 4222
	User root
```

When changes are made in the workspace, update the bsd host, then follow steps
above to rebuild the drivers.

```sh
./rsync_to root@bsd
```

If changes are made directly on the bsd host, copy those source files back.
Only files that already exist in this workspace will be copied.  Touch new
files in the workspace to be copied.  Only files that exist in the host
workspace will be copied.

```sh
# if needed on host: touch file-to-copy
rsync_from root@bsd
```

The copy will overwrite any differences.  Be careful not to make changes in the
workspace and on the host at the same time.  The next copy in either direction
will discard anthing different at the destination of the copy.

If you delete changes on the bsd side, they will remain on the host.  To help
with this, a script is provided to delete driver files on the bsd host.  This
does not bring the workspace back to a pristine upstream state, just removes
files that we added.  It is recommended to follow immediately with rsync to.

```sh
./purge root@bsd
./rsync_to root@bsd
```

To see the current diff between the linux and bsd drivers, a diff script is
provided.  Some files were removed in the port, such as debugfs, and will not
appear in the diff.  This will be useful to compare rdma drivers.  Depending on
the diff between ethernet drivers for linux and bsd, we might drop the eth
driver from the dif.

```sh
./diff
```
