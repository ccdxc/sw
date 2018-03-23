package vcli

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"reflect"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"syscall"
	"text/tabwriter"
	"time"

	"github.com/ghodss/yaml"
	log "github.com/sirupsen/logrus"
	"github.com/urfave/cli"
	"golang.org/x/crypto/ssh/terminal"

	swapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cli/api"
	"github.com/pensando/sw/venice/cli/gen/pregen"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

// editCmd hands the edit command, which allows editing an existing object or a
// new object (from an example template)
func editCmd(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "create"); err != nil {
		return
	}

	filename := ""
	if len(c.Args()) > 1 {
		log.Fatalf("One record update permitted at a time")
	} else if len(c.Args()) == 1 {
		objName := c.Args()[0]

		obj, _ := getObj(ctx)
		url := getURL(ctx, objName)
		err := restGet(url, ctx.tenant, obj)
		if err != nil {
			log.Fatalf("Error getting %s '%s': %v", ctx.subcmd, objName, err)
		}
		log.Debugf("Fetched following object %v", obj)

		tf, err := ioutil.TempFile("/tmp", ctx.subcmd)
		if err != nil {
			log.Fatalf("Error creating temp file %s '%s': %v", ctx.subcmd, objName, err)
		}
		defer os.Remove(tf.Name())

		dumpBytes := dumpStruct(ctx, obj)
		tf.Write(dumpBytes)
		filename = tf.Name()
	}

	if err := editFromFile(ctx, filename); err != nil {
		log.Printf("Edit error: '%s'", err)
	}
}

// createCmd is called when user executes create verb on an object
// this is primary method for creating a new object, note that some objects
// post is not allowed and thus crate would fail
func createCmd(c *cli.Context) {
	createCmdInternal(c, false)
}

// createCmd is called when user executes update verb on an object
// update function updates (does http put) an existing object by updating
// specified fields to be changed for an existing object
func updateCmd(c *cli.Context) {
	createCmdInternal(c, true)
}

// createCmdInternal handles create and update
func createCmdInternal(c *cli.Context, rmw bool) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "create"); err != nil {
		log.Fatalf("error processing global flags: %s", err)
		return
	}

	if c.Bool("file") {
		filename := ""
		if len(c.Args()) > 1 {
			log.Fatalf("One file is permitted to be edited at a time")
		} else if len(c.Args()) == 1 {
			filename = c.Args()[0]
		}
		if err := editFromFile(ctx, filename); err != nil {
			log.Fatalf("Edit error: '%s'", err)
		}
		return
	}

	if len(c.Args()) != 1 {
		log.Fatalf("%s name is not provided", ctx.subcmd)
	}
	objName := c.Args()[0]

	stringLabels := []string{}
	for key, value := range ctx.labels {
		stringLabels = append(stringLabels, fmt.Sprintf("%s:%s", key, value))
	}
	metaKvs := make(map[string]ref.FInfo)
	metaKvs["kind"] = ref.NewFInfo([]string{ctx.subcmd})
	metaKvs["tenant"] = ref.NewFInfo([]string{ctx.tenant})
	if len(stringLabels) != 0 {
		metaKvs["labels"] = ref.NewFInfo(stringLabels)
	}

	metaKvs["name"] = ref.NewFInfo([]string{objName})

	obj, _ := getObj(ctx)
	if rmw {
		url := getURL(ctx, objName)
		err := restGet(url, ctx.tenant, obj)
		if err != nil {
			log.Printf("Error getting %s '%s': %v", ctx.subcmd, objName, err)
			return
		}
		log.Debugf("Fetched following object %v", obj)
	}
	specKvs, err := getSpecKvs(ctx)
	if err != nil {
		log.Printf("error parsing arguments: %s, obj %+v", err, obj)
		return
	}

	newObj := writeObj(obj, metaKvs, specKvs)

	if rmw {
		if err = putObj(ctx, newObj); err != nil {
			log.Printf("%s", err)
		}
	} else {
		if err = postObj(ctx, newObj); err != nil {
			log.Printf("%s", err)
		}
	}
}

