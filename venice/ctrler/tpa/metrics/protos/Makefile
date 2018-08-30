NICDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi
SRC := $(shell ls *.proto)
DEST := $(SRC:%.proto=%.pb.go)
NICFILES := ${NICDIR}/metrics_api.go ${NICDIR}/metrics_api_test.go

default: ${DEST} template

%.pb.go: %.proto Makefile 
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        --gofast_out=plugins=grpc:. $<
	sed -i.bak_ 's/api\/protos/api\/generated\/monitoring/g' $@
	rm -f *.bak_
	goimports -local "github.com/pensando/sw" -w *.go

template: 
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/tpa/metrics/restapi/templates/rest.yml,log_dir=${NICDIR}/tmp,logtostderr=false,v=7:${NICDIR} *.proto
	goimports -local "github.com/pensando/sw" -w ${NICFILES}

clean:
	rm -f *.pb.go
	rm -f ${NICFILES}
