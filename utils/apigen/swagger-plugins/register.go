package plugins

import (
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	reg "github.com/pensando/grpc-gateway/protoc-gen-swagger/plugins"
	mutator "github.com/pensando/sw/utils/apigen/autogrpc"
)

//--- Mutators functions ---//
func reqMutator(req *plugin.CodeGeneratorRequest) {
	mutator.AddAutoGrpcEndpoints(req)
}

func init() {
	// Register request mutators
	reg.RegisterReqMutator("pensando", reqMutator)
}
