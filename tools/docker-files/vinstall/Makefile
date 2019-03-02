
TOPDIR       := ${realpath ${CURDIR}/../../../}
GIT_COMMIT ?= $(shell git rev-list -1 HEAD --abbrev-commit)
GIT_VERSION ?= $(shell git describe --dirty --always)
BUILD_DATE ?= $(shell date   +%Y-%m-%dT%H:%M:%S%z)
export GIT_COMMIT GIT_VERSION BUILD_DATE TOPDIR

all:
	echo commit: ${GIT_COMMIT} > PEN-VERSION
	echo version: ${GIT_VERSION} >> PEN-VERSION
	echo build-date: ${BUILD_DATE} >> PEN-VERSION
	docker run --rm --privileged  -v ${CURDIR}/isolinux.cfg:/pen/isolinux.cfg \
		-v ${CURDIR}/create-iso.sh:/pen/create-iso.sh \
		-v ${CURDIR}/PEN-VERSION:/pen/PEN-VERSION \
		-v ${CURDIR}/venice-os.cfg:/pen/venice-os.cfg \
		-v ${CURDIR}/grub-efi.cfg:/pen/grub-efi.cfg \
		-v ${CURDIR}/venice-cleaninstall.sh:/pen/venice-cleaninstall.sh \
		-v ${TOPDIR}/bin:/venice-bin \
		registry.test.pensando.io:5000/pens-vinstall:v0.1 /pen/create-iso.sh

bash:
	echo commit: ${GIT_COMMIT} > PEN-VERSION
	echo version: ${GIT_VERSION} >> PEN-VERSION
	echo build-date: ${BUILD_DATE} >> PEN-VERSION
	docker run -it --rm --privileged  -v ${CURDIR}/isolinux.cfg:/pen/isolinux.cfg \
		-v ${CURDIR}/create-iso.sh:/pen/create-iso.sh \
		-v ${CURDIR}/PEN-VERSION:/pen/PEN-VERSION \
		-v ${CURDIR}/venice-os.cfg:/pen/venice-os.cfg \
		-v ${CURDIR}/grub-efi.cfg:/pen/grub-efi.cfg \
		-v ${CURDIR}/venice-cleaninstall.sh:/pen/venice-cleaninstall.sh \
		-v ${TOPDIR}/bin:/venice-bin \
		registry.test.pensando.io:5000/pens-vinstall:v0.1 bash 

venice-iso:
	echo commit: ${GIT_COMMIT} > PEN-VERSION
	echo version: ${GIT_VERSION} >> PEN-VERSION
	echo build-date: ${BUILD_DATE} >> PEN-VERSION
	docker run -it --rm --privileged  -v ${CURDIR}/isolinux.cfg:/pen/isolinux.cfg \
		-v ${CURDIR}/create-iso.sh:/pen/create-iso.sh \
		-v ${CURDIR}/add-venice-iso.sh:/pen/add-venice-iso.sh \
		-v ${CURDIR}/PEN-VERSION:/pen/PEN-VERSION \
		-v ${CURDIR}/venice-os.cfg:/pen/venice-os.cfg \
		-v ${CURDIR}/grub-efi.cfg:/pen/grub-efi.cfg \
		-v ${CURDIR}/venice-cleaninstall.sh:/pen/venice-cleaninstall.sh \
		-v ${TOPDIR}/bin:/venice-bin \
		registry.test.pensando.io:5000/pens-vinstall:v0.1  /pen/add-venice-iso.sh
