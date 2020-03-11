// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package state

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/nic/agent/nmd/state/ipif"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	conv "github.com/pensando/sw/venice/utils/strconv"
)

const capriStoragePath = "/sys/bus/mmc/devices/mmc0:0001"

// GetRunningFirmwareName returns the currently running firmware name
func GetRunningFirmwareName() string {
	out, err := exec.Command("/bin/bash", "-c", "/nic/tools/fwupdate -r").Output()
	if err != nil {
		// TODO : Remove this. Adding this temporarily to ensure progress.
		// All these APIs can be moved under nmd/platform as it feels a more natural
		// home for this platform dependent code.
		// The platformAgent allows for easy mocking of this API
		// Tracker Jira : PS-1172

		return "mainfwa"
	}

	log.Infof("Got running software version : %v", string(out))
	return strings.TrimSuffix(string(out), "\n")
}

func getStartupFirmwareName() (string, error) {
	out, err := exec.Command("/bin/bash", "-c", "/nic/tools/fwupdate -S").Output()
	if err != nil {
		// TODO : Remove this. Adding this temporarily to ensure progress.
		// All these APIs can be moved under nmd/platform as it feels a more natural
		// home for this platform dependent code.
		// The platformAgent allows for easy mocking of this API
		// Tracker Jira : PS-1172

		return "mainfwa", nil
	}

	log.Infof("Got startup software version : %v", string(out))
	return strings.TrimSuffix(string(out), "\n"), err
}

func getInstalledSoftware() (*nmd.DSCInstalledSoftware, error) {
	out, err := exec.Command("/bin/bash", "-c", "/nic/tools/fwupdate -l").Output()
	if err != nil {
		log.Errorf("Returning Software info default values")
		n := &nmd.DSCInstalledSoftware{
			// TODO : Remove this. Adding this temporarily to ensure progress.
			// All these APIs can be moved under nmd/platform as it feels a more natural
			// home for this platform dependent code.
			// The platformAgent allows for easy mocking of this API
			// Tracker Jira : PS-1172
			Uboot: &nmd.UbootFw{
				Image: &nmd.ImageInfo{
					BaseVersion:     "1.0E",
					SoftwareVersion: "0.0X",
				},
			},
			MainFwA: &nmd.FwVersion{
				SystemImage: &nmd.ImageInfo{
					SoftwareVersion: "1.0E",
				},
			},
			MainFwB: &nmd.FwVersion{
				SystemImage: &nmd.ImageInfo{
					SoftwareVersion: "1.0E",
				},
			},
			Boot0: &nmd.UbootFw{
				Image: &nmd.ImageInfo{
					BaseVersion:     "1.0E",
					SoftwareVersion: "0.0X",
				},
			},
		}
		return n, nil
	}

	var naplesVersion nmd.DSCInstalledSoftware
	json.Unmarshal([]byte(out), &naplesVersion)

	log.Infof("Got Naples Version %v", naplesVersion)

	return &naplesVersion, nil
}

// GetRunningFirmware gets details about the currently running naples firmware
func GetRunningFirmware() *nmd.DSCRunningSoftware {
	out, err := exec.Command("/bin/bash", "-c", "/nic/tools/fwupdate -L").Output()
	if err != nil {
		log.Errorf("Returning Software info default values")
		n := &nmd.DSCRunningSoftware{
			// TODO : Remove this. Adding this temporarily to ensure progress.
			// All these APIs can be moved under nmd/platform as it feels a more natural
			// home for this platform dependent code.
			// The platformAgent allows for easy mocking of this API
			// Tracker Jira : PS-1172
			Uboot: &nmd.UbootFw{
				Image: &nmd.ImageInfo{
					BaseVersion:     "1.0E",
					SoftwareVersion: "0.0X",
				},
			},
			MainFwA: &nmd.FwVersion{
				SystemImage: &nmd.ImageInfo{
					SoftwareVersion: "1.0E",
				},
			},
			Boot0: &nmd.UbootFw{
				Image: &nmd.ImageInfo{
					BaseVersion:     "1.0E",
					SoftwareVersion: "0.0X",
				},
			},
		}
		return n
	}

	var naplesVersion nmd.DSCRunningSoftware
	json.Unmarshal([]byte(out), &naplesVersion)

	log.Infof("Got Naples Running Version %v", naplesVersion)

	return &naplesVersion
}

