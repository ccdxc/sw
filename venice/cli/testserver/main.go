package main

import (
	"github.com/pensando/sw/venice/cli/testserver/tserver"
)

func main() {
	tserver.Start()
	select {}
}
