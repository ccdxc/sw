package main

import "github.com/pensando/sw/penctl/cmd"

// GitCommit is the variable to hold the sha
var GitCommit string

// PenCtlVer is the variable to hold penctl version
var PenCtlVer string

// BuiltTime of penctl
var BuiltTime string

func main() {
	cmd.GitCommit = GitCommit
	cmd.PenCtlVer = PenCtlVer
	cmd.BuiltTime = BuiltTime
	cmd.Execute()
}
