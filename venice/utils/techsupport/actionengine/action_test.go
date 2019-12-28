// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
// This file contains the actions needed for TechSupport

package actionengine

import (
	//"fmt"
	"encoding/json"
	"io/ioutil"
	"os"
	"testing"

	//"github.com/pensando/sw/api"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	tsconfig "github.com/pensando/sw/venice/ctrler/tsm/config"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	defaultName           = "default"
	defaultFileSystemPath = "/tmp"
	defaultConfigPath     = "/tmp/default.cfg"
)

func createTechSupportActionItem(name string, method tsconfig.ActionItem_ActionMethod, command string) *tsconfig.ActionItem {
	return &tsconfig.ActionItem{
		Name:    name,
		Method:  method,
		Command: command,
	}
}

func createDiagnosticsRequest(name, query, command, args string) *diagnostics.DiagnosticsRequest {
	return &diagnostics.DiagnosticsRequest{
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Query:      query,
		Parameters: map[string]string{"command": command, "args": args},
	}
}

func deleteFile(path string) {
	os.Remove(path)
	log.Infof("Deleted file : %v", path)
}

func createDefaultTechSupportConfig(name string, fileSystemPath string) *tsconfig.TechSupportConfig {
	techSupportConfig := &tsconfig.TechSupportConfig{
		Name:           name,
		FileSystemRoot: fileSystemPath,
	}

	action1 := createTechSupportActionItem("action1", tsconfig.ActionItem_ShellCmd, "ls -l")
	action2 := createTechSupportActionItem("action2", tsconfig.ActionItem_ShellCmd, "df -k")
	action3 := createTechSupportActionItem("action3", tsconfig.ActionItem_DelphiObj, "delphictl get NaplesStatus")
	action4 := createTechSupportActionItem("action4", tsconfig.ActionItem_RESTCall, "curl")
	action5 := createTechSupportActionItem("action5", tsconfig.ActionItem_CollectFile, "touch")

	techSupportConfig.PrepActions = append(techSupportConfig.PrepActions, action1)
	techSupportConfig.PrepActions = append(techSupportConfig.PrepActions, action2)

	techSupportConfig.CollectActions = append(techSupportConfig.CollectActions, action1)
	techSupportConfig.CollectActions = append(techSupportConfig.CollectActions, action2)
	techSupportConfig.CollectActions = append(techSupportConfig.CollectActions, action3)
	techSupportConfig.CollectActions = append(techSupportConfig.CollectActions, action4)
	techSupportConfig.CollectActions = append(techSupportConfig.CollectActions, action5)

	techSupportConfig.ExportActions = append(techSupportConfig.ExportActions, action1)
	techSupportConfig.ExportActions = append(techSupportConfig.ExportActions, action2)

	return techSupportConfig
}

func writeConfigToFile(config *tsconfig.TechSupportConfig, filePath string) {
	configMarsh, _ := json.Marshal(config)
	ioutil.WriteFile(filePath, configMarsh, 0644)
}

func CreateDefaultTechSupportConfigFile(name, fileSystemPath, configLocation string) {
	config := createDefaultTechSupportConfig(name, fileSystemPath)
	writeConfigToFile(config, configLocation)
}

func buildTechSupportRequest() *tsproto.TechSupportRequest {
	spec := &tsproto.TechSupportRequestSpec{
		SkipCores: false,
	}
	tsr := &tsproto.TechSupportRequest{
		Spec: *spec,
	}
	return tsr
}

func TestReadValidConfig(t *testing.T) {
	CreateDefaultTechSupportConfigFile(defaultName, defaultFileSystemPath, defaultConfigPath)

	cfg, err := ReadConfig(defaultConfigPath)
	deleteFile(defaultConfigPath)

	Assert(t, err == nil, "Could not read valid config. Err : %v", err)
	Assert(t, cfg.Name == defaultName, "Incorrect name read")
	Assert(t, cfg.FileSystemRoot == defaultFileSystemPath, "Incorrect file system root read")
}

func TestInvalidConfig(t *testing.T) {
	techSupportConfig := &tsconfig.TechSupportConfig{
		Name:           "invalid",
		FileSystemRoot: "/garbage",
		Retention:      5,
	}

	writeConfigToFile(techSupportConfig, defaultConfigPath)

	_, err := ReadConfig(defaultConfigPath)
	deleteFile(defaultConfigPath)

	Assert(t, err != nil, "Bad config passed")
}

func TestRunActionsNilTechSupport(t *testing.T) {
	err := RunActions(nil, "", false)
	Assert(t, err != nil, "RunActions failed")
}

func TestRunActionsMany(t *testing.T) {
	outDir := "/tmp/default-tsc"
	defaultConfig := createDefaultTechSupportConfig(defaultName, defaultFileSystemPath)

	err := RunActions(defaultConfig.CollectActions, outDir, false)
	Assert(t, err == nil, "RunActions for many actions failed")
}