// readCmd is the function that gets called for any read operation, it gets an object or a list of
// objects; if there are multiple objects, it would show them in a tabulated form
func readCmd(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "read"); err != nil {
		return
	}

	obj, objList := getObj(ctx)
	if obj == nil || objList == nil {
		log.Printf("unable to get object information for kind '%s'", ctx.subcmd)
		return
	}

	if len(ctx.names) == 1 {
		url := getURL(ctx, ctx.names[0])
		err := restGet(url, ctx.tenant, obj)
		if err != nil {
			log.Printf("Error getting %s '%s': %v", ctx.subcmd, ctx.names[0], err)
		} else {
			dumpStructStdout(ctx, obj)
		}
		return
	}

	url := getURL(ctx, "")
	err := restGet(url, ctx.tenant, objList)
	if err != nil {
		log.Printf("Error getting %ss: %v", ctx.subcmd, err)
		return
	}

	if ctx.dumpStruct {
		dumpStructStdout(ctx, objList)
		return
	}

	hdrBytes, err := json.Marshal(objList)
	if err != nil {
		log.Printf("Unable to find header in object (marshal): %+v", objList)
		return
	}
	listHdr := api.ListHeader{}
	if err := json.Unmarshal(hdrBytes, &listHdr); err != nil {
		log.Printf("Unable to find header in object (unmarshal): '%s'", string(hdrBytes))
		return
	}

	if len(listHdr.Items) <= 0 {
		return
	}

	if ctx.quiet {
		names := ""
		for _, obj := range listHdr.Items {
			names += obj.ObjectMeta.Name + "\n"
		}
		os.Stdout.WriteString(names)
		return
	}

	if len(listHdr.Items) == 1 {
		dumpObj := getObjFromList(objList, 0)
		dumpStructStdout(ctx, dumpObj)
		return
	}

	header := false
	writer := tabwriter.NewWriter(os.Stdout, 0, 2, 2, ' ', 0)
	defer writer.Flush()

	objmKvs, specKvs, objmValidKvs, specValidKvs := getAllKvs(ctx, objList)

	for idx, obj := range listHdr.Items {
		if skipAny(ctx, &obj.ObjectMeta) {
			continue
		}

		if !header {
			oneLine := getLineHeader(ctx, objmValidKvs, specValidKvs, "")
			writer.Write(oneLine)
			header = true
		}

		recIdx := 0
		for {
			oneLine, more := getLineData(ctx, &recIdx, objmKvs[idx], specKvs[idx], objmValidKvs, specValidKvs, "")
			writer.Write(oneLine)
			if !more {
				break
			}
		}
	}
}

// deleteCmd is called for all verbs that perform delete on an object; it invokes REST delete APIs in turn
func deleteCmd(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}

	if err := processGlobalFlags(ctx, "delete"); err != nil {
		return
	}

	names := ctx.names
	if ctx.labels != nil && len(ctx.labels) > 0 {
		nc := *c
		readCmd(&nc)
		names = getFilteredNames(ctx)
	}

	for _, name := range names {
		url := getURL(ctx, name)
		err := restDelete(c.Command.Name, url, ctx.tenant)
		if err != nil {
			fmt.Printf("Error deleting %s '%s': %v\n", c.Command.Name, name, err)
			return
		}
	}
}

// updateMetaLabel function is a utility function that adds/deletes a label within object's meta
func updateMetaLabel(objm *swapi.ObjectMeta, newLabels map[string]string) error {
	for key, value := range newLabels {
		if value == "-" {
			delete(objm.Labels, key)
		} else {
			if objm.Labels == nil {
				objm.Labels = make(map[string]string)
			}
			objm.Labels[key] = value
		}
	}
	return nil
}

