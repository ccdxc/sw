// Package main is the standalone program for running package penlop
// Copyright 2018 Pensando Systems, Inc.
package main

import (
	"fmt"
	"os"

	"github.com/pensando/sw/nic/hal/plugins/app_redir/tools/penlop"
)

const helpStr string = `
Supported options are:
  -i <input_file>
  -o <output_file>
  -d  // debug on
  -v  // verbose
  -g <generator_type>  // Only "Lua" supported right now
`

func main() {
	// get filenames from commandline
	var infilename, outfilename string
	debugOn := false
	verbose := false
	outputType := penlop.OutputTypeLUA
	for i := 1; i < len(os.Args); i++ {
		switch os.Args[i] {
		// Single options first
		case "-d":
			debugOn = true
		case "-v":
			verbose = true
		default:
			// Double options below this point
			if (i + 1) == len(os.Args) {
				fmt.Fprintf(os.Stderr, "Unknown or incomplete option %s\n", os.Args[i])
				fmt.Fprintf(os.Stderr, helpStr)
				os.Exit(1)
			}

			switch os.Args[i] {
			case "-i":
				infilename = os.Args[i+1]
			case "-o":
				outfilename = os.Args[i+1]
			case "-g":
				switch os.Args[i+1] {
				case "lua":
					outputType = penlop.OutputTypeLUA
				case "Lua":
					outputType = penlop.OutputTypeLUA
				case "C":
					outputType = penlop.OutputTypeLUA
				case "none":
					outputType = penlop.OutputTypeNONE
				case "None":
					outputType = penlop.OutputTypeNONE
				default:
					fmt.Fprintf(os.Stderr, "Unknown output type %s\n", os.Args[i+1])
					fmt.Fprintf(os.Stderr, helpStr)
					os.Exit(1)
				}
			default:
				fmt.Fprintf(os.Stderr, "Unknown option %s\n", os.Args[i])
				fmt.Fprintf(os.Stderr, helpStr)
				os.Exit(1)
			} // switch
			i++
		} // switch
	} // for

	// Check required parameters
	if len(infilename) == 0 {
		fmt.Fprintf(os.Stderr, "No input filename specified\n")
		fmt.Fprintf(os.Stderr, helpStr)
		os.Exit(1)
	}

	app, err := penlop.Parse(infilename, verbose)
	if (err == nil) && (outputType != penlop.OutputTypeNONE) {
		err = penlop.Generate(app, outfilename, outputType, debugOn)
	}
}
