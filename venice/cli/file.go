package vcli

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strings"

	"github.com/ghodss/yaml"
	"github.com/urfave/cli"

	"github.com/pensando/sw/venice/cli/api"
	"github.com/pensando/sw/venice/utils/netutils"
)

// File processing
// step1: validate file(s), or directory(ies), or URL(s). Read their content and process them one at a time
// step2: find if the contents are json or yaml content
// step3: split the file into individual records, if there are multiple records
// step4: read type/object meta to know the record types
// step5: validate each record (json decode into a structure), if not, display the exact record giving the error, show the line with error
// step6: find if some junk chars are included in the definition, based on reconverted content, warn user if different
// step7: execute all commands in a loop, honoring flags like 'no-action' or 'rmw'
// step8: report error if any command fails, stop executing on first failure (or should we proceed further?)

// createFromFile is top level command handler called when upload from file/url/directory is specified
func createFromFile(c *cli.Context) {
	ctx := &context{cli: c, tenant: defaultTenant}
	if err := processGlobalFlags(ctx, "create"); err != nil {
		return
	}
	if len(c.Args()) == 0 {
		fmt.Println("A list of files, a directory path or URL path is required")
		return
	}

	createFromFileName(ctx, c.Args()[0])
}

// createFromFileName takes a list of comma separated file/url/directory names and calls
// routine to process one file/url/directory at a time
func createFromFileName(ctx *context, fileArg string) {
	if len(fileArg) == 0 {
		fmt.Printf("Null file name")
		return
	}

	for _, kind := range objOrder {
		filenames := strings.Split(fileArg, ",")
		for _, filename := range filenames {
			if err := processOne(ctx, filename, kind); err != nil {
				fmt.Printf("%s, Aborting", err)
				return
			}
		}
	}
}

// processOne process one file, url or a directory
func processOne(ctx *context, filename, kind string) error {
	if strings.HasPrefix(filename, "http") {
		return processURL(ctx, filename, kind)
	}
	return processDir(ctx, filename, kind)
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
		if err := processFile(ctx, filename, kind); err != nil {
			return err
		}
	}
	return nil
}

// processURL fetches the objects to be crate from a URL and posts them
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
	if !strings.HasSuffix(filename, "yaml") && !strings.HasSuffix(filename, "yml") && !strings.HasSuffix(filename, "json") {
		return nil
	}

	return processFilePostchecks(ctx, filename, kind)

}

// processFilePostChecks reads the file contents and process vairous records
func processFilePostchecks(ctx *context, filename, kind string) error {
	b, err := ioutil.ReadFile(filename)
	if err != nil {
		fmt.Printf("Error reading file contents: %s", err)
	}

	if err := processRecs(ctx, string(b), kind); err != nil {
		return fmt.Errorf("File '%s' %s", filename, err)
	}

	return nil
}

// isJSON tries to approximate if the input (from a file) contains a json record
// if not the logic tries to parst it as yaml
func isJSON(inp string) bool {
	inp = strings.TrimSpace(inp)
	if len(inp) > 0 && inp[0] == '{' {
		return true
	}
	return false
}

// processRecs processes one or multiple records present in a json or yml file
// each record is treated independently i.e. can be of different kind
func processRecs(ctx *context, inp, kind string) error {
	if isJSON(inp) {
		depth := 0
		recBegin := 0
		recEnd := 0
		for ii := 0; ii < len(inp); ii++ {
			if inp[ii] == '{' {
				if depth == 0 {
					recBegin = ii
				}
				depth++
			}
			if inp[ii] == '}' {
				depth--
				if depth == 0 {
					recEnd = ii
					if err := processRec(ctx, inp[recBegin:recEnd+1], kind); err != nil {
						return err
					}
				}
			}
		}
		return nil
	}
	lines := strings.Split(inp, "\n")
	recBegin := 0
	for idx, line := range lines {
		if line == "---" || idx == len(lines)-1 {
			if idx <= recBegin {
				continue
			}
			yamlRec := strings.Join(lines[recBegin:idx-1], "\n")
			jsonRec, err := yaml.YAMLToJSON([]byte(yamlRec))
			if err != nil {
				fmt.Printf("Error converting yaml to json: %v\nRec:\n%s\n\n", err, yamlRec)
				return err
			}

			if err := processRec(ctx, string(jsonRec), kind); err != nil {
				fmt.Printf("Error processing record: %+v %s\n", jsonRec, err)
				return err
			}
			recBegin = idx + 1
		}
	}
	return nil
}

// processRec creates an object from multiple records that were obtained from a file
func processRec(ctx *context, inp, kind string) error {
	hdr := &api.ObjectHeader{}

	if err := json.Unmarshal([]byte(inp), &hdr); err != nil {
		return fmt.Errorf("Unmarshling error: %s\nrec: %s", err, inp)
	}

	if hdr.TypeMeta.Kind != kind {
		return nil
	}

	ctx.subcmd = hdr.TypeMeta.Kind
	if err := createObjFromBytes(ctx, hdr.ObjectMeta.Name, inp); err != nil {
		return fmt.Errorf("Error creating from raw bytes: %s\nrec: %s", err, inp)
	}

	return nil
}
