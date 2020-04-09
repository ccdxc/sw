//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"fmt"
	"strings"

	ps "github.com/bhendo/go-powershell"
	"github.com/bhendo/go-powershell/backend"
	"github.com/spf13/cobra"
)

func getNaplesInfo(cmd *cobra.Command, args []string) error {
	// choose a backend
	back := &backend.Local{}

	// start a local powershell process
	shell, err := ps.New(back)
	if err != nil {
		fmt.Errorf("%v", err)
	}
	defer shell.Exit()

	// powershell commands to get network hardware with respective slot no
	command := "Get-NetAdapterHardwareInfo -InterfaceDescription 'Pensando*' | Group-Object Slot | convertto-json -depth 1"
	stdout, _, err := shell.Execute(command)

	if strings.HasPrefix(stdout, "{") {
		stdout = fmt.Sprintf("[%s]", stdout)
	}

	netAdapterHardwareInfo := make([]map[string]interface{}, 0)
	err = json.Unmarshal([]byte(stdout), &netAdapterHardwareInfo)

	managementInterfaces := make([]string, 0)

	for _, val := range netAdapterHardwareInfo {
		slot := val["Name"].(string)
		command := fmt.Sprintf("Get-NetAdapterHardwareInfo -InterfaceDescription 'Pensando*' | Where-Object {$_.Slot -eq '%s'} | Sort-Object 'Bus' -Descending | Select-Object -first 1 | convertto-json -depth 1", slot)
		stdout, _, err = shell.Execute(command)
		managementInterface := make(map[string]interface{})
		if len(stdout) > 0 {
			err = json.Unmarshal([]byte(stdout), &managementInterface)
			if err != nil {
				fmt.Errorf("%v", err)
			} else {
				managementInterfaces = append(managementInterfaces, managementInterface["Name"].(string))
			}
		}
	}

	var pattern string
	//e.g - ^Ethernet 3$|^Ethernet 2$|^Ethernet$
	for _, name := range managementInterfaces {
		pattern += fmt.Sprintf("^%s$|", name)
	}

	if len(pattern) > 0 {
		var naplesOut string
		command = fmt.Sprintf("Get-NetAdapter | Where-Object { $_.Name -match '%s'}", strings.TrimSuffix(pattern, "|"))
		if !tabularFormat {
			command += " | select Name,InterfaceDescription,ifIndex,Status,MacAddress,LinkSpeed | convertto-json"
		}

		naplesOut, _, err = shell.Execute(command)
		if err != nil {
			fmt.Errorf("%v", err)
		} else {
			if len(naplesOut) > 0 {
				fmt.Println(naplesOut)
			} else {
				fmt.Println("No naples information found")
			}
		}
	} else {
		fmt.Println("No naples information found")
	}
	return err
}
