package main

import (
	"fmt"
	"os"
	"penlop"
)

const help_str string = `
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
	debug_on := false
	verbose := false
	output_type := penlop.OUTPUT_LUA
	for i := 1; i < len(os.Args); i++ {
		switch os.Args[i] {
		// Single options first
		case "-d":
			debug_on = true
		case "-v":
			verbose = true
		default:
			// Double options below this point
			if (i + 1) == len(os.Args) {
				fmt.Fprintf(os.Stderr, "Unknown or incomplete option %s\n", os.Args[i])
				fmt.Fprintf(os.Stderr, help_str)
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
					output_type = penlop.OUTPUT_LUA
				case "Lua":
					output_type = penlop.OUTPUT_LUA
				case "C":
					output_type = penlop.OUTPUT_LUA
				case "none":
					output_type = penlop.OUTPUT_NONE
				case "None":
					output_type = penlop.OUTPUT_NONE
				default:
					fmt.Fprintf(os.Stderr, "Unknown output type %s\n", os.Args[i+1])
					fmt.Fprintf(os.Stderr, help_str)
					os.Exit(1)
				}
			default:
				fmt.Fprintf(os.Stderr, "Unknown option %s\n", os.Args[i])
				fmt.Fprintf(os.Stderr, help_str)
				os.Exit(1)
			} // switch
			i++
		} // switch
	} // for

	// Check required parameters
	if len(infilename) == 0 {
		fmt.Fprintf(os.Stderr, "No input filename specified\n")
		fmt.Fprintf(os.Stderr, help_str)
		os.Exit(1)
	}

	app, err := penlop.Parse(infilename, verbose)
	if (err == nil) && (output_type != penlop.OUTPUT_NONE) {
		err = penlop.Generate(app, outfilename, output_type, debug_on)
	}
}
