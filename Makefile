# Makefile for building packages

# :cached valid only for mac
platform=$(shell uname)
ifeq ($(platform),Darwin)
CACHEMOUNT := ":cached"
else
CACHEMOUNT :=
endif

# Lists excluded patterns to "go list"
EXCLUDE_PATTERNS := "apollo|generated|halproto|proto|model_sim|labels|vendor|bazel|e2etests|iota|buildroot|gometrics|netagent|dscagent|tsagent|tpagent|agent/tpa|nic/metaswitch/rtrctl/utils|nic/metaswitch/clientutils"

# these are run as part of integ test
INTEG_TEST_PATTERNS := "sw.test.integ|api.integration|citadel.test.integ"

# Lists targets to be prebuilt before generation
TO_PREGEN := venice/utils/apigen/annotations

# Lists venice venice protos and all things auto generated.
TO_GEN := api api/labels api/fields events venice/cmd/types/protos venice/cmd/grpc \
venice/ctrler/ckm/rpcserver/ckmproto \
nic/agent/protos/netproto \
nic/agent/protos/tpmprotos  \
nic/agent/protos/tsproto \
nic/agent/protos/nmd \
nic/agent/protos/evtprotos \
nic/agent/nmd/protos/delphi \
nic/sysmgr/proto \
nic/upgrade_manager/export/upggosdk \
venice/citadel/collector/rpcserver/metric venice/utils/runtime/test \
venice/cmd/grpc/server/certificates/certapi \
venice/evtsproxy/rpcserver/evtsproxyproto \
venice/utils/authn/radius                      \
venice/ctrler/rollout/rpcserver/protos \
venice/utils/objstore/client/mock \
venice/citadel/broker/mock \
venice/ctrler/orchhub/orchestrators/vchub/vcprobe/mock \
venice/utils/diagnostics/protos \
metrics \
nic/metaswitch/protos \
nic/apollo/agent/protos

PARALLEL := $(shell command -v parallel 2> /dev/null)


# Vendored packages to be installed before generation
TO_PREGEN_INSTALL := ./vendor/github.com/gogo/protobuf/protoc-gen-gofast \
						./vendor/github.com/gogo/protobuf/protoc-gen-gogofast \
						./vendor/github.com/golang/protobuf/protoc-gen-go \
						./vendor/golang.org/x/tools/cmd/goimports

# Lists all the vendored packages that need to be installed prior to the build.
TO_INSTALL := ./vendor/github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway \
							./venice/utils/apigen/protoc-gen-pensando \
							./vendor/github.com/pensando/grpc-gateway/protoc-gen-swagger \
							./vendor/github.com/GeertJohan/go.rice/rice \
							./vendor/github.com/golang/mock/gomock \
							./vendor/github.com/golang/mock/mockgen \
							./vendor/github.com/golang/lint/golint \
							./vendor/github.com/golang/dep/cmd/dep \
							./asset-build/... \
							./nic/delphi/compiler/... \
							./vendor/layeh.com/radius/cmd/radius-dict-gen \
							./vendor/github.com/pseudomuto/protoc-gen-doc/cmd/protoc-gen-doc \
							./venice/utils/doctool \
							./vendor/github.com/mgechev/revive

# Lists the binaries to be containerized
TO_DOCKERIZE ?= apigw apiserver npm cmd tpm netagent spyglass evtsmgr tsm evtsproxy vos citadel rollout vtsa orchhub vcsim pegasus perseus

GOIMPORTS_CMD := goimports -local "github.com/pensando/sw" -l
SHELL := /bin/bash
GOCMD = /usr/local/go/bin/go
PENS_AGENTS ?= 50
REGISTRY_URL ?= registry.test.pensando.io:5000
BUILD_CONTAINER ?= pens-bld:v0.14
UI_BUILD_CONTAINER ?= pens-ui-bld:v0.29
DIND_CONTAINER ?= pens-dind:v0.5
E2E_CONTAINER ?= pens-e2e:v0.6
TARGETS ?= ws-tools pull-assets-venice gen build
BUILD_CMD ?= bash -c  "make ${TARGETS}"
UPGRADE_TARGETS ?= ws-tools pull-assets-venice gen upgrade-build
UPGRADE_BUILD_CMD ?= bash -c  "make ${UPGRADE_TARGETS}"
PENCTL_BUILD_CMD ?= bash -c  "cd penctl && make"
E2E_CONFIG ?= test/e2e/cluster/tb_config_dev.json
E2E_CP_CONFIG ?= test/e2e/cloud/tb_config_cp.json
E2E_CUSTOM_CONFIG ?= test/e2e/cluster/venice-conf.json
GIT_COMMIT ?= $(shell git rev-list -1 HEAD --abbrev-commit)
GIT_VERSION ?= $(shell git describe --tags --dirty --always)
GIT_UPGRADE_VERSION ?= iota-upgrade
#IMAGE_VERSION is venice image version
IMAGE_VERSION ?= ${GIT_VERSION}
BUNDLE_VERSION ?= ${IMAGE_VERSION}
IMAGE_UPGRADE_VERSION ?= ${GIT_UPGRADE_VERSION}
BUNDLE_UPGRADE_VERSION ?= ${IMAGE_UPGRADE_VERSION}
BUILD_DATE ?= $(shell date   +%Y-%m-%dT%H:%M:%S%z)
export GIT_COMMIT GIT_VERSION BUILD_DATE GIT_UPGRADE_VERSION

default:
	$(MAKE) pregen-clean
	$(MAKE) ws-tools
	$(MAKE) pull-assets-venice
	$(MAKE) gen-clean
	$(MAKE) gen
	$(MAKE) checks
	$(MAKE) build
	$(MAKE) unit-test-cover

ut-odd:
	$(MAKE) pregen-clean
	$(MAKE) ws-tools
	$(MAKE) pull-assets-venice
	$(MAKE) gen-clean
	$(MAKE) gen
	$(MAKE) checks
	$(MAKE) build
	$(MAKE) unit-test-cover-odd

ut-even:
	$(MAKE) pregen-clean
	$(MAKE) ws-tools
	$(MAKE) pull-assets-venice
	$(MAKE) gen-clean
	$(MAKE) gen
	$(MAKE) build
	$(MAKE) unit-test-cover-even



pregen:
	@CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install ${TO_PREGEN_INSTALL}
	@for c in ${TO_PREGEN}; do printf "\n+++++++++++++++++ Generating $${c} +++++++++++++++++\n"; PATH=$$PATH make -C $${c} || exit 1; done

# ws-tools installs all the binaries needed to generate, lint and build go sources
ws-tools: pregen
	$(info +++ building WS tools)
	@CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install ${TO_INSTALL}

# checks ensure that the go code meets standards
checks: nic-protos goimports-src golint-src govet-src ginkgo-src

# nic-protos builds protos from nic to help generate go files that are needed
nic-protos:
	make -C nic/upgrade_manager/export/upggosdk
	make -C nic/agent/nmd/protos/delphi
	make -C nic/sysmgr/proto
	make -C nic/agent/protos/netproto delphi-gen
	make -C nic/metaswitch/protos
	make -C nic/apollo/agent/protos

# gen does all the autogeneration. viz venice cli, proto sources. Ensure that any new autogen target is added to TO_GEN above
gen:
	@for c in ${TO_GEN}; do printf "\n+++++++++++++++++ Generating $${c} +++++++++++++++++\n"; PATH=$$PATH make -C $${c} || exit 1; done

