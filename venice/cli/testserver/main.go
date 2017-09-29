package main

import (
	"github.com/pensando/sw/venice/cli/testserver/tserver"
)

func main() {
	port := ":19001"
	tserver.Start(port)
	select {}
}
