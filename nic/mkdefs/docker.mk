#################################################
################### DOCKER ######################
#################################################
CUR_DIR:=$(shell pwd)
SW_DIR:=$(shell dirname ${CUR_DIR})
CUR_USER:=$(shell whoami)
CUR_TIME:=$(shell date +%Y-%m-%d_%H.%M.%S)
CONTAINER_NAME:=${CUR_USER}_${CUR_TIME}
NIC_CONTAINER_VERSION:=1.41

# get a shell with the dependencies image loaded, with the host filesystem mounted.
ifeq ($(USER),)
docker/shell:
	docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw  -v /vol/builds:/vol/builds -w /sw/nic pensando/nic bash
else
docker/shell: docker/build-shell-image
	docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw  -v /vol/builds:/vol/builds -w /sw/nic pensando/nic su -l $(CUR_USER)
endif

ifeq ($(PIPELINE),$(filter $(PIPELINE),apulu athena))
docker/customer-shell: docker/build-customer-shell-image
	docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw  -v /vol/builds:/vol/builds -w /sw/nic pensando/nic su -l $(CUR_USER)
else
docker/customer-shell:
	echo "pipeline unsupported"
endif 

docker/background-shell: docker/build-shell-image
	docker run -dit --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v /vol/builds:/vol/builds -v $(SW_DIR):/sw  -w /sw/nic pensando/nic su -l $(CUR_USER)

docker/build-shell-image: docker/install_box
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:${NIC_CONTAINER_VERSION}; fi
	cd .. && BOX_INCLUDE_ENV="USER USER_UID USER_GID GROUP_NAME" USER_UID=$$(id -u) USER_GID=$$(id -g) GROUP_NAME=$$(id -gn) box -t pensando/nic nic/box.rb

docker/build-customer-shell-image: docker/install_box
	echo "custom install_box"
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:${NIC_CONTAINER_VERSION}; fi
	cd .. && BOX_INCLUDE_ENV="USER USER_UID USER_GID GROUP_NAME" USER_UID=$$(id -u) USER_GID=$$(id -g) GROUP_NAME=$$(id -gn) box -t pensando/nic nic/apollo/tools/$(PIPELINE)/customer-docker/box.rb

docker/coverage: docker/build-runtime-image
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -v /home/asic/tools:/home/asic/tools -w /sw/nic pensando/nic  su  -l $(CUR_USER)  -c 'tools/coverage_script.sh'

docker/jenkins-coverage: docker/build-runtime-image
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -v /home/asic/tools:/home/asic/tools -w /sw/nic pensando/nic su -l $(CUR_USER) -c 'HARDWARE_TESTBED=${HARDWARE_TESTBED_COPIED}  EXTRA_ARGS="${EXTRA_ARGS}" ${COV_SCRIPT}'

docker/firmware: docker/build-runtime-image
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -w /sw/nic pensando/nic su -l $(CUR_USER) -c 'cd /usr/src/github.com/pensando/sw/nic && make JOB_ID=1 PIPELINE=${PIPELINE} BUILD_SW_VERSION=${UPSTREAM_BUILD_TAG} BUILD_BR_VERSION=${UPSTREAM_BUILD_TAG} jobd/firmware'

docker/naples-sim: docker/build-runtime-image
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -w /sw/nic pensando/nic su -l $(CUR_USER) -c 'cd /usr/src/github.com/pensando/sw/nic && make JOB_ID=1 PIPELINE=${PIPELINE} BUILD_SW_VERSION=${UPSTREAM_BUILD_TAG} BUILD_BR_VERSION=${UPSTREAM_BUILD_TAG} jobd/e2e/naples-sim-image'

docker/coverage-shell: docker/build-runtime-image
	docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -v /home/asic/tools:/home/asic/tools -w /sw/nic pensando/nic  su -l $(CUR_USER)

docker/e2e-sanity-build: docker/build-runtime-image
	docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -w /sw/nic pensando/nic bash -c 'make'

docker/e2e-sanity-hal-bringup:
	@bash -c '(./tools/start-model.sh &) && ./tools/start-hal.sh'

docker/e2e-sanity-tls-build: docker/build-runtime-image-skip-box
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -w /sw/nic pensando/nic bash -c 'make pull-assets && make nic && make sanity-tests && ./run.py  --topo proxy --feature proxy --config-only --e2e-tls-dol ; ret=$? ; make sanity_logs ; exit $(ret)'

# run `make gtest` with the dependencies image.
docker/test: docker/build-runtime-image
	docker run -it --rm  --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -w /sw/nic pensando/nic bash -c 'make gtest'

# run a build with the dependencies image.
docker/build: docker/build-runtime-image
	docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -w /sw/nic pensando/nic bash -c 'make gtest'

docker/clean-docker: docker/build-runtime-image
	docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -w /sw/nic pensando/nic bash -c 'make clean'

REGISTRY = registry.test.pensando.io:5000

docker/build-runtime-image: docker/install_box
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:${NIC_CONTAINER_VERSION}; fi
	cd .. && BOX_INCLUDE_ENV="NO_COPY USER USER_UID USER_GID GROUP_NAME" NO_COPY=1 USER_UID=$$(id -u) USER_GID=$$(id -g) GROUP_NAME=$$(id -gn) box -t pensando/nic nic/box.rb

docker/build-runtime-image-skip-box:
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:${NIC_CONTAINER_VERSION}; fi
	cd .. && BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box -t pensando/nic nic/box.rb


docker/install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

# make a trial dependencies image. pass RELEASE=1 or run `make deps-release` to
# do a release build.
docker/deps: docker/install_box
	cd .. && RELEASE=${RELEASE} BOX_INCLUDE_ENV="RELEASE" box -t '$(REGISTRY)/pensando/nic:${NIC_CONTAINER_VERSION}' nic/box-deps.rb

# make a release build of the dependencies image
docker/deps-release:
	RELEASE=1 make docker/deps

# make a shell based on the local build of the dependencies image, for testing
docker/deps-test-shell:
	NO_PULL=1 make docker/shell

# do a test build with a local build of the dependencies image.
docker/deps-test-build:
	NO_PULL=1 make docker/build

docker/pegasus:
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -w /sw/nic pensando/nic su -l $(CUR_USER) -c 'make PIPELINE=apulu pegasus && cd .. && python ./nic/tools/package/package.py --target pegasus --pipeline apulu --no-strip'
