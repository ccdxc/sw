OutDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi

default: restgen swaggergen goimports
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--gofast_out=plugins=grpc:. endpoint.proto interface.proto ipsec.proto match.proto namespace.proto nat.proto network.proto port.proto route.proto security.proto service.proto sgpolicy.proto tenant.proto tunnel.proto tcp_proxy.proto

restgen:
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi/templates/rest.yml,log_dir=${OutDir}/tmp,logtostderr=false,v=7:${OutDir} endpoint.proto interface.proto ipsec.proto namespace.proto nat.proto network.proto route.proto security.proto sgpolicy.proto tenant.proto tunnel.proto tcp_proxy.proto
	goimports -local "github.com/pensando/sw" -l -w ${OutDir}/*.go

# Generating documentation for interface.proto is ignnored, Currently we don't intened to expose enic and uplink creates directly to the user
swaggergen:
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--swagger_out=logtostderr=false,v=7,log_dir=${OutDir}/tmp:${OutDir}/docs/swagger endpoint.proto ipsec.proto namespace.proto nat.proto network.proto route.proto security.proto sgpolicy.proto tenant.proto tunnel.proto tcp_proxy.proto

goimports: restgen
	goimports -local "github.com/pensando/sw" -l -w *.go
