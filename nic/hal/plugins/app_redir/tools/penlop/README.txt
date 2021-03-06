(Work in progress)

PenLOP - Pensando Language of Parsers

This is a standalone tool for the purpose of converting a YAML or JSON file to a Lua file used by Snort3.
The YAML/JSON format should conform to the PenLOP schema, as described by:

https://docs.google.com/document/d/1j8CRxS2CYhH4eT7BZiptVGtlpEa6uJhrHjm3f8SR4JU

Files:

Main package:
  penlop.go - Processes the command line arguments and runs the tool.  Run without arguments to see the options.

PenLOP package:
  parser.go - First level parsing, converts a YAML/JSON file into a binary format
  generator.go - Second level generator, takes the parsed input and generates the specified output format, one of:
    lua_generator.go - Generate Lua script for use by Snort3 OpenAppID
    nil_generator.go - Blank implementation of the interface

Samples:
  sample1.yaml - A PenLOP description of an imaginary protocol PTTP which looks similar to HTTP
  sample1.lua  - Autogenerated from running the PenLOP tool, included here for easy reference
