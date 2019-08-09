// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
// This file contains the actions needed for TechSupport

package actionengine

import (
	"fmt"
	"os/exec"

	"github.com/pensando/sw/venice/utils/log"
)

// RunShellCmd runs shell command
func RunShellCmd(cmdStr string) ([]byte, error) {
	log.Infof("Running : " + cmdStr)
	cmd := exec.Command("bash", "-c", cmdStr)
	return cmd.Output()
}

// GetDelphiObject gets delphi object details
func GetDelphiObject(obj string) []byte {
	log.Infof("Getting DelphiObject : %v", obj)
	ret, _ := RunShellCmd("delphictl db get " + obj)
	return ret
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
	_, err := RunShellCmd(cmd)
	return err
}
