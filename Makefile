# Makefile for building packages

# Lists excluded patterns to "go list"
EXCLUDE_PATTERNS := "generated|halproto|proto|model_sim|labels|fields|vendor"

# Lists venice venice protos and all things auto generated.
TO_GEN := api api/labels api/fields venice/cmd/types/protos venice/cmd/grpc \
venice/ctrler/ckm/rpcserver/ckmproto venice/ctrler/npm/rpcserver/netproto \
venice/collector/rpcserver/metric venice/utils/runtime/test \
venice/utils/apigen/annotations venice/orch \
venice/cmd/grpc/server/certificates/certapi \
venice/ctrler/evtsmgr/rpcserver/eventsproto \
nic/agent/nmd/protos nic/agent/netagent/protos

# Lists all the vendored packages that need to be installed prior to the build.
TO_INSTALL := ./vendor/github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway \
							./vendor/github.com/gogo/protobuf/protoc-gen-gofast \
							./vendor/github.com/golang/protobuf/protoc-gen-go \
							./venice/utils/apigen/protoc-gen-pensando \
							./vendor/golang.org/x/tools/cmd/goimports \
							./vendor/github.com/pensando/grpc-gateway/protoc-gen-swagger \
							./vendor/github.com/GeertJohan/go.rice/rice \
							./vendor/github.com/golang/mock/gomock \
							./vendor/github.com/golang/mock/mockgen \
							./vendor/github.com/golang/lint/golint \
							./vendor/github.com/golang/dep/cmd/dep \
							./asset-build/... \

# Lists the binaries to be containerized
TO_DOCKERIZE := apigw apiserver vchub npm vcsim cmd n4sagent collector nmd tpm
# Install gopkgs
INSTALL := $(shell cd ${GOPATH}/src/github.com/pensando/sw && CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install ./vendor/github.com/haya14busa/gopkgs/cmd/gopkgs)
# Lists all go packages. Auto ignores vendor
GO_PKG := $(shell ${GOPATH}/bin/gopkgs -short | grep github.com/pensando/sw | egrep -v ${EXCLUDE_PATTERNS})

GOIMPORTS_CMD := goimports -local "github.com/pensando/sw" -l
SHELL := /bin/bash
GOCMD = /usr/local/go/bin/go
PENS_AGENTS ?= 50
REGISTRY_URL ?= registry.test.pensando.io:5000

default:
	$(MAKE) ws-tools
	$(MAKE) checks
	$(MAKE) build
	$(MAKE) unit-test-cover

# ws-tools installs all the binaries needed to generate, lint and build go sources
ws-tools:
	$(info +++ building WS tools)
	@CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install ${TO_INSTALL}

# checks ensure that the go code meets standards
checks: gen goimports-src golint-src govet-src

# gen does all the autogeneration. viz venice cli, proto sources. Ensure that any new autogen target is added to TO_GEN above
gen:
	@for c in ${TO_GEN}; do printf "\n+++++++++++++++++ Generating $${c} +++++++++++++++++\n"; PATH=$$PATH make -C $${c} || exit 1; done
	@${PWD}/venice/cli/scripts/gen.sh

# goimports-src formats the source and orders imports
# Target directories is needed only for goipmorts where it doesn't accept package names.
# Doing the go list here avoids an additional global go list
goimports-src: gen
	$(info +++ goimports sources)
	$(eval GO_FILES=`gopkgs -short -f '{{.Dir}}/*.go' | grep github.com/pensando/sw | egrep -v ${EXCLUDE_PATTERNS}`)
ifdef JOB_ID
	@echo "Running in CI; checking goimports and fmt"
	@$(eval IMPRT := $(shell ${GOIMPORTS_CMD} ${GO_FILES}))
	@echo "goimports found errors in the following files(if any):"
	@echo $(IMPRT)
	@test -z "$(IMPRT)"
endif
	@${GOIMPORTS_CMD} -w ${GO_FILES}

# golint-src runs go linter and verifies code style matches golang recommendations
golint-src: gen
	$(info +++ golint sources)
	@$(eval LINT := $(shell golint ${GO_PKG} | grep -v pb.go))
	@echo $(LINT)
	@test -z "$(LINT)"

# govet-src validates source code and reports suspicious constructs
govet-src: gen
	$(info +++ govet sources)
	@go vet -source ${GO_PKG}

