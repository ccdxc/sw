SRC := $(shell ls ${GOPATH}/src/github.com/pensando/sw/nic/proto/nicmgr/metrics.proto)
#DEST := $(SRC:%.proto=%.pb.go)
NAMES = $(notdir $(basename $(SRC)))
NICDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi
NICFILES := ${NICDIR}/${NAMES}_apigen.go
PENDIR := ${GOPATH}/src/github.com/pensando/sw/penctl/cmd
PENFILES := ${PENDIR}/${NAMES}_gen.go

$(info $$NICDIR: ${NICDIR})
$(info $$PENDIR: ${PENDIR})
$(info $$NAMES: ${NAMES})
$(info $$SRC: ${SRC})
$(info $$DEST: ${DEST})
$(info $$NICFILES: ${NICFILES})
$(info $$PENFILES: ${PENFILES})

#default: ${DEST} template penctl_template
default: template penctl_template

%.pb.go: %.proto Makefile 
	mkdir -p ${NICDIR}
	protoc -I/usr/local/include -I. -I../../../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
        -I ${GOPATH}/src/github.com/pensando/sw/nic/delphi/proto/delphi \
        -I${GOPATH}/src/github.com/pensando/sw/ \
        -I${GOPATH}/src/github.com/pensando/sw/api/ \
	-I../../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        --gofast_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,plugins=grpc:. $<
	mv $(DEST:/usr/src/%=%) .
	sed -i.bak_ 's/api\/protos/api\/generated\/monitoring/g' $(notdir $@)
	sed -i.bak_ '/import _ "."/d' $(notdir $@)
	rm -f *.bak_
	goimports -local "github.com/pensando/sw" -w *.go
	rm -rf github.com

template: 
	protoc -I/usr/local/include -I. -I../../../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
        -I${GOPATH}/src/github.com/pensando/sw/ \
        -I ${GOPATH}/src/github.com/pensando/sw/nic/delphi/proto/delphi \
        -I${GOPATH}/src/github.com/pensando/sw/api/ \
	-I../../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I${GOPATH}/src/github.com/pensando/sw/api/protos \
	-I../../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi/templates/rest.yml,log_dir=${NICDIR}/tmp,logtostderr=false,v=7:${NICDIR} $(SRC)
	goimports -local "github.com/pensando/sw" -w ${NICFILES}

penctl_template:
	protoc -I/usr/local/include -I. -I../../../../../../vendor -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos/ \
        -I${GOPATH}/src/github.com/pensando/sw/ \
        -I${GOPATH}/src/github.com/pensando/sw/api/ \
        -I ${GOPATH}/src/github.com/pensando/sw/nic/delphi/proto/delphi \
	-I../../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I${GOPATH}/src/github.com/pensando/sw/api/protos \
	-I../../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/penctl/cmd/metrics/templates/rest.yml,log_dir=${PENDIR}/tmp,logtostderr=false,v=7:${PENDIR} $(SRC)
	goimports -local "github.com/pensando/sw" -w ${PENFILES}


clean:
	rm -f ${NICFILES} ${PENFILES}
