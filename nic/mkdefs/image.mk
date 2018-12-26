export CONFIG_DIR=/sw/nic/buildroot/
export BR2_CONFIG=$(CONFIG_DIR)/.config
export BINARIES_DIR=/sw/nic/buildroot/output/images

HOSTPATH=PATH="/sw/nic/buildroot/output/host/bin:/sw/nic/buildroot/output/host/sbin:/opt/coreutils-8.30/bin:/opt/rh/devtoolset-7/root/usr/bin:$(PATH)"

.PHONY: copy-overlay
copy-overlay: TARGET_DIR=/sw/nic/buildroot/output/target
copy-overlay:
	rsync -a --ignore-times --keep-dirlinks \
		--chmod=u=rwX,go=rX --exclude .empty --exclude '*~' \
		/sw/fake_root_target/aarch64/ $(TARGET_DIR)/
	PATH=/sw/nic/buildroot/output/host/bin:/sw/nic/buildroot/output/host/sbin:/opt/rh/devtoolset-7/root/usr/bin:/opt/coreutils-8.30/bin:$(PATH) /sw/nic/buildroot/board/pensando/capri/post-build.sh

.PHONY: build-rootfs
build-rootfs: export BR2_CONFIG=/sw/nic/buildroot/.config
build-rootfs: copy-overlay
	rm -rf /sw/nic/buildroot/output/build/buildroot-fs              
	mkdir -p /sw/nic/buildroot/output/build/buildroot-fs
	rsync -auH /sw/nic/buildroot/output/target/ /sw/nic/buildroot/output/build/buildroot-fs/target
	echo '#!/bin/sh' > /sw/nic/buildroot/output/build/buildroot-fs/fakeroot.fs
	echo "set -e" >> /sw/nic/buildroot/output/build/buildroot-fs/fakeroot.fs
	echo "chown -h -R 0:0 /sw/nic/buildroot/output/build/buildroot-fs/target" >> /sw/nic/buildroot/output/build/buildroot-fs/fakeroot.fs
	cat /sw/nic/buildroot/board/pensando/capri/users_table.txt >> /sw/nic/buildroot/output/build/buildroot-fs/users_table.txt
	printf '        sshd -1 sshd -1 * - - - SSH drop priv user\n\n' >> /sw/nic/buildroot/output/build/buildroot-fs/users_table.txt
	$(HOSTPATH) /sw/nic/buildroot/support/scripts/mkusers /sw/nic/buildroot/output/build/buildroot-fs/users_table.txt /sw/nic/buildroot/output/build/buildroot-fs/target >> /sw/nic/buildroot/output/build/buildroot-fs/fakeroot.fs
	cat /sw/nic/buildroot/board/pensando/capri/device_table.txt > /sw/nic/buildroot/output/build/buildroot-fs/device_table.txt
	printf '        /bin/busybox                     f 4755 0  0 - - - - -\n        /bin/ping        f 4755 0 0 - - - - -\n /bin/traceroute6 f 4755 0 0 - - - - -\n\n' >> /sw/nic/buildroot/output/build/buildroot-fs/device_table.txt
	echo "/sw/nic/buildroot/output/host/bin/makedevs -d /sw/nic/buildroot/output/build/buildroot-fs/device_table.txt /sw/nic/buildroot/output/build/buildroot-fs/target" >> /sw/nic/buildroot/output/build/buildroot-fs/fakeroot.fs
	printf '        tar cf /sw/nic/buildroot/output/build/buildroot-fs/rootfs.common.tar --numeric-owner --exclude=THIS_IS_NOT_YOUR_ROOT_FILESYSTEM -C /sw/nic/buildroot/output/build/buildroot-fs/target .\n' >> /sw/nic/buildroot/output/build/buildroot-fs/fakeroot.fs
	chmod a+x /sw/nic/buildroot/output/build/buildroot-fs/fakeroot.fs
	$(HOSTPATH) /sw/nic/buildroot/output/host/bin/fakeroot -- /sw/nic/buildroot/output/build/buildroot-fs/fakeroot.fs $(HOST_DIR)/bin/fakeroot -- $(FAKEROOT_SCRIPT)

.PHONY: build-squashfs
build-squashfs: build-rootfs
	rm -rf /sw/nic/buildroot/output/build/buildroot-fs/squashfs
	mkdir -p /sw/nic/buildroot/output/build/buildroot-fs/squashfs
	echo '#!/bin/sh' > /sw/nic/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	echo "set -e" >> /sw/nic/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	printf '        mkdir -p /sw/nic/buildroot/output/build/buildroot-fs/squashfs/target\n  tar xf /sw/nic/buildroot/output/build/buildroot-fs/rootfs.common.tar -C /sw/nic/buildroot/output/build/buildroot-fs/squashfs/target\n' >> /sw/nic/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	printf '   \n' >> /sw/nic/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	printf '        /sw/nic/buildroot/output/host/bin/mksquashfs /sw/nic/buildroot/output/build/buildroot-fs/squashfs/target /sw/nic/buildroot/output/images/rootfs.squashfs -noappend -processors 73 -comp gzip\n' >> /sw/nic/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	chmod a+x /sw/nic/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	$(HOSTPATH) /sw/nic/buildroot/output/host/bin/fakeroot -- /sw/nic/buildroot/output/build/buildroot-fs/squashfs/fakeroot

.PHONY: build-image
build-image: BINARIES_DIR=/sw/nic/buildroot/output/images
build-image: build-squashfs
	$(HOSTPATH) /sw/nic/buildroot/board/pensando/capri/post-image.sh
