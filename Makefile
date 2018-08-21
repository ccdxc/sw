# Makefile for building packages

# :cached valid only for mac
platform=$(shell uname)
ifeq ($(platform),Darwin)
CACHEMOUNT := ":cached"
else
CACHEMOUNT :=
endif

# Lists excluded patterns to "go list"
EXCLUDE_PATTERNS := "generated|halproto|proto|model_sim|labels|fields|vendor|bazel|e2etests"

# Lists venice venice protos and all things auto generated.
TO_GEN := api api/labels api/fields venice/cmd/types/protos venice/cmd/grpc \
venice/ctrler/ckm/rpcserver/ckmproto venice/ctrler/npm/rpcserver/netproto \
venice/ctrler/tpm/rpcserver/protos  \
venice/ctrler/tsm/rpcserver/tsproto \
venice/collector/rpcserver/metric venice/utils/runtime/test \
venice/utils/apigen/annotations venice/orch \
venice/cmd/grpc/server/certificates/certapi \
venice/ctrler/evtsmgr/rpcserver/evtsmgrproto \
venice/evtsproxy/rpcserver/evtsproxyproto \
nic/agent/nmd/protos nic/agent/netagent/protos \
venice/utils/authn/radius                      \
venice/utils/objstore/client/mock 

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
							./nic/delphi/compiler/... \
							./vendor/layeh.com/radius/cmd/radius-dict-gen \

# Lists the binaries to be containerized
TO_DOCKERIZE := apigw apiserver vchub npm vcsim cmd collector nmd tpm netagent spyglass evtsmgr tsm evtsproxy aggregator vos citadel
ifneq ($(NOGOLANG),1)
# Install gopkgs
INSTALL := $(shell cd ${GOPATH}/src/github.com/pensando/sw && CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install ./vendor/github.com/haya14busa/gopkgs/cmd/gopkgs)
# Lists all go packages. Auto ignores vendor
GO_PKG := $(shell ${GOPATH}/bin/gopkgs -short 2>/dev/null | grep github.com/pensando/sw | egrep -v ${EXCLUDE_PATTERNS})
endif

GOIMPORTS_CMD := goimports -local "github.com/pensando/sw" -l
SHELL := /bin/bash
GOCMD = /usr/local/go/bin/go
PENS_AGENTS ?= 50
REGISTRY_URL ?= registry.test.pensando.io:5000
BUILD_CONTAINER ?= pens-bld:v0.12
UI_BUILD_CONTAINER ?= pens-ui-bld:v0.14
DIND_CONTAINER ?= pens-dind:v0.3
E2E_CONTAINER ?= pens-e2e:v0.3
TARGETS ?= ws-tools gen build
BUILD_CMD ?= bash -c  "make ${TARGETS}"
E2E_CONFIG ?= test/e2e/cluster/tb_config_dev.json
GIT_COMMIT ?= $(shell git rev-list -1 HEAD --abbrev-commit)
GIT_VERSION ?= $(shell git describe --dirty --always)
BUILD_DATE ?= $(shell date   +%Y-%m-%dT%H:%M:%S%z)
export GIT_COMMIT GIT_VERSION BUILD_DATE

default:
	$(MAKE) ws-tools
	$(MAKE) gen
	$(MAKE) checks
	$(MAKE) build
	$(MAKE) unit-test-cover
	$(MAKE) container-compile

# ws-tools installs all the binaries needed to generate, lint and build go sources
ws-tools:
	$(info +++ building WS tools)
	@CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install ${TO_INSTALL}

# checks ensure that the go code meets standards
checks: goimports-src golint-src govet-src

# gen does all the autogeneration. viz venice cli, proto sources. Ensure that any new autogen target is added to TO_GEN above
gen:
	@for c in ${TO_GEN}; do printf "\n+++++++++++++++++ Generating $${c} +++++++++++++++++\n"; PATH=$$PATH make -C $${c} || exit 1; done
	@${PWD}/venice/cli/scripts/gen.sh

# goimports-src formats the source and orders imports
# Target directories is needed only for goipmorts where it doesn't accept package names.
# Doing the go list here avoids an additional global go list
goimports-src:
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
golint-src:
	$(info +++ golint sources)
	@$(eval LINT := $(shell golint ${GO_PKG} | grep -v pb.go))
	@echo $(LINT)
	@test -z "$(LINT)"

