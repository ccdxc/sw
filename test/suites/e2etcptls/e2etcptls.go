package main

import (
	"os"

	"github.com/pensando/sw/test/suites/runner"
)

func main() {
	// pass the make target, as in "make e2e"
	err := runner.RunCmd("/tmp/git/pensando/sw/test/suites/e2etcptls/run.sh")
	if err != nil {
		os.Exit(-1)
	}
}
