
TOPDIR       := ${realpath ${CURDIR}/../../../}
GIT_COMMIT ?= $(shell git rev-list -1 HEAD --abbrev-commit)
GIT_VERSION ?= $(shell git describe --tags --dirty --always)
BUILD_DATE ?= $(shell date   +%Y-%m-%dT%H:%M:%S%z)
export GIT_COMMIT GIT_VERSION BUILD_DATE TOPDIR

base-iso:
	docker run --rm --privileged  -v ${CURDIR}:/pen \
		-v ${TOPDIR}/bin:/venice-bin \
		-v ${TOPDIR}/nic:/nic \
		registry.test.pensando.io:5000/pens-vinstall:v0.1 /pen/create-iso.sh

bash:
	echo commit: ${GIT_COMMIT} > PEN-VERSION
	echo version: ${GIT_VERSION} >> PEN-VERSION
	echo build-date: ${BUILD_DATE} >> PEN-VERSION
	docker run -it --rm --privileged  -v ${CURDIR}:/pen \
		-v ${TOPDIR}/bin:/venice-bin \
		-v ${TOPDIR}/nic:/nic \
		registry.test.pensando.io:5000/pens-vinstall:v0.1 bash

venice-iso:
	echo commit: ${GIT_COMMIT} > PEN-VERSION
	echo version: ${GIT_VERSION} >> PEN-VERSION
	echo build-date: ${BUILD_DATE} >> PEN-VERSION
	docker run --rm --privileged  -v ${CURDIR}:/pen \
		-v ${TOPDIR}/bin:/venice-bin \
		-v ${TOPDIR}/nic:/nic \
		-v ${TOPDIR}/tools:/tools \
		registry.test.pensando.io:5000/pens-vinstall:v0.1  /pen/add-venice-iso.sh