// labelcmd performs label update on any object; this is invoked when label verb is invoked for any object
func labelCmd(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "read"); err != nil {
		return
	}

	obj, objList := getObj(ctx)
	if obj == nil || objList == nil {
		log.Printf("unable to get object information for kind '%s'", ctx.subcmd)
		return
	}

	newLabels, err := sliceToMap(c.StringSlice("update-label"))
	if len(newLabels) == 0 || err != nil {
		if err != nil {
			log.Printf("%s", err)
		}
		return
	}
	if len(ctx.names) == 1 {
		url := getURL(ctx, ctx.names[0])
		err := restGet(url, ctx.tenant, obj)
		if err != nil {
			log.Printf("Error getting %s '%s': %s", ctx.subcmd, ctx.names[0], err)
		} else {
			log.Debugf("Updating labels to the following object(s):\n")
			dumpStructStdout(ctx, obj)
			if err := updateLabel(obj, newLabels); err != nil {
				log.Printf("Error updating labels, obj %v, labels %s: %s\n", obj, newLabels, err)
			}
			if err := putObj(ctx, obj); err != nil {
				log.Printf("%s", err)
			}
		}
		return
	}

	url := getURL(ctx, "")
	err = restGet(url, ctx.tenant, objList)
	if err != nil {
		log.Printf("Error getting %ss: %v", ctx.subcmd, err)
		return
	}

	hdrBytes, err := json.Marshal(objList)
	if err != nil {
		log.Printf("Unable to find header in object (marshal): %+v", objList)
		return
	}
	listHdr := api.ListHeader{}
	if err := json.Unmarshal(hdrBytes, &listHdr); err != nil {
		log.Printf("Unable to find header in object (unmarshal): '%s'", string(hdrBytes))
		return
	}

	if len(listHdr.Items) <= 0 {
		return
	}

	for idx, objHdr := range listHdr.Items {
		if skipAny(ctx, &objHdr.ObjectMeta) {
			continue
		}

		newObj := getObjFromList(objList, idx)
		updateLabel(newObj, newLabels)
		if err := putObj(ctx, newObj); err != nil {
			log.Printf("%s", err)
		}
	}
}

// eampleCmd is called when example verb is specified for any object
// this command shows an example of a specific operation on an object
func exampleCmd(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "example"); err != nil {
		return
	}

	obj, _ := getObj(ctx)
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj}
	fmt.Printf("%s\n", ref.WalkStruct(obj, refCtx))
}

// definitionCmd shows object definition
func definitionCmd(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "definition"); err != nil {
		return
	}

	obj, _ := getObj(ctx)
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj}
	fmt.Printf("%s\n", ref.WalkStruct(obj, refCtx))
}

// treeCmd shows the relationship tree of all (or a kind of) instantiated objects
func treeCmd(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "tree"); err != nil {
		return
	}

	subCmds := c.Args()
	if len(subCmds) == 0 {
		for key := range api.Objs {
			subCmds = append(subCmds, key)
		}
	}

	level := 0
	sort.Strings(subCmds)
	for _, subcmd := range subCmds {
		newCtx := ctx
		newCtx.cmd = "tree"
		newCtx.subcmd = subcmd

		fmt.Printf("+%s%s\n", ref.GetIndent(level), subcmd)
		level++
		_, objList := getObj(newCtx)
		if objList == nil {
			fmt.Printf("unable to get object information for kind '%s'", newCtx.subcmd)
			return
		}

		url := getURL(newCtx, "")
		err := restGet(url, ctx.tenant, objList)
		if err != nil {
			fmt.Printf("Error getting %ss: %v", newCtx.subcmd, err)
			return
		}

		hdrBytes, err := json.Marshal(objList)
		if err != nil {
			fmt.Printf("Unable to find header in object (marshal): %+v", objList)
			return
		}
		listHdr := api.ListHeader{}
		if err := json.Unmarshal(hdrBytes, &listHdr); err != nil {
			fmt.Printf("Unable to find header in object (unmarshal): '%s'", string(hdrBytes))
			return
		}

		if len(listHdr.Items) > 0 {
			for idx, hdrObj := range listHdr.Items {
				fmt.Printf("%s%s\n", ref.GetIndent(level), hdrObj.ObjectMeta.Name)

				obj := getObjFromList(objList, idx)
				objValue := reflect.Indirect(reflect.ValueOf(obj))
				objKind := ref.FieldByName(objValue, "TypeMeta.Kind")[0]
				if obj, ok := api.Objs[objKind]; ok {
					rlns := obj.Rlns
					level++
					for _, rln := range rlns {
						vals := ref.FieldByName(objValue, rln.Field)
						if len(vals) > 0 {
							rlnStr := ""
							if rln.Type == api.NamedRef {
								rlnStr = "->"
							} else if rln.Type == api.SelectorRef {
								rlnStr = "*->"
							} else if rln.Type == api.BackRef {
								rlnStr = "<-"
							}

							valsStr := strings.Join(vals, ",")
							splitF := strings.Split(rln.Field, ".")
							rlnField := splitF[len(splitF)-1]
							fmt.Printf("%s%s %s %s: %s\n", ref.GetIndent(level), rln.ToObj, rlnStr, rlnField, valsStr)
						}
					}
					level--
				}
			}
		}
		level--
	}
}

