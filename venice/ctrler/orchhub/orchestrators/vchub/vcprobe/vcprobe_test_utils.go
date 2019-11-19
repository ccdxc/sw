package vcprobe

import (
	"errors"
	"fmt"

	"github.com/vmware/govmomi/vim25/types"
)

type testPVLANPair struct {
	primaryVlanID   int32
	secondaryVlanID int32
	pvlanType       string
}

type vcprobeTestParams struct {
	testHostName           string
	testUser               string
	testPassword           string
	testDCName             string
	testDVSName            string
	testPGNameBase         string
	testMaxPorts           int32
	testNumStandalonePorts int32
	testNumPVLANPair       int
	startPVLAN             int32
	testNumPG              int
	testNumPortsPerPG      int
}

// TestValidateParams validates input test parameters
func TestValidateParams(testParams *vcprobeTestParams) error {
	var err string
	if testParams.testNumPG != testParams.testNumPVLANPair {
		err = fmt.Sprintf("testNumPG: %d, testNumPVLANPair: %d. testNumPG must equal to testNumPVLANPair",
			testParams.testNumPG, testParams.testNumPVLANPair)
		return errors.New(err)
	}

	if int32(testParams.testNumPG*testParams.testNumPortsPerPG) > (testParams.testMaxPorts - testParams.testNumStandalonePorts) {
		err = fmt.Sprintf("Test is trying to create: %d which is larger than the max ports supported: %d",
			testParams.testNumPG*testParams.testNumPortsPerPG, testParams.testMaxPorts-testParams.testNumStandalonePorts)
		return errors.New(err)
	}

	if (testParams.startPVLAN + int32(testParams.testNumPVLANPair*2) + int32(testParams.testNumPortsPerPG*testParams.testNumPG)) > 4094 {
		err = fmt.Sprintf("Test is trying to use max vlan id: %d which is greater than 4094",
			testParams.startPVLAN+int32(testParams.testNumPVLANPair*2)+int32(testParams.testNumPortsPerPG*testParams.testNumPG))
		return errors.New(err)
	}

	return nil
}

// genPVLANPairArray generates testPVLANPair array
func genPVLANPairArray(numPVLANPair int, startPVLAN int32) []testPVLANPair {
	testPVLANPairArray := make([]testPVLANPair, numPVLANPair*2)
	currentVLAN := startPVLAN
	isPromisc := true

	for i := 0; i < numPVLANPair*2; i++ {
		if isPromisc == true {
			testPVLANPairArray[i].primaryVlanID = currentVLAN
			testPVLANPairArray[i].secondaryVlanID = currentVLAN
			testPVLANPairArray[i].pvlanType = "promiscuous"
		} else {
			testPVLANPairArray[i].primaryVlanID = currentVLAN - 1
			testPVLANPairArray[i].secondaryVlanID = currentVLAN
			testPVLANPairArray[i].pvlanType = "isolated"
		}
		isPromisc = !isPromisc
		currentVLAN++
	}

	return testPVLANPairArray
}

// GenPVLANConfigSpecArray generates Pvlan configuration array
func GenPVLANConfigSpecArray(testParams *vcprobeTestParams, opType string) []types.VMwareDVSPvlanConfigSpec {
	numPVLANPair := testParams.testNumPVLANPair
	pvlanConfigSpecArray := make([]types.VMwareDVSPvlanConfigSpec, numPVLANPair*2)
	pvlanMapEntryArray := make([]*types.VMwareDVSPvlanMapEntry, numPVLANPair*2)

	testPVLANPairArray := genPVLANPairArray(numPVLANPair, testParams.startPVLAN)

	for i := 0; i < numPVLANPair*2; i++ {
		pvlanMapEntryArray[i] = &types.VMwareDVSPvlanMapEntry{
			PrimaryVlanId:   testPVLANPairArray[i].primaryVlanID,
			PvlanType:       testPVLANPairArray[i].pvlanType,
			SecondaryVlanId: testPVLANPairArray[i].secondaryVlanID,
		}

		pvlanConfigSpecArray[i] = types.VMwareDVSPvlanConfigSpec{
			PvlanEntry: *pvlanMapEntryArray[i],
			Operation:  opType,
		}
	}

	return pvlanConfigSpecArray
}

