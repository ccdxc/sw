package main

import (
	"flag"
	"fmt"
	"go/build"
	"io/ioutil"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"gopkg.in/yaml.v2"

	"github.com/russross/blackfriday"
)

type docFile struct {
	Input  string `yaml:"input"`
	Output string `yaml:"output"`
	Tag    string `yaml:"tag"`
	Embed  []string
}

type docConfig struct {
	Basepath string `yaml:"basepath"`
	Files    []docFile
}

func errorExit(err error, format string, args ...interface{}) {
	fmt.Printf(format, args...)
	if err != nil {
		fmt.Printf(" error:(%s)\n", err)
	} else {
		fmt.Printf("\n")
	}
	os.Exit(1)
}

type docContext struct {
	gopath string
	tags   map[string]docFile
	files  map[string]docFile
	cfg    docConfig
}

func processFile(dctx *docContext, fl docFile) ([]byte, error) {
	inpf := filepath.Join(dctx.gopath, dctx.cfg.Basepath, fl.Input)

	inbuf, err := ioutil.ReadFile(inpf)
	if err != nil {
		errorExit(err, "error processing file %s", inpf)
	}
	if fl.Embed != nil {
		for _, tag := range fl.Embed {
			efl, eok := dctx.tags[tag]
			if !eok {
				errorExit(nil, "Could not find embedded file %s", tag)
			}
			eflname := filepath.Join(dctx.gopath, dctx.cfg.Basepath, efl.Input)
			ebuf, err := ioutil.ReadFile(eflname)
			if err != nil {
				errorExit(err, "error processing file %s", eflname)
			}
			restr := fmt.Sprintf("\\[//embed\\]:\\s+#\\(\\-%s\\-\\)", tag)
			re := regexp.MustCompile(restr)
			inbuf = re.ReplaceAll(inbuf, ebuf)
		}
	}
	output := blackfriday.MarkdownCommon(inbuf)
	if fl.Output != "" {
		outf := filepath.Join(dctx.gopath, dctx.cfg.Basepath, fl.Output)
		file, err := os.OpenFile(outf, os.O_CREATE|os.O_TRUNC|os.O_WRONLY, 0644)
		if err != nil {
			errorExit(err, "could not open output file")
		}
		fmt.Fprint(file, string(output))
		file.Close()
		fmt.Printf("wrote file [%s]\n", outf)
	}
	return output, nil
}

func main() {
	fname := flag.String("c", "", "configuration file to process")
	flag.Parse()

	if *fname == "" {
		errorExit(nil, "config file is mandatory (relative to gopath")
	}
	gopath := os.Getenv("GOPATH")
	if gopath == "" {
		gopath = build.Default.GOPATH
	}
	cfile := filepath.Join(gopath, *fname)
	if strings.HasPrefix(*fname, ".") || strings.HasPrefix(*fname, "/") {
		cfile = *fname
	}

	buf, err := ioutil.ReadFile(cfile)
	if err != nil {
		errorExit(err, "could not open file [%s]", cfile)
	}
	cfg := docConfig{}
	err = yaml.Unmarshal(buf, &cfg)
	if err != nil {
		errorExit(err, "could not unmarshal config file")
	}
	ctx := docContext{
		gopath: gopath,
		tags:   make(map[string]docFile),
		files:  make(map[string]docFile),
		cfg:    cfg,
	}

	for _, fl := range cfg.Files {
		if fl.Tag != "" {
			ctx.tags[fl.Tag] = fl
		}
		ctx.files[fl.Input] = fl

	}
	for _, fl := range cfg.Files {
		processFile(&ctx, fl)
	}
}
