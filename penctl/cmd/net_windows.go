//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"

	ps "github.com/bhendo/go-powershell"
	"github.com/bhendo/go-powershell/backend"
	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/globals"
)

func getBusNumber() (bus string, err error) {
	// choose a backend
	back := &backend.Local{}

	// start a local powershell process
	shell, err := ps.New(back)
	if err != nil {
		fmt.Errorf("%v", err)
	}
	defer shell.Exit()

	// Powershell commands to get highest bus no of pensando nic
	command := "Get-NetAdapterHardwareInfo -InterfaceDescription 'Pensando*' | Sort-Object 'Bus' -Descending | Select-Object -first 1 | select Name,Slot,Bus | convertto-json"
	stdout, _, err := shell.Execute(command)

	netAdapterHardwareInfo := make(map[string]interface{}, 0)
	err = json.Unmarshal([]byte(stdout), &netAdapterHardwareInfo)

	if err == nil {
		bus = strconv.Itoa(int(netAdapterHardwareInfo["Bus"].(float64)))
	}
	return bus, err
}

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
		bus, err := getBusNumber()
		if err == nil {
			naplesURL = "http://169.254." + bus + ".1"
			// check if naples is reachable
			naplesIP = stripURLScheme(naplesURL)
			naplesURL += ":" + revProxyPort + "/"
			if isNaplesReachable() == nil {
				return nil
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
