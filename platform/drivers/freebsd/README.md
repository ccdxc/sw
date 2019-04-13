# IONIC FreeBSD Device Drivers

## Build and Install (Kernel and Drivers)

Copy or mount the platform/drivers/freebsd/usr/src directory onto a FreeBSD system.

Locate the FreeBSD OS source tree, and refer to that tree in the command to
build Pensando drivers.

Run the build command from the same directory as the script.

```sh
cd platform/drivers/freebsd/usr/src
env OS_DIR=/path/to/freebsd_src ./build.sh
```

### Prerequisite Configuration of the FreeBSD System (for RDMA)

Edit in `/etc/src.conf`:

```txt
WITH_OFED='yes'
```

Edit in `$OS_DIR/sys/amd64/conf/GENERIC`:

```txt
options OFED
options COMPAT_LINUXKPI
```

Run in `$OS_DIR`:

```sh
make buildkernel
make installkernel
make buildworld
make installworld
```
