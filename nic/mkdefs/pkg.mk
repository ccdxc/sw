# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
PKG_PREREQS:= all

platform-clean:
    ifeq ($(ARCH),aarch64)
		cd ${TOPDIR}/platform && make BUILD_ARCHES=aarch64 clean
    else
		cd ${TOPDIR}/platform && make BUILD_ARCHES=x86_64 clean
    endif

package-clean:
	@rm -rf $(NICDIR)/../fake_root_target
	@rm -f  $(NICDIR)/nic.tgz
	@rm -f  $(NICDIR)/nic.tar

package: ${PKG_PREREQS}
ifneq ($(PIPELINE),hello)
	${MAKE} package-clean
	cd $(NICDIR)/../ && python $(NICDIR)/tools/package/package.py --target host
    ifeq ($(ARCH),aarch64)
		${MAKE} -j1 BUILD_ARCHES=aarch64 -C ${TOPDIR}/platform
		ARCH=${ARCH} ${TOPDIR}/nic/tools/update_version.sh
		cd $(NICDIR)/../ && python $(NICDIR)/tools/package/package.py
    else
        ifeq ($(PIPELINE),iris)
			${MAKE} -j1 BUILD_ARCHES=x86_64 -C ${TOPDIR}/platform
        endif
		ARCH=${ARCH} ${TOPDIR}/nic/tools/update_version.sh
		cd $(NICDIR)/../ && python $(NICDIR)/tools/package/package.py \
			--pipeline $(PIPELINE) --target sim --no-strip
    endif
endif

package-clean-debug-arm:
	@rm -f  $(TOPDIR)/debug_aarch64_iris.tgz
	@rm -f  $(TOPDIR)/debug_aarch64_iris.tar

package-clean-debug:
	@rm -f  $(TOPDIR)/debug_x86_64_iris.tgz
	@rm -f  $(TOPDIR)/debug_x86_64_iris.tar

package-debug: package-clean-debug
	cd $(NICDIR)/../ && python $(NICDIR)/tools/package/package.py --target debug --no-strip

package-debug-arm: package-clean-debug-arm
	cd $(NICDIR)/../ && python $(NICDIR)/tools/package/package.py --target debug-arm --no-strip

package-zebu: package-clean ${PKG_PREREQS}
    ifneq ($(ARCH),aarch64)
		cd $(NICDIR)/../ && python $(NICDIR)/tools/package/package.py --target zebu --no-strip
    endif

package-arm-dev: package-clean ${PKG_PREREQS}
	${MAKE} -j1 BUILD_ARCHES=aarch64 OPT=-g -C ${TOPDIR}/platform
	ARCH=${ARCH} ${TOPDIR}/nic/tools/update_version.sh
	cd $(NICDIR)/../ && python $(NICDIR)/tools/package/package.py --target arm-dev --no-strip

package-haps-dbg: package-clean ${PKG_PREREQS}
	${MAKE} -j1 BUILD_ARCHES=aarch64 OPT=-g -C ${TOPDIR}/platform
	ARCH=${ARCH} ${TOPDIR}/nic/tools/update_version.sh
	cd $(NICDIR)/../ && python $(NICDIR)/tools/package/package.py --target haps-dbg

.PHONY: package-storage-offload
package-storage-offload:
	@echo "Building Storage Offload driver package."
	${TOPDIR}/storage/offload/tools/drivers-linux.sh

.PHONY: package-ionic
package-ionic:
	@echo "Building IONIC driver package."
	${TOPDIR}/platform/tools/drivers-linux.sh

.PHONY: package-drivers
package-drivers: package-ionic package-storage-offload package-freebsd

.PHONY: package-freebsd
package-freebsd:
	@echo "Building FreeBSD IONIC driver package."
	${TOPDIR}/platform/tools/package-freebsd.sh

.PHONY: release
release: ${PKG_PREREQS}
    ifneq ($(ARCH),aarch64)
	    ${NICDIR}/tools/release.sh
    endif

release-clean:
	@rm -rf obj/release