// GetRunningFirmwareVersion gets the version of the current firmware
func GetRunningFirmwareVersion(currentFw string, naplesVersion *nmd.DSCRunningSoftware) string {
	if naplesVersion == nil {
		log.Errorf("Naples version passed is nil.")
		return ""
	}

	switch currentFw {
	case "mainfwa":
		return naplesVersion.MainFwA.SystemImage.SoftwareVersion
	case "mainfwb":
		return naplesVersion.MainFwB.SystemImage.SoftwareVersion
	default:
		return ""
	}
}

func getStartupFirmwareVersion(naplesVersion *nmd.DSCInstalledSoftware) (string, error) {
	startupFw, err := getStartupFirmwareName()
	if err != nil || len(startupFw) == 0 || naplesVersion == nil {
		return "", err
	}

	switch startupFw {
	case "mainfwa":
		return naplesVersion.MainFwA.SystemImage.SoftwareVersion, nil
	case "mainfwb":
		return naplesVersion.MainFwB.SystemImage.SoftwareVersion, nil
	default:
		return "", errors.New("unknown firmware version")
	}
}

// GetNaplesSoftwareInfo gets the software versions which are running and will run at startup
func (n *NMD) GetNaplesSoftwareInfo() (*nmd.DSCSoftwareVersionInfo, error) {
	installedImages, err := getInstalledSoftware()
	if err != nil {
		return nil, err
	}

	startupFirmwareVersion, err := getStartupFirmwareVersion(installedImages)
	if err != nil {
		return nil, err
	}

	startupFw, err := getStartupFirmwareName()
	if err != nil {
		return nil, err
	}

	if n.RunningFirmware.Uboot != nil && installedImages.Uboot != nil {
		return &nmd.DSCSoftwareVersionInfo{
			RunningFw:           n.RunningFirmwareName,
			RunningFwVersion:    n.RunningFirmwareVersion,
			StartupFw:           startupFw,
			StartupFwVersion:    startupFirmwareVersion,
			RunningUbootVersion: n.RunningFirmware.Uboot.Image.SoftwareVersion,
			StartupUbootVersion: installedImages.Uboot.Image.SoftwareVersion,
			InstalledImages:     installedImages,
		}, nil
	} else if n.RunningFirmware.Boot0 != nil && installedImages.Boot0 != nil {
		return &nmd.DSCSoftwareVersionInfo{
			RunningFw:           n.RunningFirmwareName,
			RunningFwVersion:    n.RunningFirmwareVersion,
			StartupFw:           startupFw,
			StartupFwVersion:    startupFirmwareVersion,
			RunningUbootVersion: n.RunningFirmware.Boot0.Image.SoftwareVersion,
			StartupUbootVersion: installedImages.Boot0.Image.SoftwareVersion,
			InstalledImages:     installedImages,
		}, nil
	}

	return &nmd.DSCSoftwareVersionInfo{
		RunningFw:        n.RunningFirmwareName,
		RunningFwVersion: n.RunningFirmwareVersion,
		StartupFw:        startupFw,
		StartupFwVersion: startupFirmwareVersion,
		InstalledImages:  installedImages,
	}, nil
}

// ReadFruFromJSON reads the fru.json file created at Startup
func ReadFruFromJSON() *nmd.DistributedServiceCardFru {
	log.Infof("Updating FRU from /tmp/fru.json")
	fruJSON, err := readFruWithRetries()
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

	var macStr string
	if macStr, err = conv.ParseMacAddr(dat["mac-address"].(string)); err != nil {
		log.Errorf("Failed to parse mac address. Err : %v", err)
		return nil
	}

	return &nmd.DistributedServiceCardFru{
		ManufacturingDate: dat["manufacturing-date"].(string),
		Manufacturer:      dat["manufacturer"].(string),
		ProductName:       dat["product-name"].(string),
		SerialNum:         dat["serial-number"].(string),
		PartNum:           dat["part-number"].(string),
		BoardId:           dat["board-id"].(string),
		EngChangeLevel:    dat["engineering-change-level"].(string),
		NumMacAddr:        dat["num-mac-address"].(string),
		MacStr:            macStr,
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
		log.Errorf("Couldn't get number of processors. Err : %v", err)
		return nil
	}

	cpuMHz, err := exec.Command("/bin/bash", "-c", "cat /proc/cpuinfo | grep BogoMIPS | head -1 | awk -F: '{print $2}'").Output()
	if err != nil {
		log.Errorf("Couldn't get cpu speed. Err : %v", err)
		return nil
	}

	cores, _ := strconv.Atoi(string(numProc))
	cpuGHz, err := strconv.ParseFloat(strings.TrimSpace(string(cpuMHz)), 64)

	if err != nil {
		log.Errorf("Failed to parse cpu speed. Err : %v", err)
		return nil
	}

	return &cmd.CPUInfo{
		Speed:    fmt.Sprintf("%.2f GHz", cpuGHz/1000.0),
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
		Processor:     "ARMv8",
	}
}

