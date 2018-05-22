OutDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi

default: restgen goimports
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	--gofast_out=plugins=grpc:. network.proto endpoint.proto security.proto tenant.proto interface.proto nat.proto route.proto namespace.proto ipsec.proto sgpolicy.proto match.proto service.proto

restgen:
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi/templates/rest.yml,log_dir=${OutDir}/tmp,logtostderr=false,v=7:${OutDir} network.proto endpoint.proto security.proto tenant.proto interface.proto nat.proto route.proto namespace.proto ipsec.proto
	goimports -local "github.com/pensando/sw" -l -w ${OutDir}/*.go

goimports: restgen
	goimports -local "github.com/pensando/sw" -l -w *.go