# goimports-src formats the source and orders imports
# Target directories is needed only for goipmorts where it doesn't accept package names.
# Doing the go list here avoids an additional global go list
goimports-src: gopkgsinstall
	$(info +++ goimports sources)
	$(eval GO_FILES=`${GOPATH}/bin/gopkgs -short -f '{{.Dir}}/*.go' | grep github.com/pensando/sw | egrep -v ${EXCLUDE_PATTERNS}`)
ifdef JOB_ID
	@echo "Running in CI; checking goimports and fmt"
	@$(eval IMPRT := $(shell ${GOIMPORTS_CMD} ${GO_FILES}))
	@if [ "$(IMPRT)" != "" ]; then \
		echo "*** goimports found errors in following files:"; \
		echo "$(IMPRT)"; \
		exit 1; \
	fi
else
	@${GOIMPORTS_CMD} -w ${GO_FILES}
endif

# golint-src runs go linter and verifies code style matches golang recommendations
golint-src: gopkglist
	$(info +++ golint sources)
	@$(eval LINT := $(shell golint ${GO_PKG} | grep -v pb.go))
	@echo "$(LINT)"
	@test -z "$(LINT)"

# govet-src validates source code and reports suspicious constructs
govet-src: gopkglist
	$(info +++ govet sources)
	@go vet -source ${GO_PKG}

ginkgo-src: 
	$(info +++ ginkgo dryrun)
	@go test github.com/pensando/sw/test/e2e/cluster/... -ginkgo.v -ginkgo.dryRun

.PHONY: build gopkglist gopkgsinstall ginkgo-src netagent-build penctl upgrade-penctl

gopkgsinstall:
	@$(shell cd ${GOPATH}/src/github.com/pensando/sw && CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install ./vendor/github.com/haya14busa/gopkgs/cmd/gopkgs)
gopkglist: gopkgsinstall
	@$(eval GO_PKG := $(shell ${GOPATH}/bin/gopkgs -short 2>/dev/null | grep github.com/pensando/sw | egrep -v ${EXCLUDE_PATTERNS}))
	$(eval GO_PKG_UTEST := $(shell ${GOPATH}/bin/gopkgs -short 2>/dev/null | grep github.com/pensando/sw | egrep -v ${EXCLUDE_PATTERNS} | egrep -v ${INTEG_TEST_PATTERNS} | sort ))
	$(eval GO_PKG_INTEGTEST := $(shell ${GOPATH}/bin/gopkgs -short 2>/dev/null | grep github.com/pensando/sw | egrep -v ${EXCLUDE_PATTERNS} | egrep ${INTEG_TEST_PATTERNS} | sort))

# netagent-build builds pipeline aware netagent
netagent-build:
	$(MAKE) -C nic/agent/dscagent

# build installs all go binaries. Use VENICE_CCOMPILE_FORCE=1 to force a rebuild of all packages
build: gopkglist netagent-build
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install -ldflags '-X main.GitVersion=${GIT_VERSION} -X main.GitCommit=${GIT_COMMIT} -X main.BuildDate=${BUILD_DATE}' ${GO_PKG};\
	else \
		echo "+++ rebuilding all go sources";CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install -a -ldflags '-X main.GitVersion=${GIT_VERSION} -X main.GitCommit=${GIT_COMMIT} -X main.BuildDate=${BUILD_DATE}' ${GO_PKG};\
	fi

# build installs all go binaries. Use VENICE_CCOMPILE_FORCE=1 to force a rebuild of all packages
upgrade-build: gopkglist netagent-build
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install -ldflags '-X main.GitVersion=${GIT_UPGRADE_VERSION} -X main.GitCommit=${GIT_COMMIT} -X main.BuildDate=${BUILD_DATE}' ${GO_PKG};\
	else \
		echo "+++ rebuilding all go sources";CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go install -a -ldflags '-X main.GitVersion=${GIT_UPGRADE_VERSION} -X main.GitCommit=${GIT_COMMIT} -X main.BuildDate=${BUILD_DATE}' ${GO_PKG};\
	fi

# VENICE_DEV=1 switches behavior of Venice components and libraries to test mode.
# CGO_LDFLAGS_ALLOW captures whiltelisted CGO Flags. This will avoid code injection
# attacks on go get.
# For rpckit, it changes the default connection mode from "TLS on" to "TLS off"
# See venice/utils/testenv
# unit-test-cover uses go test wrappers in scripts/report/report.go and runs coverage tests.
# this will return a non 0 error when coverage for a package is < 75.0%
unit-test-cover: gopkglist
	$(info +++ running go tests on $(GO_PKG_UTEST))
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go run scripts/report/report.go ${GO_PKG_UTEST}

unit-test-cover-odd: gopkglist
	@$(eval ODDF := $(shell echo ${GO_PKG_UTEST} | awk '{for (i=1; i<=NF; i+=2) printf("%s ",$$i)}'))
	$(info +++ running go tests on $(ODDF))
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go run scripts/report/report.go ${ODDF}
unit-test-cover-even: gopkglist
	@$(eval EVENF := $(shell echo ${GO_PKG_UTEST} | awk '{for (i=2; i<=NF; i+=2) printf("%s ",$$i)}'))
	$(info +++ running go tests on $(EVENF))
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go run scripts/report/report.go ${EVENF}

integ-test: gopkglist
	$(MAKE) integ-test-odd
	$(MAKE) integ-test-even

integ-test-odd: gopkglist
	@$(eval ODDF := $(shell echo ${GO_PKG_INTEGTEST} | awk '{for (i=1; i<=NF; i+=2) printf("%s ",$$i)}'))
	$(info +++ running go tests on $(ODDF))
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go run scripts/report/report.go ${ODDF}
integ-test-even: gopkglist
	@$(eval EVENF := $(shell echo ${GO_PKG_INTEGTEST} | awk '{for (i=2; i<=NF; i+=2) printf("%s ",$$i)}'))
	$(info +++ running go tests on $(EVENF))
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go run scripts/report/report.go ${EVENF}

ci-integ-test:
	$(MAKE) pregen-clean
	$(MAKE) ws-tools
	$(MAKE) pull-assets-venice
	$(MAKE) gen-clean
	$(MAKE) gen
	$(MAKE) integ-test-odd
	$(MAKE) integ-test-even

ci-integ-test-odd:
	$(MAKE) pregen-clean
	$(MAKE) ws-tools
	$(MAKE) pull-assets-venice
	$(MAKE) gen-clean
	$(MAKE) gen
	$(MAKE) integ-test-odd

ci-integ-test-even:
	$(MAKE) pregen-clean
	$(MAKE) ws-tools
	$(MAKE) pull-assets-venice
	$(MAKE) gen-clean
	$(MAKE) gen
	$(MAKE) integ-test-even


unit-race-test: gopkglist
	$(info +++ running go tests with race detector)
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" go test -race ${GO_PKG_UTEST}

unit-test-verbose: gopkglist
	$(info +++ running go tests verbose)
	@VENICE_DEV=1 CGO_LDFLAGS_ALLOW="-I/usr/local/share/libtool" $(GOCMD) test -v -p 1 ${GO_PKG_UTEST}; \

c-start:
	@tools/scripts/create-container.sh startCluster

c-stop:
	@tools/scripts/create-container.sh stopCluster || echo

