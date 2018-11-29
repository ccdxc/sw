OutDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi

DEST := mirror.pb.go  techsupport.pb.go ${OutDir}/mirror_api.go ${OutDir}/mirror_api_test.go ${OutDir}/techsupport_api.go
default: ${DEST}

mirror.pb.go: mirror.proto
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--gofast_out=plugins=grpc:. mirror.proto
	sed -i.bak_ 's/api\/protos/api\/generated\/monitoring/g' *.go
	rm -f *.bak_

techsupport.pb.go: techsupport.proto
	protoc -I/usr/local/include -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--gofast_out=plugins=grpc:. techsupport.proto
	sed -i.bak_ 's/api\/protos/api\/generated\/monitoring/g' *.go
	rm -f *.bak_

${OutDir}/%_api.go ${OutDir}/%_api_test.go: %.proto
	protoc -I. -I../../../../../vendor -I${GOPATH}/src -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../../vendor/github.com/gogo/protobuf/protobuf \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/troubleshooting/ctrlerif/restapi/templates/rest.yml,log_dir=${OutDir}/tmp,logtostderr=false,v=7:${OutDir} $*.proto
	goimports -local "github.com/pensando/sw" -l -w ${OutDir}/$*_api.go ${OutDir}/$*_api_test.go

gen-clean:
	rm -f ${DEST}
