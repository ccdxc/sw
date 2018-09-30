package main

import (
	"flag"

	"github.com/pensando/sw/iota/svcs/agent"
)

func main() {
	var stubMode = flag.Bool("stubmode", false, "Start IOTA Agent in stub mode")
	flag.Parse()
	agent.StartIOTAAgent(stubMode)
}