# govet-src validates source code and reports suspicious constructs
govet-src:
	$(info +++ govet sources)
	@go vet -source ${GO_PKG}

.PHONY: build
# build installs all go binaries. Use VENICE_CCOMPILE_FORCE=1 to force a rebuild of all packages
build:
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install -ldflags '-X main.GitVersion=${GIT_VERSION} -X main.GitCommit=${GIT_COMMIT} -X main.BuildDate=${BUILD_DATE}' ${GO_PKG};\
	else \
		echo "+++ rebuilding all go sources";CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install -a '-X main.GitVersion=${GIT_VERSION} -X main.GitCommit=${GIT_COMMIT} -X main.BuildDate=${BUILD_DATE}' ${GO_PKG};\
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
	@#copy the agent binaries to netagent
	@cp -p ${PWD}/bin/cbin/nmd tools/docker-files/netagent/nmd
	@cp -p ${PWD}/bin/cbin/nevtsproxy tools/docker-files/netagent/nevtsproxy
	@# npm is special - The executable is called pen-npm since it conflicts with node.js npm. Hence copy it explicitly here
	@cp -p ${PWD}/bin/cbin/pen-npm tools/docker-files/npm/pen-npm
	@for c in $(TO_DOCKERIZE); do echo "+++ Dockerizing $${c}"; cp -p ${PWD}/bin/cbin/$${c} tools/docker-files/$${c}/$${c}; docker build --label org.label-schema.build-date="${BUILD_DATE}" --label org.label-schema.vcs-ref="${GIT_COMMIT}" --label org.label-schema.version="${GIT_VERSION}" --label org.label-schema.schema-version="1.0"  --rm --no-cache -t pen-$${c}:latest -f tools/docker-files/$${c}/Dockerfile tools/docker-files/$${c} ; done
	@tools/scripts/createImage.py
	@# the above script populates venice.json which needs to be 'installed' on the venice. Hence creation of installer is done at the end
	@# For now the installer is a docker container.
	@# In the future, this can be a shell script, rpm, curl script or whatever..
	docker build --label org.label-schema.build-date="${BUILD_DATE}" --label org.label-schema.vcs-ref="${GIT_COMMIT}" --label org.label-schema.version="${GIT_VERSION}" --label org.label-schema.schema-version="1.0" --rm --no-cache -t pen-install:latest -f tools/docker-files/install/Dockerfile tools/docker-files/install
	docker save -o bin/tars/pen-install.tar pen-install:latest
	#todo compress later in the release cycle with better compression level. As of now compression takes too much time for development
	cd bin && tar -cvf - tars/*.tar venice-install.json -C ../tools/scripts INSTALL.sh | gzip -1 -c > venice.tgz

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
	@cd tools/docker-files/build-container; docker build -t ${REGISTRY_URL}/${BUILD_CONTAINER} .
	@cd tools/docker-files/dind; docker build -t ${REGISTRY_URL}/${DIND_CONTAINER}  .
	@cd tools/docker-files/e2e; docker build -t ${REGISTRY_URL}/${E2E_CONTAINER} .
	@cd tools/docker-files/elasticsearch; docker build -t ${REGISTRY_URL}/elasticsearch-cluster:v0.3 .
	@cd tools/test-build; docker build -t ${REGISTRY_URL}/pen-test-build:v0.1 .

ui-container-helper:
	cat venice/ui/venice-sdk/npm-shrinkwrap.json | jq 'del(.dependencies."@pensando/swagger-ts-generator")' > tools/docker-files/ui-container/venice-sdk/npm-shrinkwrap.json
	cat venice/ui/venice-sdk/package.json | jq 'del(.dependencies."@pensando/swagger-ts-generator")' > tools/docker-files/ui-container/venice-sdk/package.json
	cp venice/ui/web-app-framework/package.json tools/docker-files/ui-container/web-app-framework/package.json
	cp venice/ui/web-app-framework/npm-shrinkwrap.json tools/docker-files/ui-container/web-app-framework/npm-shrinkwrap.json
	cat venice/ui/webapp/npm-shrinkwrap.json | jq 'del(.dependencies."web-app-framework")' > tools/docker-files/ui-container/webapp/npm-shrinkwrap.json
	cat venice/ui/webapp/package.json | jq 'del(.dependencies."web-app-framework")' > tools/docker-files/ui-container/webapp/package.json
	@cd tools/docker-files/ui-container; docker build --squash -t ${REGISTRY_URL}/${UI_BUILD_CONTAINER} .

# running as 'make container-compile UI_FRAMEWORK=1' will also force the UI-framework compilation
container-compile:
	@mkdir -p ${PWD}/bin/cbin
	@mkdir -p ${PWD}/bin/pkg
	@if [ ! -z ${UI_FRAMEWORK} ]; then \
		echo "+++ populating node_modules from cache for ui-framework";\
		echo docker run --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}"  --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} sh -c 'cp /usr/local/lib/web-app-framework/node_modules.tgz bin/web-app-framework-node-modules.tgz ' ; \
		docker run --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} sh -c 'cp /usr/local/lib/web-app-framework/node_modules.tgz bin/web-app-framework-node-modules.tgz ' ; \
		cd venice/ui/web-app-framework && tar zxf ../../../bin/web-app-framework-node-modules.tgz ;\
		docker run --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} make  ui-framework; \
	fi
	@if [ ! -f bin/webapp-node-modules.tgz ]; then \
		echo "+++ populating node_modules from cache for ui";\
		echo docker run --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1"  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} sh -c 'cp /usr/local/lib/venice-sdk/node_modules.tgz bin/venice-sdk-node-modules.tgz; cp /usr/local/lib/webapp/node_modules.tgz bin/webapp-node-modules.tgz' ; \
		docker run --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} sh -c 'cp /usr/local/lib/venice-sdk/node_modules.tgz bin/venice-sdk-node-modules.tgz; cp /usr/local/lib/webapp/node_modules.tgz bin/webapp-node-modules.tgz' ; \
		cd venice/ui/webapp && tar zxf ../../../bin/webapp-node-modules.tgz ;\
		cd ../venice-sdk && tar zxf ../../../bin/venice-sdk-node-modules.tgz ;\
	fi
	@if [ -z ${BYPASS_UI} ]; then \
	    echo "+++ building ui sources" ; \
		echo docker run --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} ; \
		docker run --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} ; \
		echo cp -r venice/ui/webapp/dist tools/docker-files/apigw ;\
		cp -r venice/ui/webapp/dist tools/docker-files/apigw ;\
	fi
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${BUILD_CMD} ; \
	else \
		echo "+++ rebuilding all go sources"; docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${BUILD_CMD} ;\
	fi; \
	echo "++ generating documentation"; \
	tools/scripts/gendocs.sh "${REGISTRY_URL}/${UI_BUILD_CONTAINER}"

shell:
		docker run -it --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} bash

container-qcompile:
	mkdir -p ${PWD}/bin/cbin
	mkdir -p ${PWD}/bin/pkg
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; \
		echo docker run -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER}; \
		docker run -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER}; \
	else \
		echo "+++ rebuilding all go sources"; \
		echo docker run -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER};\
		docker run -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER};\
	fi


unit-race-test:
	$(info +++ running go tests with race detector)
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go test -race ${GO_PKG}

unit-test-verbose:
	$(info +++ running go tests verbose)
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" $(GOCMD) test -v -p 1 ${GO_PKG}; \

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

# Target to create env for Venice-UI unit test
test-debug-ui: install_box
	box -t venice-ui:test-debug venice/ui/box.rb
	docker rm -f venice-ui || :
	docker run --name venice-ui -p 80:3000 -it -v "${PWD}:/go/src/github.com/pensando/sw" venice-ui:test-debug bash

# Target to run on Mac to start kibana docker, this connects to the Elastic running on vagrant cluster
start-kibana:
	docker run --name kibana \
	-e ELASTICSEARCH_URL=http://192.168.30.10:9200 \
	-e xpack.security.enabled=false \
	-e xpack.logstash.enabled=false \
	-e xpack.graph.enable=false \
	-e xpack.watcher.enabled=false \
	-e xpack.ml.enabled=false \
	-e xpack.monitoring.enabled=false \
	-p 127.0.0.1:5601:5601 -d registry.test.pensando.io:5000/kibana:6.3.0

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

dind-cluster:
	$(MAKE) dind-cluster-stop
	$(MAKE) container-compile
	$(MAKE) install
	./test/e2e/dind/do.py -configFile ${E2E_CONFIG}

dind-cluster-stop:
	./test/e2e/dind/do.py -delete

# Target to run venice e2e on mac using a dind environment. Uses Agent with its datapath mocked
e2e:
	$(MAKE) dind-cluster
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/cluster -configFile=/import/src/github.com/pensando/sw/${E2E_CONFIG}'
	# enable auto delete after e2e tests pass consistently. For now - keep the cluster running so that we can debug failures
	#./test/e2e/dind/do.py -delete

e2e-api:
	$(MAKE) dind-cluster
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/api -configFile=/import/src/github.com/pensando/sw/${E2E_CONFIG}'

e2e-ui:
	docker run --privileged  -it -l pens --network pen-dind-net --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1"  --rm -v ${PWD}:/import/src/github.com/pensando/sw -e "E2E_BASE_URL=http://192.168.30.10:9000" -w /import/src/github.com/pensando/sw/venice/ui/webapp ${REGISTRY_URL}/${UI_BUILD_CONTAINER} ng e2e -s=false

# Target to run venice e2e a dind environment. Uses real HAL as Agent Datapath and starts HAL with model
e2e-sanities:
	$(MAKE) container-compile
	$(MAKE) install
	$(MAKE) -C nic e2e-sanity-build
	./test/e2e/dind/do.py -configFile test/e2e/cluster/tb_config_sanities.json
	@stty sane
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/cluster -configFile=/import/src/github.com/pensando/sw/test/e2e/cluster/tb_config_sanities.json '
	# enable auto delete after e2e tests pass consistently. For now - keep the cluster running so that we can debug failures
	#./test/e2e/dind/do.py -delete

# Target to run telemetry e2e test cases
e2e-telemetry:
	$(MAKE) container-compile
	$(MAKE) install
	$(MAKE) -C nic e2e-sanity-build
	@echo "Setting up cluster..."
	./test/e2e/dind/do.py -configFile test/e2e/telemetry/tb_config.json
	@stty sane
	@echo "Running tests..."
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/telemetry -configFile=/import/src/github.com/pensando/sw/test/e2e/telemetry/tb_config.json'
	# enable auto delete after e2e tests pass consistently. For now - keep the cluster running so that we can debug failures
	#./test/e2e/dind/do.py -delete

ui-framework:
	npm version;
	cd venice/ui/web-app-framework && npm run pack

ui-venice-sdk:
	cd venice/ui/venice-sdk && npm install --prefer-cache pensando-swagger-ts-generator-1.1.29.tgz
	cd venice/ui/venice-sdk && node swaggerGen.js

ui:
	npm version;
	cd venice/ui/webapp && npm install --prefer-cache ../web-app-framework/dist/web-app-framework-0.0.0.tgz;
	$(MAKE) ui-venice-sdk
	cd venice/ui/webapp && ng build --e prod

ui-autogen:
	printf "\n+++++++++++++++++ Generating ui-autogen +++++++++++++++++\n";
	echo docker run --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} make ui-venice-sdk; \
	docker run --user $(shell id -u):$(shell id -g) -e "NOGOLANG=1" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} make ui-venice-sdk; \

VENICE_RELEASE_TAG := v0.1
venice-release:
	docker pull ${REGISTRY_URL}/${DIND_CONTAINER}
	docker save -o bin/pen-dind.tar ${REGISTRY_URL}/${DIND_CONTAINER}
	docker pull ${REGISTRY_URL}/${E2E_CONTAINER}
	docker save -o bin/pen-e2e.tar ${REGISTRY_URL}/${E2E_CONTAINER}
	cd nic/sim/naples && tar -cvf venice-sim.tar venice-bootstrap.sh  -C ../../../test/e2e dind -C ../topos/gs testbed.json -C ../../../bin venice.tgz pen-dind.tar pen-e2e.tar
	ln -f nic/sim/naples/venice-sim.tar tools/docker-files/venice/venice-sim.tar
	cp test/topos/gs/venice-conf.json nic/sim/naples/venice-conf.json
	cd tools/docker-files/venice/ && docker build -t pensando/venice:${VENICE_RELEASE_TAG} .

