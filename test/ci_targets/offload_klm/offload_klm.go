package main

import (
	"os"

	"github.com/pensando/sw/test/utils/infra"
)

func main() {
	// pass the make target, as in "make e2e"
	err := infra.RunCmd("/tmp/git/pensando/sw/test/ci_targets/offload_klm/run.sh")
	if err != nil {
		os.Exit(-1)
	}
}
