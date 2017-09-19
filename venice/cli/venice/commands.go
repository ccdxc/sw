package main

import (
	"github.com/urfave/cli"
)

const (
	defaultTenant = "default"
)

// various types of flags used by various top level commands
var noActionFlag = cli.BoolFlag{
	Name:  "dry-run, d",
	Usage: "Mock/Dry run of the command(s)",
}

var jsonFlag = cli.BoolFlag{
	Name:  "json, j",
	Usage: "Output list in JSON format",
}

var ymlFlag = cli.BoolFlag{
	Name:  "yaml, yml, y",
	Usage: "Output list in YAML format",
}

var quietFlag = cli.BoolFlag{
	Name:  "quiet, q",
	Usage: "Only display name field",
}

var reFlag = cli.StringFlag{
	Name:  "regular-expression, re",
	Usage: "Apply command on objects matching the regular expression",
}

var showDefinitionFlag = cli.BoolFlag{
	Name:  "show-definition, s",
	Usage: "Show JSON definition of the object",
}

var showExampleFlag = cli.BoolFlag{
	Name:  "example, e",
	Usage: "Show examples and typical usage",
}

var labelFlag = cli.StringSliceFlag{
	Name:  "label, l",
	Usage: "Apply command on objects matching a set of labels",
}

var updateLabelFlag = cli.StringSliceFlag{
	Name:  "update-label, u",
	Usage: "Label to be added or deleted to an object",
}

var resourceVersionFlag = cli.StringFlag{
	Name:  "resource-version, rv",
	Usage: "Specify the resource version to match before update an object",
}

var fileFlag = cli.BoolFlag{
	Name:  "file, f",
	Usage: "Create object from an editable yaml file",
}

var idFlag = cli.StringFlag{
	Name:  "id, f",
	Usage: "Snapshot name or identifier",
}

var restoreFlag = cli.BoolFlag{
	Name:  "restore, r",
	Usage: "Restore from a snapshot",
}

var createFlags = []cli.Flag{labelFlag, noActionFlag, fileFlag}
var editFlags = []cli.Flag{noActionFlag, ymlFlag}
var updateFlags = []cli.Flag{labelFlag, noActionFlag}
var readFlags = []cli.Flag{labelFlag, showDefinitionFlag, reFlag, showExampleFlag, jsonFlag, ymlFlag, quietFlag}
var deleteFlags = []cli.Flag{labelFlag, noActionFlag, reFlag}
var labelFlags = []cli.Flag{labelFlag, noActionFlag, reFlag, updateLabelFlag}
var exampleFlags = []cli.Flag{ymlFlag, jsonFlag}
var snapshotFlags = []cli.Flag{ymlFlag, jsonFlag, restoreFlag, idFlag}
var definitionFlags = []cli.Flag{}

// terminating flags are special in the sense that if they are present, no other options are permitted after them
var terminatingFlags = []cli.Flag{showDefinitionFlag, showExampleFlag, fileFlag}

// The top level command hierarchy
var commands = []cli.Command{
	{
		Name:   "version",
		Usage:  "Version Information",
		Action: showVersion,
	},
	{
		Name:         "create",
		Aliases:      []string{"add"},
		Usage:        "Create or update objects",
		BashComplete: bashCreateCompleter,
		Subcommands:  createCommands,
	},
	{
		Name:         "update",
		Aliases:      []string{"patch"},
		Usage:        "Patch field(s) of an existing object",
		BashComplete: bashPatchCompleter,
		Subcommands:  updateCommands,
	},
	{
		Name:         "edit",
		Usage:        "Edit an object in an editor",
		BashComplete: bashEditCompleter,
		Subcommands:  editCommands,
	},
	{
		Name:         "read",
		Aliases:      []string{"ls", "list", "show"},
		Usage:        "Read/show/list object(s)",
		BashComplete: bashReadCompleter,
		Subcommands:  readCommands,
	},
	{
		Name:         "delete",
		Aliases:      []string{"rm", "del"},
		Usage:        "Delete Object(s)",
		BashComplete: bashDeleteCompleter,
		Subcommands:  deleteCommands,
	},
	{
		Name:         "label",
		Aliases:      []string{"tag"},
		Usage:        "Label/tag Object(s)",
		BashComplete: bashLabelCompleter,
		Subcommands:  labelCommands,
	},
	{
		Name:         "example",
		Usage:        "Show Example(s)",
		BashComplete: bashExampleCompleter,
		Subcommands:  exampleCommands,
	},
	{
		Name:         "tree",
		ArgsUsage:    " ",
		Usage:        "Show object tree and relationships",
		Action:       treeCmd,
		BashComplete: bashTreeCompleter,
	},
	{
		Name:      "logout",
		ArgsUsage: " ",
		Usage:     "Log out of venice management system",
		Action:    logoutCmd,
	},
	{
		Name:      "login",
		ArgsUsage: " ",
		Usage:     "Log into venice management system",
		Action:    loginCmd,
	},
	{
		Name:         "snapshot",
		ArgsUsage:    " ",
		Usage:        "Take configuration snapshot or restore them",
		Action:       snapshotCmd,
		Flags:        snapshotFlags,
		BashComplete: bashSnapshotCompleter,
	},
	{
		Name:         "definition",
		Usage:        "Show definition of a specific object type",
		BashComplete: bashDefinitionCompleter,
		Subcommands:  definitionCommands,
	},
}
