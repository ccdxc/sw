OutDir := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/generated/restapi/tsagent

DEST := techsupport.pb.go diagnostics.pb.go ${OutDir}/mirror_api.go ${OutDir}/mirror_api_test.go
default: ${DEST}

techsupport.pb.go: techsupport.proto
	protoc -I/usr/local/include -I. -I../../../../vendor -I${GOPATH}/src -I../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	--gofast_out=plugins=grpc:. techsupport.proto
	sed -i.bak_ 's/api\/protos/api\/generated\/monitoring/g' *.go
	rm -f *.bak_

diagnostics.pb.go: diagnostics.proto
	protoc -I/usr/local/include -I. -I../../../../vendor -I${GOPATH}/src -I../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	--gofast_out=plugins=grpc:. diagnostics.proto
	sed -i.bak_ 's/api\/protos/api\/generated\/diagnostics/g' *.go
	rm -f *.bak_

${OutDir}/%_api.go ${OutDir}/%_api_test.go: mirror.proto
	protoc -I. -I../../../../vendor -I${GOPATH}/src -I../../../../nic/agent/protos/netproto -I../../../../vendor/github.com/pensando/grpc-gateway/third_party -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
	--grpc-gateway_out=templates=${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/restapi/tsagent/rest.yml,log_dir=${OutDir}/tmp,logtostderr=false,v=7:${OutDir} $*.proto
	goimports -local "github.com/pensando/sw" -l -w ${OutDir}/$*_api.go ${OutDir}/$*_api_test.go

gen-clean:
	rm -f ${DEST}
