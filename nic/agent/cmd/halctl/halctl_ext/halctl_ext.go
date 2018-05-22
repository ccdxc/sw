package main

import (
	"fmt"
	"os"

	"github.com/spf13/cobra/doc"

	"github.com/pensando/sw/nic/agent/cmd/halctl/cmd"
)

func main() {
	halctl := cmd.NewHalctlCommand()

	// For Bash Completion
	halctl.GenBashCompletionFile("bash_completion/out.sh")

	// Generate Man Page
	header := &doc.GenManHeader{
		Title:   "MINE",
		Section: "3",
	}
	err := doc.GenManTree(halctl, header, "./man_pages/")
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	// Generate Markdown format files
	err1 := doc.GenMarkdownTree(halctl, "./markdown/")
	if err1 != nil {
		fmt.Println(err1)
		os.Exit(1)
	}
}
