NICDIR := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi
SRC := $(shell ls *.proto)
DEST := $(SRC:%.proto=%.pb.go)

default: ${DEST}

%.pb.go: %.proto Makefile
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        --gofast_out=plugins=grpc:. $<
	sed -i.bak_ 's/api\/protos/api\/generated\/monitoring/g' $@
	rm -f *.bak_

#	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src \
#	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party \
#	-I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
#	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi/templates/rest.yml,log_dir=${OutDir}/tmp,logtostderr=false,v=7:${NICDIR} $<
#	goimports -local "github.com/pensando/sw" -l -w ${NICDIR}/*.go
	goimports -local "github.com/pensando/sw" -l -w *.go

clean:
	rm -f *.pb.go