install:
	@#copy the agent binaries to netagent
	@cp -p ${PWD}/bin/cbin/fakehal tools/docker-files/netagent/fakehal
	@cp -p ${PWD}/bin/cbin/fakedelphihub tools/docker-files/netagent/fakedelphihub
	@cp -p ${PWD}/bin/cbin/fwgen tools/docker-files/netagent/fwgen
	@cp -p ${PWD}/bin/cbin/ctctl tools/docker-files/citadel/ctctl
	@cp -p ${PWD}/bin/cbin/nmd tools/docker-files/netagent/nmd
	@cp -p ${PWD}/bin/cbin/nevtsproxy tools/docker-files/netagent/nevtsproxy
	@cp -p ${PWD}/bin/cbin/tmagent tools/docker-files/netagent/tmagent
	@cp -p ${PWD}/nic/tools/fakefwupdate tools/docker-files/netagent/fwupdate
	@cp -p ${PWD}/bin/cbin/ntsa tools/docker-files/netagent/ntsa
	@cp -p ${PWD}/bin/cbin/rtrctl tools/docker-files/perseus/rtrctl
	@cp -p ${PWD}/nic/conf/naples-tsa.json tools/docker-files/netagent/naples-tsa.json
	@echo  ${GIT_VERSION} > tools/docker-files/netagent/VERSION
	@cp -p ${PWD}/venice/vtsa/cmd/vtsa/vtsa.json tools/docker-files/vtsa/vtsa.json
	@# npm is special - The executable is called pen-npm since it conflicts with node.js npm. Hence copy it explicitly here
	@cp -p ${PWD}/bin/cbin/pen-npm tools/docker-files/npm/pen-npm
ifndef PARALLEL
	@for c in $(TO_DOCKERIZE); \
	 do echo "+++ Dockerizing $${c}"; \
	 cp -p ${PWD}/bin/cbin/$${c} tools/docker-files/$${c}/$${c};\
	 number=1 ; \
	 EXIT_CODE=1 ; \
	 while [[ $$number -le 5 && $$EXIT_CODE -ne 0 ]] ; \
		do echo $$number ; \
			docker build --label org.label-schema.build-date="${BUILD_DATE}" --label org.label-schema.vendor="Pensando" --label org.label-schema.vcs-ref="${GIT_COMMIT}" --label org.label-schema.version="${GIT_VERSION}" --label org.label-schema.schema-version="1.0"  --rm -t pen-$${c}:latest -f tools/docker-files/$${c}/Dockerfile tools/docker-files/$${c} ; \
			EXIT_CODE=$$?;\
			((number = number + 1)) ; \
			echo "+++ Dockerizing $${c} exited with $${EXIT_CODE}" ; \
		done ; \
	 done
else
	@for c in $(TO_DOCKERIZE); do echo "+++ Dockerizing $${c}"; cp -p ${PWD}/bin/cbin/$${c} tools/docker-files/$${c}/$${c} ; done
	@for c in $(TO_DOCKERIZE); do (echo docker build --label org.label-schema.build-date="${BUILD_DATE}" --label org.label-schema.vendor="Pensando" --label org.label-schema.vcs-ref="${GIT_COMMIT}" --label org.label-schema.version="${GIT_VERSION}" --label org.label-schema.schema-version="1.0"  --rm -t pen-$${c}:latest -f tools/docker-files/$${c}/Dockerfile tools/docker-files/$${c}) ; done | parallel --will-cite --jobs 4
endif
	@tools/scripts/createImage.py ${IMAGE_VERSION} ${GIT_VERSION}
	@# the above script populates venice.json which needs to be 'installed' on the venice. Hence creation of installer is done at the end
	@# For now the installer is a docker container.
	@# In the future, this can be a shell script, rpm, curl script or whatever..
	docker build --label org.label-schema.build-date="${BUILD_DATE}" --label org.label-schema.vendor="Pensando" --label org.label-schema.vcs-ref="${GIT_COMMIT}" --label org.label-schema.version="${GIT_VERSION}" --label org.label-schema.schema-version="1.0" --rm --no-cache -t pen-install:latest -f tools/docker-files/install/Dockerfile tools/docker-files/install
	docker save -o bin/tars/pen-netagent.tar pen-netagent:latest
	docker save -o bin/tars/pen-install.tar pen-install:latest

upgrade-install:
	@#copy the agent binaries to netagent
	@cp -p ${PWD}/bin/cbin/fakedelphihub tools/docker-files/netagent/fakedelphihub
	@cp -p ${PWD}/bin/cbin/fakehal tools/docker-files/netagent/fakehal
	@cp -p ${PWD}/bin/cbin/fwgen tools/docker-files/netagent/fwgen
	@cp -p ${PWD}/bin/cbin/ctctl tools/docker-files/citadel/ctctl
	@cp -p ${PWD}/bin/cbin/nmd tools/docker-files/netagent/nmd
	@cp -p ${PWD}/bin/cbin/nevtsproxy tools/docker-files/netagent/nevtsproxy
	@cp -p ${PWD}/bin/cbin/tmagent tools/docker-files/netagent/tmagent
	@cp -p ${PWD}/nic/tools/fakefwupdate tools/docker-files/netagent/fwupdate
	@cp -p ${PWD}/bin/cbin/ntsa tools/docker-files/netagent/ntsa
	@cp -p ${PWD}/nic/conf/naples-tsa.json tools/docker-files/netagent/naples-tsa.json
	@echo  ${GIT_UPGRADE_VERSION} > tools/docker-files/netagent/VERSION
	@cp -p ${PWD}/venice/vtsa/cmd/vtsa/vtsa.json tools/docker-files/vtsa/vtsa.json
	@# npm is special - The executable is called pen-npm since it conflicts with node.js npm. Hence copy it explicitly here
	@cp -p ${PWD}/bin/cbin/pen-npm tools/docker-files/npm/pen-npm
ifndef PARALLEL
	@for c in $(TO_DOCKERIZE); \
	 do echo "+++ Dockerizing $${c}"; \
	 cp -p ${PWD}/bin/cbin/$${c} tools/docker-files/$${c}/$${c};\
	 number=1 ; \
	 EXIT_CODE=1 ; \
	 while [[ $$number -le 5 && $$EXIT_CODE -ne 0 ]] ; \
		do echo $$number ; \
			docker build --label org.label-schema.build-date="${BUILD_DATE}" --label org.label-schema.vendor="Pensando" --label org.label-schema.vcs-ref="${GIT_COMMIT}" --label org.label-schema.version="${GIT_UPGRADE_VERSION}" --label org.label-schema.schema-version="1.0"  --rm -t pen-$${c}:latest -f tools/docker-files/$${c}/Dockerfile tools/docker-files/$${c} ; \
			EXIT_CODE=$$?;\
			((number = number + 1)) ; \
			echo "+++ Dockerizing $${c} exited with $${EXIT_CODE}" ; \
		done ; \
	 done
else
	@for c in $(TO_DOCKERIZE); do echo "+++ Dockerizing $${c}"; cp -p ${PWD}/bin/cbin/$${c} tools/docker-files/$${c}/$${c} ; done
	@for c in $(TO_DOCKERIZE); do (echo docker build --label org.label-schema.build-date="${BUILD_DATE}" --label org.label-schema.vendor="Pensando" --label org.label-schema.vcs-ref="${GIT_COMMIT}" --label org.label-schema.version="${GIT_UPGRADE_VERSION}" --label org.label-schema.schema-version="1.0"  --rm -t pen-$${c}:latest -f tools/docker-files/$${c}/Dockerfile tools/docker-files/$${c}) ; done | parallel --will-cite --jobs 4
