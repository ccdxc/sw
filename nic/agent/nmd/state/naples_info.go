// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package state

import (
	"encoding/json"
	"errors"
	"io/ioutil"
	"net"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	"github.com/pensando/sw/venice/utils/log"
)

func getRunningFirmwareName() (string, error) {
	out, err := exec.Command("/bin/bash", "-c", "/nic/tools/fwupdate -r").Output()
	if err != nil {
		// TODO : Remove this. Adding this temporarily to ensure progress.
		// All these APIs can be moved under nmd/platform as it feels a more natural
		// home for this platform dependent code.
		// The platformAgent allows for easy mocking of this API
		// Tracker Jira : PS-1172

		return "mainfwa\n", nil
	}

	log.Infof("Got running software version : %v", string(out))
	return string(out), err
}

func getStartupFirmwareName() (string, error) {
	out, err := exec.Command("/bin/bash", "-c", "/nic/tools/fwupdate -S").Output()
	if err != nil {
		// TODO : Remove this. Adding this temporarily to ensure progress.
		// All these APIs can be moved under nmd/platform as it feels a more natural
		// home for this platform dependent code.
		// The platformAgent allows for easy mocking of this API
		// Tracker Jira : PS-1172

		return "mainfwa\n", nil
	}

	log.Infof("Got startup software version : %v", string(out))
	return string(out), err
}

func getInstalledSoftware() (*nmd.NaplesInstalledSoftware, error) {
	out, err := exec.Command("/bin/bash", "-c", "/nic/tools/fwupdate -l").Output()
	if err != nil {
		log.Errorf("Returning Software info default values")
		n := &nmd.NaplesInstalledSoftware{
			// TODO : Remove this. Adding this temporarily to ensure progress.
			// All these APIs can be moved under nmd/platform as it feels a more natural
			// home for this platform dependent code.
			// The platformAgent allows for easy mocking of this API
			// Tracker Jira : PS-1172
			Uboot: &nmd.UbootFw{
				Image: &nmd.ImageInfo{
					BaseVersion: "1.0E",
				},
			},
			MainFwA: &nmd.FwVersion{
				SystemImage: &nmd.ImageInfo{
					SoftwareVersion: "1.0E",
				},
			},
		}
		return n, nil
	}

	var naplesVersion nmd.NaplesInstalledSoftware
	json.Unmarshal([]byte(out), &naplesVersion)

	log.Infof("Got Naples Version %v", naplesVersion)

	return &naplesVersion, nil
}

func getRunningSoftware() (*nmd.NaplesRunningSoftware, error) {
	out, err := exec.Command("/bin/bash", "-c", "/nic/tools/fwupdate -L").Output()
	if err != nil {
		log.Errorf("Returning Software info default values")
		n := &nmd.NaplesRunningSoftware{
			// TODO : Remove this. Adding this temporarily to ensure progress.
			// All these APIs can be moved under nmd/platform as it feels a more natural
			// home for this platform dependent code.
			// The platformAgent allows for easy mocking of this API
			// Tracker Jira : PS-1172
			Uboot: &nmd.UbootFw{
				Image: &nmd.ImageInfo{
					BaseVersion: "1.0E",
				},
			},
			MainFwA: &nmd.FwVersion{
				SystemImage: &nmd.ImageInfo{
					SoftwareVersion: "1.0E",
				},
			},
		}
		return n, nil
	}

	var naplesVersion nmd.NaplesRunningSoftware
	json.Unmarshal([]byte(out), &naplesVersion)

	log.Infof("Got Naples Running Version %v", naplesVersion)

	return &naplesVersion, nil
}

func getRunningFirwmwareVersion(naplesVersion *nmd.NaplesRunningSoftware) (string, error) {
	currentFw, err := getRunningFirmwareName()
	if err != nil {
		return "", err
	}

	switch currentFw {
	case "mainfwa\n":
		return naplesVersion.MainFwA.SystemImage.SoftwareVersion, nil
	case "mainfwb\n":
		return naplesVersion.MainFwB.SystemImage.SoftwareVersion, nil
	default:
		return "", errors.New("unknown firmware version")
	}
}

func getStartupFirmwareVersion(naplesVersion *nmd.NaplesInstalledSoftware) (string, error) {
	startupFw, err := getStartupFirmwareName()
	if err != nil {
		return "", err
	}

	switch startupFw {
	case "mainfwa\n":
		return naplesVersion.MainFwA.SystemImage.SoftwareVersion, nil
	case "mainfwb\n":
		return naplesVersion.MainFwB.SystemImage.SoftwareVersion, nil
	default:
		return "", errors.New("unknown firmware version")
	}
}

