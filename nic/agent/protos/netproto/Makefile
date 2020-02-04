OutDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/generated/restapi/netagent
NimbusDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/generated/nimbus
RestClientDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/generated/restclient

# all the sources for which .pb.go is generated
SRC := app.proto agg_watch.proto endpoint.proto  flowexport.proto interface.proto ipam.proto match.proto mirror.proto namespace.proto network.proto port.proto profile.proto route.proto sgpolicy.proto secprofile.proto tenant.proto tunnel.proto types.proto vrf.proto
NIMBUS_SRC := app.proto endpoint.proto interface.proto ipam.proto network.proto profile.proto route.proto sgpolicy.proto secprofile.proto vrf.proto

#these wont have _api.go generated
FILTER_OUT_API := agg_watch.proto ipam.proto  match.proto namespace.proto tenant.proto port.proto route.proto

#additionally these wont have swagger spec generated
FILTER_OUT_SWAGGER := qos.proto copp.proto

SRC1 :=  $(filter-out ${FILTER_OUT_API}, ${SRC})
SRC2 :=  $(filter-out ${FILTER_OUT_SWAGGER}, ${SRC1})

DEST := $(SRC:%.proto=%.pb.go) $(SRC1:%.proto=$(OutDir)/%_api.go) $(SRC1:%.proto=$(RestClientDir)/%_client.go) $(SRC2:%.proto=$(RestClientDir)/docs/swagger/%.swagger.json)
NIMBUS_DEST := $(NIMBUS_SRC:%.proto=$(NimbusDir)/%_mserver.go)
DELPHI_SRC := $(NIMBUS_SRC:%.proto=./delphi/%.proto)

default: ${DEST} nimbus delphi-gen

app.pb.% endpoint.pb.% flowexport.pb.% interface.pb.% ipam.pb.% match.pb.% mirror.pb.% namespace.pb.% network.pb.% port.pb.% secprofile.pb.% sgpolicy.pb.% tenant.pb.% tunnel.pb.% types.pb.% vrf.pb.%:  ${SRC}
	protoc -I. -I../../../../vendor -I${GOPATH}/src -I../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../vendor/github.com/gogo/protobuf/gogoproto \
	-I ../../../delphi/proto/delphi \
	--gogofast_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,plugins=grpc:. ${SRC}
	goimports -local "github.com/pensando/sw" -l -w *.go

${OutDir}/%_api.go : %.proto
	protoc -I. -I../../../../vendor -I${GOPATH}/src -I../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../vendor/github.com/gogo/protobuf/gogoproto \
	-I ../../../delphi/proto/delphi \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/restapi/netagent/rest.yml,log_dir=${OutDir}/tmp,logtostderr=false,v=7:${OutDir} $^
	cd ${OutDir} && goimports -local "github.com/pensando/sw" -l -w $*_api.go

${RestClientDir}/%_client.go : %.proto
	protoc -I. -I../../../../vendor -I${GOPATH}/src -I../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../vendor/github.com/gogo/protobuf/gogoproto \
	-I ../../../delphi/proto/delphi \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/restclient/restclient.yml,log_dir=${RestClientDir}/tmp,logtostderr=false,v=7:${RestClientDir} $^
	cd ${RestClientDir} && goimports -local "github.com/pensando/sw" -l -w $*_client.go

${RestClientDir}/docs/swagger/%.swagger.json : %.proto
	protoc -I. -I../../../../vendor -I${GOPATH}/src -I../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../vendor/github.com/gogo/protobuf/gogoproto \
	-I ../../../delphi/proto/delphi \
	--swagger_out=logtostderr=false,v=7,log_dir=${RestClientDir}/tmp:${RestClientDir}/docs/swagger $^

nimbus: ${NIMBUS_DEST}

${NimbusDir}/%_mserver.go : %.proto
	protoc -I. -I../../../../vendor -I${GOPATH}/src -I../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../vendor/github.com/gogo/protobuf/gogoproto \
	-I ../../../delphi/proto/delphi \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/nimbus/pre_nimbus_gen.yml,log_dir=${NimbusDir}/tmp,logtostderr=false,v=7:${NimbusDir} $^

	protoc -I. -I../../../../vendor -I${GOPATH}/src -I../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	-I../../../../vendor/github.com/gogo/protobuf/gogoproto \
	-I ../../../delphi/proto/delphi \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/nimbus/nimbus.yml,log_dir=${NimbusDir}/tmp,logtostderr=false,v=7:${NimbusDir} $^
	cd ${NimbusDir} && goimports -local "github.com/pensando/sw" -l -w $*_mserver.go

# Include paths lists netproto before the delphi path to workaround protoc duplicate checks for included files.
delphi-gen: ${DELPHI_SRC}
	protoc --delphigo_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mroute.proto=github.com/pensando/sw/nic/agent/protos/netproto,Mvrf.proto=github.com/pensando/sw/nic/agent/protos/netproto:../generated/delphi/netproto/ ${DELPHI_SRC} \
              -I ${GOPATH}/src -I ${GOPATH}/src/github.com/pensando/sw  -I../ -I .\
            	-I ../../../../vendor -I ../../../../vendor/github.com/gogo/protobuf/gogoproto/ \
				-I ../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
                -I ../../../delphi/proto/delphi
	goimports -w ../generated/delphi/netproto/delphi/*.go

gen-clean:
	rm -f ${DEST}
	rm -f ${NIMBUS_DEST}
	rm -f *schema.go
