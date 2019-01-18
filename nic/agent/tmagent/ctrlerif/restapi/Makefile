TMPLATEDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi/templates
SRCDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi/
APIROUTES := $(shell grep "func[ ]*add.*MetricsAPIRoutes[ ]*(.*)" *.go | cut -d ' ' -f2 | sed -e 's/add//' | sed -e 's/APIRoutes.*$$//g')
PROTOFILE := /tmp/register-temp.proto
SRCFILE := register.go

# create .proto file and translate it using templates
gen:gen-clean
	@echo "syntax = \"proto3\";" > ${PROTOFILE}
	@echo "package registerproto;" >> ${PROTOFILE}
	@echo $(addprefix message ,$(addsuffix {},${APIROUTES})) >> ${PROTOFILE}
	protoc -I/usr/local/include -I. -I/tmp \
        -I../../../../../vendor -I${GOPATH}/src \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	--grpc-gateway_out=templates=${TMPLATEDIR}/register.yml,log_dir=${SRCDIR}/tmp,logtostderr=false,v=7:${SRCDIR} ${PROTOFILE}
	sed -i.bak_ -e '/^$$/d' ${SRCDIR}/${SRCFILE}
	rm -f *.bak_
	goimports -local "github.com/pensando/sw" -w ${SRCDIR}/${SRCFILE}

all:gen
gen-clean:
	rm -f ${SRCFILE}
