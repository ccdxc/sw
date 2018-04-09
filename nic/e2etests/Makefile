#################################################
################### DOCKER ######################
#################################################

REGISTRY = registry.test.pensando.io:5000

shell: build-runtime-image
        docker run -it --rm --privileged --name e2e_app pensando/nic/e2e bash

build-runtime-image: install_box
        if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic/e2e:1.0; fi
        BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box -t pensando/nic/e2e  box.rb

install_box:
        @if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

# make a trial dependencies image. pass RELEASE=1 or run `make deps-release` to
# do a release build.
deps: install_box
        RELEASE=${RELEASE} BOX_INCLUDE_ENV="RELEASE" box -t '$(REGISTRY)/pensando/nic/e2e:1.0' box.rb

# make a release build of the dependencies image
deps-release:
        RELEASE=1 make deps

# make a shell based on the local build of the dependencies image, for testing
deps-test-shell:
        NO_PULL=1 make shell

# do a test build with a local build of the dependencies image.
deps-test-build:
        NO_PULL=1 make build

