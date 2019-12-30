TPADIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/generated/restapi/tpagent
TMAGENTDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/generated/restapi/tmagent
SRC := $(shell ls *.proto)
DEST := $(SRC:%.proto=%.pb.go)
NICFILES := ${TPADIR}/tpm_api.go ${TPADIR}/tpm_api_test.go ${TMAGENTDIR}/tpm_api.go ${TMAGENTDIR}/tpm_api_test.go
METRICSGENDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/tests/fwloggen

TMPLATEDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/restapi/tmagent
APIROUTES := $(shell grep "Add.*MetricsAPIRoutes[ ]*(.*)" ../generated/restapi/tmagent/*.go | cut -d ' ' -f4 | sed -e 's/Add//' | sed -e 's/APIRoutes.*$$//g')
PROTOFILE := /tmp/register-temp.proto
SRCFILE := register.go

.PHONY: gen-tmagent

default: ${DEST} ${NICFILES} gen-tmagent gen-restserver

%.pb.go: %.proto Makefile
	protoc -I/usr/local/include -I. -I../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
        -I../../../../nic/agent/protos/netproto \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
        --gofast_out=plugins=grpc:. $<
	sed -i.bak_ '/import monitoring "."/d' $@
	sed -i.bak_ 's/import monitoring2.*/import monitoring "github.com\/pensando\/sw\/api\/generated\/monitoring"/g' $@
	sed -i.bak_ 's/monitoring2/monitoring/g' $@
	rm -f *.bak_
	goimports -local "github.com/pensando/sw" -w ${TMAGENTDIR}/*.go

${TPADIR}/%_api.go ${TPADIR}/%_api_test.go: %.proto
	protoc -I/usr/local/include -I. -I../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../nic/agent/protos/netproto \
	-I${GOPATH}/src/github.com/pensando/sw/api/protos \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/restapi/tpagent/rest.yml,log_dir=${TPADIR}/tmp,logtostderr=false,v=7:${TPADIR} *.proto
	goimports -local "github.com/pensando/sw" -w ${TPADIR}/*.go 

gen-restserver:
	 protoc -I/usr/local/include -I. -I../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
        -I../../../../vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos \
        -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I../../../../vendor/github.com/gogo/protobuf/protobuf \
        -I../../../../nic/agent/protos/netproto \
        --grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/restapi/tmagent/rest.yml,log_dir=${TMAGENTDIR}/tmp,logtostderr=false,v=7:${TMAGENTDIR} *.proto 
	rm -f ${TMAGENTDIR}/tpm_apigen.go
	goimports -local "github.com/pensando/sw" -w ${TMAGENTDIR}/*.go 

${TMAGENTDIR}/tpm_api.go:${TPADIR}/tpm_api.go
	cp $< $@

${TMAGENTDIR}/tpm_api_test.go:${TPADIR}/tpm_api_test.go
	cp $< $@

# create .proto file and translate it using templates
gen-tmagent:
	@echo "syntax = \"proto3\";" > ${PROTOFILE}
	@echo "package registerproto;" >> ${PROTOFILE}
	@echo $(addprefix message ,$(addsuffix {},${APIROUTES})) >> ${PROTOFILE}
	protoc -I/usr/local/include -I. -I/tmp \
        -I../../../../vendor -I${GOPATH}/src \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	--grpc-gateway_out=templates=${TMPLATEDIR}/register.yml,log_dir=${TMAGENTDIR}/tmp,logtostderr=false,v=7:${TMAGENTDIR} ${PROTOFILE}
	sed -i.bak_ -e '/^$$/d' ${TMAGENTDIR}/${SRCFILE}
	rm -f ${TMAGENTDIR}/*.bak_
	goimports -local "github.com/pensando/sw" -w ${TMAGENTDIR}/*.go
	protoc -I/usr/local/include -I. -I/tmp \
            -I../../../../vendor -I${GOPATH}/src \
    	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party \
    	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
    	-I../../../../nic/agent/protos/netproto \
    	--grpc-gateway_out=templates=${TMPLATEDIR}/metricsgen.yml,log_dir=${TMAGENTDIR}/tmp,logtostderr=false,v=7:${METRICSGENDIR} ${PROTOFILE}
	goimports -local "github.com/pensando/sw" -w ${METRICSGENDIR}/*.go


clean:
	rm -f *.pb.go
	rm -f ${NICFILES}
	rm -f ${TPADIR}/*_api.go ${TPADIR}/*_api_test.go
	rm -f ${TMAGENTDIR}/restapi.go ${TMAGENTDIR}/restapi_test.go

gen-clean: clean