var loginUser string

// logoutCmd is called when user executes user logout
// TODO: tie this to RBAC
func logoutCmd(c *cli.Context) {
	fmt.Printf("logging out... %s\n", loginUser)
}

// loginCmd is called when a user logs in; upon login user's tenancy
// and JWT tokens are determined that are then used in subsequent REST calls
// TODO: tie this to RBAC
func loginCmd(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "login"); err != nil {
		return
	}

	loginUser = ""
	if len(c.Args()) > 1 {
		log.Fatalf("Only one user login is allowed")
	} else if len(c.Args()) == 1 {
		loginUser = c.Args()[0]
	}

	reader := bufio.NewReader(os.Stdin)
	if loginUser == "" {
		fmt.Print("Enter Username: ")
		loginUser, _ = reader.ReadString('\n')
		loginUser = strings.TrimSpace(loginUser)
	}

	fmt.Print("Enter Password: ")
	_, err := terminal.ReadPassword(int(syscall.Stdin))
	if err != nil {
		log.Fatalf("error reading user password: %s", err)
	}
	// password := string(bytePassword)
	// password = strings.TrimSpace(password)

	fmt.Printf("\nSuccessfully logged in as '%s' tenant '%s'\n", loginUser, defaultTenant)
}

// snapshotCmd takes the snapshot (inventory) of all objects, removes their
// operational/changeable state, which can then be played back into the system sometime later
func snapshotCmd(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "snapshot"); err != nil {
		return
	}

	subCmds := c.Args()
	if len(subCmds) == 0 {
		for key := range api.Objs {
			subCmds = append(subCmds, key)
		}
	}

	dirname := "./"
	if c.String("id") == "" {
		dirname += time.Now().String()
	} else {
		dirname += c.String("id")
	}
	dirname += "/"

	if c.Bool("restore") {
		newctx := ctx
		if err := processGlobalFlags(newctx, "create"); err != nil {
			log.Fatalf("unable to create new context")
		}
		createFromFileName(newctx, dirname)
		return
	}

	if err := os.Mkdir(dirname, 0777); err != nil {
		log.Fatalf("error creating directory: %s", dirname)
	}

	sort.Strings(subCmds)
	for _, subcmd := range subCmds {
		newCtx := ctx
		newCtx.cmd = "snapshot"
		newCtx.subcmd = subcmd

		_, objList := getObj(newCtx)
		if objList == nil {
			fmt.Printf("unable to get object information for kind '%s'", newCtx.subcmd)
			return
		}

		url := getURL(newCtx, "")
		err := restGet(url, ctx.tenant, objList)
		if err != nil {
			log.Fatalf("Error getting %ss: %v", newCtx.subcmd, err)
		}

		hdrBytes, err := json.Marshal(objList)
		if err != nil {
			log.Fatalf("Unable to find header in object (marshal): %+v", objList)
		}
		listHdr := api.ListHeader{}
		if err := json.Unmarshal(hdrBytes, &listHdr); err != nil {
			fmt.Printf("Unable to find header in object (unmarshal): '%s'", string(hdrBytes))
			return
		}

		if len(listHdr.Items) == 0 {
			continue
		}

		subdir := dirname + "/" + subcmd + "s/"
		if err := os.Mkdir(subdir, 0777); err != nil {
			log.Fatalf("error creating subdir '%s'", subdir)
		}

		for idx, hdrObj := range listHdr.Items {

			obj := getObjFromList(objList, idx)
			if err := removeObjOper(obj); err != nil {
				log.Fatalf("Error removing oper info from object %+v\n", obj)
			}
			dumpBytes := dumpStruct(ctx, obj)

			filename := subdir + hdrObj.ObjectMeta.Name
			if ctx.dumpYml {
				filename += ".yml"
			} else {
				filename += ".json"
			}
			tf, err := os.Create(filename)
			if err != nil {
				log.Fatalf("Error creating file %s '%s'", filename, err)
			}
			tf.Write(dumpBytes)
			tf.Close()
		}
	}

	fmt.Printf("Successful - stored snapshot in '%s' directory\n", dirname)
}