.PHONY: build
# build installs all go binaries. Use VENICE_CCOMPILE_FORCE=1 to force a rebuild of all packages
build:
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install ${GO_PKG}; \
	else \
		echo "+++ rebuilding all go sources";CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install -a ${GO_PKG};\
	fi

# VENICE_DEV=1 switches behavior of Venice components and libraries to test mode.
# CGO_LDFLAGS_ALLOW captures whiltelisted CGO Flags. This will avoid code injection
# attacks on go get.
# For rpckit, it changes the default connection mode from "TLS on" to "TLS off"
# See venice/utils/testenv
# unit-test-cover uses go test wrappers in scripts/report/report.go and runs coverage tests.
# this will return a non 0 error when coverage for a package is < 75.0%
unit-test-cover:
	$(info +++ running go tests)
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go run scripts/report/report.go ${GO_PKG}

c-start:
	@tools/scripts/create-container.sh startCluster

c-stop:
	@tools/scripts/create-container.sh stopCluster

install:
	@cp -p ${PWD}/bin/cbin/nmd tools/docker-files/n4sagent/nmd
	@for c in $(TO_DOCKERIZE); do cp -p ${PWD}/bin/cbin/$${c} tools/docker-files/$${c}/$${c}; tools/scripts/create-container.sh $${c}; done
	@tools/scripts/create-container.sh createBinContainerTarBall

deploy:
	$(MAKE) container-compile
	$(MAKE) install
	$(MAKE) c-start

cluster:
	$(MAKE) build
	$(MAKE) container-compile
	$(MAKE) install
	tools/scripts/startCluster.py -nodes ${PENS_NODES} -quorum ${PENS_QUORUM_NODENAMES}
	tools/scripts/startSim.py

cluster-stop:
	tools/scripts/startCluster.py -nodes ${PENS_NODES} -stop

cluster-restart:
	tools/scripts/startCluster.py -nodes ${PENS_NODES} -quorum ${PENS_QUORUM_NODENAMES}
	tools/scripts/startSim.py

clean: c-stop

helper-containers:
	@cd tools/docker-files/ntp; docker build -t ${REGISTRY_URL}/pens-ntp:v0.2 .
	@cd tools/docker-files/pens-base; docker build -t ${REGISTRY_URL}/pens-base:v0.2 .
	@cd tools/docker-files/build-container; docker build -t ${REGISTRY_URL}/pens-bld:v0.9 .
	@cd tools/docker-files/dind; docker build -t ${REGISTRY_URL}/pens-dind:v0.1 .
	@cd tools/docker-files/e2e; docker build -t ${REGISTRY_URL}/pens-e2e:v0.2 .
	@cd tools/docker-files/elasticsearch; docker build -t ${REGISTRY_URL}/elasticsearch-cluster:6.2.2 .

container-compile:
	mkdir -p ${PWD}/bin/cbin
	mkdir -p ${PWD}/bin/pkg
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; docker run --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw:cached -v/usr/local/include/google/protobuf:/usr/local/include/google/protobuf -v${PWD}/bin/pkg:/import/pkg:cached -v${PWD}/bin/cbin:/import/bin:cached -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/pens-bld:v0.9 sh -c "make ws-tools gen build"; \
	else \
		echo "+++ rebuilding all go sources"; docker run --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" -v/usr/local/include/google/protobuf:/usr/local/include/google/protobuf -v${PWD}:/import/src/github.com/pensando/sw:cached -v${PWD}/bin/pkg:/import/pkg:cached -v${PWD}/bin/cbin:/import/bin:cached -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/pens-bld:v0.9 sh -c "make ws-tools gen build";\
	fi

container-qcompile:
	mkdir -p ${PWD}/bin/cbin
	mkdir -p ${PWD}/bin/pkg
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; docker run --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v/usr/local/include/google/protobuf:/usr/local/include/google/protobuf -v${PWD}:/import/src/github.com/pensando/sw:cached -v${PWD}/bin/pkg:/import/pkg:cached -v${PWD}/bin/cbin:/import/bin:cached -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/pens-bld:v0.9; \
	else \
		echo "+++ rebuilding all go sources"; docker run --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" -v/usr/local/include/google/protobuf:/usr/local/include/google/protobuf -v${PWD}:/import/src/github.com/pensando/sw:cached -v${PWD}/bin/pkg:/import/pkg:cached -v${PWD}/bin/cbin:/import/bin:cached -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/pens-bld:v0.9;\
	fi


