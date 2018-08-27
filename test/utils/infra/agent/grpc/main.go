package main

import (
	"flag"

	server "github.com/pensando/sw/test/utils/infra/agent/grpc/server"
)

//CmdArgs struct
type CmdArgs struct {
	Port *int
}

var _CmdArgs = CmdArgs{}

func main() {
	flag.Parse()
	server.StartServer(*_CmdArgs.Port)
}

func init() {
	_CmdArgs.Port = flag.Int("port", 9999, "Agent port")
}
