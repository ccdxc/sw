package main

import (
	"flag"

	"github.com/pensando/sw/iota/svcs/server"
)

func main() {
	var stubMode = flag.Bool("stubmode", false, "Start IOTA Server in stub mode")
    var grpcPort = flag.Int("port", 60000, "IOTA Server GRPC Port")
	flag.Parse()
	server.StartIOTAService(*stubMode, *grpcPort)
}
