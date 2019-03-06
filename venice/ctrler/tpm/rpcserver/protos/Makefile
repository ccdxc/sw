NETAGENTDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi
TMAGENTDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi
SRC := $(shell ls *.proto)
DEST := $(SRC:%.proto=%.pb.go)
NICFILES := ${NETAGENTDIR}/tpm_api.go ${NETAGENTDIR}/tpm_api_test.go ${TMAGENTDIR}/tpm_api.go ${TMAGENTDIR}/test/tpm_api_test.go

default: ${DEST} ${NICFILES}

%.pb.go: %.proto Makefile
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
        --gofast_out=plugins=grpc:. $<
	sed -i.bak_ '/import monitoring "."/d' $@
	sed -i.bak_ 's/import monitoring2.*/import monitoring "github.com\/pensando\/sw\/api\/generated\/monitoring"/g' $@
	sed -i.bak_ 's/monitoring2/monitoring/g' $@
	rm -f *.bak_
	goimports -local "github.com/pensando/sw" -w *.go

${NETAGENTDIR}/%_api.go ${NETAGENTDIR}/%_api_test.go: %.proto
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I${GOPATH}/src/github.com/pensando/sw/api/protos \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/tpa/ctrlerif/restapi/templates/rest.yml,log_dir=${NETAGENTDIR}/tmp,logtostderr=false,v=7:${NETAGENTDIR} *.proto
	goimports -local "github.com/pensando/sw" -w ${NETAGENTDIR}/$*_api.go ${NETAGENTDIR}/$*_api_test.go

${TMAGENTDIR}/tpm_api.go:${NETAGENTDIR}/tpm_api.go
	cp $< $@


${TMAGENTDIR}/test/tpm_api_test.go:${NETAGENTDIR}/tpm_api_test.go
	cp $< $@
	sed -i.bak_ 's/package restapi/package resttest/' $@
	rm -f ${TMAGENTDIR}/test/*.bak_

clean:
	rm -f *.pb.go
	rm -f ${NICFILES}

gen-clean: clean