// GetNaplesSoftwareInfo gets the software versions which are running and will run at startup
func GetNaplesSoftwareInfo() (*nmd.NaplesSoftwareVersionInfo, error) {
	runningImages, err := getRunningSoftware()
	if err != nil {
		return nil, err
	}

	installedImages, err := getInstalledSoftware()
	if err != nil {
		return nil, err
	}

	runningFirmwareVersion, err := getRunningFirwmwareVersion(runningImages)
	if err != nil {
		return nil, err
	}

	startupFirmwareVersion, err := getStartupFirmwareVersion(installedImages)
	if err != nil {
		return nil, err
	}

	runningFw, err := getRunningFirmwareName()
	if err != nil {
		return nil, err
	}

	startupFw, err := getStartupFirmwareName()
	if err != nil {
		return nil, err
	}

	return &nmd.NaplesSoftwareVersionInfo{
		RunningFw:           runningFw,
		RunningFwVersion:    runningFirmwareVersion,
		StartupFw:           startupFw,
		StartupFwVersion:    startupFirmwareVersion,
		RunningUbootVersion: runningImages.Uboot.Image.BaseVersion,
		StartupUbootVersion: installedImages.Uboot.Image.BaseVersion,
		InstalledImages:     installedImages,
	}, nil
}

// ReadFruFromJSON reads the fru.json file created at Startup
func ReadFruFromJSON() *nmd.NaplesFru {
	log.Infof("Updating FRU from /tmp/fru.json")
	fruJSON, err := os.Open("/tmp/fru.json")
	if err != nil {
		log.Errorf("Failed to open /tmp/fru.json.")
		return nil
	}
	defer fruJSON.Close()
	var dat map[string]interface{}

	byt, err := ioutil.ReadFile("/tmp/fru.json")
	if err != nil {
		log.Errorf("Failed to read contents of fru.json")
		return nil
	}

	if err := json.Unmarshal(byt, &dat); err != nil {
		log.Errorf("Failed to unmarshal fru.json.")
		return nil
	}

	return &nmd.NaplesFru{
		ManufacturingDate: dat["Manufacturing date"].(string),
		Manufacturer:      dat["Manufacturer"].(string),
		ProductName:       dat["Product Name"].(string),
		SerialNum:         dat["Serial Number"].(string),
		PartNum:           dat["Part Number"].(string),
		BoardId:           dat["Engineering Change level"].(string),
		EngChangeLevel:    dat["Board Id Number"].(string),
		NumMacAddr:        dat["NumMac Address"].(string),
		MacStr:            dat["Mac Address"].(string),
	}
}

// listInterfaces - updates all the interfaces present within Naples
func listInterfaces() []string {
	log.Info("updating interfaces in the SmartNIC object")
	var interfaces []string

	l, err := net.Interfaces()
	if err != nil {
		log.Errorf("Getting interfaces on the system failed.")
		return nil
	}

	for _, f := range l {
		interfaces = append(interfaces, f.Name)
	}
	return interfaces
}

// updateCPUInfo - queries cardconfig and gets the CPU information
func updateCPUInfo() *cmd.CPUInfo {
	log.Info("updating CPU Info in SmartNIC object")
	numProc, err := exec.Command("/bin/bash", "-c", "nproc").Output()
	if err != nil {
		return nil
	}

	cores, _ := strconv.Atoi(string(numProc))

	return &cmd.CPUInfo{
		Speed:    "2.0 Ghz",
		NumCores: int32(cores),
	}
}

// updateOSInfo - updates the OS information in SmartNIC
func updateOSInfo() *cmd.OsInfo {
	log.Info("updating OS Info in SmartNIC object")
	name, err := exec.Command("/bin/bash", "-c", "uname -s").Output()
	if err != nil {
		return nil
	}

	kernelRelease, err := exec.Command("/bin/bash", "-c", "uname -r").Output()
	if err != nil {
		return nil
	}

	return &cmd.OsInfo{
		Name:          strings.TrimSuffix(string(name), "\n"),
		KernelRelease: strings.TrimSuffix(string(kernelRelease), "\n"),
		Processor:     "ARMv7",
	}
}

// updateMemoryInfo - updates the memory information in SmartNIC
func updateMemoryInfo() *cmd.MemInfo {
	log.Info("updating Memory Info in SmartNIC object")
	return &cmd.MemInfo{
		Type: "HBM",
	}
}

// updateStorageInfo - update the storage information in SmartNIC
func updateStorageInfo() *cmd.StorageInfo {
	log.Info("updating Storage Info in SmartNIC object")
	return nil
}