func bashCreateCompleter(c *cli.Context) {
	BashCompleter(c, createCommands, []cli.Flag{})
}
func bashEditCompleter(c *cli.Context) {
	BashCompleter(c, editCommands, []cli.Flag{})
}
func bashPatchCompleter(c *cli.Context) {
	BashCompleter(c, updateCommands, []cli.Flag{})
}
func bashDeleteCompleter(c *cli.Context) {
	BashCompleter(c, deleteCommands, []cli.Flag{})
}
func bashReadCompleter(c *cli.Context) {
	BashCompleter(c, readCommands, []cli.Flag{})
}
func bashLabelCompleter(c *cli.Context) {
	BashCompleter(c, labelCommands, []cli.Flag{})
}
func bashExampleCompleter(c *cli.Context) {
	BashCompleter(c, exampleCommands, []cli.Flag{})
}
func bashDefinitionCompleter(c *cli.Context) {
	BashCompleter(c, definitionCommands, []cli.Flag{})
}
func bashTreeCompleter(c *cli.Context) {
	BashCompleter(c, readCommands, []cli.Flag{})
}
func bashSnapshotCompleter(c *cli.Context) {
	BashCompleter(c, readCommands, snapshotFlags)
	BashCompleter(c, readCommands, []cli.Flag{})
}

// Generic utils used by command handlers
func getSpecKvs(ctx *context) (map[string]ref.FInfo, error) {
	c := ctx.cli

	kvs := make(map[string]ref.FInfo)
	for _, f := range c.Command.VisibleFlags() {
		fi := ref.FInfo{}
		if sf, ok := f.(cli.StringSliceFlag); ok {
			flagVal := c.StringSlice(sf.Name)
			if len(flagVal) == 0 {
				continue
			}
			fi.TypeStr = "slice"
			// fi.ValueStr = []string{strings.Join(flagVal, ",")}
			fi.ValueStr = flagVal
			kvs[sf.Name] = fi
		} else if sf, ok := f.(cli.StringFlag); ok && sf.Name != "" {
			flagVal := c.String(sf.Name)
			if len(flagVal) == 0 {
				continue
			}
			fi.TypeStr = "string"
			fi.ValueStr = []string{flagVal}
			kvs[sf.Name] = fi
		} else if bf, ok := f.(cli.BoolFlag); ok {
			if strings.HasPrefix(bf.Name, "help, h") {
				continue
			}
			kvs[bf.Name] = fi
		} else if intf, ok := f.(cli.IntFlag); ok {
			flagval := strconv.Itoa(c.Int(intf.Name))
			fi.TypeStr = "string"
			fi.ValueStr = []string{flagval}
			kvs[intf.Name] = fi
		} else if uintf, ok := f.(cli.UintFlag); ok {
			uintVal := (uint64)(c.Uint(uintf.Name))
			flagval := strconv.FormatUint(uintVal, 10)
			fi.TypeStr = "string"
			fi.ValueStr = []string{flagval}
			kvs[uintf.Name] = fi
		} else {
			fmt.Printf("Unknown Flag = %#v\n", f)
		}
	}
	return kvs, nil
}

