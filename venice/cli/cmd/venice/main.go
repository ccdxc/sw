package main

import (
	"os"

	vcli "github.com/pensando/sw/venice/cli"
)

func main() {
	vcli.InvokeCLI(os.Args, false)
}
