package main

import (
	"os"

	vcli "github.com/pensando/sw/venice/ncli"
)

func main() {
	vcli.InvokeCLI(os.Args, false)
}
