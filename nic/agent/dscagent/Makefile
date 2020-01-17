GIT_VERSION ?= $(shell git describe --tags --dirty --always)
BUILD_DATE ?= $(shell date   +%Y-%m-%dT%H:%M:%S%z)
GIT_COMMIT ?= $(shell git rev-list -1 HEAD --abbrev-commit)
DIRS=$(shell go list -f {{.Dir}} github.com/pensando/sw/nic/agent/dscagent/... github.com/pensando/sw/nic/agent/cmd/netagent | grep -v irisproto)
GOFILES=$(shell find ${CURDIR} -type f -name "*.go")
GOIMPORTS_CMD := goimports -local "github.com/pensando/sw" -l -e
default: checks build

checks:
	$(info +++ goimports sources)
ifdef JOB_ID
	@echo "Running in CI; checking goimports and fmt"
	@$(eval IMPRT := $(shell ${GOIMPORTS_CMD} ${DIRS}))
	@if [ "$(IMPRT)" != "" ]; then \
		echo "*** goimports found errors in following files:"; \
		echo "$(IMPRT)"; \
		exit 1; \
	fi
else
	@${GOIMPORTS_CMD} -w ${DIRS}
endif
	$(info +++ revive sources)
	@revive --formatter friendly ${GOFILES}

build:
	$(info +++ Netagent build)
	CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install -tags iris -ldflags '-X main.GitVersion=${GIT_VERSION} -X main.GitCommit=${GIT_COMMIT} -X main.BuildDate=${BUILD_DATE} -s -w' github.com/pensando/sw/nic/agent/cmd/netagent

clean:
	rm -f ${GOPATH}/bin/netagent