// getURL incorporates the CLI global commands for a specific object to derive its URL
// TODO: combine this with api generation code
func getURL(ctx *context, objName string) string {
	if _, ok := api.Objs[ctx.subcmd]; !ok {
		fmt.Printf("Unable to find base URL for cmd '%s'", ctx.cmd)
		return "/invalid"
	}
	objURL := strings.Replace(api.Objs[ctx.subcmd].URL, ":tenant", ctx.tenant, 1)

	url := ctx.cli.GlobalString("server")
	if ctx.cli.Bool("dry-run") {
		url += "/test"
	}
	url += objURL
	if objName != "" {
		url += "/" + objName
	}
	return url
}

func dumpStructStdout(ctx *context, obj interface{}) {
	os.Stdout.Write(dumpStruct(ctx, obj))
	os.Stdout.WriteString("\n")
}

// dumpStruct prints the structure in yaml or json format
func dumpStruct(ctx *context, obj interface{}) []byte {
	j, err := json.MarshalIndent(obj, "", "  ")
	if err != nil {
		fmt.Printf("Unable to decode following output to json:\n%v\n", obj)
		return []byte{}
	}
	if !ctx.dumpYml {
		return j
	}

	y, err := yaml.JSONToYAML(j)
	if err != nil {
		fmt.Printf("Unable to convert following json to yaml:\n%s\n", j)
		return []byte{}
	}
	return y
}

// DefaultVersion is the bakend in image version for the CLI frontend
// TODO: obtain the server version from the backend
const DefaultVersion = "v0.1-alpha"

func showVersion(c *cli.Context) {
	if len(c.Args()) != 0 {
		log.Warnf("More arguments than required")
	}
	fmt.Printf("Client Version: %s\n", DefaultVersion)
	fmt.Printf("Server Version: %s\n", DefaultVersion)
}

// sliceToMap converts a colon separted command line option strings to a go map
// while largely it is used for labels, it can be used for other maps too
func sliceToMap(labelStrs []string) (map[string]string, error) {
	labels := make(map[string]string)
	if len(labelStrs) != 0 {
		for _, label := range labelStrs {
			kv := []string{}
			if strings.Contains(label, ":") {
				kv = strings.Split(label, ":")
			} else if strings.Contains(label, "=") {
				kv = strings.Split(label, "=")
			} else if strings.HasSuffix(label, "-") {
				kv = []string{strings.TrimSuffix(label, "-"), "-"}
			} else {
				return labels, fmt.Errorf("No separator found in label '%s', e.g. `--label dept:finance` or `--label dept=finance", label)
			}
			if len(kv) != 2 {
				return labels, fmt.Errorf("Invalid labels '%s', e.g. `--label dept:finance` or `--label dept=finance", label)
			}
			labels[kv[0]] = kv[1]
		}
	}
	return labels, nil
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

	ctx.labelStrs = c.StringSlice("label")
	ctx.labels, err = sliceToMap(ctx.labelStrs)
	if err != nil {
		return err
	}
	ctx.cmd = cmd
	ctx.subcmd = c.Command.Name

	if cmd != "" && c.Bool("example") {
		showExamples(cmd, c.Command.Name)
		return fmt.Errorf("noop")
	}

	ctx.dumpStruct = c.Bool("json") || c.Bool("yml")
	ctx.dumpYml = c.Bool("yml")
	ctx.quiet = c.Bool("quiet")

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
		obj, _ := getObj(ctx)
		refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj}
		fmt.Printf("%s\n", ref.WalkStruct(obj, refCtx))
		return fmt.Errorf("all done")
	}

	return nil
}

// skipRe returns true if the object name matches a regular expression
// used to filter the output of certain specific fields
// TODO: implement this in the backend
func skipRe(re *regexp.Regexp, objName string) bool {
	if re == nil || re.MatchString(objName) {
		return false
	}
	return true
}