// GenDVSCreateSpec generates DVS create specification
func GenDVSCreateSpec(testParams *vcprobeTestParams, pvlanConfigSpecArray []types.VMwareDVSPvlanConfigSpec) *types.DVSCreateSpec {
	var dvsCreateSpec types.DVSCreateSpec
	var dvsConfigSpec types.DVSConfigSpec
	dvsConfigSpec.GetDVSConfigSpec().Name = testParams.testDVSName
	dvsConfigSpec.GetDVSConfigSpec().NumStandalonePorts = testParams.testNumStandalonePorts
	dvsConfigSpec.GetDVSConfigSpec().MaxPorts = testParams.testMaxPorts

	dvsCreateSpec.ConfigSpec = &types.VMwareDVSConfigSpec{
		DVSConfigSpec:   dvsConfigSpec,
		PvlanConfigSpec: pvlanConfigSpecArray,
	}

	return &dvsCreateSpec
}

// GenPGConfigSpecArray generates port group configuration array
func GenPGConfigSpecArray(testParams *vcprobeTestParams, pvlanConfigSpecArray []types.VMwareDVSPvlanConfigSpec) []types.DVPortgroupConfigSpec {
	pgConfigSpecArray := make([]types.DVPortgroupConfigSpec, testParams.testNumPG)
	pvlanPGConfigSpecArray := make([]types.VmwareDistributedVirtualSwitchPvlanSpec, testParams.testNumPG)
	pgDefaultPortSettingArray := make([]types.VMwareDVSPortSetting, testParams.testNumPG)

	j := 1

	// We have validated before, the number of port group equals to number of PVLAN pair at testDVSSetup()
	for i := 0; i < testParams.testNumPG; i++ {
		pgConfigSpecArray[i].Name = fmt.Sprint(testParams.testPGNameBase, i)
		pgConfigSpecArray[i].Type = string(types.DistributedVirtualPortgroupPortgroupTypeEarlyBinding)
		pgConfigSpecArray[i].NumPorts = int32(testParams.testNumPortsPerPG)
		pgConfigSpecArray[i].Policy = &types.VMwareDVSPortgroupPolicy{
			DVPortgroupPolicy: types.DVPortgroupPolicy{
				BlockOverrideAllowed:               true,
				ShapingOverrideAllowed:             false,
				VendorConfigOverrideAllowed:        false,
				LivePortMovingAllowed:              false,
				PortConfigResetAtDisconnect:        true,
				NetworkResourcePoolOverrideAllowed: types.NewBool(false),
				TrafficFilterOverrideAllowed:       types.NewBool(false),
			},
			VlanOverrideAllowed:           true,
			UplinkTeamingOverrideAllowed:  false,
			SecurityPolicyOverrideAllowed: false,
			IpfixOverrideAllowed:          types.NewBool(false),
		}

		pvlanPGConfigSpecArray[i].PvlanId = pvlanConfigSpecArray[j].PvlanEntry.SecondaryVlanId
		pgDefaultPortSettingArray[i].Vlan = &pvlanPGConfigSpecArray[i]
		pgConfigSpecArray[i].DefaultPortConfig = &pgDefaultPortSettingArray[i]
		j = j + 2
	}

	return pgConfigSpecArray
}

/*
// GenMicroSegVlanMappingPerPG generates map setting of micro segmentation vlan. The key of this map represents port key,
// the value of this map represents the setting
func GenMicroSegVlanMappingPerPG(testParams *vcprobeTestParams, penPG *PenPG, startMicroSegVlanID *int32) (*PenDVSPortSettings, error) {
	mapPortsSetting := make(PenDVSPortSettings, testParams.testNumPortsPerPG)
	portsSetting := make([]types.VMwareDVSPortSetting, testParams.testNumPortsPerPG)
	microSegVlanConfigSpecArray := make([]types.VmwareDistributedVirtualSwitchVlanIdSpec, testParams.testNumPortsPerPG)

	pg, err := penPG.getMoPGRef()
	if err != nil {
		penPG.Log.Errorf("Failed to convert to mo.DistributedVirtualPortgroup, err: %s", err)
		return nil, err
	}

	for i := 0; i < testParams.testNumPortsPerPG; i++ {
		microSegVlanConfigSpecArray[i].VlanId = *startMicroSegVlanID
		portsSetting[i].Vlan = &microSegVlanConfigSpecArray[i]
		mapPortsSetting[pg.PortKeys[i]] = &portsSetting[i]
		(*startMicroSegVlanID)++
	}

	return &mapPortsSetting, nil
}
*/

// GenPGNamesForComp generates a group of string for verification purpose
func GenPGNamesForComp(testParams *vcprobeTestParams) *map[string]int {
	mapPGNamesWithIndex := make(map[string]int, testParams.testNumPG)
	for i := 0; i < testParams.testNumPG; i++ {
		mapPGNamesWithIndex[fmt.Sprint(testParams.testPGNameBase, i)] = i
	}

	return &mapPGNamesWithIndex
}