// UpdateNaplesHealth - queries sysmanager and gets the current health of Naples
func UpdateNaplesHealth() []cmd.SmartNICCondition {
	log.Info("updating Health Info in SmartNIC object")

	// TODO : Get status from platform and fill nic Status
	Conditions := []cmd.SmartNICCondition{
		{
			Type:               cmd.SmartNICCondition_HEALTHY.String(),
			Status:             cmd.ConditionStatus_TRUE.String(),
			LastTransitionTime: time.Now().UTC().Format(time.RFC3339),
		},
	}

	return Conditions
}

// UpdateNaplesInfo - updates all the information related to Naples
func UpdateNaplesInfo() *cmd.SmartNICInfo {
	SystemInfo := &cmd.SmartNICInfo{}

	SystemInfo.OsInfo = updateOSInfo()
	SystemInfo.CpuInfo = updateCPUInfo()
	SystemInfo.MemoryInfo = updateMemoryInfo()
	SystemInfo.StorageInfo = updateStorageInfo()

	log.Infof("Successfully update Naples Info in SmartNIC object of NMD.")
	return SystemInfo
}

// updateBiosInfo - queries cardconfig and gets the BIOS information
func updateBiosInfo(naplesVersion *nmd.NaplesRunningSoftware) *cmd.BiosInfo {
	log.Info("updating Bios Info in SmartNIC object")
	return &cmd.BiosInfo{
		Version: naplesVersion.Uboot.Image.BaseVersion,
	}
}

// UpdateNaplesInfoFromConfig - Updates the fields within the SmartNIC object using NaplesConfig
func (n *NMD) UpdateNaplesInfoFromConfig() error {
	log.Info("Updating Smart NIC information.")
	nic, _ := n.GetSmartNIC()

	naplesVersion, err := getRunningSoftware()
	if err != nil {
		log.Errorf("GetVersion failed. Err : %v", err)
		return err
	}

	ver, err := getRunningFirwmwareVersion(naplesVersion)
	if err != nil {
		log.Errorf("Failed to get running software version. : %v", err)
	}

	// TODO : Improve the code below. Too clunky
	if nic != nil {

		// Update smartNIC object
		nic.TypeMeta.Kind = "SmartNIC"
		nic.ObjectMeta.Name = n.config.Status.SmartNicName
		nic.Spec.IPConfig = n.config.Spec.IPConfig
		nic.Spec.Hostname = n.config.Spec.Hostname
		nic.Spec.MgmtMode = n.config.Spec.Mode
		nic.Spec.NetworkMode = n.config.Spec.NetworkMode
		nic.Spec.MgmtVlan = n.config.Spec.MgmtVlan
		nic.Spec.Controllers = n.config.Spec.Controllers

		nic.Status.AdmissionPhase = n.config.Status.AdmissionPhase
		nic.Status.Conditions = nil
		nic.Status.SerialNum = n.config.Status.Fru.SerialNum
		nic.Status.PrimaryMAC = n.config.Status.Fru.MacStr
		nic.Status.IPConfig = n.config.Status.IPConfig
		nic.Status.Interfaces = listInterfaces()
		nic.Status.SmartNICVersion = ver
		nic.Status.SmartNICSku = n.config.Status.Fru.PartNum
	} else {

		// Construct new smartNIC object
		nic = &cmd.SmartNIC{
			TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
			ObjectMeta: api.ObjectMeta{
				Name: n.config.Status.SmartNicName,
			},
			Spec: cmd.SmartNICSpec{
				Hostname:    n.config.Spec.Hostname,
				IPConfig:    n.config.Spec.IPConfig,
				MgmtMode:    n.config.Spec.Mode,
				NetworkMode: n.config.Spec.NetworkMode,
				MgmtVlan:    n.config.Spec.MgmtVlan,
				Controllers: n.config.Spec.Controllers,
			},
			// TODO: get these from platform
			Status: cmd.SmartNICStatus{
				AdmissionPhase:  n.config.Status.AdmissionPhase,
				Conditions:      nil,
				SerialNum:       n.config.Status.Fru.SerialNum,
				PrimaryMAC:      n.config.Status.Fru.MacStr,
				IPConfig:        n.config.Status.IPConfig,
				SystemInfo:      nil,
				Interfaces:      listInterfaces(),
				SmartNICVersion: ver,
				SmartNICSku:     n.config.Status.Fru.PartNum,
			},
		}
	}

	// SystemInfo has static information of Naples which does not change during the lifetime of Naples.
	if nic.Status.SystemInfo == nil {
		nic.Status.SystemInfo = UpdateNaplesInfo()
		nic.Status.SystemInfo.BiosInfo = updateBiosInfo(naplesVersion)
	}

	nic.Status.Conditions = UpdateNaplesHealth()
	n.SetSmartNIC(nic)
	return nil
}