// skipRe returns true if the object matches a specified label
// used to filter the output of certain specific fields
// TODO: implement this in the backend
func skipLabel(ctxLabels []string, objLabels map[string]string) bool {
	skip := true
	for _, label := range ctxLabels {
		kv := strings.Split(label, ":")
		if objLabels != nil && objLabels[kv[0]] == kv[1] {
			skip = false
			break
		}
	}

	if len(ctxLabels) > 0 && skip {
		return true
	}

	return false
}

// skipName returns true if the object matches a specified object name
// used to filter the output of certain specific fields
// TODO: implement this in the backend
func skipName(ctx *context, name string) bool {
	if len(ctx.names) == 0 {
		return false
	}
	for _, n := range ctx.names {
		if n == name {
			return false
		}
	}
	return true
}

// skipAny returns true if for any reason this object doesn't match
// the specified match criteria
func skipAny(ctx *context, meta *swapi.ObjectMeta) bool {
	if skipRe(ctx.re, meta.Name) {
		return true
	}
	if skipLabel(ctx.labelStrs, meta.Labels) {
		return true
	}
	if skipName(ctx, meta.Name) {
		return true
	}
	return false
}

// getFilteredNames, fetches all names and filter out the ones
// that do not meet the specified match criteria; this is used by
// BashCompleter functions
func getFilteredNames(ctx *context) []string {
	names := []string{}
	objs := &api.ListHeader{}
	url := getURL(ctx, "")
	err := restGet(url, ctx.tenant, objs)
	if err != nil {
		fmt.Printf("Error getting %ss: %v", ctx.subcmd, err)
		return names
	}

	for _, obj := range objs.Items {
		if skipAny(ctx, &obj.ObjectMeta) {
			continue
		}
		names = append(names, obj.ObjectMeta.Name)
	}

	return names
}

// post an object over the REST interface
func postObj(ctx *context, obj interface{}) error {
	url := getURL(ctx, "")

	hdrBytes, err := json.Marshal(obj)
	if err != nil {
		return fmt.Errorf("Unable to find header in object (marshal): %+v", obj)
	}
	hdr := api.ObjectHeader{}
	if err := json.Unmarshal(hdrBytes, &hdr); err != nil {
		return fmt.Errorf("Unable to find header in object (unmarshal): '%s'", string(hdrBytes))
	}

	err = restPost(url, ctx.tenant, obj)
	if err != nil {
		return fmt.Errorf("Error creating %s '%s': %v", ctx.subcmd, hdr.ObjectMeta.Name, err)
	}

	return nil
}

// put an object over the REST interface
func putObj(ctx *context, obj interface{}) error {
	runObj, ok := obj.(runtime.Object)
	if !ok {
		return fmt.Errorf("Not runtime object")
	}
	objm, err := runtime.GetObjectMeta(runObj)
	if err != nil {
		return err
	}

	url := getURL(ctx, objm.Name)

	hdrBytes, err := json.Marshal(obj)
	if err != nil {
		return fmt.Errorf("Unable to find header in object (marshal): %+v", obj)
	}
	hdr := api.ObjectHeader{}
	if err := json.Unmarshal(hdrBytes, &hdr); err != nil {
		return fmt.Errorf("Unable to find header in object (unmarshal): '%s'", string(hdrBytes))
	}

	err = restPut(url, ctx.tenant, obj)
	if err != nil {
		return fmt.Errorf("Error updating %s '%s': %v", ctx.subcmd, hdr.ObjectMeta.Name, err)
	}

	return nil
}

// get underline string of sepcified length
// TODO: replace with equivalent predefined strings function
func getHdrUL(len int) string {
	lineStr := ""
	for ii := 0; ii < len; ii++ {
		lineStr = lineStr + "-"
	}
	return lineStr
}