// updateMemoryInfo - updates the memory information in SmartNIC
func updateMemoryInfo() *cmd.MemInfo {
	log.Info("updating Memory Info in SmartNIC object")
	return &cmd.MemInfo{
		Type: "HBM",
	}
}

func getLifeUsed(hexStr string) int32 {
	d, err := strconv.ParseInt(hexStr, 0, 32)
	if err != nil {
		log.Errorf("Not a Number. Err: %v", err)
		return -1
	}

	return int32(d * 10)
}

// updateStorageInfo - update the storage information in SmartNIC
// Currently this is heavily focused on eMMC for Capri. Changes will have to be made as we move forward.
func updateStorageInfo() *cmd.StorageInfo {
	storageSizePath := capriStoragePath + "/block/mmcblk0/size"
	size, err := ioutil.ReadFile(storageSizePath)
	if err != nil {
		return nil
	}

	log.Infof("Got size : %s", string(size))
	sizeBytes := strings.TrimSuffix(string(size), "\n")
	if err != nil {
		return nil
	}

	wearPath := capriStoragePath + "/life_time"
	wearBytes, err := ioutil.ReadFile(wearPath)
	if err != nil {
		return nil
	}
	wearStr := strings.TrimSuffix(string(wearBytes), "\n")
	wear := strings.Split(wearStr, " ")
	wearTypeA := wear[0]
	wearTypeB := wear[1]

	serialPath := capriStoragePath + "/serial"
	serialNumber, err := ioutil.ReadFile(serialPath)
	if err != nil {
		return nil
	}
	serialNumberStr := strings.TrimSuffix(string(serialNumber), "\n")

	return &cmd.StorageInfo{
		Devices: []cmd.StorageDeviceInfo{
			{
				SerialNumber:         serialNumberStr,
				Type:                 "MMC",
				Capacity:             fmt.Sprintf("%s Bytes", sizeBytes),
				TypeAPercentLifeUsed: getLifeUsed(wearTypeA),
				TypeBPercentLifeUsed: getLifeUsed(wearTypeB),
			},
		},
	}
}

// UpdateNaplesHealth - queries sysmanager and gets the current health of Naples
func (n *NMD) UpdateNaplesHealth() []cmd.DSCCondition {
	log.Info("updating Health Info in SmartNIC object")
	health := cmd.DSCCondition_HEALTHY.String()
	status := cmd.ConditionStatus_TRUE.String()
	reason := ""

	if n.Pipeline != nil && n.Pipeline.GetPipelineType() == globals.NaplesPipelineIris {
		status, reason = n.Pipeline.GetSysmgrSystemStatus()
	}
	Conditions := []cmd.DSCCondition{
		{
			Type:               health,
			Status:             status,
			Reason:             reason,
			LastTransitionTime: time.Now().UTC().Format(time.RFC3339),
		},
	}

	//if n.rebootNeeded {
	//	needsRebootCondition := cmd.DSCCondition{
	//		Type:               cmd.DSCCondition_REBOOT_NEEDED.String(),
	//		Status:             cmd.ConditionStatus_TRUE.String(),
	//		LastTransitionTime: time.Now().UTC().Format(time.RFC3339),
	//	}
	//
	//	Conditions = append(Conditions, needsRebootCondition)
	//}

	return Conditions
}

