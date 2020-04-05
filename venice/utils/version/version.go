package version

import (
	"strconv"
	"strings"

	"github.com/pensando/sw/venice/utils/log"
)

// Version is expected to be of the format major.minor.patch-ImageType-buildNum

// exported variables
var (
	Version = "0.1"
)

//RolloutType const for type of rollout Upgrade/Downgrade
type RolloutType string

const (
	//Patch Rollout is using devImage
	Patch = RolloutType("Patch")
	//Upgrade Rollout is upgrade
	Upgrade = RolloutType("Upgrade")
	//Downgrade Rollout is Downgrade
	Downgrade = RolloutType("Downgrade")
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

//ValidateVersionString Validate if the given string is in valid format
func ValidateVersionString(version string) bool {
	if version == "" {
		log.Errorf("Image version is empty")
		return false
	}
	dashVersions := strings.SplitN(version, "-", 3)
	dotVersions := strings.SplitN(dashVersions[0], ".", 3)
	if len(dotVersions) < 3 { //major.minor.patch
		log.Errorf("Image version not in standard format. version(%s)", version)
		return false
	}
	return true
}

//GetImageType Cloud Vs Enterprise
func GetImageType(version string) ImageType {
	if ValidateVersionString(version) == false {
		log.Errorf("Invalid image version")
		return None
	}
	if strings.Contains(version, "-E-") {
		return Enterprise
	}
	if strings.Contains(version, "-C-") {
		return Cloud
	}
	return None
}

//GetMajorVersion returns the Major field from version string
func GetMajorVersion(version string) int {
	if version == "" {
		log.Errorf("Image version is empty")
		return 0
	}
	dashVersions := strings.SplitN(version, "-", 3)
	dotVersions := strings.SplitN(dashVersions[0], ".", 3)
	if len(dotVersions) < 3 { //major.minor.patch
		log.Errorf("Image version not in standard format. version(%s)", version)
		return 0
	}
	majorVersion, err := strconv.Atoi(dotVersions[0])
	if err != nil {
		log.Errorf("Couldnt determine major version %+v", err)
		return 0
	}
	return majorVersion
}

//GetMinorVersion returns the Minor field from the version string
func GetMinorVersion(version string) int {
	if version == "" {
		log.Errorf("Image version is empty")
		return 0
	}
	dashVersions := strings.SplitN(version, "-", 3)
	dotVersions := strings.SplitN(dashVersions[0], ".", 3)
	if len(dotVersions) < 3 { //major.minor.patch
		log.Errorf("Image version not in standard format. version(%s)", version)
		return 0
	}
	minorVersion, err := strconv.Atoi(dotVersions[1])
	if err != nil {
		log.Errorf("Couldnt determine minor version %+v", err)
		return 0
	}
	return minorVersion
}

//GetRolloutType the type of rollout
func GetRolloutType(srcVersion, destVersion string) RolloutType {

	log.Infof("GetRolloutType: %s %s", srcVersion, destVersion)
	srcMajorVersion := GetMajorVersion(srcVersion)
	srcMinorVersion := GetMinorVersion(srcVersion)
	if srcMajorVersion == 0 || srcMinorVersion == 0 {
		log.Errorf("Invalid Version: %s", srcVersion)
		return Patch
	}
	destMajorVersion := GetMajorVersion(destVersion)
	destMinorVersion := GetMinorVersion(destVersion)
	if destMajorVersion == 0 || destMinorVersion == 0 {
		log.Errorf("Invalid Version: %s", srcVersion)
		return Patch
	}

	if destMajorVersion < srcMajorVersion || (destMajorVersion == srcMajorVersion && destMinorVersion < srcMinorVersion) {
		return Downgrade
	}
	return Upgrade
}
