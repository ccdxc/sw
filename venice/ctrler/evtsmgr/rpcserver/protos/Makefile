EXTRA_INCLUDES =  -I${GOPATH}/src/github.com/pensando/sw/api/protos
EXTRA_GOFAST_OUT = Mgithub.com/pensando/sw/api/protos/events.proto=github.com/pensando/sw/api/generated/events,Mgithub.com/pensando/sw/api/protos/eventpolicy.proto=github.com/pensando/sw/api/generated/monitoring,
AGENTDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/nevtsproxy/ctrlerif/restapi
AGENTFILES := ${AGENTDIR}/eventpolicy_api.go ${AGENTDIR}/eventpolicy_api_test.go
SERVICE_PROTOS := $(shell ls *.proto)

all:  $(SERVICE_PROTOS:.proto=.pb.go) ${AGENTFILES}
include ../../../../Makefile.proto

${AGENTDIR}/%_api.go ${AGENTDIR}/%_api_test.go: %.proto
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I${GOPATH}/src/github.com/pensando/sw/api/protos \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/nevtsproxy/ctrlerif/restapi/templates/rest.yml,log_dir=${AGENTDIR}/tmp,logtostderr=false,v=7:${AGENTDIR} $*.proto
	goimports -local "github.com/pensando/sw" -w ${AGENTDIR}/$*_api.go ${AGENTDIR}/$*_api_test.go

gen-clean:
	rm -f ${DEST}
	rm -f ${AGENTFILES}