OutDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi
NimbusDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/protos/generated/nimbus

# all the sources for which .pb.go is generated
SRC := app.proto endpoint.proto copp.proto interface.proto ipsec.proto match.proto namespace.proto nat.proto network.proto port.proto qos.proto route.proto security.proto service.proto sgpolicy.proto secprofile.proto tenant.proto tunnel.proto tcp_proxy.proto
NIMBUS_SRC := app.proto endpoint.proto namespace.proto  network.proto  security.proto sgpolicy.proto secprofile.proto tenant.proto

#these wont have _api.go generated
FILTER_OUT_API := match.proto service.proto qos.proto copp.proto

#additionally these wont have swagger spec generated
FILTER_OUT_SWAGGER := interface.proto qos.proto copp.proto

SRC1 :=  $(filter-out ${FILTER_OUT_API}, ${SRC})
SRC2 :=  $(filter-out ${FILTER_OUT_SWAGGER}, ${SRC1})

DEST := $(SRC:%.proto=%.pb.go) $(SRC1:%.proto=$(OutDir)/%_api_test.go) $(SRC1:%.proto=$(OutDir)/%_api.go) $(SRC1:%.proto=$(OutDir)/restclient/%_client.go) $(SRC2:%.proto=$(OutDir)/docs/swagger/%.swagger.json)
NIMBUS_DEST := $(NIMBUS_SRC:%.proto=$(NimbusDir)/%_mserver.go) 

default: ${DEST} nimbus

copp.pb.% app.pb.% endpoint.pb.% interface.pb.% ipsec.pb.% match.pb.% namespace.pb.% nat.pb.% network.pb.% port.pb.% route.pb.% security.pb.% secprofile.pb.% service.pb.% sgpolicy.pb.% tenant.pb.% tunnel.pb.% tcp_proxy.pb.%:  ${SRC}
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../../vendor/github.com/gogo/protobuf/gogoproto \
	--gofast_out=plugins=grpc:. ${SRC}
	goimports -local "github.com/pensando/sw" -l -w *.go

${OutDir}/%_api.go ${OutDir}/%_api_test.go ${OutDir}/restclient/%_client.go : %.proto
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../../vendor/github.com/gogo/protobuf/gogoproto \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi/templates/rest.yml,log_dir=${OutDir}/tmp,logtostderr=false,v=7:${OutDir} $^
	cd ${OutDir} && goimports -local "github.com/pensando/sw" -l -w $*_api.go $*_api_test.go restclient/$*_client.go

# Generating documentation for interface.proto is ignnored, Currently we don't intened to expose enic and uplink creates directly to the user
${OutDir}/docs/swagger/%.swagger.json : %.proto
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../../vendor/github.com/gogo/protobuf/gogoproto \
	--swagger_out=logtostderr=false,v=7,log_dir=${OutDir}/tmp:${OutDir}/docs/swagger $^

nimbus: ${NIMBUS_DEST}

${NimbusDir}/%_mserver.go : %.proto
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../../vendor/github.com/gogo/protobuf/gogoproto \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/protos/templates/nimbus/nimbus.yml,log_dir=${NimbusDir}/tmp,logtostderr=false,v=7:${NimbusDir} $^
	cd ${NimbusDir} && goimports -local "github.com/pensando/sw" -l -w $*_mserver.go


gen-clean:
	rm -f ${DEST}