unit-race-test:
	$(info +++ running go tests with race detector)
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go test -race ${GO_PKG}

unit-test-verbose:
	$(info +++ running go tests verbose)
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" $(GOCMD) test -v -p 1 ${GO_PKG}; \

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

test-debug-ui: install_box
	box -t venice-ui:test-debug venice/ui/box.rb
	docker rm -f venice-ui || :
	docker run --name venice-ui -p 80:3000 -it -v "${PWD}:/go/src/github.com/pensando/sw" venice-ui:test-debug bash

# Target to run on Mac to start kibana docker, this connects to the Elastic running on vagrant cluster
start-kibana:
	docker run --name kibana -e ELASTICSEARCH_URL=http://192.168.30.10:9200 -e XPACK_SECURITY_ENABLED=false -p 127.0.0.1:5601:5601 -d kibana:5.4.1

# Target to run on Mac to stop kibana docker
stop-kibana:
	docker stop kibana
	docker rm kibana

# Dev environment targets
dev:
	scripts/bringup-dev.sh Vagrantfile.dev

arm64dev:
	scripts/bringup-dev.sh Vagrantfile.arm64dev

node:
	scripts/bringup-dev.sh Vagrantfile.node

dev-clean:
	scripts/cleanup-dev.sh

test-cluster:
	scripts/bringup-dev.sh Vagrantfile.e2e
	vagrant ssh node1 -- 'cd /import/src/github.com/pensando/sw/; make build && make cluster'
	scripts/setup_hostsim.sh
	vagrant ssh node1 -- '/import/src/github.com/pensando/sw/tools/scripts/startSim.py -nodes $$NAPLES_NODES -simnodes $$HOSTSIM_NODES -hostif eth2 -uplink eth3 -simif eth2 -gobin /import/bin/cbin -simbin /import/bin'

restart-test-cluster:
	vagrant ssh node1 -- '/import/src/github.com/pensando/sw/tools/scripts/startSim.py -nodes $$NAPLES_NODES -simnodes $$HOSTSIM_NODES -hostif eth2 -uplink eth3 -simif eth2 -gobin /import/bin/cbin -simbin /import/bin -stop'
	vagrant ssh node1 -- '/import/src/github.com/pensando/sw/tools/scripts/startCluster.py -nodes $$PENS_NODES -quorum $$PENS_QUORUM_NODENAMES'
	vagrant ssh node1 -- '/import/src/github.com/pensando/sw/tools/scripts/startSim.py -nodes $$NAPLES_NODES -simnodes $$HOSTSIM_NODES -hostif eth2 -uplink eth3 -simif eth2 -gobin /import/bin/cbin -simbin /import/bin'

e2e-test:
	vagrant ssh node1 -- 'cd /import/src/github.com/pensando/sw/test/e2e; sudo -E E2E_TEST=1 GOPATH=/import /usr/local/go/bin/go test -v .'

log-integ-test:
	vagrant ssh node2 -- 'cd /import/src/github.com/pensando/sw/test/integ/loginfra; sudo -E LOG_INTEG_TEST=1 GOPATH=/import /usr/local/go/bin/go test -v .'

start-agents:
	vagrant ssh node2 -- " /import/src/github.com/pensando/sw/tools/scripts/agentScale.py -num-agents $(PENS_AGENTS)  |  sudo bash "
stop-agents:
	vagrant ssh node2 -- " /import/src/github.com/pensando/sw/tools/scripts/agentScale.py -num-agents $(PENS_AGENTS) -stop |  sudo bash "

test-clean:
	scripts/cleanup-dev.sh

create-assets:
	bash scripts/create-assets.sh

toolchain-clean:
	if [ -d "/tool/toolchain" ]; then sudo umount /tool; sudo rm -rf /tool; fi

toolchain:
	if [ ! -d "/tool/toolchain" ]; then sudo rm -rf /tool; sudo mkdir -p /tool; sudo mount asic20.pensando.io:/export/tool /tool; fi

pull-assets:
	bash scripts/pull-assets.sh

e2e:
	$(MAKE) container-compile
	$(MAKE) install
	./test/e2e/dind/do.py -configFile test/e2e/cluster/tb_config.json
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/cluster -configFile=/import/src/github.com/pensando/sw/test/e2e/cluster/tb_config.json '
	# enable auto delete after e2e tests pass consistently. For now - keep the cluster running so that we can debug failures
	#./test/e2e/dind/do.py -delete
