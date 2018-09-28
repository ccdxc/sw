package main

import (
	"github.com/pensando/sw/iota/svcs/server"
)

func main() {
	waitCh := make(chan bool)
	server.StartIOTAService()
	<-waitCh
}
