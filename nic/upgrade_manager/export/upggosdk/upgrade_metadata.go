package upggosdk

import (
	"encoding/json"
	"io/ioutil"
	"os"

	"github.com/pensando/sw/venice/utils/log"
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
		} `json:"kernel_fit"`
		SystemImage struct {
			BuildDate       string `json:"build_date"`
			BuildUser       string `json:"build_user"`
			BaseVersion     string `json:"base_version"`
			SoftwareVersion string `json:"software_version"`
			NicmgrVersion   string `json:"nicmgr_compat_version"`
			KernelVersion   string `json:"kernel_compat_version"`
			PcieVersion     string `json:"pcie_compat_version"`
		} `json:"system_image"`
	} `json:"mainfwa"`
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
			file = "/tmp/running_meta.json"
		} else {
			file = "/sw/nic/upgrade_manager/meta/upgrade_metadata.json"
		}
		preUpgJSONFile, err := os.Open(file)
		if err != nil {
			log.Infof("Error %s", err)
			return err
		}
		defer preUpgJSONFile.Close()
		byteValue, _ := ioutil.ReadAll(preUpgJSONFile)

		var preImgMeta preUpgImgMeta
		err = json.Unmarshal(byteValue, &preImgMeta)
		if err != nil {
			log.Infof("Unable to unmarshal the json file %s", err)
			return err
		}
		upgCtx.PreUpgMeta.NicmgrVersion = preImgMeta.Uboot.Image.NicmgrVersion
		upgCtx.PreUpgMeta.KernelVersion = preImgMeta.Uboot.Image.KernelVersion
		upgCtx.PreUpgMeta.PcieVersion = preImgMeta.Uboot.Image.PcieVersion
		upgCtx.PreUpgMeta.BuildDate = preImgMeta.Uboot.Image.BuildDate
		upgCtx.PreUpgMeta.BuildUser = preImgMeta.Uboot.Image.BuildUser
		upgCtx.PreUpgMeta.BaseVersion = preImgMeta.Uboot.Image.BaseVersion
		upgCtx.PreUpgMeta.SoftwareVersion = preImgMeta.Uboot.Image.SoftwareVersion
	} else {
		_, err := os.Stat("/nic/tools/fwupdate")
		if err == nil {
			file = "/tmp/upg_meta.json"
		} else {
			file = "/sw/nic/upgrade_manager/meta/MANIFEST.json"
		}
		postUpgJSONFile, err := os.Open(file)
		if err != nil {
			log.Infof("Error %s", err)
			return err
		}
		defer postUpgJSONFile.Close()
		byteValue, _ := ioutil.ReadAll(postUpgJSONFile)

		var postImgMeta postUpgImgMeta
		err = json.Unmarshal(byteValue, &postImgMeta)
		if err != nil {
			log.Infof("Unable to unmarshal the json file %s", err)
			return err
		}
		upgCtx.PostUpgMeta.NicmgrVersion = postImgMeta.NicmgrVersion
		upgCtx.PostUpgMeta.KernelVersion = postImgMeta.KernelVersion
		upgCtx.PostUpgMeta.PcieVersion = postImgMeta.PcieVersion
		upgCtx.PostUpgMeta.BuildDate = postImgMeta.BuildDate
		upgCtx.PostUpgMeta.BuildUser = postImgMeta.BuildUser
		upgCtx.PostUpgMeta.BaseVersion = postImgMeta.BaseVersion
		upgCtx.PostUpgMeta.SoftwareVersion = postImgMeta.SoftwareVersion
	}
	return nil
}

func getUpgCtxFromMeta(upgCtx *UpgCtx) error {
	getUpgCtxFromImgMeta(upgCtx, true)
	getUpgCtxFromImgMeta(upgCtx, false)
	return nil
}

func isPrePostImageMetaSame(upgCtx *UpgCtx) bool {
	if upgCtx.PostUpgMeta.NicmgrVersion == upgCtx.PreUpgMeta.NicmgrVersion &&
		upgCtx.PostUpgMeta.KernelVersion == upgCtx.PreUpgMeta.KernelVersion &&
		upgCtx.PostUpgMeta.PcieVersion == upgCtx.PreUpgMeta.PcieVersion &&
		upgCtx.PostUpgMeta.BuildDate == upgCtx.PreUpgMeta.BuildDate &&
		upgCtx.PostUpgMeta.BuildUser == upgCtx.PreUpgMeta.BuildUser &&
		upgCtx.PostUpgMeta.BaseVersion == upgCtx.PreUpgMeta.BaseVersion &&
		upgCtx.PostUpgMeta.SoftwareVersion == upgCtx.PreUpgMeta.SoftwareVersion {
		return true
	}
	return false
}
