package main

import (
	"flag"

	Globals "github.com/pensando/sw/iota/svcs/globals"
	server "github.com/pensando/sw/iota/svcs/server"
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
	_CmdArgs.Port = flag.Int("port", Globals.IotaSvcPort, "Iota port")
}
