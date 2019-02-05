package utils

import (
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
)

//VersionChecker to verify NIC version for admission
type VersionChecker struct {
}

//CheckNICVersionForAdmission verifies if a smartnic is running a non compatible version
func (v VersionChecker) CheckNICVersionForAdmission(nicSku string, nicVersion string) (string, string) {

	if nicSku == "" {
		return InvalidNaplesSKU, ""
	}

	if nicVersion == "" {
		return InvalidNaplesVersion, ""
	}

	rolloutVersions := cmdutils.GetSupportedNaplesVersions()
	log.Debugf("rolloutVersions %v", rolloutVersions)
	skuVersions := rolloutVersions[nicSku]

	if len(skuVersions) == 0 {
		return IncompatibleNaplesSKU, ""
	}

	flag := false
	for _, ver := range skuVersions {
		if string(ver) == nicVersion {
			flag = true
		}
	}
	if !flag {
		return RequestRolloutNaples, string(skuVersions[0])
	}
	return "", ""
}
