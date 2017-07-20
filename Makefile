# Makefile for building packages

EXCLUDE_DIRS := bin docs Godeps vendor scripts grpc-gateway
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
	CGO_ENABLED=0 go install -v $(TO_BUILD)

c-start:
	@tools/scripts/create-container.sh startCluster

c-stop:
	@tools/scripts/create-container.sh stopCluster

install:
	@docker run -it --rm -v${PWD}/../../..:/import/src -v${PWD}/bin:/import/bin srv1.pensando.io:5000/pens-bld strip /import/bin/cmd /import/bin/apigw /import/bin/apiserver
	@cp ${PWD}/bin/cmd tools/docker-files/pencmd/target/usr/bin/pen-cmd
	@cp ${PWD}/bin/apigw tools/docker-files/apigw/apigw
	@cp ${PWD}/bin/apiserver tools/docker-files/apiserver/apiserver
	@chmod 755 tools/docker-files/pencmd/target/usr/bin/pen-cmd tools/docker-files/apigw/apigw tools/docker-files/apiserver/apiserver
	@tools/scripts/create-container.sh createBaseContainer
	@tools/scripts/create-container.sh createApiGwContainer
	@tools/scripts/create-container.sh createApiSrvContainer
	@tools/scripts/create-container.sh createBinContainerTarBall

deploy:
	$(MAKE) container-compile
	$(MAKE) install
	$(MAKE) c-start

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
	box box.rb
	docker run -it --rm pensando/sw

docker-test-debug: install_box
	box box-base.rb
	docker run -it -v "${PWD}:/go/src/github.com/pensando/sw" pensando/sw:dependencies

# Dev environment targets
dev:
	scripts/bringup-dev.sh Vagrantfile.dev

arm64dev:
	scripts/bringup-dev.sh Vagrantfile.arm64dev

node:
	scripts/bringup-dev.sh Vagrantfile.node

dev-clean:
	scripts/cleanup-dev.sh
