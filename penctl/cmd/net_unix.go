//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

// +build linux freebsd

package cmd

import (
	"os"
	"os/exec"
	"strconv"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/globals"
)

func pickNetwork(cmd *cobra.Command, args []string) error {
	if mockMode {
		return nil
	}
	revProxyPort = globals.AgentProxyPort
	if val, ok := os.LookupEnv("DSC_PORT"); ok {
		revProxyPort = val
	}
	if len(dscURL) > 0 {
		naplesURL = dscURL
	} else if val, ok := os.LookupEnv("DSC_URL"); ok {
		for strings.HasSuffix(val, "/") {
			val = val[:len(val)-1]
		}
		naplesURL = val
	} else if val, ok := os.LookupEnv("NAPLES_URL"); ok {
		for strings.HasSuffix(val, "/") {
			val = val[:len(val)-1]
		}
		naplesURL = val
	} else if cmd.Flags().Changed("localhost") {
		naplesURL = "http://127.0.0.1"
	} else {
		// figuring out the right IP now
		out, err := exec.Command("/bin/bash", "-c", "/usr/bin/lspci -nd 1dd8:1004 | /usr/bin/head -1").Output()
		if err == nil {
			s := strings.Split(string(out), ":")
			if len(s) != 0 && s[0] != "" {
				bus, _ := strconv.ParseInt(s[0], 16, 64)
				naplesURL = "http://169.254." + strconv.Itoa(int(bus)) + ".1"

				// check if naples is reachable
				naplesIP = stripURLScheme(naplesURL)
				naplesURL += ":" + revProxyPort + "/"
				if isNaplesReachable() == nil {
					return nil
				}
			}
		}
		// should never be set to this. but trying this ip as a last resort anyways
		// also needed for backwards compatibility.
		naplesURL = "http://169.254.0.1"
	}
	naplesIP = stripURLScheme(naplesURL)
	naplesURL += ":" + revProxyPort + "/"

	return isNaplesReachable()
}
