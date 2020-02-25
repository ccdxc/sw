# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
PKG_PREREQS:= all

package-clean:
	@rm -rf $(NICDIR)/../fake_root_target
	@rm -f  $(NICDIR)/nic.tgz
	@rm -f  $(NICDIR)/nic.tar

package: ${PKG_PREREQS}
ifeq (,$(filter $(PIPELINE),hello l2switch elektra))
	${MAKE} package-clean
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py --target host
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py --target test-utils
ifeq ($(ARCH),aarch64)
ifeq ($(FWTYPE),gold)
	ARCH=${ARCH} ${TOPDIR}/nic/tools/upgrade_version.sh
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py \
	--pipeline $(PIPELINE) --target gold $(PKG_ARGS)
else
	ARCH=${ARCH} ${TOPDIR}/nic/tools/upgrade_version.sh
	$(eval STRIP := )
    ifeq ($(PIPELINE),$(filter $(PIPELINE),apollo artemis apulu athena))
	    $(eval STRIP := --no-strip)
    endif
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py \
		--pipeline $(PIPELINE)$(FLAVOR) $(PKG_ARGS) $(STRIP)
	cd $(NICDIR) && $(NICDIR)/sdk/platform/mputrace/captrace.py gen_syms \
		--pipeline $(PIPELINE)
endif
else
	ARCH=${ARCH} ${TOPDIR}/nic/tools/upgrade_version.sh
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py \
		--pipeline $(PIPELINE) --target sim --no-strip
endif
endif

package-clean-debug-arm:
	@rm -f  $(TOPDIR)/debug_aarch64_iris.tgz
	@rm -f  $(TOPDIR)/debug_aarch64_iris.tar

package-clean-debug:
	@rm -f  $(TOPDIR)/debug_x86_64_iris.tgz
	@rm -f  $(TOPDIR)/debug_x86_64_iris.tar

package-clean-pegasus:
	@rm -f $(TOPDIR)/pegasus.tar
	@rm -f $(TOPDIR)/pegasus.tgz

package-pegasus: package-clean-pegasus pegasus
	 cd $(NICDIR)/../ && python $(NICDIR)/tools/package/package.py \
		--target pegasus --pipeline apulu --no-strip

package-debug: package-clean-debug
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py --target debug --no-strip

package-debug-arm: package-clean-debug-arm
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py --target debug-arm --no-strip

package-zebu: package-clean ${PKG_PREREQS}
ifneq ($(ARCH),aarch64)
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py --target zebu --no-strip
endif

package-arm-dev: package-clean ${PKG_PREREQS}
	ARCH=${ARCH} ${TOPDIR}/nic/tools/upgrade_version.sh
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py --target arm-dev --no-strip

package-haps-dbg: package-clean ${PKG_PREREQS}
	ARCH=${ARCH} ${TOPDIR}/nic/tools/upgrade_version.sh
	cd $(NICDIR)/../ && python2 $(NICDIR)/tools/package/package.py --target haps-dbg

.PHONY: package-storage-offload
package-storage-offload:
	@echo "Building Storage Offload driver package."
	${TOPDIR}/storage/offload/tools/drivers-linux.sh

.PHONY: package-ionic
package-ionic:
	@echo "Building IONIC driver package."
	${TOPDIR}/platform/tools/drivers-linux.sh
	@echo "Building IONIC driver package (Ethernet Only)."
	${TOPDIR}/platform/tools/drivers-linux-eth.sh

.PHONY: package-esx
package-esx:
	@echo "Building ESX IONIC driver package."
	${TOPDIR}/platform/tools/drivers-esx.sh 65
	${TOPDIR}/platform/tools/drivers-esx.sh 67

.PHONY: package-ipxe
package-ipxe:
	@echo "Building IPXE IONIC driver package."
	make PLATFORM=efi ARCH=x86_64 -C ${TOPDIR}/platform/drivers/pxe ionic-ipxepatch

.PHONY: package-drivers
package-drivers: package-ionic package-storage-offload package-freebsd package-esx package-ipxe

.PHONY: package-freebsd
package-freebsd:
	@echo "Building FreeBSD IONIC driver package."
	${TOPDIR}/platform/tools/package-freebsd.sh

.PHONY: release
release: ${PKG_PREREQS}
ifneq ($(ARCH),aarch64)
	${NICDIR}/tools/release.sh
endif

.PHONY: release-clean
release-clean:
	@rm -rf obj/release
