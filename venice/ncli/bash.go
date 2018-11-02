package vcli

import (
	"fmt"
	"io/ioutil"
	"regexp"
	"strconv"
	"strings"

	"github.com/urfave/cli"
)

func bashEditCompleter(c *cli.Context) {
	bashCompleter(c, editCommands, []cli.Flag{})
}

func bashCreateCompleter(c *cli.Context) {
	bashCompleter(c, createCommands, []cli.Flag{})
}

func bashUpdateCompleter(c *cli.Context) {
	bashCompleter(c, updateCommands, []cli.Flag{})
}

func bashPatchCompleter(c *cli.Context) {
	bashCompleter(c, patchCommands, []cli.Flag{})
}

func bashDeleteCompleter(c *cli.Context) {
	bashCompleter(c, deleteCommands, []cli.Flag{})
}

func bashReadCompleter(c *cli.Context) {
	bashCompleter(c, readCommands, []cli.Flag{})
}

func bashLabelCompleter(c *cli.Context) {
	bashCompleter(c, labelCommands, []cli.Flag{})
}

func bashExampleCompleter(c *cli.Context) {
	bashCompleter(c, exampleCommands, []cli.Flag{})
}

func bashDefinitionCompleter(c *cli.Context) {
	bashCompleter(c, definitionCommands, []cli.Flag{})
}

func bashTreeCompleter(c *cli.Context) {
	bashCompleter(c, readCommands, []cli.Flag{})
}

func bashSnapshotCompleter(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, snapshotFlags)
}

func bashEditObjCompleter(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, editFlags)
	bashObjNameCompleter(c)
}

func bashUpdateObjCompleter(c *cli.Context) {
	cFlags := getSubCommandFlags("create", c.Command.Name)
	bashCompleter(c, updateCommands, cFlags)
	bashCompleter(c, []cli.Command{}, updateFlags)
	bashObjNameCompleter(c)
}

func bashPatchObjCompleter(c *cli.Context) {
	cFlags := getSubCommandFlags("create", c.Command.Name)
	bashCompleter(c, patchCommands, cFlags)
	bashObjNameCompleter(c)
}

func bashCreateObjCompleter(c *cli.Context) {
	cFlags := getSubCommandFlags("create", c.Command.Name)
	bashCompleter(c, []cli.Command{}, cFlags)
}

func bashReadObjCompleter(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, readFlags)
	bashObjNameCompleter(c)
}

func bashDeleteObjCompleter(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, deleteFlags)
	bashObjNameCompleter(c)
}

func bashLabelObjCompleter(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, readFlags)
	bashObjNameCompleter(c)
}

func bashExampleObjCompleter(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, exampleFlags)
	bashObjNameCompleter(c)
}

func bashDefinitionObjCompleter(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, definitionFlags)
	bashObjNameCompleter(c)
}

// bash completion for the specified sub-commands and cli flags
func bashCompleter(c *cli.Context, cmds []cli.Command, flags []cli.Flag) {
	if retStr, found := getLastFlagSuggestion(c, flags); found {
		fmt.Printf("%s ", retStr)
		return
	}

	for _, cmd := range cmds {
		fmt.Printf("%s ", cmd.Name)
	}

	// exit on any terminating flag
	for _, f := range flags {
		for _, t := range terminatingFlags {
			if f == t {
				_, present := flagPresent(c, f)
				if present {
					return
				}
			}
		}
	}

	for _, f := range flags {
		m, present := flagPresent(c, f)
		if present && m.kind != "slice" && m.value != "" {
			continue
		}
		fmt.Printf("--%s ", m.name)
	}
}

