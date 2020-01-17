package main

import (
	"flag"
	"fmt"

	hal "github.com/pensando/sw/nic/agent/cmd/fakehal/hal"
	"github.com/pensando/sw/nic/agent/dscagent/types"
)

func main() {
	var halPort = flag.String("port", types.HalGRPCDefaultPort, "HAL Port")
	flag.Parse()
	halURL := fmt.Sprintf("127.0.0.1:%s", *halPort)
	hal.NewFakeHalServer(halURL)
	select {}
}
