package main

import (
	"flag"

	"github.com/pensando/sw/iota/svcs/server"
)

func main() {
	var stubMode = flag.Bool("stubmode", false, "Start IOTA Server in stub mode")
	flag.Parse()
	server.StartIOTAService(*stubMode)
}
