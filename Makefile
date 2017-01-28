# Makefile for building packages

EXCLUDE_DIRS := bin docs Godeps vendor scripts
PKG_DIRS := $(filter-out $(EXCLUDE_DIRS),$(subst /,,$(sort $(dir $(wildcard */)))))
TO_BUILD := ./utils/rpckit/... ./utils/kvstore/... ./utils/runtime/... ./agent/...
GOLINT_CMD := golint -set_exit_status
GOFMT_CMD := gofmt -s -l
GOVET_CMD := go tool vet
SHELL := /bin/bash

default: build unit-test cover

deps:
	go get github.com/tools/godep
	go get github.com/golang/lint/golint

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

build: deps checks
	$(info +++ go install $(TO_BUILD))
	go install -v $(TO_BUILD)

unit-test:
	$(info +++ go test $(TO_BUILD))
	go test $(TO_BUILD)

cover:
	$(info +++ go test -cover $(TO_BUILD))
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

dev-clean:
	scripts/cleanup-dev.sh