// provides names of objects fetching their values from the backend
func bashObjNameCompleter(c *cli.Context) {

	ctx := func(c *cli.Context, cmd string) *context {
		ctx := &context{cli: c, tenant: defaultTenant}
		ctx.cmd = cmd
		ctx.subcmd = c.Command.Name
		ctx.server = getLoginServer()
		ctx.token = getLoginToken()

		if err := populateGenCtx(ctx); err != nil {
			fmt.Printf("error populating generated context (subcmd %s): %s", ctx.subcmd, err)
			return nil
		}

		if len(c.Args()) > 0 {
			namesRe := c.Args()[len(c.Args())-1]
			var err error
			ctx.re, err = regexp.Compile(namesRe)
			if err != nil {
				fmt.Printf("Unable to compile regular expression err '%s'", err)
				return nil
			}
		}

		return ctx
	}(c, "read")

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-objects! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

// flagMeta keeps meatdata associated with each CLI flag useful to keep misc information about various flags
type flagMeta struct {
	kind  string
	name  string
	value string
}

// getLastFlagSuggestion provides suggestion on partial last flag argument
// more specifically it suggests whether to enter a value or key:value (for maps)
// note that maps are specified using separator identified in 'kvSplitter' constant
func getLastFlagSuggestion(c *cli.Context, flags []cli.Flag) (string, bool) {
	retStr := ""
	osArgs := c.App.Metadata["osArgs"].([]string)
	if len(osArgs) > 2 {
		lastArg := osArgs[len(osArgs)-2]
		if strings.HasPrefix(lastArg, "-") {
			flagName := strings.TrimLeft(lastArg, "-")
			for _, f := range flags {
				m, _ := flagPresent(c, f)
				if m.name != flagName {
					continue
				}
				switch m.kind {
				case "string":
					retStr = fmt.Sprintf("{value} ")
				case "slice":
					retStr = fmt.Sprintf("{key%svalue} ", kvSplitter)
				}
				return retStr, true
			}
		}
	}
	return "", false
}

// flagPresent finds out whether a flag was specified in the actual command line
// if it was present, return the flagMeta and true, otherwise false
func flagPresent(c *cli.Context, f cli.Flag) (flagMeta, bool) {
	m := flagMeta{}
	present := false
	if bf, ok := f.(cli.BoolFlag); ok {
		m.kind = "bool"
		m.name = strings.Split(bf.Name, ",")[0]
		if c.GlobalIsSet(m.name) || c.IsSet(m.name) {
			m.value = "true"
			present = true
		}
	} else if intf, ok := f.(cli.IntFlag); ok {
		m.kind = "int"
		m.name = strings.Split(intf.Name, ",")[0]
		if c.IsSet(m.name) {
			m.value = strconv.Itoa(c.Int(m.name))
			present = true
		}
	} else if uintf, ok := f.(cli.UintFlag); ok {
		m.kind = "uint"
		m.name = strings.Split(uintf.Name, ",")[0]
		if c.IsSet(m.name) {
			uintVal := (uint64)(c.Uint(m.name))
			m.value = strconv.FormatUint(uintVal, 10)
			present = true
		}
	} else if sf, ok := f.(cli.StringFlag); ok {
		m.kind = "string"
		m.name = strings.Split(sf.Name, ",")[0]
		if c.GlobalIsSet(m.name) {
			m.value = c.GlobalString(m.name)
			present = true
		}
		if c.IsSet(m.name) {
			m.value = c.String(m.name)
			present = true
		}
	} else if sf, ok := f.(cli.StringSliceFlag); ok {
		m.kind = "slice"
		m.name = strings.Split(sf.Name, ",")[0]
		m.name = strings.Trim(m.name, " ")
		if c.IsSet(m.name) {
			vs := c.StringSlice(m.name)
			if len(vs) != 0 {
				m.value = strings.Join(vs, ",")
			}
			present = true
		}
	}
	return m, present
}

// bashFileNameCompleter reads the file names and returns the current list of files
// this is used when upload command is issued to auto-fill the file names in current directory
// TODO: this needs to be enhanced to include files within a path and not just the files locally
func bashFileNameCompleter(c *cli.Context) {
	fileName := "."
	if len(c.Args()) > 0 {
		fileName = c.Args()[0]
	}
	if fis, err := ioutil.ReadDir(fileName); err == nil {
		for _, fi := range fis {
			fmt.Printf("%s ", fi.Name())
		}
	}
}
