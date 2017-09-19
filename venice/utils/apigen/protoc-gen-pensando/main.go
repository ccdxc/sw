package main

import (
	"github.com/gogo/protobuf/vanity"
	"github.com/gogo/protobuf/vanity/command"
	"github.com/golang/glog"
	"github.com/pensando/sw/venice/utils/apigen/autogrpc"
)

func main() {
	defer glog.Flush()
	req := command.Read()
	files := req.GetProtoFile()

	autogrpc.AddAutoGrpcEndpoints(req)

	files = vanity.FilterFiles(files, vanity.NotGoogleProtobufDescriptorProto)

	vanity.ForEachFile(files, vanity.TurnOnMarshalerAll)
	vanity.ForEachFile(files, vanity.TurnOnSizerAll)
	vanity.ForEachFile(files, vanity.TurnOnUnmarshalerAll)

	resp := command.Generate(req)
	command.Write(resp)
}
