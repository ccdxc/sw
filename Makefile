# Makefile for building packages

EXCLUDE_DIRS := "bazel-cache|vendor|generated|model_sim|bin|Godeps|scripts"
TO_BUILD := ./venice/utils/... ./nic/agent/... ./venice/cmd/... ./venice/apigw/... ./venice/orch/... \
./venice/apiserver/... ./venice/globals/... ./venice/ctrler/... ./test/... ./api/ ./api/hooks/... \
./api/listerwatcher/... ./api/cache/... ./api/integration/... ./venice/exe/venice/... ./venice/collector/... \
./venice/cli/...

TO_DOCKERIZE := apigw apiserver vchub npm vcsim cmd n4sagent
TO_STRIP := $(addprefix /import/bin/, ${TO_DOCKERIZE})

GOFMT_CMD := gofmt -s -l
GOVET_CMD := go vet
SHELL := /bin/bash
GOCMD = /usr/local/go/bin/go
PENS_AGENTS ?= 50
REGISTRY_URL ?= registry.test.pensando.io:5000

default: build unit-test cover

deps:
	@( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/golang/lint/golint/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/kardianos/govendor/ && go install )

gofmt-src:
	$(info +++ gofmt $(PKG_DIRS))
ifdef JOB_ID
	@echo "Running in CI; checking go fmt"
	@$(eval FMT=`find . -name '*.go' -print | egrep -v ${EXCLUDE_DIRS} | xargs ${GOFMT_CMD}`)
	@echo "go fmt found errors in the following files(if any):"
	@echo $(FMT)
	@test -z "$(FMT)"
endif
	@find . -name '*.go' -print | egrep -v ${EXCLUDE_DIRS} | xargs ${GOFMT_CMD} -w

golint-src:
	$(info +++ golint $(TO_BUILD))
	@scripts/validate-lint.sh $(TO_BUILD)

govet-src:
	$(info +++ govet $(PKG_DIRS))
	@${GOVET_CMD} $(shell go list -e ./... | egrep -v ${EXCLUDE_DIRS})

checks: gofmt-src golint-src govet-src

# pregen target generates code that is needed by other binaries
pregen:
	@$(PWD)/venice/cli/scripts/gen.sh

qbuild:
	$(info +++ go install $(TO_BUILD))
	go install -v $(TO_BUILD)

build:
	$(MAKE) deps
	$(MAKE) ws-tools
	$(MAKE) checks
	$(MAKE) qbuild

c-start:
	@tools/scripts/create-container.sh startCluster

c-stop:
	@tools/scripts/create-container.sh stopCluster

install:
	@docker run --rm -v${PWD}/../../..:/import/src -v${PWD}/bin/cbin:/import/bin ${REGISTRY_URL}/pens-bld:v0.4 strip ${TO_STRIP}
	@for c in $(TO_DOCKERIZE); do cp -p ${PWD}/bin/cbin/$${c} tools/docker-files/$${c}/$${c}; tools/scripts/create-container.sh $${c}; done
	@tools/scripts/create-container.sh createBinContainerTarBall

qdeploy:
	$(MAKE) container-qcompile
	$(MAKE) install
	$(MAKE) c-start

deploy:
	$(MAKE) container-compile
	$(MAKE) install
	$(MAKE) c-start

cluster:
	$(MAKE) qbuild
	$(MAKE) container-qcompile
	$(MAKE) install
	tools/scripts/startCluster.py -nodes ${PENS_NODES} -quorum ${PENS_QUORUM_NODENAMES}
	tools/scripts/startSim.py

cluster-stop:
	tools/scripts/startCluster.py -nodes ${PENS_NODES} -stop

clean: c-stop

helper-containers:
	@cd tools/docker-files/ntp; docker build -t ${REGISTRY_URL}/pens-ntp:v0.2 .
	@cd tools/docker-files/pens-base; docker build -t ${REGISTRY_URL}/pens-base:v0.1 .
	@cd tools/docker-files/build-container; docker build -t ${REGISTRY_URL}/pens-bld:v0.4 .

container-qcompile:
	mkdir -p ${PWD}/bin/cbin
	docker run --rm -v${PWD}/../../..:/import/src -v${PWD}/bin/pkg:/import/pkg -v${PWD}/bin/cbin:/import/bin ${REGISTRY_URL}/pens-bld:v0.4 make qbuild

container-compile:
	mkdir -p ${PWD}/bin/cbin
	docker run --rm -v${PWD}/../../..:/import/src -v${PWD}/bin/pkg:/import/pkg -v${PWD}/bin/cbin:/import/bin ${REGISTRY_URL}/pens-bld:v0.4

ws-tools:
	$(info +++ building WS tools)
	@( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/golang/protobuf/protoc-gen-go/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protoc-gen-gofast/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/venice/utils/apigen/protoc-gen-pensando && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/GeertJohan/go.rice/rice/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/protoc-gen-swagger/ && go install )

unit-test:
	$(info +++ go test $(TO_BUILD))
	$(GOCMD) test -p 1 $(TO_BUILD); \

unit-race-test:
	$(info +++ go test -race $(TO_BUILD))
	go test -race $(TO_BUILD)

unit-test-verbose:
	$(info +++ go test $(TO_BUILD))
	$(GOCMD) test -v -p 1 $(TO_BUILD); \

cover:
	$(info +++ go test -cover -tags test $(TO_BUILD))
	@scripts/test-coverage.sh $(TO_BUILD)

ci-testrun:
	tools/scripts/CI.sh

ci-test:
	make dev
	make ci-testrun
	make dev-clean

jobd-test: build unit-test-verbose cover

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

clean-grpc:
	rm -rf grpc-gateway

update-grpc: clean-grpc
	@if git remote show origin | grep -q git@github.com; \
	then \
		echo Using ssh, trying git submodule...; \
		git submodule update --init; \
	else \
		echo Using https, trying direct clone...; \
		git clone https://github.com/pensando/grpc-gateway; \
	fi

docker-test: install_box
	NO_COPY=1 box box.rb
	docker run -it -v "${PWD}:/go/src/github.com/pensando/sw" --rm pensando/sw

test-debug: install_box
	NO_COPY=1 box box-base.rb
	docker run -it -v "${PWD}:/go/src/github.com/pensando/sw" pensando/sw:dependencies

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
	vagrant ssh node1 -- 'cd /import/src/github.com/pensando/sw/; make qbuild && make cluster'
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

pull-assets:
	bash scripts/pull-assets.sh