endif
	@tools/scripts/createImage.py ${IMAGE_UPGRADE_VERSION} ${GIT_UPGRADE_VERSION}
	@# the above script populates venice.json which needs to be 'installed' on the venice. Hence creation of installer is done at the end
	@# For now the installer is a docker container.
	@# In the future, this can be a shell script, rpm, curl script or whatever..
	docker build --label org.label-schema.build-date="${BUILD_DATE}" --label org.label-schema.vendor="Pensando" --label org.label-schema.vcs-ref="${GIT_COMMIT}" --label org.label-schema.version="${GIT_UPGRADE_VERSION}" --label org.label-schema.schema-version="1.0" --rm --no-cache -t pen-install:latest -f tools/docker-files/install/Dockerfile tools/docker-files/install
	docker save -o bin/tars/pen-netagent.tar pen-netagent:latest
	docker save -o bin/tars/pen-install.tar pen-install:latest

gen-clean:
	@for c in ${TO_GEN}; do printf "\n+++++++++++++++++ Cleaning $${c} +++++++++++++++++\n"; PATH=$$PATH make -C $${c} gen-clean || exit 1; done

pregen-clean:
	@for c in ${TO_PREGEN}; do printf "\n+++++++++++++++++ Cleaning $${c} +++++++++++++++++\n"; PATH=$$PATH make -C $${c} gen-clean || exit 1; done

