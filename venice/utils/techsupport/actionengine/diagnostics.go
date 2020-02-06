// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
// This file contains the actions needed for TechSupport

package actionengine

import (
	"fmt"
	"os/exec"
	"regexp"

	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/log"
)

var allowedCommands = map[string]string{
	"ls":          "ls",
	"ps":          "ps",
	"df":          "df",
	"top":         "top -n 1",
	"ip_link":     "ip link",
	"ip_route":    "ip route",
	"ping":        "ping -c 4",
	"netstat":     "netstat",
	"lsof":        "lsof",
	"capmon":      "capmon",
	"delphictl":   "delphictl",
	"halctl_show": "halctl show",
	"halctl_dbg":  "/nic/tools/halctl_dbg.sh",
}

// RunDiagnosticsActions runs all the actions described in diagnostics request
func RunDiagnosticsActions(diagnosticsReq *diagnostics.DiagnosticsRequest) ([]byte, error) {
	cmdKey, ok := diagnosticsReq.Parameters["command"]
	if !ok {
		log.Error("No command passed for diagnostics request")
		return nil, fmt.Errorf("diagnostics request passed without any command key")
	}

	bin, ok := allowedCommands[cmdKey]
	if !ok {
		log.Errorf("Binary %v is not in the allowed list of commands", cmdKey)
		return nil, fmt.Errorf("%v id not in the allowed list of commands", cmdKey)
	}

	cmd := fmt.Sprintf("%v ", bin)
	args, ok := diagnosticsReq.Parameters["args"]
	if ok {
		cmd = fmt.Sprintf("%v %v", cmd, args)
	}

	reg, err := regexp.Compile(`[^-\.\,a-zA-Z0-9/_ ]+`)
	if err != nil {
		log.Errorf("Failed to compile regular expression. Err : %v", err)
		return nil, fmt.Errorf("failed to compule regular expression %v", err)
	}

	matchStr := reg.ReplaceAllString(cmd, "")
	if matchStr != cmd {
		log.Errorf("Command has illegal characters. CMD : %v", cmd)
		return nil, fmt.Errorf("illegal characters in cmd %v", cmd)
	}

	cmdOut := exec.Command("bash", "-c", matchStr)
	outBin, err := cmdOut.Output()
	if err != nil {
		log.Errorf("Failed running command. Err : %v", err)
		return nil, fmt.Errorf("Failed to run commands %v. Err : %v", cmd, err)
	}

	log.Info("Successfully run diagnostics request.")
	return outBin, nil
}
