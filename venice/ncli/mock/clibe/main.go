package main

import (
	"fmt"
	"os"
	"os/signal"
	"time"

	"github.com/pensando/sw/venice/ncli/mock"
)

func main() {
	ti := mock.Start()
	fmt.Printf("========================= starting server =========================\n")
	time.Sleep(1)
	fmt.Printf("venice --server http://localhost:" + ti.VenicePort + " login -u " + ti.UserCred.Username + " -p " + ti.UserCred.Password)

	signalCh := make(chan os.Signal, 1)
	signal.Notify(signalCh, os.Interrupt)
	select {
	case <-signalCh:
		break
	}
	ti.Stop()
	fmt.Printf("========================= stopping server =========================\n")
}
