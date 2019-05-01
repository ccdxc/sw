TPADIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/generated/restapi/tpagent
TMAGENTDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/generated/restapi/tmagent
SRC := $(shell ls *.proto)
DEST := $(SRC:%.proto=%.pb.go)
NICFILES := ${TPADIR}/tpm_api.go ${TPADIR}/tpm_api_test.go ${TMAGENTDIR}/tpm_api.go ${TMAGENTDIR}/tpm_api_test.go

TMPLATEDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/restapi/tmagent
APIROUTES := $(shell grep "Add.*MetricsAPIRoutes[ ]*(.*)" ../generated/restapi/tmagent/*.go | cut -d ' ' -f4 | sed -e 's/Add//' | sed -e 's/APIRoutes.*$$//g')
PROTOFILE := /tmp/register-temp.proto
SRCFILE := register.go

.PHONY: gen-tmagent

default: ${DEST} ${NICFILES} gen-tmagent

%.pb.go: %.proto Makefile
	protoc -I/usr/local/include -I. -I../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
        --gofast_out=plugins=grpc:. $<
	sed -i.bak_ '/import monitoring "."/d' $@
	sed -i.bak_ 's/import monitoring2.*/import monitoring "github.com\/pensando\/sw\/api\/generated\/monitoring"/g' $@
	sed -i.bak_ 's/monitoring2/monitoring/g' $@
	rm -f ${TMAGENTDIR}/*.bak_
	goimports -local "github.com/pensando/sw" -w ${TMAGENTDIR}/*.go

${TPADIR}/%_api.go ${TPADIR}/%_api_test.go: %.proto
	protoc -I/usr/local/include -I. -I../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I${GOPATH}/src/github.com/pensando/sw/api/protos \
	-I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/restapi/tpagent/rest.yml,log_dir=${TPADIR}/tmp,logtostderr=false,v=7:${TPADIR} *.proto
	goimports -local "github.com/pensando/sw" -w ${TPADIR}/$*_api.go ${TPADIR}/$*_api_test.go

${TMAGENTDIR}/tpm_api.go:${TPADIR}/tpm_api.go
	cp $< $@


${TMAGENTDIR}/tpm_api_test.go:${TPADIR}/tpm_api_test.go
	cp $< $@
	rm -f ${TMAGENTDIR}/test/*.bak_


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
	rm -f *.bak_
	goimports -local "github.com/pensando/sw" -w ${TMAGENTDIR}/${SRCFILE}

clean:
	rm -f *.pb.go
	rm -f ${NICFILES}

gen-clean: clean
