GIT_VERSION ?= $(shell git describe --tags --dirty --always)
BUILD_DATE ?= $(shell date   +%Y-%m-%dT%H:%M:%S%z)
GIT_COMMIT ?= $(shell git rev-list -1 HEAD --abbrev-commit)
GOFILES=$(shell find ${CURDIR} -type f -name "*.go")
default: build

build:
	$(info +++ Orchestrator Simulator build)
	CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install -ldflags '-X main.GitVersion=${GIT_VERSION} -X main.GitCommit=${GIT_COMMIT} -X main.BuildDate=${BUILD_DATE} -s -w' github.com/pensando/sw/venice/ctrler/orchhub/cmd/orch-sim/

clean:
	rm -f ${GOPATH}/venice/ctrler/orchhub/cmd/orch-sim/orch-sim

