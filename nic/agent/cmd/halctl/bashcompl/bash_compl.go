package main

import "github.com/pensando/sw/nic/agent/cmd/halctl/cmd"

func main() {
	halctl := cmd.NewHalctlCommand()
	halctl.GenBashCompletionFile("out.sh")
}
