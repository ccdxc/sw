shell: build-image
	docker run -it --rm --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/dol -v ${PWD}/../nic:/sw/nic pensando/dol bash

test: build-image
	docker run -it --rm -v ${PWD}:/sw/dol -v ${PWD}/../nic:/sw/nic pensando/dol bash -c './ut_run.sh'

build: build-image
	docker run -it --rm -v ${PWD}:/sw/dol -v ${PWD}/../nic:/sw/nic pensando/dol sh -c 'make -j$$(grep -c processor /proc/cpuinfo)'

clean-docker: build-image
	docker run -it --rm -v ${PWD}:/sw/dol -v ${PWD}/../nic:/sw/nic pensando/dol make clean

build-image: install_box
	cd .. && BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box dol/box.rb

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi
