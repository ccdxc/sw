OutDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi

default: restgen goimports
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	--gofast_out=plugins=grpc:. mirror.proto

restgen:
	goimports -local "github.com/pensando/sw" -l -w ${OutDir}/*.go

goimports: restgen
	goimports -local "github.com/pensando/sw" -l -w *.go