clean:
	@$(MAKE) c-stop >/dev/null 2>&1
	@rm -fr bin/* venice/ui/webapp/node_modules  venice/ui/web-app-framework/node_modules  venice/ui/venice-sdk/node_modules venice/ui/webapp/dist
	@rm -f nic/sim/naples/venice-sim.tar tools/docker-files/venice/venice-sim.tar test/topos/gs/venice_sim_addons.tar
	@rm -f tools/docker-files/netagent/nmd tools/docker-files/netagent/nevtsproxy tools/docker-files/netagent/tmagent tools/docker-files/npm/pen-npm tools/docker-files/netagent/fakedelhihub tools/docker-files/netagent/fwgen tools/docker-files/netagent/ntsa tools/docker-files/netagent/naples-tsa.json
	@for c in $(TO_DOCKERIZE); do rm -f tools/docker-files/$${c}/$${c};  done
	@rm -fr tools/docker-files/ova/output-venice-livecd

ova:
	$(MAKE) -C tools/docker-files/ova

apulu-ova:
	$(MAKE) -C tools/docker-files/ova venice-apulu-ova

helper-containers:
	@cd tools/docker-files/pens-base; docker build -t ${REGISTRY_URL}/pens-base:v0.9 .
	@cd tools/docker-files/pens-base-2; docker build -t ${REGISTRY_URL}/pens-base-2:v0.5 .
	@cd tools/docker-files/vinstall; docker build -t ${REGISTRY_URL}/pens-vinstall:v0.1 .
	@#keep pens-ntp version in sync in tools/scripts/createImage.py
	@cd tools/docker-files/ntp; docker build -t ${REGISTRY_URL}/pens-ntp:v0.6 .
	@cd tools/docker-files/build-container; docker build -t ${REGISTRY_URL}/${BUILD_CONTAINER} .
	@cd tools/docker-files/dind; docker build -t ${REGISTRY_URL}/${DIND_CONTAINER}  .
	@cd tools/docker-files/e2e; docker build -t ${REGISTRY_URL}/${E2E_CONTAINER} .
	@cd tools/docker-files/elasticsearch; docker build -t ${REGISTRY_URL}/elasticsearch-cluster:v0.15 .
	@cd tools/docker-files/elasticsearch-curator; docker build -t ${REGISTRY_URL}/elasticsearch-curator:v0.2 .
	@cd tools/docker-files/filebeat; docker build -t ${REGISTRY_URL}/pen-filebeat:v0.2 .
	@cd tools/test-build; docker build -t ${REGISTRY_URL}/pen-test-build:v0.3 .

debug-container:
	scripts/create-debug-container.sh

ui-container-helper:
	mkdir -p tools/docker-files/ui-container/web-app-framework/dist tools/docker-files/ui-container/venice-sdk
	# copy local packages
	cp venice/ui/web-app-framework/web-app-framework.tgz tools/docker-files/ui-container/web-app-framework/web-app-framework.tgz
	cp venice/ui/venice-sdk/pensando-swagger-ts-generator-1.1.29.tgz tools/docker-files/ui-container/venice-sdk/pensando-swagger-ts-generator-1.1.29.tgz
	# copy package.json and yarn.locks files, removing local packages
	$(eval FILENAME := package.json)
	cp venice/ui/web-app-framework/$(FILENAME) tools/docker-files/ui-container/web-app-framework/$(FILENAME)
	sed '/@pensando/ d' venice/ui/venice-sdk/$(FILENAME) > tools/docker-files/ui-container/venice-sdk/$(FILENAME)
	sed '/web-app-framework/ d' venice/ui/webapp/$(FILENAME) > tools/docker-files/ui-container/webapp/$(FILENAME)
	$(eval FILENAME := yarn.lock)
	cp venice/ui/web-app-framework/$(FILENAME) tools/docker-files/ui-container/web-app-framework/$(FILENAME)
	sed '/@pensando/,/^$$/ d' venice/ui/venice-sdk/$(FILENAME) > tools/docker-files/ui-container/venice-sdk/$(FILENAME)
	sed '/web-app-framework/,/^$$/ d' venice/ui/webapp/$(FILENAME) > tools/docker-files/ui-container/webapp/$(FILENAME)
	@cd tools/docker-files/ui-container; docker build --squash -t ${REGISTRY_URL}/${UI_BUILD_CONTAINER} .

# runs the ui-container in interactive mode
ui-container:
	docker run -it --network none --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} bash; \

ui-container-serve:
	$(MAKE) ui-dependencies
	docker run -p 4200:4200 --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} make ui-serve; \

# update web-app-framework.tgz asset to minio
ui-update-asset: ui-framework
	tar czvf asset.tgz venice/ui/web-app-framework/web-app-framework.tgz
	cd asset-build/asset-upload/ && go install
	python tools/scripts/ui_update_minio_version.py
	rm asset.tgz venice/ui/web-app-framework/web-app-framework.tgz

# running as 'make container-compile UI_FRAMEWORK=1' will also force the UI-framework compilation
container-compile:
	@mkdir -p ${PWD}/bin/cbin
	@mkdir -p ${PWD}/bin/pkg
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; echo docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${BUILD_CMD} ; \
		docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${BUILD_CMD} ; \
	else \
		echo "+++ rebuilding all go sources"; echo docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${BUILD_CMD} ;\
		docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${BUILD_CMD} ;\
	fi
# running as 'make container-compile UI_FRAMEWORK=1' will also force the UI-framework compilation
upgrade-container-compile:
	@mkdir -p ${PWD}/bin/cbin
	@mkdir -p ${PWD}/bin/pkg
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; echo docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_UPGRADE_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${UPGRADE_BUILD_CMD} ; \
		docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_UPGRADE_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${UPGRADE_BUILD_CMD} ; \
	else \
		echo "+++ rebuilding all go sources"; echo docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_UPGRADE_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${UPGRADE_BUILD_CMD} ;\
		docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_UPGRADE_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${UPGRADE_BUILD_CMD} ;\
	fi


populate-web-app-framework-node-modules:
	echo "+++ populating node_modules from cache for ui-framework";\
	echo docker run --network none --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}"  --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} sh -c 'cp /usr/local/lib/web-app-framework/node_modules.tgz bin/web-app-framework-node-modules.tgz ' ; \
	docker run --network none --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} sh -c 'cp /usr/local/lib/web-app-framework/node_modules.tgz bin/web-app-framework-node-modules.tgz ' ; \
	cd venice/ui/web-app-framework && tar zxf ../../../bin/web-app-framework-node-modules.tgz ;\
	cd ../../.. ;\

populate-webapp-node-modules:
	echo "+++ populating node_modules from cache for ui";\
	echo docker run --network none --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} sh -c 'cp /usr/local/lib/venice-sdk/node_modules.tgz bin/venice-sdk-node-modules.tgz; cp /usr/local/lib/webapp/node_modules.tgz bin/webapp-node-modules.tgz' ; \
	docker run --network none --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} sh -c 'cp /usr/local/lib/venice-sdk/node_modules.tgz bin/venice-sdk-node-modules.tgz; cp /usr/local/lib/webapp/node_modules.tgz bin/webapp-node-modules.tgz' ; \
	cd venice/ui/webapp && tar zxf ../../../bin/webapp-node-modules.tgz ;\
	cd ../venice-sdk && tar zxf ../../../bin/venice-sdk-node-modules.tgz ;\
	cd ../../.. ;\

ui-dependencies:
	mkdir -p bin
	if [ ! -z ${UI_FRAMEWORK} ]; then \
		if [ ! -f bin/web-app-node-modules.tgz ]; then \
			$(MAKE) populate-web-app-framework-node-modules ;\
		fi ; \
	fi; \
	if [ ! -f bin/webapp-node-modules.tgz ]; then \
		$(MAKE) populate-webapp-node-modules ;\
	fi ; \
	if [ ! -z ${UI_FRAMEWORK} ]; then \
		docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} make ui-framework; \
	else \
		cd venice/ui/web-app-framework && tar xf web-app-framework.tgz && cd ../../..; \
	fi; \


upgrade-fixtures:
	mkdir -p bin
	@if [ -z ${BYPASS_UI} ]; then \
		$(MAKE) ui-dependencies ;\
		echo "+++ building ui sources" ; \
		echo docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_UPGRADE_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} ; \
		docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_UPGRADE_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} ; \
		echo rm -r tools/docker-files/apigw/dist ;\
		rm -r tools/docker-files/apigw/dist ;\
		echo cp -r venice/ui/webapp/dist tools/docker-files/apigw ;\
		cp -r venice/ui/webapp/dist tools/docker-files/apigw ;\
	fi
	@if [ -z ${BYPASS_DOCS} ]; then \
		echo "++ generating documentation"; \
		tools/scripts/gendocs.sh "${REGISTRY_URL}/${BUILD_CONTAINER}" "${REGISTRY_URL}/${UI_BUILD_CONTAINER}" ;\
		cp -r api/docs tools/docker-files/apigw; \
		cp -r docs/examples tools/docker-files/apigw/docs; \
	fi

fixtures:
	mkdir -p bin
	@if [ -z ${BYPASS_UI} ]; then \
		$(MAKE) ui-dependencies ;\
		echo "+++ building ui sources" ; \
		echo docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} ; \
		docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} ; \
		echo rm -r tools/docker-files/apigw/dist ;\
		rm -r tools/docker-files/apigw/dist ;\
		echo cp -r venice/ui/webapp/dist tools/docker-files/apigw ;\
		cp -r venice/ui/webapp/dist tools/docker-files/apigw ;\
	fi
	@if [ -z ${BYPASS_DOCS} ]; then \
		echo "++ generating documentation"; \
		tools/scripts/gendocs.sh "${REGISTRY_URL}/${BUILD_CONTAINER}" "${REGISTRY_URL}/${UI_BUILD_CONTAINER}" ;\
		cp -r api/docs tools/docker-files/apigw; \
		cp -r docs/examples tools/docker-files/apigw/docs; \
	fi
	rm -rf tools/docker-files/pegasus/nic; mkdir -p tools/docker-files/pegasus/nic
	if [ -a nic/pegasus.tgz ]; then tar -xvf nic/pegasus.tgz -C tools/docker-files/pegasus/ ;  fi

shell:
	@mkdir -p ${PWD}/bin/cbin
	@mkdir -p ${PWD}/bin/pkg
	docker run -it --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} bash

container-qcompile:
	mkdir -p ${PWD}/bin/cbin
	mkdir -p ${PWD}/bin/pkg
	@if [ -z ${VENICE_CCOMPILE_FORCE} ]; then \
		echo "+++ building go sources"; \
		echo docker run --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER}; \
		docker run --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER}; \
	else \
		echo "+++ rebuilding all go sources"; \
		echo docker run --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER};\
		docker run --user $(shell id -u):$(shell id -g) -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -e "VENICE_CCOMPILE_FORCE=1" -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER};\
	fi

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

# Target to create env for Venice-UI unit test
test-debug-ui: install_box
	box -t venice-ui:test-debug venice/ui/box.rb
	docker rm -f venice-ui || :
	docker run --name venice-ui -p 80:3000 -it -v "${PWD}:/go/src/github.com/pensando/sw" venice-ui:test-debug bash

# Target to start Kibana on a local machine and point it to a dind cluster.
# Kibana UI is then available on localhost:5601
start-kibana:
	docker cp node0:/var/lib/pensando/pki/shared/elastic-client-auth/ /tmp
	docker run --rm --name kibana --network pen-dind-net \
	-v /tmp/elastic-client-auth:/usr/share/kibana/config/auth \
	-e ELASTICSEARCH_URL=https://192.168.30.11:9200 \
	-e ELASTICSEARCH_SSL_CERTIFICATEAUTHORITIES="config/auth/ca-bundle.pem" \
	-e ELASTICSEARCH_SSL_CERTIFICATE="config/auth/cert.pem" \
	-e ELASTICSEARCH_SSL_KEY="config/auth/key.pem" \
	-e xpack.security.enabled=false \
	-e xpack.logstash.enabled=false \
	-e xpack.graph.enable=false \
	-e xpack.watcher.enabled=false \
	-e xpack.ml.enabled=false \
	-e xpack.monitoring.enabled=false \
	-p 5601:5601 -d registry.test.pensando.io:5000/kibana:6.3.0

# Target to run on Mac to stop kibana docker
stop-kibana:
	docker stop kibana
	docker rm kibana
	rm -rf /tmp/elastic-client-auth

# Dev environment targets
dev:
	scripts/bringup-dev.sh Vagrantfile.dev

arm64dev:
	scripts/bringup-dev.sh Vagrantfile.arm64dev

node:
	scripts/bringup-dev.sh Vagrantfile.node

dev-clean:
	scripts/cleanup-dev.sh

create-assets:
	bash scripts/create-assets.sh

toolchain-clean:
	if [ -d "/tool/toolchain" ]; then sudo umount /tool; sudo rm -rf /tool; fi

toolchain:
	if [ ! -d "/tool/toolchain" ]; then sudo rm -rf /tool; sudo mkdir -p /tool; sudo mount asic20.pensando.io:/export/tool /tool; fi

pull-assets-venice:
	bash scripts/pull-assets-venice.sh

pull-assets:
	bash scripts/pull-assets.sh
	$(MAKE) ws-tools

dind-cluster:
	$(MAKE) dind-cluster-stop
	$(MAKE) venice-image
	./test/e2e/dind/do.py -configFile ${E2E_CONFIG} -custom_config_file ${E2E_CUSTOM_CONFIG} -deployvc

dind-cluster-cp:
	$(MAKE) dind-cluster-stop
	$(MAKE) venice-image
	./test/e2e/dind/do.py -configFile ${E2E_CP_CONFIG} -custom_config_file ${E2E_CUSTOM_CONFIG} -deployvc

dind-cluster-stop:
	./test/e2e/dind/do.py -delete

dind-cluster-restart:
	./test/e2e/dind/do.py -restart -configFile=./${E2E_CONFIG} -custom_config_file=./${E2E_CUSTOM_CONFIG}

# Target to run venice e2e on mac using a dind environment. Uses Agent with its datapath mocked
e2e:
	$(MAKE) dind-cluster
	$(MAKE) e2e-test
	# enable auto delete after e2e tests pass consistently. For now - keep the cluster running so that we can debug failures
	#./test/e2e/dind/do.py -delete

cloud-e2e:
	if [ -z ${BYPASS_CLOUD_SIM} ]; then \
		PIPELINE=apulu FLAVOR=-venice IGNORE_BUILD_PIPELINE=1 ${MAKE} -C nic docker/naples-sim; \
		tar xvf nic/obj/images/naples-release-v1.tgz -C nic/obj/images/; \
		docker load -i nic/obj/images/naples-docker-v1.tgz; \
	fi
	if [ -z ${BYPASS_PEGASUS} ]; then \
		$(MAKE) -C nic docker/pegasus; \
	fi
	$(MAKE) dind-cluster-cp
	$(MAKE) cloud-e2e-test

cloud-e2e-test:
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/cloud -configFile=/import/src/github.com/pensando/sw/${E2E_CP_CONFIG} -ginkgo.v -timeout 60m ${E2E_SEED}'

cloud-e2e-retest:
	docker exec -it node0 sh -c 'PENS_SKIP_BOOTSTRAP=1 PENS_SKIP_AUTH=1 E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/cloud -configFile=/import/src/github.com/pensando/sw/${E2E_CP_CONFIG} -ginkgo.v -timeout 60m ${E2E_SEED}'
# this assumes that venice is already compiled and starts with cluster creation
e2e-ci:
	docker load -i bin/tars/pen-netagent.tar
	./test/e2e/dind/do.py -configFile ${E2E_CONFIG} -custom_config_file ${E2E_CUSTOM_CONFIG}
	$(MAKE) e2e-test

e2e-retest:
	docker exec -it node0 sh -c 'PENS_SKIP_BOOTSTRAP=1 PENS_SKIP_AUTH=1 E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/cluster -configFile=/import/src/github.com/pensando/sw/${E2E_CONFIG} -ginkgo.v -timeout 45m ${E2E_SEED}'
e2e-test:
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/cluster -configFile=/import/src/github.com/pensando/sw/${E2E_CONFIG} -ginkgo.v -timeout 60m ${E2E_SEED}'

# bring up e2e cluster
e2e-cluster: dind-cluster
	# run a small test to bring the cluster up
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/cluster -configFile=/import/src/github.com/pensando/sw/${E2E_CONFIG} -ginkgo.v -ginkgo.focus="validate fwlog policy" ${E2E_SEED}'

e2e-ui:
	docker run --privileged  -it -l pens --network pen-dind-net --user $(shell id -u):$(shell id -g) -v ${PWD}:/import/src/github.com/pensando/sw -e "E2E_BASE_URL=https://192.168.30.10:443" -w /import/src/github.com/pensando/sw/venice/ui/webapp ${REGISTRY_URL}/${UI_BUILD_CONTAINER} /bin/bash -c "yarn run webdriver-update-ci ; ng e2e --configuration=e2e-ci  --webdriverUpdate=false --suite=all| tee  /import/src/github.com/pensando/sw/e2e-ui.log" 

	

# Target to run venice e2e a dind environment. Uses real HAL as Agent Datapath and starts HAL with model
e2e-sanities:
	$(MAKE) venice-image
	$(MAKE) -C nic e2e-sanity-build
	./test/e2e/dind/do.py -configFile test/e2e/cluster/tb_config_sanities.json
	@stty sane
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/cluster -configFile=/import/src/github.com/pensando/sw/test/e2e/cluster/tb_config_sanities.json -ginkgo.v -timeout 25m'
	# enable auto delete after e2e tests pass consistently. For now - keep the cluster running so that we can debug failures
	#./test/e2e/dind/do.py -delete

# Target to run rollout e2e test cases
e2e-rollout-ci:
	docker load -i bin/tars/pen-netagent.tar
	./test/e2e/dind/do.py -configFile ${E2E_CONFIG} -custom_config_file ${E2E_CUSTOM_CONFIG}
	$(MAKE) e2e-rollout
e2e-rollout:
	@stty sane
	@echo "Running rollout tests..."
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/rollout -configFile=/import/src/github.com/pensando/sw/${E2E_CONFIG} -ginkgo.v -timeout 60m ${E2E_SEED}'
	# enable auto delete after e2e tests pass consistently. For now - keep the cluster running so that we can debug failures
	#./test/e2e/dind/do.py -delete

# Target to run telemetry e2e test cases
e2e-telemetry:
	$(MAKE) venice-image
	$(MAKE) -C nic docker/e2e-sanity-build
	@echo "Setting up cluster..."
	./test/e2e/dind/do.py -configFile test/e2e/telemetry/tb_config.json
	@stty sane
	@echo "Running tests..."
	docker exec -it node0 sh -c 'E2E_TEST=1 CGO_LDFLAGS_ALLOW="-I/usr/share/libtool" go test -v ./test/e2e/telemetry -configFile=/import/src/github.com/pensando/sw/test/e2e/telemetry/tb_config.json'
	# enable auto delete after e2e tests pass consistently. For now - keep the cluster running so that we can debug failures
	#./test/e2e/dind/do.py -delete

e2e-naples: 
	$(MAKE) ws-tools
	$(MAKE) pull-assets
	$(MAKE) -C nic
	$(MAKE) -C nic release

naples-firmware-gold:
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 clean
	$(MAKE) ws-tools
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 FWTYPE=gold firmware
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 package-drivers

naples-firmware:
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 clean
	$(MAKE) ws-tools
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 firmware
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 package-drivers

naples-firmware-iterative:
	$(MAKE) ws-tools
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 firmware
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 package-drivers

naples-firmware-elba:
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 ASIC=elba clean
	$(MAKE) ws-tools
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 ASIC=elba firmware
	$(MAKE) -C nic PLATFORM=hw ARCH=aarch64 ASIC=elba package-drivers

naples-firmware-tarball:
	@if [ "x${RELEASE}" = "x" ]; then echo "RELEASE is not set"; else cd ../ ; asset-push --assets-server-colo NULL --remote-name sw-${PIPELINE}.tar.gz builds hourly ${RELEASE} sw || cd sw; fi
	tar -zcf $(NAPLES_FW_TAR) nic/naples_fw.tar nic/naples_fw_.tar --ignore-failed-read nic/naples_upg_fw.tar --ignore-failed-read nic/naples_upg_fw_.tar platform/gen/drivers-*.tar.xz platform/goldfw/naples/naples_fw.tar platform/hosttools nic/host.tar nic/test-utils.tgz  nic/box.rb nic/entrypoint.sh tools/test-build storage/gen/*.tar.xz

naples-firmware-apulu-tarball:
	@if [ "x${RELEASE}" = "x" ]; then echo "RELEASE is not set"; else cd ../ ; asset-push --assets-server-colo NULL --remote-name sw-${PIPELINE}.tar.gz builds hourly ${RELEASE} sw || cd sw; fi
	tar -zcf $(NAPLES_FW_TAR) nic/naples_fw.tar nic/naples_fw_venice.tar nic/naples_fw_.tar --ignore-failed-read nic/naples_upg_fw.tar --ignore-failed-read nic/naples_upg_fw_.tar platform/gen/drivers-*.tar.xz platform/goldfw/naples/naples_fw.tar platform/hosttools nic/host.tar nic/test-utils.tgz  nic/box.rb nic/entrypoint.sh tools/test-build storage/gen/*.tar.xz

naples-firmware-tarball-iris: NAPLES_FW_TAR=naples_fw_all.tgz
naples-firmware-tarball-iris: naples-firmware-tarball

naples-firmware-tarball-apulu: NAPLES_FW_TAR=naples_fw_all_apulu.tgz
naples-firmware-tarball-apulu: naples-firmware-apulu-tarball

naples-protos-apulu:
	tar -zcf naples-protos-apulu.tgz nic/build/aarch64/${PIPELINE}/gen/proto/

naples-firmware-tarball-athena: NAPLES_FW_TAR=naples_fw_all_athena.tgz
naples-firmware-tarball-athena: naples-firmware-tarball

e2e-iota: e2e-naples
	$(MAKE) venice-image
	$(MAKE) venice-upgrade-image
	$(MAKE) -C iota
	$(MAKE) -C iota/test/venice

# venice emulator
palazzo:
	# make ui
	rm -rf $(GOPATH)/src/github.com/pensando/sw/tools/docker-files/apigw/dist && cp -r venice/ui/webapp/dist $(GOPATH)/src/github.com/pensando/sw/tools/docker-files/apigw/
	go install -v github.com/pensando/sw/test/integ/palazzo/

ui-framework:
	yarn -v;
	cd venice/ui/web-app-framework && yarn run pack
	$(MAKE) ui-link-framework

ui-link-framework:
	cd venice/ui/web-app-framework && cd dist && yalc publish --private
	cd venice/ui/webapp && yalc add web-app-framework

ui-venice-sdk:
	cd venice/ui/venice-sdk && yarn add file:pensando-swagger-ts-generator-1.1.29.tgz --offline
	cd venice/ui/venice-sdk && yarn run build && yarn run gen

ui:
	yarn -v;
	$(MAKE) ui-link-framework
	$(MAKE) ui-venice-sdk
	cd venice/ui/webapp && yarn run build-prod && ./gzipDist.sh

ui-serve:
	yarn -v;
	$(MAKE) ui-link-framework
	$(MAKE) ui-venice-sdk
	cd venice/ui/webapp && yarn run dev-container

ui-mac:
	@echo "you will need node v10.15.3, yarn installed";
	@echo "Issues related to node-sass require running make clean before this target";
	node -v;
	yarn -v && yarn global add yalc;
	cd venice/ui/web-app-framework && yarn;
	$(MAKE) ui-framework
	$(MAKE) ui-link-framework
	cd venice/ui/venice-sdk && yarn;
	cd venice/ui/webapp && yarn;
	$(MAKE) ui-venice-sdk
	cd venice/ui/webapp && yarn run build-prod && ./gzipDist.sh

ui-autogen:
	printf "\n+++++++++++++++++ Generating ui-autogen +++++++++++++++++\n";
	echo docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} make ui-venice-sdk; \
	docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} make ui-venice-sdk; \

ui-test-web-app-framework:
	echo docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} bash -c "cd venice/ui/web-app-framework && yarn run test-prod"; \
	docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} bash -c "cd venice/ui/web-app-framework && yarn run test-prod"; \

ui-test-webapp:
	echo docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} bash -c "cd venice/ui/webapp && yarn run test-prod"; \
	docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} bash -c "cd venice/ui/webapp && yarn run test-prod"; \

# runs all UI unit tests
ui-unit-test-cover:
	@BYPASS_DOCS=1 UI_FRAMEWORK=1	$(MAKE) fixtures
	$(MAKE) ui-test-web-app-framework
	$(MAKE) ui-test-webapp

# ui-checks ensure that the UI code meets standards
ui-checks:
	echo docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} bash -c "cd venice/ui/webapp && yarn run lint-ci"; \
	docker run --user $(shell id -u):$(shell id -g)  -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" --rm -v ${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${UI_BUILD_CONTAINER} bash -c "cd venice/ui/webapp && yarn run lint-ci"; \

penctl:
		echo "+++ building penctl sources"; echo docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${PENCTL_BUILD_CMD} ; \
		docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${PENCTL_BUILD_CMD} ; \

upgrade-penctl:
		echo "+++ building penctl sources"; echo docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_UPGRADE_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${PENCTL_BUILD_CMD} ; \
		docker run --user $(shell id -u):$(shell id -g) -e "GIT_COMMIT=${GIT_COMMIT}" -e "GIT_VERSION=${GIT_UPGRADE_VERSION}" -e "BUILD_DATE=${BUILD_DATE}" -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -v${PWD}:/import/src/github.com/pensando/sw${CACHEMOUNT} -v${PWD}/bin/pkg:/import/pkg${CACHEMOUNT} -v${PWD}/bin/cbin:/import/bin${CACHEMOUNT} -w /import/src/github.com/pensando/sw ${REGISTRY_URL}/${BUILD_CONTAINER} ${PENCTL_BUILD_CMD} ; \

venice-image:
	printf "\n+++++++++++++++++ start container-compile $$(date) +++++++++++++++++\n"
	$(MAKE) container-compile
	printf "\n+++++++++++++++++ start fixture $$(date) +++++++++++++++++\n"
	$(MAKE) fixtures
	printf "\n+++++++++++++++++ start penctl $$(date) +++++++++++++++++\n"
	$(MAKE) penctl
	printf "\n+++++++++++++++++ start install $$(date) +++++++++++++++++\n"
	$(MAKE) install
	printf "\n+++++++++++++++++ start tar $$(date) +++++++++++++++++\n"
	#todo compress later in the release cycle with better compression level. As of now compression takes too much time for development
	if [ -a nic/pegasus.tgz ]; then \
		cd bin && tar -cf - tars/*.tar venice-install.json -C ../tools/scripts INSTALL.sh | gzip -1 -c > venice.apulu.tgz; \
        else \
		cd bin && tar -cf - tars/*.tar venice-install.json -C ../tools/scripts INSTALL.sh | gzip -1 -c > venice.tgz; \
        fi
	printf "\n+++++++++++++++++ complete venice-image $$(date) +++++++++++++++++\n"

venice-upgrade-image:
	printf "\n+++++++++++++++++ start upgrade-container-compile $$(date) +++++++++++++++++\n"
	$(MAKE) upgrade-container-compile
	printf "\n+++++++++++++++++ start upgrade-fixture $$(date) +++++++++++++++++\n"
	$(MAKE) upgrade-fixtures
	printf "\n+++++++++++++++++ start upgrade-penctl $$(date) +++++++++++++++++\n"
	$(MAKE) upgrade-penctl
	printf "\n+++++++++++++++++ start upgrade-install $$(date) +++++++++++++++++\n"
	$(MAKE) upgrade-install
	printf "\n+++++++++++++++++ start tar $$(date) +++++++++++++++++\n"
	#todo compress later in the release cycle with better compression level. As of now compression takes too much time for development
	cd bin && tar -cf - tars/*.tar venice-install.json -C ../tools/scripts INSTALL.sh | gzip -1 -c > venice.upg.tgz
	printf "\n+++++++++++++++++ complete venice-upgrade-image $$(date) +++++++++++++++++\n"

ci-venice-apulu-image:
	ls -al nic/pegasus.tgz
	$(MAKE) venice-image APULU_PIPELINE=1
ci-venice-image:
	$(MAKE) venice-image
ci-venice-upgrade-image:
	echo "ci-venice-upgrade-image"
	echo ${GIT_UPGRADE_VERSION}
	echo ${GIT_COMMIT}
	$(MAKE) venice-base-iso
	sudo mkdir -p bin/venice-install
	sudo mkdir -p bin/pxe
	$(MAKE) venice-iso
	cd bin/venice-install && tar -cf - initrd0.img  squashfs.img  vmlinuz0  | gzip -1 -c > venice_appl_os.tgz
	$(MAKE) venice-upgrade-image

# this creates the OS image - like buildroot for venice from centos DVD image
# Only needed to be run when contents of FS need to change
venice-base-iso:
	$(MAKE) -C tools/docker-files/vinstall

# this adds pensando specific scripts and bin/venice.tgz to the squashfs
# and creates the venice iso file (from base iso downloaded from pull-assets)
# This also creates files for the ability to pxe boot appliance installation inside pensando
venice-iso:
	mkdir -p bin/venice-install
	mkdir -p bin/pxe
	$(MAKE) -C tools/docker-files/vinstall venice-iso

venice-apulu-iso:
	mkdir -p bin/venice-install
	mkdir -p bin/pxe
	$(MAKE) -C tools/docker-files/vinstall venice-apulu-iso

bundle-image:
	cd bin/venice-install && tar -cf - initrd0.img  squashfs.img  vmlinuz0  | gzip -1 -c > venice_appl_os.tgz
	mkdir -p bin/bundle
	@ #bundle.py creates metadata.json for the bundle image
	@tools/scripts/bundle.py -v ${BUNDLE_VERSION}  -d ${BUILD_DATE}
	ln -f bin/venice.tgz bin/bundle/venice.tgz
	ln -f nic/naples_fw_.tar bin/bundle/naples_fw.tar
	ln -f bin/venice-install/venice_appl_os.tgz bin/bundle/venice_appl_os.tgz
	cd bin/bundle && tar -cf bundle.tar venice.tgz  naples_fw.tar venice_appl_os.tgz metadata.json

bundle-apulu-image:
	cd bin/venice-install && tar -cf - initrd0.img  squashfs.img  vmlinuz0  | gzip -1 -c > venice_appl_os.tgz
	mkdir -p apulu-bundle/bin
	mkdir -p apulu-bundle/nic
	mkdir -p apulu-bundle/bin/venice-install
	ln -f bin/venice.apulu.tgz apulu-bundle/bin/venice.tgz
	ln -f nic/naples_fw_venice.tar apulu-bundle/nic/naples_fw.tar
	ln -f bin/venice-install/venice_appl_os.tgz apulu-bundle/bin/venice-install/venice_appl_os.tgz
	@ #bundle.py creates metadata.json for the bundle image
	@tools/scripts/bundle.py -v ${BUNDLE_VERSION}  -d ${BUILD_DATE} -p "apulu-bundle/"
	ln -f apulu-bundle/bin/venice.tgz apulu-bundle/venice.tgz
	ln -f apulu-bundle/nic/naples_fw.tar apulu-bundle/naples_fw.tar
	ln -f apulu-bundle/bin/venice-install/venice_appl_os.tgz apulu-bundle/venice_appl_os.tgz
	cd apulu-bundle && tar -cf bundle.tar venice.tgz  naples_fw.tar venice_appl_os.tgz metadata.json
	cd apulu-bundle && cat metadata.json ; ls -al; tar -tvf bundle.tar

bundle-upgrade-image:
	mkdir -p upgrade-bundle/bin
	mkdir -p upgrade-bundle/nic
	mkdir -p upgrade-bundle/bin/venice-install
	ln -f bin/venice.upg.tgz upgrade-bundle/bin/venice.tgz
	ln -f nic/naples_fw_.tar upgrade-bundle/nic/naples_fw.tar
	ln -f bin/venice-install/venice_appl_os.tgz upgrade-bundle/bin/venice-install/venice_appl_os.tgz
	@ #bundle.py creates metadata.json for the bundle image
	@tools/scripts/bundle.py -v ${BUNDLE_UPGRADE_VERSION}  -d ${BUILD_DATE} -p "upgrade-bundle/"
	ln -f upgrade-bundle/bin/venice.tgz upgrade-bundle/venice.tgz
	ln -f upgrade-bundle/nic/naples_fw.tar upgrade-bundle/naples_fw.tar
	ln -f upgrade-bundle/bin/venice-install/venice_appl_os.tgz upgrade-bundle/venice_appl_os.tgz
	cd upgrade-bundle && tar -cf bundle.tar venice.tgz  naples_fw.tar venice_appl_os.tgz metadata.json
	cd upgrade-bundle && cat metadata.json ; ls -al; tar -tvf bundle.tar

VENICE_RELEASE_TAG := v0.3
gs-venice-release: venice-image
	docker pull ${REGISTRY_URL}/${DIND_CONTAINER}
	docker save -o bin/pen-dind.tar ${REGISTRY_URL}/${DIND_CONTAINER}
	docker pull ${REGISTRY_URL}/${E2E_CONTAINER}
	docker save -o bin/pen-e2e.tar ${REGISTRY_URL}/${E2E_CONTAINER}
	cd nic/sim/naples && tar -cf venice-sim.tar venice-bootstrap.sh  -C ../../../test/e2e dind -C ../../bin venice.tgz pen-dind.tar pen-e2e.tar
	ln -f nic/sim/naples/venice-sim.tar tools/docker-files/venice/venice-sim.tar
	cp test/topos/gs/venice-conf.json nic/sim/naples/venice-conf.json
	cd tools/docker-files/venice/ && docker build -t pensando/venice:${VENICE_RELEASE_TAG} .
	cd test/topos/gs && tar -cvf venice_sim_addons.tar naples_admit.py start.sh stop.sh testbed.json venice-conf.json authbootstrap_postman_collection.json login_postman_collection.json postman_collection.json customroles_postman_collection.json


# After testing venice-release upload the script assets with a command like below
# cd test/topos/gs && asset-upload venice_sim_addons.tar v0.2 ./venice_sim_addons.tar