func TestCollectTechSupportNil(t *testing.T) {
	err := CollectTechSupport(nil, "", nil)
	Assert(t, err != nil, "Collection of techsupport passed for bad parameters")
}

func TestCollectTechSupport(t *testing.T) {
	CreateDefaultTechSupportConfigFile(defaultName, defaultFileSystemPath, defaultConfigPath)

	cfg, err := ReadConfig(defaultConfigPath)
	deleteFile(defaultConfigPath)
	tsWork := buildTechSupportRequest()

	err = CollectTechSupport(cfg, "techsupport-collect", tsWork)
	Assert(t, err == nil, "Collection of techsupport failed")
}

func TestEmptyPathConfig(t *testing.T) {
	_, err := ReadConfig("")

	Assert(t, err != nil, "Test with Empty Path for config passed.")
}

func TestNonExistingConfig(t *testing.T) {
	_, err := ReadConfig("/tmp/def.cfg")

	Assert(t, err != nil, "Test with incorrect path for config passed.")
}

func TestEmptyNameConfig(t *testing.T) {
	techSupportConfig := &tsconfig.TechSupportConfig{
		FileSystemRoot: "/garbage",
	}

	writeConfigToFile(techSupportConfig, defaultConfigPath)

	_, err := ReadConfig(defaultConfigPath)
	deleteFile(defaultConfigPath)

	Assert(t, err != nil, "Bad config passed")
}

func TestEmptyFileSystemRootConfig(t *testing.T) {
	techSupportConfig := &tsconfig.TechSupportConfig{
		Name: "empty-filesystemroot",
	}

	writeConfigToFile(techSupportConfig, defaultConfigPath)

	_, err := ReadConfig(defaultConfigPath)
	deleteFile(defaultConfigPath)

	Assert(t, err != nil, "Bad config passed")
}

func TestBadJsonConfig(t *testing.T) {
	badJSON := "/tmp/bad.json"

	ioutil.WriteFile(badJSON, []byte("{"), 0644)
	_, err := ReadConfig(badJSON)
	deleteFile(badJSON)

	Assert(t, err != nil, "Bad config passed")
}

func TestRunActionsEmpty(t *testing.T) {
	techSupportConfig := &tsconfig.TechSupportConfig{
		Name:           "no-actions",
		FileSystemRoot: "/tmp",
	}

	err := RunActions(techSupportConfig.CollectActions, "/tmp", false)
	Assert(t, err != nil, "RunActions with empty actions passed")
}

func TestRunActionsEmptyOutDir(t *testing.T) {
	techSupportConfig := &tsconfig.TechSupportConfig{
		Name:           "no-actions",
		FileSystemRoot: "/tmp",
	}

	action1 := createTechSupportActionItem("action1", tsconfig.ActionItem_CollectFile, "touch")

	techSupportConfig.CollectActions = append(techSupportConfig.CollectActions, action1)

	err := RunActions(techSupportConfig.CollectActions, "", false)
	Assert(t, err != nil, "RunActions with empty actions passed")
}

func TestRunActionsNonExistingDir(t *testing.T) {
	outDir := "/tmp/doesnotexist"
	deleteFile(outDir)

	techSupportConfig := &tsconfig.TechSupportConfig{
		Name:           "no-actions",
		FileSystemRoot: "/tmp",
	}

	action1 := createTechSupportActionItem("action1", tsconfig.ActionItem_CollectFile, "touch")

	techSupportConfig.CollectActions = append(techSupportConfig.CollectActions, action1)

	err := RunActions(techSupportConfig.CollectActions, outDir, false)
	Assert(t, err == nil, "RunActions failed")

	_, err = os.Stat(outDir)
	Assert(t, !os.IsNotExist(err), "RunActions failed to create non-existing directory")
	deleteFile(outDir)
}

func TestDiagnosticsLs(t *testing.T) {
	diagnosticsRequest := createDiagnosticsRequest("valid", "ACTION", "ls", "-l")

	_, err := RunDiagnosticsActions(diagnosticsRequest)

	Assert(t, err == nil, "valid diagnostics request failed")
}

func TestDiagnosticsInvalid(t *testing.T) {
	diagnosticsRequest := createDiagnosticsRequest("valid", "ACTION", "touch", "/diagnosticsTest")

	_, err := RunDiagnosticsActions(diagnosticsRequest)

	Assert(t, err != nil, "invalid diagnostics request passed")
}

func TestDiagnosticsInsecure(t *testing.T) {
	diagnosticsRequest := createDiagnosticsRequest("valid", "ACTION", "ls", "-l && touch /diagnosticsTest")

	_, err := RunDiagnosticsActions(diagnosticsRequest)

	Assert(t, err != nil, "insecure diagnostics request passed")
}
