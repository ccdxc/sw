package vcli

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"strings"

	"github.com/pensando/sw/venice/cli/api"
	"github.com/pensando/sw/venice/ncli/gen"
	"github.com/pensando/sw/venice/utils/netutils"
)

// processFiles takes a list of comma separated file/url/directory names
// and invokes routine to process one file/url/directory at a time
func processFiles(ctx *context, fileArg string) {
	if len(fileArg) == 0 {
		fmt.Printf("Null file name")
		return
	}

	for _, objKind := range gen.GetInfo().GetAllKeys() {
		filenames := strings.Split(fileArg, ",")
		for _, filename := range filenames {
			var err error

			// processOne process one file, url or a directory
			if strings.HasPrefix(filename, "http") {
				err = processURL(ctx, filename, objKind)
			} else {
				err = processDir(ctx, filename, objKind)
			}

			if err != nil {
				fmt.Printf("%s, Aborting", err)
				return
			}
		}
	}
}

// processDir process a file or directory
func processDir(ctx *context, filename, kind string) error {
	fstat, err := os.Stat(filename)
	if err != nil {
		if os.IsNotExist(err) {
			return fmt.Errorf("File does not exist: '%s'", filename)
		}
		return err
	}

	if fstat.Mode().IsDir() {
		fis, err := ioutil.ReadDir(filename)
		if err != nil {
			return fmt.Errorf("Error reading directory '%s': %s", filename, err)
		}

		for _, fi := range fis {
			if err := processDir(ctx, filename+"/"+fi.Name(), kind); err != nil {
				return err
			}
		}
	} else {
		if !strings.HasSuffix(filename, "yaml") && !strings.HasSuffix(filename, "yml") && !strings.HasSuffix(filename, "json") {
			return nil
		}

		if err := processFile(ctx, filename, kind); err != nil {
			return err
		}
	}
	return nil
}

// processURL creates/updates objects (json/yml) on a http URL
func processURL(ctx *context, url, kind string) error {
	inp, err := netutils.HTTPGetRaw(url)
	if err != nil {
		return fmt.Errorf("Error fetching URL %s: %s", url, err)
	}

	if err := processRecs(ctx, string(inp), kind); err != nil {
		return fmt.Errorf("URL %s, %s", url, err)
	}
	return nil
}

// processFile reads one file and creates/udpates objects present in a file
func processFile(ctx *context, filename, kind string) error {
	b, err := ioutil.ReadFile(filename)
	if err != nil {
		fmt.Printf("Error reading file contents: %s", err)
	}

	if err := processRecs(ctx, string(b), kind); err != nil {
		return fmt.Errorf("File '%s' %s", filename, err)
	}

	return nil
}

// processRecs processes one or multiple records present in a json or yml file
// each record is treated independently i.e. can be of different kind
func processRecs(ctx *context, inp, kind string) error {
	isJSON := isJSON(inp)
	for recBegin := 0; ; {
		recBytes := ""
		if isJSON {
			recBytes, recBegin = findJSONRecord(inp, recBegin)
		} else {
			recBytes, recBegin = findYmlRecord(inp, recBegin)
		}
		if recBytes == "" || recBegin < 0 {
			break
		}
		if err := processRec(ctx, recBytes, kind); err != nil {
			return err
		}
	}

	return nil
}

// processRec creates an object from multiple records that were obtained from a file
func processRec(ctx *context, inp, kind string) error {
	hdr := &api.ObjectHeader{}

	if err := json.Unmarshal([]byte(inp), hdr); err != nil {
		return fmt.Errorf("Unmarshling error: %s\nrec: %s", err, inp)
	}

	ctx.subcmd = strings.ToLower(hdr.Kind)
	if ctx.subcmd != kind {
		return nil
	}

	if err := createObjFromBytes(ctx, inp); err != nil {
		return fmt.Errorf("Error creating from raw bytes: %s\nrec: %s", err, inp)
	}

	return nil
}

func editFromFile(ctx *context, filename string) error {
	if filename == "" {
		return fmt.Errorf("invalid filename %s", filename)
	}

	editorBin := os.Getenv("VENICE_EDITOR")
	if editorBin == "" {
		editorBin = os.Getenv("EDITOR")
	}
	if editorBin == "" {
		editorBin = "vi"
	}
	cmdArgs := strings.Split(editorBin, " ")
	cmdArgs = append(cmdArgs, filename)

	cmd := exec.Command(cmdArgs[0], cmdArgs[1:]...)
	cmd.Stdout = os.Stdout
	cmd.Stdin = os.Stdin
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("unable to edit config file: %s", err)
	}

	if err := processFile(ctx, filename, ctx.subcmd); err != nil {
		return fmt.Errorf("error processing the file: %s", err)
	}

	return nil
}
