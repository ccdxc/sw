package main

import (
	"os"

	"github.com/pensando/sw/test/suites/runner"
)

func main() {
	// pass the make target, as in "make e2e"
	err := runner.RunSingle("e2e-sanities")
	if err != nil {
		os.Exit(-1)
	}
}
