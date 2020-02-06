// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
// This file contains the actions needed for TechSupport

package actionengine

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"strings"

	"github.com/pensando/sw/nic/agent/protos/tsproto"
	tsconfig "github.com/pensando/sw/venice/ctrler/tsm/config"
	"github.com/pensando/sw/venice/utils/log"
)

// RunActions runs all the actions provided in the techsupport config
func RunActions(actions []*tsconfig.ActionItem, techsupportDir string, skipCores bool) error {
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

	cmdsDir := fmt.Sprintf("%s/cmds/", techsupportDir)
	if _, err := os.Stat(cmdsDir); os.IsNotExist(err) {
		log.Infof("Creating techsupport directory : %v", cmdsDir)
		res := os.MkdirAll(cmdsDir, 0777)
		if res != nil {
			return err
		}
	}

	for _, action := range actions {
		switch action.GetMethod() {
		case tsconfig.ActionItem_ShellCmd:
			if !(skipCores && strings.Contains(action.Command, "core")) {
				RunShellCmd(action.Command, cmdsDir)
			} else {
				log.Infof("==== Skipping Command : %v ====", action.Command)
			}
		case tsconfig.ActionItem_DelphiObj:
			GetDelphiObject(action.Command, cmdsDir)
		case tsconfig.ActionItem_RESTCall:
			MakeRESTCall(action.Command)
		case tsconfig.ActionItem_CollectFile:
			CollectFile(techsupportDir, action.Command)
		default:
			log.Errorf("Unknown Action method specified : %v", action.Method)
		}
		log.Infof("ACTION : %v", action)
	}

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

	if techsupportConfig.Retention != tsconfig.TechSupportConfig_DelOnExport && techsupportConfig.Retention != tsconfig.TechSupportConfig_Manual {
		return nil, fmt.Errorf("invalid retention option passed")
	}

	if _, err := os.Stat(techsupportConfig.FileSystemRoot); os.IsNotExist(err) {
		log.Errorf("Directory does not exist. %v", techsupportConfig.FileSystemRoot)
		os.MkdirAll(techsupportConfig.FileSystemRoot, 0777)
	}

	return &techsupportConfig, nil
}

// CollectTechSupport runs all actions in config file
func CollectTechSupport(config *tsconfig.TechSupportConfig, targetID string, tsWork *tsproto.TechSupportRequest) error {
	if config == nil {
		log.Error("TechSupportConfig passed is nil.")
		return fmt.Errorf("config passed is nil")
	}

	if tsWork == nil {
		log.Error("TechSupportRequest passed is nil.")
		return fmt.Errorf("tsWork passed is nil")
	}

	baseDir := config.FileSystemRoot + "/" + targetID

	if _, err := os.Stat(baseDir); os.IsNotExist(err) {
		log.Infof("Creating techsupport directory : %v", baseDir)
		res := os.MkdirAll(baseDir, 0777)
		log.Infof("RES : %v", res)

		if res != nil {
			return res
		}
	}

	err := os.Chdir(baseDir)
	if err != nil {
		log.Errorf("Failed to change to :%v. Techsupport collection may behave abnormally. Err: %v", baseDir, err)
	}

	log.Infof("Starting to collect TechSupport in directory : %v", baseDir)

	log.Infof("======== Prep Actions - Start ========")
	err = RunActions(config.PrepActions, baseDir+"/PrepActions", tsWork.Spec.SkipCores)
	log.Infof("======== Prep Actions - End ========")
	if err != nil {
		log.Errorf("Failed to run PrepActions. %v", err)
	}

	log.Infof("======== Collect Actions - Start ========")
	err = RunActions(config.CollectActions, baseDir+"/CollectActions", tsWork.Spec.SkipCores)
	log.Infof("======== Collect Actions - End ========")
	if err != nil {
		log.Errorf("Failed to run CollectActions. %v", err)
	}

	log.Infof("======== Export Actions - Start ========")
	err = RunActions(config.ExportActions, baseDir+"/ExportActions", tsWork.Spec.SkipCores)
	log.Infof("======== Export Actions - End ========")
	if err != nil {
		log.Errorf("Failed to run ExportActions. %v", err)
	}
	os.Chdir("/")

	return nil
}

// CleanTechsupportDirectory delete all files and directory within the techsupport directory
func CleanTechsupportDirectory(config *tsconfig.TechSupportConfig) error {
	log.Infof("Cleaning techsupport directory")
	dir, err := ioutil.ReadDir(config.FileSystemRoot)
	if err != nil {
		return err
	}

	for _, d := range dir {
		os.RemoveAll(path.Join([]string{config.FileSystemRoot, d.Name()}...))
	}

	return nil
}
