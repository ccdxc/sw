package upggosdk

import (
	"encoding/json"
	"errors"
	"io/ioutil"
	"os"
	"strings"

	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	sameImageUpgradeCheckFile = "/data/upgrade_to_same_firmware_allowed"
)

type preUpgImgMeta struct {
	Uboot struct {
		Image struct {
			BuildDate       string `json:"build_date"`
			BuildUser       string `json:"build_user"`
			BaseVersion     string `json:"base_version"`
			SoftwareVersion string `json:"software_version"`
			NicmgrVersion   string `json:"nicmgr_compat_version"`
			KernelVersion   string `json:"kernel_compat_version"`
			PcieVersion     string `json:"pcie_compat_version"`
			DevConfVersion  string `json:"dev_conf_compat_version"`
		} `json:"image"`
	} `json:"uboot"`
	Mainfwa struct {
		KernelFit struct {
			BuildDate       string `json:"build_date"`
			BuildUser       string `json:"build_user"`
			BaseVersion     string `json:"base_version"`
			SoftwareVersion string `json:"software_version"`
			NicmgrVersion   string `json:"nicmgr_compat_version"`
			KernelVersion   string `json:"kernel_compat_version"`
			PcieVersion     string `json:"pcie_compat_version"`
			DevConfVersion  string `json:"dev_conf_compat_version"`
		} `json:"kernel_fit"`
		SystemImage struct {
			BuildDate       string `json:"build_date"`
			BuildUser       string `json:"build_user"`
			BaseVersion     string `json:"base_version"`
			SoftwareVersion string `json:"software_version"`
			NicmgrVersion   string `json:"nicmgr_compat_version"`
			KernelVersion   string `json:"kernel_compat_version"`
			PcieVersion     string `json:"pcie_compat_version"`
			DevConfVersion  string `json:"dev_conf_compat_version"`
		} `json:"system_image"`
	} `json:"mainfwa"`
	Mainfwb struct {
		KernelFit struct {
			BuildDate       string `json:"build_date"`
			BuildUser       string `json:"build_user"`
			BaseVersion     string `json:"base_version"`
			SoftwareVersion string `json:"software_version"`
			NicmgrVersion   string `json:"nicmgr_compat_version"`
			KernelVersion   string `json:"kernel_compat_version"`
			PcieVersion     string `json:"pcie_compat_version"`
			DevConfVersion  string `json:"dev_conf_compat_version"`
		} `json:"kernel_fit"`
		SystemImage struct {
			BuildDate       string `json:"build_date"`
			BuildUser       string `json:"build_user"`
			BaseVersion     string `json:"base_version"`
			SoftwareVersion string `json:"software_version"`
			NicmgrVersion   string `json:"nicmgr_compat_version"`
			KernelVersion   string `json:"kernel_compat_version"`
			PcieVersion     string `json:"pcie_compat_version"`
			DevConfVersion  string `json:"dev_conf_compat_version"`
		} `json:"system_image"`
	} `json:"mainfwb"`
}

type postUpgImgMeta struct {
	MetadataVersion int    `json:"metadata_version"`
	BuildDate       string `json:"build_date"`
	BuildUser       string `json:"build_user"`
	BaseVersion     string `json:"base_version"`
	SoftwareVersion string `json:"software_version"`
	NicmgrVersion   string `json:"nicmgr_compat_version"`
	KernelVersion   string `json:"kernel_compat_version"`
	PcieVersion     string `json:"pcie_compat_version"`
	DevConfVersion  string `json:"dev_conf_compat_version"`
	Firmware        struct {
		Boot struct {
			Type  string `json:"type"`
			Files struct {
				Image struct {
					Name   string `json:"name"`
					Verify struct {
						Algorithm string `json:"algorithm"`
						Hash      string `json:"hash"`
					} `json:"verify"`
				} `json:"image"`
			} `json:"files"`
		} `json:"boot"`
		Main struct {
			Type  string `json:"type"`
			Files struct {
				KernelFit struct {
					Name   string `json:"name"`
					Verify struct {
						Algorithm string `json:"algorithm"`
						Hash      string `json:"hash"`
					} `json:"verify"`
				} `json:"kernel_fit"`
				SystemImage struct {
					Name   string `json:"name"`
					Verify struct {
						Algorithm string `json:"algorithm"`
						Hash      string `json:"hash"`
					} `json:"verify"`
				} `json:"system_image"`
			} `json:"files"`
		} `json:"main"`
	} `json:"firmware"`
}

