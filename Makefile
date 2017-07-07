# Makefile for building packages

EXCLUDE_DIRS := bin docs Godeps vendor scripts
PKG_DIRS := $(filter-out $(EXCLUDE_DIRS),$(subst /,,$(sort $(dir $(wildcard */)))))
TO_BUILD := ./utils/rpckit/... ./utils/kvstore/... ./utils/runtime/... ./agent/... ./cmd/... ./utils/certs ./utils/mdns ./utils/sysif \
  ./apiserver/... ./apigw/... ./utils/log/... ./orch/... ./utils/resource/...
GOFMT_CMD := gofmt -s -l
GOVET_CMD := go tool vet
SHELL := /bin/bash

default: build unit-test cover

deps:
	@( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/golang/lint/golint/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/kardianos/govendor/ && go install )

gofmt-src: $(PKG_DIRS)
	$(info +++ gofmt $(PKG_DIRS))
	@for dir in $?; do $(GOFMT_CMD) $${dir} | grep "go"; [[ $$? -ne 0 ]] || exit 1; done

golint-src: $(PKG_DIRS)
	$(info +++ golint $(TO_BUILD))
	@scripts/validate-lint.sh $(TO_BUILD)

govet-src: $(PKG_DIRS)
	$(info +++ govet $(PKG_DIRS))
	@for dir in $?; do $(GOVET_CMD) $${dir} || exit 1;done

checks: gofmt-src golint-src govet-src

build: deps ws-tools checks
	$(info +++ go install $(TO_BUILD))
	go install -v $(TO_BUILD)

c-start:
	@tools/scripts/create-container.sh startCluster

c-stop:
	@tools/scripts/create-container.sh stopCluster

install:
	@cp $(GOPATH)/bin/cmd tools/docker-files/pencmd/target/usr/bin/pen-cmd
	@tools/scripts/create-container.sh createBaseContainer
	@tools/scripts/create-container.sh createBinContainerTarBall

deploy: container-compile install c-start

clean: c-stop

build-container:
	@cd tools/docker-files/build-container; docker build -t srv1.pensando.io:5000/pens-bld .

container-compile:
	docker run -it --rm -v${PWD}/../../..:/import/src -v${PWD}/bin:/import/bin srv1.pensando.io:5000/pens-bld

ws-tools:
	$(info +++ building WS tools)
	@( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/golang/protobuf/protoc-gen-go/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protoc-gen-gofast/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/GeertJohan/go.rice/rice/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/ && go install ) && \
	( cd $(GOPATH)/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/protoc-gen-swagger/ && go install )

unit-test:
	$(info +++ go test $(TO_BUILD))
	go test $(TO_BUILD)

unit-test-verbose:
	$(info +++ go test $(TO_BUILD))
	go test -v $(TO_BUILD)

cover:
	$(info +++ go test -cover -tags test $(TO_BUILD))
	@scripts/test-coverage.sh $(TO_BUILD)

ci-testrun:
	tools/scripts/CI.sh

ci-test:
	make dev
	make ci-testrun
	make dev-clean

# Dev environment targets
dev:
	scripts/bringup-dev.sh Vagrantfile.dev

arm64dev:
	scripts/bringup-dev.sh Vagrantfile.arm64dev

node:
	scripts/bringup-dev.sh Vagrantfile.node

dev-clean:
	scripts/cleanup-dev.sh