// UpdateNaplesInfo - updates all the information related to Naples
func UpdateNaplesInfo() *cmd.DSCInfo {
	SystemInfo := &cmd.DSCInfo{}

	SystemInfo.OsInfo = updateOSInfo()
	SystemInfo.CpuInfo = updateCPUInfo()
	SystemInfo.MemoryInfo = updateMemoryInfo()
	SystemInfo.StorageInfo = updateStorageInfo()

	log.Infof("Successfully update Naples Info in SmartNIC object of NMD.")
	return SystemInfo
}

// updateBiosInfo - queries cardconfig and gets the BIOS information
func updateBiosInfo(naplesVersion *nmd.DSCRunningSoftware) *cmd.BiosInfo {
	log.Info("updating Bios Info in SmartNIC object")
	if naplesVersion.Boot0 != nil {
		return &cmd.BiosInfo{
			Version: naplesVersion.Boot0.Image.SoftwareVersion,
		}
	}

	return &cmd.BiosInfo{
		Version: naplesVersion.Uboot.Image.SoftwareVersion,
	}
}

// UpdateNaplesInfoFromConfig - Updates the fields within the SmartNIC object using NaplesConfig
func (n *NMD) UpdateNaplesInfoFromConfig() error {
	log.Info("Updating Smart NIC information.")
	nic, _ := n.GetSmartNIC()

	// TODO : Improve the code below. Too clunky
	if nic != nil {

		// Update smartNIC object
		nic.TypeMeta.Kind = "DistributedServiceCard"
		nic.ObjectMeta.Name = n.config.Status.DSCName
		nic.Spec.IPConfig = n.config.Spec.IPConfig
		nic.Spec.ID = n.config.Spec.ID
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
		nic.Status.DSCVersion = n.RunningFirmwareVersion
		nic.Status.DSCSku = n.config.Status.Fru.PartNum
	} else {

		// Construct new smartNIC object
		nic = &cmd.DistributedServiceCard{
			TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
			ObjectMeta: api.ObjectMeta{
				Name: n.config.Status.DSCName,
			},
			Spec: cmd.DistributedServiceCardSpec{
				ID:          n.config.Spec.ID,
				IPConfig:    n.config.Spec.IPConfig,
				MgmtMode:    n.config.Spec.Mode,
				NetworkMode: n.config.Spec.NetworkMode,
				MgmtVlan:    n.config.Spec.MgmtVlan,
				Controllers: n.config.Spec.Controllers,
			},
			// TODO: get these from platform
			Status: cmd.DistributedServiceCardStatus{
				AdmissionPhase: n.config.Status.AdmissionPhase,
				Conditions:     nil,
				SerialNum:      n.config.Status.Fru.SerialNum,
				PrimaryMAC:     n.config.Status.Fru.MacStr,
				IPConfig:       n.config.Status.IPConfig,
				SystemInfo:     nil,
				Interfaces:     listInterfaces(),
				DSCVersion:     n.RunningFirmwareVersion,
				DSCSku:         n.config.Status.Fru.PartNum,
			},
		}
	}

	// SystemInfo has static information of Naples which does not change during the lifetime of Naples.
	if nic.Status.SystemInfo == nil {
		nic.Status.SystemInfo = UpdateNaplesInfo()
		nic.Status.SystemInfo.BiosInfo = updateBiosInfo(n.RunningFirmware)
	}

	nic.Status.Conditions = n.UpdateNaplesHealth()
	n.SetSmartNIC(nic)
	return nil
}

func readFruWithRetries() (fd *os.File, err error) {
	fruParsed := make(chan bool, 1)
	ticker := time.NewTicker(time.Second * 10)
	timeout := time.After(time.Minute * 2)
	fd, err = os.Open(ipif.FRUFilePath)
	if err == nil {
		log.Infof("FRU JSON Parsed from: %v", ipif.FRUFilePath)
		return
	}

	for {
		select {
		case <-ticker.C:
			fd, err = os.Open(ipif.FRUFilePath)
			if err != nil {
				fruParsed <- true
			}
		case <-fruParsed:
			log.Infof("FRU JSON Parsed from: %v", ipif.FRUFilePath)
			return
		case <-timeout:
			log.Errorf("Failed to parse FRU JSON from: %v. | Err: %v", ipif.FRUFilePath, err)
			return nil, err
		}
	}
}
