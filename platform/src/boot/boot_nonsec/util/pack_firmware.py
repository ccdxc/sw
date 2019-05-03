#!/usr/bin/env python

""" Copyright (c) 2017-2018 Silex Inside. All Rights reserved """

import optparse
import os
import utils

def main():
   # Parse arguments
   parser = optparse.OptionParser(description="Pack the firmware")
   parser.add_option("--algo", help="authentication algorithm")
   parser.add_option("--version", help="", metavar="NUMBER", type=int, default=0)
   parser.add_option("--sign", help="sign firmware using developer's signing key", metavar="FILE.sk")
   parser.add_option("--cert", help="include developer's certificate", metavar="FILE.cert")
   parser.add_option("--encrypt", help="encrypt firmware using developer's KEK", metavar="FILE.kek")
   parser.add_option("--rootkeys_sz", help="Boot keys (FRK, SRK, ..) size in bytes", default=16)
   parser.add_option("--frk_select", help="firmware root key index", metavar="NUMBER", type=int, default=0)
   parser.add_option("--input", "-i", help="input binary - mandatory", metavar="FILE.bin")
   parser.add_option("--output", "-o", help="packed binary", metavar="FILE.bin")
   parser.add_option("--outhex", help="packed firmware in hex format", metavar="FILE.hex")
   options, args = parser.parse_args()

   # Check arguments
   if args:
      parser.error("too many arguments")
   if not options.input:
      parser.error("missing --input argument")
   if not options.algo:
      parser.error("missing --algo argument")
   if not options.output:
      parser.error("missing --output argument")
   if options.input==options.output:
      parser.error("input and output files are identical")
   if options.sign and not options.cert:
      parser.error("--cert option is required when using --sign option")

   # Read inputs
   bin = utils.read(options.input)
   KEK = utils.read(options.encrypt, int(options.rootkeys_sz))
   signing_key = utils.read(options.sign, utils.get_privkey_size(options.algo))
   certificate = utils.read(options.cert, utils.get_cert_size(options.algo))

   # Pack firmware
   firmware = utils.gen_firmware(options.algo, bin, options.version, KEK,
                  int(options.rootkeys_sz), options.frk_select, signing_key,
                  certificate)

   # Write output
   utils.write(options.output, firmware, tohex=False)
   utils.write(options.outhex, firmware, tohex=True)

utils.run(main)
