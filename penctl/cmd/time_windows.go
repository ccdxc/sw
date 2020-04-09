//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"fmt"
	"strings"
	"time"

	ps "github.com/bhendo/go-powershell"
	"github.com/bhendo/go-powershell/backend"
	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

func setSystemTimeCmdHandler(cmd *cobra.Command, args []string) error {
	// choose a backend
	back := &backend.Local{}

	// start a local powershell process
	shell, err := ps.New(back)
	if err != nil {
		fmt.Errorf("%v", err)
	}
	defer shell.Exit()

	// powershell commands to get timezone ID
	command := "Get-TimeZone | select Id | convertto-json"
	stdout, _, err := shell.Execute(command)
	timezoneDetails := make(map[string]interface{}, 0)
	err = json.Unmarshal([]byte(stdout), &timezoneDetails)
	if err == nil {
		if timezone, ok := timezones[timezoneDetails["Id"].(string)]; ok {
			v := &nmd.DistributedServiceCardCmdExecute{
				Executable: "pensettimezone",
				Opts:       strings.Join([]string{string(timezone)}, ""),
			}
			if err := naplesExecCmd(v); err != nil {
				return err
			}
			symlink := fmt.Sprintf("/usr/share/zoneinfo/%s", timezone)
			if err == nil {
				v := &nmd.DistributedServiceCardCmdExecute{
					Executable: "lnlocaltime",
					Opts:       strings.Join([]string{symlink}, " "),
				}
				if err := naplesExecCmd(v); err != nil {
					return err
				}
			}
		}
	}

	dateString := time.Now().Format("Jan 2 15:04:05 2006")
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "setdate",
		Opts:       strings.Join([]string{dateString}, ""),
	}
	if err := naplesExecCmd(v); err != nil {
		return err
	}

	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "sethwclock",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}
