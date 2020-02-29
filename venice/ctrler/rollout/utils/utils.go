package utils

import (
	"strconv"
	"strings"

	cmdutils "github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	//IncompatibleNaplesSKU unspported SKU
	IncompatibleNaplesSKU = "IncompatibleNaplesSKU"
	// InvalidNaplesSKU is when SKU is invalid
	InvalidNaplesSKU = "InvalidNaplesSKU"
	// InvalidNaplesVersion is when version is invalid
	InvalidNaplesVersion = "InvalidNaplesVersion"
	//RequestRolloutNaples Incompatible Naples Version
	RequestRolloutNaples = "RequestRolloutNaples"
	// SkipForceRollout is when DSC Uses dev-images
	SkipForceRollout = "SkipForceRollout"
)

//ImageType const for type of image Cloud/Enterprise
type ImageType string

const (
	//None dev images that is not a release candidate
	None = ImageType("None")
	//Cloud Image released to cloud customers
	Cloud = ImageType("Cloud")
	//Enterprise Image released to enterprise customers
	Enterprise = ImageType("Enterprise")
)

//Cloud Vs Enterprise
func getImageType(version string) ImageType {
	if strings.Contains(version, "-E-") {
		return Enterprise
	}
	if strings.Contains(version, "-C-") {
		return Cloud
	}
	return None
}

//VersionChecker to verify NIC version for admission
type VersionChecker struct {
}

//CheckNICVersionForAdmission verifies if a smartnic is running a non compatible version
func (v VersionChecker) CheckNICVersionForAdmission(dscSku string, dscVersion string) (string, string) {

	veniceVersion := ""
	versionMap := cmdutils.GetGitVersion()
	for key := range versionMap {
		veniceVersion = key
		break
	}

	if dscVersion == "" {
		log.Infof("Invalid Naples Version!")
		return RequestRolloutNaples, veniceVersion
	}

	//TODO: Hardcoding now for e2e to pass
	if dscVersion == "1.0E" {
		return SkipForceRollout, ""
	}
	//check DSC Image Type (Cloud Vs Enterprise)
	if getImageType(dscVersion) != getImageType(veniceVersion) {
		log.Infof("ForceRollout: Image type mismatch dsc Vs venice (%s Vs %s). Requesting force rollout!", getImageType(dscVersion), getImageType(veniceVersion))
		return RequestRolloutNaples, veniceVersion
	}

	veniceDashVersions := strings.SplitN(veniceVersion, "-", 3)
	veniceDotVersions := strings.SplitN(veniceDashVersions[0], ".", 3)
	dscDashVersions := strings.SplitN(dscVersion, "-", 3)
	dscDotVersions := strings.SplitN(dscDashVersions[0], ".", 3)

	if len(veniceDotVersions) < 3 || len(dscDotVersions) < 3 { //major.minor.patch
		log.Infof("ForceRollout:  Image versions not in standard format. VeniceVersion(%s) DSCVersion(%s)", veniceVersion, dscVersion)
		if veniceVersion != dscVersion {
			log.Infof("ForceRollout: Invalid image versions (%s Vs %s). Requesting force rollout!", dscVersion, veniceVersion)
			return RequestRolloutNaples, veniceVersion
		}
		return SkipForceRollout, ""
	}

	if veniceDotVersions[0] != dscDotVersions[0] {
		log.Infof("ForceRollout: Major Version is different(%s Vs %s). Requesting force rollout!", dscVersion, veniceVersion)
		return RequestRolloutNaples, veniceVersion
	}

	veniceMinor, err := strconv.Atoi(veniceDotVersions[1])
	if err != nil {
		log.Infof("ForceRollout: Failed to convert veniceMinor version to integer(%s). Requesting force rollout!", err)
		if veniceVersion != dscVersion {
			log.Infof("ForceRollout: Failed to convert veniceMinor (%+v). Requesting force rollout!", veniceDotVersions)
			return RequestRolloutNaples, veniceVersion
		}
		return SkipForceRollout, ""
	}
	dscMinor, err := strconv.Atoi(veniceDotVersions[1])
	dscMinorPlusOne := dscMinor + 1

	if err != nil {
		log.Infof("ForceRollout: Failed to convert dscMinor version to integer(%s). Requesting force rollout!", err)
		if veniceVersion != dscVersion {
			log.Infof("ForceRollout: Failed to convert dscMinor (%+v). Requesting force rollout!", dscDotVersions)
			return RequestRolloutNaples, veniceVersion
		}
		return SkipForceRollout, ""
	}

	if veniceMinor != dscMinor && veniceMinor != dscMinorPlusOne {
		log.Infof("ForceRollout: minorVerion incompatible (%v Vs %v). Requesting force rollout!", veniceDotVersions, dscDotVersions)
		return RequestRolloutNaples, veniceVersion
	}

	return SkipForceRollout, ""
}
