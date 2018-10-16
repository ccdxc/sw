NICDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi
SRC := $(shell ls *.proto)
DEST := $(SRC:%.proto=%.pb.go)
NICFILES := ${NICDIR}/tpm_api.go ${NICDIR}/tpm_api_test.go

default: ${DEST} ${NICFILES}

%.pb.go: %.proto Makefile
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        --gofast_out=plugins=grpc:. $<
	sed -i.bak_ 's/api\/protos/api\/generated\/monitoring/g' $@
	sed -i.bak_ '/import _ "."/d' $@
	rm -f *.bak_
	goimports -local "github.com/pensando/sw" -w *.go

${NICDIR}/%_api.go ${NICDIR}/%_api_test.go: %.proto
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I${GOPATH}/src/github.com/pensando/sw/api/protos \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/tpa/ctrlerif/restapi/templates/rest.yml,log_dir=${NICDIR}/tmp,logtostderr=false,v=7:${NICDIR} *.proto
	goimports -local "github.com/pensando/sw" -w ${NICDIR}/$*_api.go ${NICDIR}/$*_api_test.go

clean:
	rm -f *.pb.go
	rm -f ${NICFILES}

gen-clean: clean
