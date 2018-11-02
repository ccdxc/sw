package vcli

import (
	"fmt"
	"regexp"
	"strconv"
	"strings"

	"github.com/urfave/cli"

	log "github.com/sirupsen/logrus"

	// cliutils are generated tags that we use to build/generate commands
	_ "github.com/pensando/sw/api/generated/exports/cliutils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/ncli/gen"
)

var (
	defaultTenant = globals.DefaultTenant

	// various types of flags used by various top level commands
	noActionFlag        = cli.BoolFlag{Name: "dry-run, d", Usage: "Mock/Dry run of the command(s)"}
	jsonFlag            = cli.BoolFlag{Name: "json, j", Usage: "Output list in JSON format"}
	ymlFlag             = cli.BoolFlag{Name: "yaml, yml, y", Usage: "Output list in YAML format"}
	quietFlag           = cli.BoolFlag{Name: "quiet, q", Usage: "Only display name field"}
	reFlag              = cli.StringFlag{Name: "regular-expression, re", Usage: "Apply command on objects matching the regular expression"}
	showDefinitionFlag  = cli.BoolFlag{Name: "show-definition, s", Usage: "Show JSON definition of the object"}
	showExampleFlag     = cli.BoolFlag{Name: "example, e", Usage: "Show examples and typical usage"}
	labelFlag           = cli.StringSliceFlag{Name: "label, l", Usage: "Apply command on objects matching a set of labels"}
	updateLabelFlag     = cli.StringSliceFlag{Name: "update-label, u", Usage: "Label to be added or deleted to an object"}
	resourceVersionFlag = cli.StringFlag{Name: "resource-version, rv", Usage: "Specify the resource version to match before update an object"}
	fileFlag            = cli.BoolFlag{Name: "file, f", Usage: "Create object from an editable yaml file"}
	idFlag              = cli.StringFlag{Name: "id, i", Usage: "Snapshot name or identifier"}
	restoreFlag         = cli.BoolFlag{Name: "restore, r", Usage: "Restore from a snapshot"}
	userFlag            = cli.StringFlag{Name: "user, u", Usage: "User name"}
	passwdFlag          = cli.StringFlag{Name: "password, p", Usage: "Password"}

	editCommands       = []cli.Command{}
	patchCommands      = []cli.Command{}
	createCommands     = []cli.Command{}
	updateCommands     = []cli.Command{}
	readCommands       = []cli.Command{}
	deleteCommands     = []cli.Command{}
	labelCommands      = []cli.Command{}
	exampleCommands    = []cli.Command{}
	definitionCommands = []cli.Command{}

	createFlags     = []cli.Flag{labelFlag, noActionFlag, fileFlag}
	editFlags       = []cli.Flag{noActionFlag, ymlFlag, fileFlag}
	updateFlags     = []cli.Flag{labelFlag, noActionFlag, fileFlag}
	readFlags       = []cli.Flag{labelFlag, showDefinitionFlag, reFlag, showExampleFlag, jsonFlag, ymlFlag, quietFlag}
	deleteFlags     = []cli.Flag{labelFlag, noActionFlag, reFlag}
	labelFlags      = []cli.Flag{labelFlag, noActionFlag, reFlag, updateLabelFlag}
	exampleFlags    = []cli.Flag{ymlFlag, jsonFlag}
	snapshotFlags   = []cli.Flag{ymlFlag, jsonFlag, restoreFlag, idFlag}
	definitionFlags = []cli.Flag{}
	loginFlags      = []cli.Flag{userFlag, passwdFlag}

	// terminating flags are special in the sense that if they are present, no other options are permitted after them
	terminatingFlags = []cli.Flag{showDefinitionFlag, showExampleFlag, fileFlag}
)

// Commands is the top level command hierarchy
var Commands = []cli.Command{
	{
		Name:   "version",
		Usage:  "Version Information",
		Action: showVersion,
	},
	{
		Name:         "create",
		Aliases:      []string{"add"},
		Usage:        "Create objects",
		BashComplete: bashCreateCompleter,
	},
	{
		Name:         "update",
		Usage:        "Update objects",
		BashComplete: bashUpdateCompleter,
	},
	{
		Name:         "patch",
		Usage:        "Patch field(s) of an existing object",
		BashComplete: bashPatchCompleter,
	},
	{
		Name:         "edit",
		Usage:        "Edit an object in an editor",
		BashComplete: bashEditCompleter,
	},
	{
		Name:         "read",
		Aliases:      []string{"ls", "list", "show"},
		Usage:        "Read/show/list object(s)",
		BashComplete: bashReadCompleter,
	},
	{
		Name:         "delete",
		Aliases:      []string{"rm", "del"},
		Usage:        "Delete Object(s)",
		BashComplete: bashDeleteCompleter,
	},
	{
		Name:         "label",
		Aliases:      []string{"tag"},
		Usage:        "Label/tag Object(s)",
		BashComplete: bashLabelCompleter,
	},
	{
		Name:         "example",
		Usage:        "Show Example(s)",
		BashComplete: bashExampleCompleter,
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
		Flags:     loginFlags,
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
	},
	{
		Name:      "auto-completion",
		ArgsUsage: " ",
		Usage:     "Run \"sudo `which venice` auto-completion\" on a linux system to enable bash autocompletion",
		Action:    configCommandCompletion,
	},
}

