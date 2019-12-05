# Generate metrics manifest.
# Each path where protos for metrics exist is listed below as a tuple of prefix and the path seperated by a :
METRICS_PATHS =\
	nicmgr:nic/proto/nicmgr \
	dropstats:nic/proto/dropstats \
	ftestats:nic/proto/ftestats \
	rulestats:nic/proto/rulestats \
	flowstats:nic/proto/flowstats \
	nmdmetrics:nic/agent/protos/nmd/nmdmetrics

# all generated files are in the generated/<prefix> path
all:
	@- $(foreach PATH,$(METRICS_PATHS), \
        $(eval PREFIX = $(word 1,$(subst :, ,$(PATH)))) \
        $(eval PROTOPATH = $(word 2,$(subst :, ,$(PATH)))) \
   		\
   		${GOPATH}/src/github.com/pensando/sw/tools/scripts/genmetrics.sh ${PREFIX} ${PROTOPATH}; \
		)

	# Auto generate field map assignment
	@- $(foreach PATH,$(METRICS_PATHS), \
        $(eval PREFIX = $(word 1,$(subst :, ,$(PATH)))) \
        $(eval PROTOPATH = $(word 2,$(subst :, ,$(PATH)))) \
		$(foreach protofile,$(shell find ${GOPATH}/src/github.com/pensando/sw/${PROTOPATH}/*.proto), \
			protoc -I. \
				-I${GOPATH}/src \
				-I${GOPATH}/src/github.com/pensando/sw/api \
				-I${GOPATH}/src/github.com/pensando/sw/api/fields \
				-I${GOPATH}/src/github.com/pensando/sw/api/labels \
				-I${GOPATH}/src/github.com/pensando/sw/api/protos \
				-I${GOPATH}/src/github.com/pensando/sw/events/protos \
				-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
				-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
				-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
				-I${GOPATH}/src/github.com/pensando/sw/vendor \
				-I${GOPATH}/src/github.com/pensando/sw/nic/delphi/proto/delphi \
				-I${GOPATH}/src/github.com/pensando/sw/venice/utils/apigen/annotations \
				-I${GOPATH}/src/github.com/pensando/sw/${PROTOPATH} \
				--grpc-gateway_out=S_prefix=${PREFIX},S_path=${GOPATH}/src/github.com/pensando/sw/metrics,logtostderr=false,v=7,gengw=false,templates=github.com/pensando/sw/metrics/templates/msgfieldmap.yaml,log_dir=${GOPATH}/src/github.com/pensando/sw/metrics/tmp:${GOPATH}/src/github.com/pensando/sw/metrics/genfields \
				${protofile}; \
			) \
		)

	# goimports generated go files
	@goimports -local "github.com/pensando/sw" -w ${GOPATH}/src/github.com/pensando/sw/metrics/genfields/*.go
	
clean:
	@rm -rf generated/*
	@rm -rf genfields/*map.go

gen-clean: clean
