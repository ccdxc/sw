package vcli

import (
	"regexp"

	"github.com/urfave/cli"
)

// context structure is internal to CLI module that is passed along various functions
// to keep the context about a specific CLI command; it stores digested information
type context struct {
	cli        *cli.Context
	tenant     string
	cmd        string
	subcmd     string
	labels     map[string]string
	labelStrs  []string
	dumpStruct bool
	dumpYml    bool
	names      []string
	quiet      bool
	debug      bool
	re         *regexp.Regexp
}
