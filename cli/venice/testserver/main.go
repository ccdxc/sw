package main

import (
	"github.com/pensando/sw/cli/venice/testserver/tserver"
)

func main() {
	port := ":9001"
	tserver.Start(port)
	select {}
}
