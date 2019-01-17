package main

import (
	"fmt"
	"time"

	"github.com/pensando/sw/nic/delphi/gosdk"
)

// This binary starts a fake delphi hub server. This is needed to get venice dind e2e to work.
func main() {
	waitCh := make(chan bool)
	fmt.Println("Starting Fake Delphi Hub")
	fakeHub := gosdk.NewFakeHub()
	fakeHub.Start()
	time.Sleep(time.Second * 5)
	fmt.Println("Delphi hub is running...")
	// wait forever
	<-waitCh
}
