
#
# To create a new buildroot minio image do inside the nic container:
# sudo cp /opt/rh/devtoolset-7/root/bin/make /bin/make
# sudo cp /opt/rh/devtoolset-7/root/bin/make /bin/gmake
# cd /sw/nic/buildroot
# make capri_defconfig
# make
#
# Once done, bump up the version in minio/VERSIONS and do:
# UPLOAD=1 make create-assets
#

export CONFIG_DIR=${NICDIR}/buildroot/
export BR2_CONFIG=$(CONFIG_DIR)/.config
export BINARIES_DIR=${NICDIR}/buildroot/output/images
export TARGET_DIR=${NICDIR}/buildroot/output/target

HOSTPATH=PATH="${NICDIR}/buildroot/output/host/bin:${NICDIR}/buildroot/output/host/sbin:/opt/coreutils/bin:/opt/rh/devtoolset-7/root/usr/bin:$(PATH)"
LIBRARYPATH=LD_LIBRARY_PATH=${NICDIR}/buildroot/output/host/lib:$(LD_LIBRARY_PATH)
FAKEROOTOPTS=-l ${NICDIR}/buildroot/output/host/lib/libfakeroot.so -f ${NICDIR}/buildroot/output/host/bin/faked

.PHONY: copy-overlay
copy-overlay:
	rsync -a --ignore-times --keep-dirlinks \
		--chmod=u=rwX,go=rX --exclude .empty --exclude '*~' \
		${TOPDIR}/fake_root_target/aarch64/ $(TARGET_DIR)/
	PATH=${NICDIR}/buildroot/output/host/bin:${NICDIR}/buildroot/output/host/sbin:/opt/rh/devtoolset-7/root/usr/bin:/opt/coreutils-8.30/bin:$(PATH) ${NICDIR}/buildroot/board/pensando/capri/post-build.sh
	PATH=${NICDIR}/buildroot/output/host/bin:${NICDIR}/buildroot/output/host/sbin:/opt/rh/devtoolset-7/root/usr/bin:/opt/coreutils-8.30/bin:$(PATH) ${NICDIR}/buildroot/board/pensando/scripts/post-fakeroot.sh

.PHONY: build-rootfs
build-rootfs: copy-overlay
	rm -rf ${NICDIR}/buildroot/output/build/buildroot-fs              
	mkdir -p ${NICDIR}/buildroot/output/build/buildroot-fs
	rsync -auH ${NICDIR}/buildroot/output/target/ ${NICDIR}/buildroot/output/build/buildroot-fs/target
	echo '#!/bin/sh' > ${NICDIR}/buildroot/output/build/buildroot-fs/fakeroot.fs
	echo "set -e" >> ${NICDIR}/buildroot/output/build/buildroot-fs/fakeroot.fs
	echo "chown -h -R 0:0 ${NICDIR}/buildroot/output/build/buildroot-fs/target" >> ${NICDIR}/buildroot/output/build/buildroot-fs/fakeroot.fs
	cat ${NICDIR}/buildroot/board/pensando/capri/users_table.txt >> ${NICDIR}/buildroot/output/build/buildroot-fs/users_table.txt
	printf '        sshd -1 sshd -1 * - - - SSH drop priv user\n\n' >> ${NICDIR}/buildroot/output/build/buildroot-fs/users_table.txt
	$(HOSTPATH) ${NICDIR}/buildroot/support/scripts/mkusers ${NICDIR}/buildroot/output/build/buildroot-fs/users_table.txt ${NICDIR}/buildroot/output/build/buildroot-fs/target >> ${NICDIR}/buildroot/output/build/buildroot-fs/fakeroot.fs
	cat ${NICDIR}/buildroot/board/pensando/capri/device_table.txt > ${NICDIR}/buildroot/output/build/buildroot-fs/device_table.txt
	printf '        /bin/busybox                     f 4755 0  0 - - - - -\n        /bin/ping        f 4755 0 0 - - - - -\n /bin/traceroute6 f 4755 0 0 - - - - -\n\n' >> ${NICDIR}/buildroot/output/build/buildroot-fs/device_table.txt
	echo "${NICDIR}/buildroot/output/host/bin/makedevs -d ${NICDIR}/buildroot/output/build/buildroot-fs/device_table.txt ${NICDIR}/buildroot/output/build/buildroot-fs/target" >> ${NICDIR}/buildroot/output/build/buildroot-fs/fakeroot.fs
	printf '        tar cf ${NICDIR}/buildroot/output/build/buildroot-fs/rootfs.common.tar --numeric-owner --exclude=THIS_IS_NOT_YOUR_ROOT_FILESYSTEM -C ${NICDIR}/buildroot/output/build/buildroot-fs/target .\n' >> ${NICDIR}/buildroot/output/build/buildroot-fs/fakeroot.fs
	chmod a+x ${NICDIR}/buildroot/output/build/buildroot-fs/fakeroot.fs
	$(LIBRARYPATH) $(HOSTPATH) ${NICDIR}/buildroot/output/host/bin/fakeroot $(FAKEROOTOPTS) -- ${NICDIR}/buildroot/output/build/buildroot-fs/fakeroot.fs

.PHONY: build-squashfs
build-squashfs: build-rootfs
	rm -rf ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs
	mkdir -p ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs
	echo '#!/bin/sh' > ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	echo "set -e" >> ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	printf '        mkdir -p ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/target\n  tar xf ${NICDIR}/buildroot/output/build/buildroot-fs/rootfs.common.tar -C ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/target\n' >> ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	printf '   \n' >> ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	printf '        ${NICDIR}/buildroot/output/host/bin/mksquashfs ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/target ${NICDIR}/buildroot/output/images/rootfs.squashfs -noappend -processors 8 -comp gzip\n' >> ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	chmod a+x ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/fakeroot
	$(LIBRARYPATH) $(HOSTPATH) ${NICDIR}/buildroot/output/host/bin/fakeroot $(FAKEROOTOPTS) -- ${NICDIR}/buildroot/output/build/buildroot-fs/squashfs/fakeroot

.PHONY: build-image
build-image: build-squashfs
	$(HOSTPATH) ${NICDIR}/buildroot/board/pensando/capri/post-image.sh

.PHONY: firmware
firmware: build-image
	cp ${NICDIR}/buildroot/output/images/naples_fw.tar ${NICDIR}/naples_fw.tar
