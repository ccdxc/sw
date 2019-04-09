// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
// This file contains the actions needed for TechSupport

package actionengine

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"

	tsconfig "github.com/pensando/sw/venice/ctrler/tsm/config"
	"github.com/pensando/sw/venice/utils/log"
)

// RunShellCmd runs shell command
func RunShellCmd(cmdStr string) ([]byte, error) {
	log.Infof("Running : " + cmdStr)
	cmd := exec.Command("bash", "-c", cmdStr)
	return cmd.Output()
}

// GetDelphiObject gets delphi object details
func GetDelphiObject(obj string) error {
	log.Infof("Getting DelphiObject : %v", obj)
	return nil
}

// MakeRESTCall  makes a rest call
func MakeRESTCall(cmd string) error {
	log.Infof("Making REST Call : %v", cmd)
	return nil
}

// CollectFile copies file into the techsupport bundle
func CollectFile(fileName string) error {
	log.Infof("Collecting File : %v", fileName)
	return nil
}

// RunActions runs all the actions provided in the techsupport config
func RunActions(actions []*tsconfig.ActionItem, techsupportDir string) error {
	log.Infof("ACTIONS : %v", actions)
	if len(actions) == 0 {
		log.Errorf("ActionItem list is empty")
		return fmt.Errorf("actionitem list is empty")
	}

	if techsupportDir == "" {
		log.Errorf("Empty techsupport directory passed.")
		return fmt.Errorf("empty techsupport directory passed")
	}

	if _, err := os.Stat(techsupportDir); os.IsNotExist(err) {
		log.Infof("Creating techsupport directory : %v", techsupportDir)
		res := os.MkdirAll(techsupportDir, 0777)
		log.Infof("RES : %v", res)

		if res != nil {
			return err
		}
	}

	fmt.Println("Creating Techsupport Directory : ", techsupportDir)
	f, err := os.OpenFile(techsupportDir+"/cmd.txt", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Errorf("Could not open file : %v", err)
	}

	for _, action := range actions {
		switch action.GetMethod() {
		case tsconfig.ActionItem_ShellCmd:
			f.WriteString("==== RUNNING Command ====\n")
			f.WriteString(action.Command + "\n")
			out, _ := RunShellCmd(action.Command)
			f.WriteString(string(out) + "\n")

		case tsconfig.ActionItem_DelphiObj:
			GetDelphiObject(action.Command)
		case tsconfig.ActionItem_RESTCall:
			MakeRESTCall(action.Command)
		case tsconfig.ActionItem_CollectFile:
			CollectFile(action.Command)
		default:
			log.Errorf("Unknown Action method specified : %v", action.Method)
		}
		log.Infof("ACTION : %v", action)
	}

	f.Close()
	return nil
}

// ReadConfig reads the config file and stores it in techsupport agent
func ReadConfig(configPath string) (*tsconfig.TechSupportConfig, error) {
	if configPath == "" {
		log.Errorf("Config Path is empty.")
		return nil, fmt.Errorf("config path is empty")
	}

	file, err := ioutil.ReadFile(configPath)
	if err != nil {
		log.Errorf("Could not open config file. Err : %v", err)
		return nil, err
	}

	var techsupportConfig tsconfig.TechSupportConfig
	err = json.Unmarshal(file, &techsupportConfig)
	if err != nil {
		log.Errorf("Unmarshall failed. Err : %v", err)
		return nil, err
	}

	if len(techsupportConfig.Name) == 0 {
		log.Errorf("Name is empty. Invalid Config.")
		return nil, fmt.Errorf("empty config name")
	}

	if len(techsupportConfig.FileSystemRoot) == 0 {
		log.Errorf("FileSystemRoot empty.")
		return nil, fmt.Errorf("empty filesystem name")
	}

	if _, err := os.Stat(techsupportConfig.FileSystemRoot); os.IsNotExist(err) {
		log.Errorf("Directory does not exist. %v", techsupportConfig.FileSystemRoot)
		return nil, err
	}

	return &techsupportConfig, nil
}

// CollectTechSupport runs all actions in config file
func CollectTechSupport(config *tsconfig.TechSupportConfig, instanceID string) error {
	if config == nil {
		log.Error("TechSupportConfig passed is nil.")
		return fmt.Errorf("config passed is nil")
	}

	baseDir := config.FileSystemRoot + "/" + instanceID

	log.Infof("Starting to collect TechSupport in directory : %v", baseDir)

	log.Infof("======== Prep Actions - Start ========")
	RunActions(config.PrepActions, baseDir+"/PrepActions")
	log.Infof("======== Prep Actions - End ========")

	log.Infof("======== Collect Actions - Start ========")
	RunActions(config.CollectActions, baseDir+"/CollectActions")
	log.Infof("======== Collect Actions - End ========")

	log.Infof("======== Export Actions - Start ========")
	RunActions(config.ExportActions, baseDir+"/ExportActions")
	log.Infof("======== Export Actions - End ========")

	return nil
}
