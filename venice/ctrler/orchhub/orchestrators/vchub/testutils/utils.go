package testutils

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

// TestParams specifies the params for vchub tests
type TestParams struct {
	TestHostName           string
	TestUser               string
	TestPassword           string
	TestDCName             string
	TestDVSName            string
	TestPGNameBase         string
	TestMaxPorts           int32
	TestNumStandalonePorts int32
	TestNumPVLANPair       int
	StartPVLAN             int32
	TestNumPG              int
	TestNumPortsPerPG      int
	TestOrchName           string
}

// ValidateParams validates input test parameters
func ValidateParams(testParams *TestParams) error {
	var err string
	if testParams.TestNumPG != testParams.TestNumPVLANPair {
		err = fmt.Sprintf("TestNumPG: %d, TestNumPVLANPair: %d. TestNumPG must equal to TestNumPVLANPair",
			testParams.TestNumPG, testParams.TestNumPVLANPair)
		return errors.New(err)
	}

	if int32(testParams.TestNumPG*testParams.TestNumPortsPerPG) > (testParams.TestMaxPorts - testParams.TestNumStandalonePorts) {
		err = fmt.Sprintf("Test is trying to create: %d which is larger than the max ports supported: %d",
			testParams.TestNumPG*testParams.TestNumPortsPerPG, testParams.TestMaxPorts-testParams.TestNumStandalonePorts)
		return errors.New(err)
	}

	if (testParams.StartPVLAN + int32(testParams.TestNumPVLANPair*2) + int32(testParams.TestNumPortsPerPG*testParams.TestNumPG)) > 4094 {
		err = fmt.Sprintf("Test is trying to use max vlan id: %d which is greater than 4094",
			testParams.StartPVLAN+int32(testParams.TestNumPVLANPair*2)+int32(testParams.TestNumPortsPerPG*testParams.TestNumPG))
		return errors.New(err)
	}

	if len(testParams.TestOrchName) < 1 || len(testParams.TestOrchName) > 64 {
		return fmt.Errorf("invalid length %v for orchestrator name", len(testParams.TestOrchName))
	}

	return nil
}

// genPVLANPairArray generates testPVLANPair array
func genPVLANPairArray(numPVLANPair int, StartPVLAN int32) []testPVLANPair {
	testPVLANPairArray := make([]testPVLANPair, numPVLANPair*2)
	currentVLAN := StartPVLAN
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
func GenPVLANConfigSpecArray(testParams *TestParams, opType string) []types.VMwareDVSPvlanConfigSpec {
	numPVLANPair := testParams.TestNumPVLANPair
	pvlanConfigSpecArray := make([]types.VMwareDVSPvlanConfigSpec, numPVLANPair*2)
	pvlanMapEntryArray := make([]*types.VMwareDVSPvlanMapEntry, numPVLANPair*2)

	testPVLANPairArray := genPVLANPairArray(numPVLANPair, testParams.StartPVLAN)

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
func GenDVSCreateSpec(testParams *TestParams, pvlanConfigSpecArray []types.VMwareDVSPvlanConfigSpec) *types.DVSCreateSpec {
	var dvsCreateSpec types.DVSCreateSpec
	var dvsConfigSpec types.DVSConfigSpec
	dvsConfigSpec.GetDVSConfigSpec().Name = testParams.TestDVSName
	dvsConfigSpec.GetDVSConfigSpec().NumStandalonePorts = testParams.TestNumStandalonePorts
	dvsConfigSpec.GetDVSConfigSpec().MaxPorts = testParams.TestMaxPorts

	dvsCreateSpec.ConfigSpec = &types.VMwareDVSConfigSpec{
		DVSConfigSpec:   dvsConfigSpec,
		PvlanConfigSpec: pvlanConfigSpecArray,
	}

	return &dvsCreateSpec
}

// GenPGConfigSpec creates PG config with the given pvlan info
func GenPGConfigSpec(pgName string, primaryVlan, secondaryVlan int) types.DVPortgroupConfigSpec {
	spec := types.DVPortgroupConfigSpec{
		Name: pgName,
		Type: string(types.DistributedVirtualPortgroupPortgroupTypeEarlyBinding),
		Policy: &types.VMwareDVSPortgroupPolicy{
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
		},
		DefaultPortConfig: &types.VMwareDVSPortSetting{
			Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
				PvlanId: int32(secondaryVlan),
			},
		},
	}
	return spec
}

// GenPGConfigSpecArray generates port group configuration array
func GenPGConfigSpecArray(testParams *TestParams, pvlanConfigSpecArray []types.VMwareDVSPvlanConfigSpec) []types.DVPortgroupConfigSpec {
	pgConfigSpecArray := make([]types.DVPortgroupConfigSpec, testParams.TestNumPG)

	j := 1
	// We have validated before, the number of port group equals to number of PVLAN pair at testDVSSetup()
	for i := 0; i < testParams.TestNumPG; i++ {
		pgConfigSpecArray[i] = GenPGConfigSpec(fmt.Sprint(testParams.TestPGNameBase, i), int(pvlanConfigSpecArray[j].PvlanEntry.PrimaryVlanId), int(pvlanConfigSpecArray[j].PvlanEntry.SecondaryVlanId))
		j = j + 2
	}

	return pgConfigSpecArray
}

/*
// GenMicroSegVlanMappingPerPG generates map setting of micro segmentation vlan. The key of this map represents port key,
// the value of this map represents the setting
func GenMicroSegVlanMappingPerPG(testParams *vcprobeTestParams, penPG *PenPG, startMicroSegVlanID *int32) (*PenDVSPortSettings, error) {
	mapPortsSetting := make(PenDVSPortSettings, testParams.TestNumPortsPerPG)
	portsSetting := make([]types.VMwareDVSPortSetting, testParams.TestNumPortsPerPG)
	microSegVlanConfigSpecArray := make([]types.VmwareDistributedVirtualSwitchVlanIdSpec, testParams.TestNumPortsPerPG)

	pg, err := penPG.getMoPGRef()
	if err != nil {
		penPG.Log.Errorf("Failed to convert to mo.DistributedVirtualPortgroup, err: %s", err)
		return nil, err
	}

	for i := 0; i < testParams.TestNumPortsPerPG; i++ {
		microSegVlanConfigSpecArray[i].VlanId = *startMicroSegVlanID
		portsSetting[i].Vlan = &microSegVlanConfigSpecArray[i]
		mapPortsSetting[pg.PortKeys[i]] = &portsSetting[i]
		(*startMicroSegVlanID)++
	}

	return &mapPortsSetting, nil
}
*/

// GenPGNamesForComp generates a group of string for verification purpose
func GenPGNamesForComp(testParams *TestParams) *map[string]int {
	mapPGNamesWithIndex := make(map[string]int, testParams.TestNumPG)
	for i := 0; i < testParams.TestNumPG; i++ {
		mapPGNamesWithIndex[fmt.Sprint(testParams.TestPGNameBase, i)] = i
	}

	return &mapPGNamesWithIndex
}