func generateCommands() error {
	// success if the commands are already populated (e.g. when programmatically invoked)
	if len(createCommands) > 0 {
		return nil
	}

	for _, objKind := range gen.GetInfo().GetAllKeys() {
		genFlags, err := gen.GetInfo().FindFlags(objKind, "create")
		if err != nil {
			continue
		}

		cliFlags := convertFlags(genFlags)

		oneCmd := cli.Command{
			Name:         objKind,
			Usage:        "create " + objKind,
			ArgsUsage:    "[" + objKind + "]",
			Action:       createCmd,
			BashComplete: bashCreateObjCompleter,
			Flags:        append(createFlags, cliFlags...),
		}
		createCommands = append(createCommands, oneCmd)

		oneCmd = cli.Command{
			Name:         objKind,
			Usage:        "update " + objKind,
			ArgsUsage:    "[" + objKind + "]",
			Action:       updateCmd,
			BashComplete: bashUpdateObjCompleter,
			Flags:        append(createFlags, cliFlags...),
		}
		updateCommands = append(updateCommands, oneCmd)

		oneCmd = cli.Command{
			Name:         objKind,
			Usage:        "patch " + objKind,
			ArgsUsage:    "[" + objKind + "]",
			Action:       patchCmd,
			BashComplete: bashPatchObjCompleter,
			Flags:        append(createFlags, cliFlags...),
		}
		patchCommands = append(patchCommands, oneCmd)

		oneCmd = cli.Command{
			Name:         objKind,
			Usage:        "edit a " + objKind,
			ArgsUsage:    "[" + objKind + "]",
			Action:       editCmd,
			BashComplete: bashEditObjCompleter,
			Flags:        append(editFlags, cliFlags...),
		}
		editCommands = append(editCommands, oneCmd)

		oneCmd = cli.Command{
			Name:         objKind,
			Usage:        "read " + objKind,
			ArgsUsage:    "[" + objKind + "]",
			Action:       readCmd,
			BashComplete: bashReadObjCompleter,
			Flags:        readFlags,
		}
		readCommands = append(readCommands, oneCmd)

		oneCmd = cli.Command{
			Name:         objKind,
			Usage:        "delete " + objKind,
			ArgsUsage:    "[" + objKind + "]",
			Action:       deleteCmd,
			BashComplete: bashDeleteObjCompleter,
			Flags:        deleteFlags,
		}
		deleteCommands = append(deleteCommands, oneCmd)

		oneCmd = cli.Command{
			Name:         objKind,
			Usage:        "label " + objKind,
			ArgsUsage:    "[" + objKind + "]",
			Action:       labelCmd,
			BashComplete: bashLabelObjCompleter,
			Flags:        labelFlags,
		}
		labelCommands = append(labelCommands, oneCmd)

		oneCmd = cli.Command{
			Name:         objKind,
			Usage:        "example " + objKind,
			ArgsUsage:    "[" + objKind + "]",
			Action:       exampleCmd,
			BashComplete: bashExampleObjCompleter,
			Flags:        exampleFlags,
		}
		exampleCommands = append(exampleCommands, oneCmd)

		oneCmd = cli.Command{
			Name:         objKind,
			Usage:        "definition " + objKind,
			ArgsUsage:    "[" + objKind + "]",
			Action:       definitionCmd,
			BashComplete: bashDefinitionObjCompleter,
			Flags:        definitionFlags,
		}
		definitionCommands = append(definitionCommands, oneCmd)
	}
	setSubCommands("create", createCommands)
	setSubCommands("update", updateCommands)
	setSubCommands("patch", patchCommands)
	setSubCommands("edit", editCommands)
	setSubCommands("read", readCommands)
	setSubCommands("label", labelCommands)
	setSubCommands("delete", deleteCommands)
	setSubCommands("example", exampleCommands)
	setSubCommands("definition", definitionCommands)

	return nil
}

func setSubCommands(command string, subCommands []cli.Command) {
	for idx := range Commands {
		if Commands[idx].Name == command {
			Commands[idx].Subcommands = subCommands
			return
		}
	}
}