// sortKeys sorts the keys of a provided map
// TODO: move this to utils file
func sortKeys(m map[string]bool) []string {
	mk := make([]string, len(m))
	i := 0
	for k := range m {
		mk[i] = k
		i++
	}
	sort.Strings(mk)
	return mk
}

// getLineHeader returns one line of the header (field names with underlines) for multi-line read output
func getLineHeader(ctx *context, objmValidKvs, specValidKvs map[string]bool, useTenant string) []byte {
	hdrNames := ""
	hdrUL := ""
	if useTenant != "" {
		if _, ok := objmValidKvs["tenant"]; ok {
			hdrNames += useTenant + "\t"
			hdrUL += getHdrUL(len(useTenant)) + "\t"
		}
	}
	if _, ok := objmValidKvs["name"]; ok {
		hdrNames += "Name" + "\t"
		hdrUL += getHdrUL(len("name"))
	}
	hdrUL += "\t"
	if _, ok := objmValidKvs["labels"]; ok {
		hdrNames += "Labels" + "\t"
		hdrUL += getHdrUL(len("labels"))
		hdrUL += "\t"
	}

	for _, key := range sortKeys(specValidKvs) {
		if !specValidKvs[key] {
			continue
		}
		hdrNames += key + "\t"
		hdrUL += getHdrUL(len(key)) + "\t"
	}
	return ([]byte(hdrNames + "\n" + hdrUL + "\n"))
}

// getLineData returns one line of the summary line in multi-line read output
func getLineData(ctx *context, specIdx *int, objmKvs, specKvs map[string]ref.FInfo, objmValidKvs, specValidKvs map[string]bool, useTenant string) ([]byte, bool) {
	more := false
	objLine := ""
	if useTenant != "" {
		if fi, ok := objmKvs["tenant"]; ok && objmValidKvs["tenant"] {
			if *specIdx == 0 {
				objLine += fi.ValueStr[0]
			}
			objLine += "\t"
		}
	}
	if fi, ok := objmKvs["name"]; ok && objmValidKvs["name"] {
		if *specIdx == 0 {
			objLine += fi.ValueStr[0]
		}
	}
	objLine += "\t"
	if fi, ok := objmKvs["labels"]; ok && objmValidKvs["labels"] {
		vals := fi.ValueStr
		sort.Strings(vals)
		if len(vals) >= *specIdx+1 {
			objLine += vals[*specIdx]
			if len(vals) >= *specIdx+2 {
				more = true
			}
		}
		objLine += "\t"
	}

	for _, key := range sortKeys(specValidKvs) {
		if !specValidKvs[key] {
			continue
		}

		fi := specKvs[key]
		if len(fi.ValueStr) >= *specIdx+1 {
			objLine += fi.ValueStr[*specIdx]
			if len(fi.ValueStr) >= *specIdx+2 {
				more = true
			}
		}
		objLine += "\t"
	}
	*specIdx++
	return []byte(objLine + "\n"), more
}

// configCommandCompletion configures a linux bash command completion
func configCommandCompletion(c *cli.Context) {

	fi, err := os.Lstat("/etc/bash_completion.d")
	if err != nil {
		fmt.Printf("Unable to stat /etc/bash_completion.d ... quitting")
		return
	}

	if fi.Mode().IsDir() {
		err = ioutil.WriteFile("/etc/bash_completion.d/venice", []byte(autoCompletionScript), 0755)
	} else {
		err = ioutil.WriteFile("/usr/local/etc/bash_completion.d/venice", []byte(autoCompletionScript), 0755)
	}
	if err != nil {
		fmt.Printf("error creating a file: %s\n", err)
		return
	}
	fmt.Printf("Almost there... please also execute following in your bash: \"source /etc/bash_completion.d/venice\"\n")
}

var autoCompletionScript = `#! /bin/bash

: ${PROG:=$(basename ${BASH_SOURCE})}

_cli_bash_autocomplete() {
    local cur opts base
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    opts=$( ${COMP_WORDS[@]:0:$COMP_CWORD} --gbc )
    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
    return 0
}

complete -F _cli_bash_autocomplete $PROG

unset PROG
`
