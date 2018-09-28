OutDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi

default: restgen goimports
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--gofast_out=plugins=grpc:. mirror.proto
	sed -i.bak_ 's/api\/protos/api\/generated\/monitoring/g' *.go
	rm -f *.bak_

restgen:
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/troubleshooting/ctrlerif/restapi/templates/rest.yml,log_dir=${OutDir}/tmp,logtostderr=false,v=7:${OutDir} mirror.proto
	goimports -local "github.com/pensando/sw" -l -w ${OutDir}/*.go

goimports: restgen
	goimports -local "github.com/pensando/sw" -l -w *.go