func getUpgCtxFromImgMeta(upgCtx *UpgCtx, isPreUpg bool) error {
	var file string
	if isPreUpg {
		_, err := os.Stat("/nic/tools/fwupdate")
		if err == nil {
			file = "/data/running_meta.json"
		} else {
			file = "/sw/nic/upgrade_manager/meta/upgrade_metadata.json"
		}
		preUpgJSONFile, err := os.Open(file)
		if err != nil {
			log.Infof("Error %s", err)
			return err
		}
		defer preUpgJSONFile.Close()
		byteValue, err := ioutil.ReadAll(preUpgJSONFile)
		if err != nil {
			log.Infof("Error %s", err)
			return err
		}

		var preImgMeta preUpgImgMeta
		err = json.Unmarshal(byteValue, &preImgMeta)
		if err != nil {
			log.Infof("Unable to unmarshal the json file %s", err)
			return err
		}
		_, err = os.Stat("/nic/tools/fwupdate")
		if err == nil {
			v := &nmd.DistributedServiceCardCmdExecute{
				Executable: "fwupdate",
				Opts:       strings.Join([]string{"-r"}, ""),
			}
			content, er := execCmd(v)
			if er != nil {
				return er
			}
			if strings.Contains(content, "mainfwa") {
				upgCtx.PreUpgMeta.NicmgrVersion = preImgMeta.Mainfwa.SystemImage.NicmgrVersion
				upgCtx.PreUpgMeta.KernelVersion = preImgMeta.Mainfwa.SystemImage.KernelVersion
				upgCtx.PreUpgMeta.PcieVersion = preImgMeta.Mainfwa.SystemImage.PcieVersion
				upgCtx.PreUpgMeta.DevConfVersion = preImgMeta.Mainfwa.SystemImage.DevConfVersion
				upgCtx.PreUpgMeta.BuildDate = preImgMeta.Mainfwa.SystemImage.BuildDate
				upgCtx.PreUpgMeta.BaseVersion = preImgMeta.Mainfwa.SystemImage.BaseVersion
				upgCtx.PreUpgMeta.SoftwareVersion = preImgMeta.Mainfwa.SystemImage.SoftwareVersion
			} else {
				upgCtx.PreUpgMeta.NicmgrVersion = preImgMeta.Mainfwb.SystemImage.NicmgrVersion
				upgCtx.PreUpgMeta.KernelVersion = preImgMeta.Mainfwb.SystemImage.KernelVersion
				upgCtx.PreUpgMeta.PcieVersion = preImgMeta.Mainfwb.SystemImage.PcieVersion
				upgCtx.PreUpgMeta.DevConfVersion = preImgMeta.Mainfwb.SystemImage.DevConfVersion
				upgCtx.PreUpgMeta.BuildDate = preImgMeta.Mainfwb.SystemImage.BuildDate
				upgCtx.PreUpgMeta.BaseVersion = preImgMeta.Mainfwb.SystemImage.BaseVersion
				upgCtx.PreUpgMeta.SoftwareVersion = preImgMeta.Mainfwb.SystemImage.SoftwareVersion
			}
		}
	} else {
		_, err := os.Stat("/nic/tools/fwupdate")
		if err == nil {
			file = "/data/upg_meta.json"
		} else {
			file = "/sw/nic/upgrade_manager/meta/MANIFEST.json"
		}
		postUpgJSONFile, err := os.Open(file)
		if err != nil {
			log.Infof("Error %s", err)
			return err
		}
		defer postUpgJSONFile.Close()
		byteValue, err := ioutil.ReadAll(postUpgJSONFile)
		if err != nil {
			return err
		}

		var postImgMeta postUpgImgMeta
		err = json.Unmarshal(byteValue, &postImgMeta)
		if err != nil {
			log.Infof("Unable to unmarshal the json file %s", err)
			return err
		}
		upgCtx.PostUpgMeta.NicmgrVersion = postImgMeta.NicmgrVersion
		upgCtx.PostUpgMeta.KernelVersion = postImgMeta.KernelVersion
		upgCtx.PostUpgMeta.PcieVersion = postImgMeta.PcieVersion
		upgCtx.PostUpgMeta.DevConfVersion = postImgMeta.DevConfVersion
		upgCtx.PostUpgMeta.BuildDate = postImgMeta.BuildDate
		upgCtx.PostUpgMeta.BaseVersion = postImgMeta.BaseVersion
		upgCtx.PostUpgMeta.SoftwareVersion = postImgMeta.SoftwareVersion
	}
	return nil
}

func getUpgCtxFromMeta(upgCtx *UpgCtx) error {
	if err := createMetaFiles(upgCtx.firmwarePkgName); err != nil {
		return err
	}
	if err := getUpgCtxFromImgMeta(upgCtx, true); err != nil {
		return err
	}
	return getUpgCtxFromImgMeta(upgCtx, false)
}

func isPrePostImageMetaSame(upgCtx *UpgCtx) bool {
	if upgCtx.PostUpgMeta.NicmgrVersion == upgCtx.PreUpgMeta.NicmgrVersion &&
		upgCtx.PostUpgMeta.KernelVersion == upgCtx.PreUpgMeta.KernelVersion &&
		upgCtx.PostUpgMeta.PcieVersion == upgCtx.PreUpgMeta.PcieVersion &&
		upgCtx.PostUpgMeta.BuildDate == upgCtx.PreUpgMeta.BuildDate &&
		upgCtx.PostUpgMeta.BaseVersion == upgCtx.PreUpgMeta.BaseVersion &&
		upgCtx.PostUpgMeta.SoftwareVersion == upgCtx.PreUpgMeta.SoftwareVersion {
		return true
	}
	return false
}

func isUpgradeAllowed(pkgName string) error {
	removeUpgMetaFiles()
	rmUpgTechSupportFiles()
	upgCtx.firmwarePkgName = pkgName
	_, err := pkgVerify(pkgName)
	if err != nil {
		return errors.New("Package verification failed")
	}
	err = getUpgCtxFromMeta(&upgCtx)
	if err != nil {
		return err
	}
	if isPrePostImageMetaSame(&upgCtx) {
		if _, err := os.Stat(sameImageUpgradeCheckFile); os.IsNotExist(err) {
			return errors.New("Upgrade image is same as running image")
		}
	}
	return nil
}

func removeUpgMetaFiles() {
	if _, err := os.Stat("/nic/tools/fwupdate"); err == nil {
		if _, err := os.Stat("/data/upg_meta.json"); err == nil {
			os.Remove("/data/upg_meta.json")
		}
		if _, err = os.Stat("/data/running_meta.json"); err == nil {
			os.Remove("/data/running_meta.json")
		}
	}
}