func getSubCommandFlags(command, objName string) []cli.Flag {
	for _, c := range Commands {
		if c.Name == command {
			for _, objCmd := range c.Subcommands {
				if objName == objCmd.Name {
					return objCmd.Flags
				}
			}
		}
	}

	return []cli.Flag{}
}

func convertFlags(genFlags []gen.CliFlag) []cli.Flag {
	cliFlags := []cli.Flag{}
	for _, genFlag := range genFlags {
		switch genFlag.Type {
		case "Bool":
			cliFlags = append(cliFlags, cli.BoolFlag{Name: genFlag.ID, Usage: genFlag.Help})
		case "String":
			cliFlags = append(cliFlags, cli.StringFlag{Name: genFlag.ID, Usage: genFlag.Help})
		case "Int":
			cliFlags = append(cliFlags, cli.IntFlag{Name: genFlag.ID, Usage: genFlag.Help})
		case "Uint":
			cliFlags = append(cliFlags, cli.UintFlag{Name: genFlag.ID, Usage: genFlag.Help})
		case "StringSlice":
			cliFlags = append(cliFlags, cli.StringSliceFlag{Name: genFlag.ID, Usage: genFlag.Help})

		}
	}
	return cliFlags
}

// Generic utils used by command handlers
func getKvsFromCliCtx(ctx *context) (map[string]cliField, error) {
	c := ctx.cli

	kvs := make(map[string]cliField)
	for _, f := range c.Command.VisibleFlags() {
		fi := cliField{}
		if sf, ok := f.(cli.StringSliceFlag); ok {
			flagVal := c.StringSlice(sf.Name)
			if len(flagVal) == 0 {
				continue
			}
			fi.typeStr = "slice"
			fi.values = flagVal
			kvs[sf.Name] = fi
		} else if sf, ok := f.(cli.StringFlag); ok && sf.Name != "" {
			flagVal := c.String(sf.Name)
			if len(flagVal) == 0 {
				continue
			}
			fi.typeStr = "string"
			fi.values = []string{flagVal}
			kvs[sf.Name] = fi
		} else if bf, ok := f.(cli.BoolFlag); ok {
			if strings.HasPrefix(bf.Name, "help, h") {
				continue
			}
			if !c.Bool(bf.Name) {
				continue
			}
			kvs[bf.Name] = fi
		} else if intf, ok := f.(cli.IntFlag); ok {
			flagval := strconv.Itoa(c.Int(intf.Name))
			fi.typeStr = "string"
			fi.values = []string{flagval}
			kvs[intf.Name] = fi
		} else if uintf, ok := f.(cli.UintFlag); ok {
			uintVal := (uint64)(c.Uint(uintf.Name))
			flagval := strconv.FormatUint(uintVal, 10)
			fi.typeStr = "string"
			fi.values = []string{flagval}
			kvs[uintf.Name] = fi
		} else {
			fmt.Printf("Unknown Flag = %#v\n", f)
		}
	}
	return kvs, nil
}

// processGlobalFlags processes the common/generic flags for all the commands
// this is usually one of the first functions called from various command handlers
// it saves the context in the supplied context structure, which is used later during
// the calling chain of functions
func processGlobalFlags(ctx *context, cmd string) error {
	var err error

	c := ctx.cli
	if c.GlobalBool("debug") {
		log.SetLevel(log.DebugLevel)
		ctx.debug = true
	}

	ctx.labels, err = sliceToMap(c.StringSlice("label"))
	if err != nil {
		return err
	}
	ctx.cmd = cmd
	ctx.subcmd = c.Command.Name
	ctx.token = getLoginToken()

	if err := populateGenCtx(ctx); cmd != "login" && cmd != "snapshot" && err != nil {
		return fmt.Errorf("error populating generated context: %s", err)
	}

	if cmd != "" && c.Bool("example") {
		// TODO: need to take examples from documentation
		return fmt.Errorf("noop")
	}

	ctx.dumpStruct = c.Bool("json") || c.Bool("yml")
	ctx.dumpYml = c.Bool("yml")
	ctx.quiet = c.Bool("quiet")
	ctx.server = getLoginServer()
	if ctx.server == "" {
		ctx.server = c.GlobalString("server")
	}

	args := c.Args()
	if len(args) > 0 {
		for _, arg := range args {
			for _, str := range strings.Split(arg, ",") {
				ctx.names = append(ctx.names, str)
			}
		}
	}

	if reStr := c.String("re"); reStr != "" {
		var err error
		ctx.re, err = regexp.Compile(reStr)
		if err != nil {
			fmt.Printf("Unable to compile regular expression err '%s'", err)
			return err
		}
	}

	if c.Bool("show-definition") {
		fmt.Printf("%s\n", walkStruct(ctx.structInfo, 0))
		return fmt.Errorf("all done")
	}

	return nil
}
