// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
// This file contains the actions needed for TechSupport

package actionengine

import (
	"fmt"
	"os/exec"
	"regexp"
	"strings"

	"github.com/pensando/sw/venice/utils/log"
)

// RunShellCmd runs shell command
func RunShellCmd(cmdStr, cmdsDir string) error {
	reg, err := regexp.Compile("[^a-zA-Z0-9_]+")
	if err != nil {
		return nil
	}
	cmdFileName := strings.ReplaceAll(cmdStr, " ", "_")
	cmdFileName = reg.ReplaceAllString(cmdFileName, "")
	cmdStr = fmt.Sprintf("%s >> %s/%s.txt", cmdStr, cmdsDir, cmdFileName)

	log.Infof("Running : " + cmdStr)
	cmdOut := exec.Command("bash", "-c", cmdStr)
	_, err = cmdOut.Output()
	return err
}

// GetDelphiObject gets delphi object details
func GetDelphiObject(obj, cmdsDir string) error {
	log.Infof("Getting DelphiObject : %v", obj)
	return RunShellCmd("delphictl db get "+obj, cmdsDir)
}

// MakeRESTCall  makes a rest call
func MakeRESTCall(cmd string) error {
	log.Infof("Making REST Call : %v", cmd)
	return nil
}

// CollectFile copies file into a bundle
func CollectFile(targetDir, fileName string) error {
	log.Infof("Collecting File : %v", fileName)
	cmd := fmt.Sprintf("mkdir -p %v/%v && cp %v/* %v/%v/", targetDir, fileName, fileName, targetDir, fileName)
	return RunShellCmd(cmd, "files.txt")
}
