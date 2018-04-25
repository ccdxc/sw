// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"os"
	"sync"
	"testing"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// Test params
var (
	// Bolt DB file
	emDBPath = "/tmp/nmd.db"

	// NIC to be admitted
	nicKey1 = "2222.2222.2222"

	// NIC to be in pending state
	nicKey2 = "4444.4444.4444"

	// NIC to be rejected
	nicKey3 = "6666.6666.6666"
)

// Mock platform agent
type mockAgent struct {
	sync.Mutex
	nicDB map[string]*cmd.SmartNIC
}

// RegisterNMD registers NMD with PlatformAgent
func (m *mockAgent) RegisterNMD(nmd NmdPlatformAPI) error {
	return nil
}

// CreateSmartNIC creates a smart NIC object
func (m *mockAgent) CreateSmartNIC(nic *cmd.SmartNIC) error {
	m.Lock()
	defer m.Unlock()

	key := objectKey(nic.ObjectMeta)
	m.nicDB[key] = nic
	return nil
}

// UpdateSmartNIC updates a smart NIC object
func (m *mockAgent) UpdateSmartNIC(nic *cmd.SmartNIC) error {
	m.Lock()
	defer m.Unlock()

	key := objectKey(nic.ObjectMeta)
	m.nicDB[key] = nic
	return nil
}

// DeleteSmartNIC deletes a smart NIC object
func (m *mockAgent) DeleteSmartNIC(nic *cmd.SmartNIC) error {
	m.Lock()
	defer m.Unlock()

	key := objectKey(nic.ObjectMeta)
	delete(m.nicDB, key)
	return nil
}

type mockCtrler struct {
	sync.Mutex
	nicDB map[string]*cmd.SmartNIC
}

func (m *mockCtrler) RegisterSmartNICReq(nic *cmd.SmartNIC) (grpc.RegisterNICResponse, error) {
	m.Lock()
	defer m.Unlock()

	key := objectKey(nic.ObjectMeta)
	m.nicDB[key] = nic
	if nic.Name == nicKey1 {
		return grpc.RegisterNICResponse{Phase: cmd.SmartNICSpec_ADMITTED.String()}, nil
	}
	if nic.Name == nicKey2 {
		return grpc.RegisterNICResponse{Phase: cmd.SmartNICSpec_PENDING.String()}, nil
	}
	return grpc.RegisterNICResponse{Phase: cmd.SmartNICSpec_REJECTED.String(), Reason: string("Invalid Cert")}, nil
}

func (m *mockCtrler) UpdateSmartNICReq(nic *cmd.SmartNIC) (*cmd.SmartNIC, error) {
	m.Lock()
	defer m.Unlock()

	key := objectKey(nic.ObjectMeta)
	m.nicDB[key] = nic
	return nic, nil
}

// createNMD creates a NMD server
func createNMD(t *testing.T, dbPath, mode, nodeID string) (*NMD, *mockAgent, *mockCtrler) {
	ag := &mockAgent{
		nicDB: make(map[string]*cmd.SmartNIC),
	}
	ct := &mockCtrler{
		nicDB: make(map[string]*cmd.SmartNIC),
	}

	// create new NMD
	nm, err := NewNMD(ag, dbPath, nodeID, "localhost:0", mode)
	if err != nil {
		log.Errorf("Error creating NMD. Err: %v", err)
		return nil, nil, nil
	}
	Assert(t, nm.GetAgentID() == nodeID, "Failed to match nodeUUID", nm)

	// fake CMD intf
	nm.RegisterCMD(ct)

	return nm, ag, ct
}

// stopNMD stops NMD server and optionally deleted emDB file
func stopNMD(t *testing.T, nm *NMD, cleanupDB bool) {

	if nm != nil {
		nm.Stop()
	}

	if cleanupDB {
		err := os.Remove(emDBPath)
		if err != nil {
			t.Fatalf("Error deleting emDB file, err: %v", err)
		}
	}
}

func TestSmartNICCreateUpdateDelete(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _ := createNMD(t, "", "classic", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, false)

	// NIC message
	nic := cmd.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   nicKey1,
		},
	}

	// create smartNIC
	err := nm.CreateSmartNIC(&nic)
	AssertOk(t, err, "Error creating nic")
	n, err := nm.GetSmartNIC()
	AssertOk(t, err, "NIC was not found in DB")
	Assert(t, n.ObjectMeta.Name == nicKey1, "NIC name did not match", n)

	// update smartNIC
	nic.Status = cmd.SmartNICStatus{
		Conditions: []*cmd.SmartNICCondition{
			{
				Type:   cmd.SmartNICCondition_HEALTHY.String(),
				Status: cmd.ConditionStatus_TRUE.String(),
			},
		},
	}
	err = nm.UpdateSmartNIC(&nic)
	AssertOk(t, err, "Error updating nic")
	n, err = nm.GetSmartNIC()
	AssertOk(t, err, "NIC was not found in DB")
	Assert(t, n.Status.Conditions[0].Status == cmd.ConditionStatus_TRUE.String() && nic.ObjectMeta.Name == "2222.2222.2222", "NIC status did not match", n)

	// delete smartNIC
	err = nm.DeleteSmartNIC(&nic)
	AssertOk(t, err, "Error deleting nic")
	nicObj, err := nm.GetSmartNIC()
	Assert(t, (nicObj == nil) && (err == nil), "NIC was still found in database after deleting", nm)
}

func TestCtrlrSmartNICRegisterAndUpdate(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _ := createNMD(t, emDBPath, "classic", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, true)

	// NIC message
	nic := cmd.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name: nicKey1,
		},
	}

	// create smartNIC
	resp, err := nm.RegisterSmartNICReq(&nic)
	AssertOk(t, err, "Error registering nic")
	Assert(t, resp.Phase == cmd.SmartNICSpec_ADMITTED.String(), "NIC is not admitted", nic)

	// update smartNIC
	nic.Status = cmd.SmartNICStatus{
		Conditions: []*cmd.SmartNICCondition{
			{
				Type:   cmd.SmartNICCondition_HEALTHY.String(),
				Status: cmd.ConditionStatus_TRUE.String(),
			},
		},
	}
	n, err := nm.UpdateSmartNICReq(&nic)
	AssertOk(t, err, "Error updating nic")
	Assert(t, n.Status.Conditions[0].Status == cmd.ConditionStatus_TRUE.String() &&
		nic.ObjectMeta.Name == "2222.2222.2222", "NIC status did not match", n)
}

func TestNaplesDefaultClassicMode(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _ := createNMD(t, emDBPath, "classic", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, true)

	f1 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		if cfg.Spec.Mode == nmd.NaplesMode_CLASSIC_MODE && nm.GetListenURL() != "" &&
			nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
			return true, nil
		}
		return false, nil
	}
	AssertEventually(t, f1, "Failed to verify mode is in Classic")

	var naplesCfg nmd.Naples

	f2 := func() (bool, interface{}) {
		err := netutils.HTTPGet(nm.GetNMDUrl()+"/", &naplesCfg)
		if err != nil {
			log.Errorf("Failed to get naples config, err:%+v", err)
			return false, nil
		}

		if naplesCfg.Spec.Mode != nmd.NaplesMode_CLASSIC_MODE {
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f2, "Failed to get the default naples config")

	// Negative testcase, start another restserver and it should fail
	err := nm.StartRestServer()
	Assert(t, err != nil, "Starting redundant REST server should have failed")
}

func TestNaplesRestartClassicMode(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _ := createNMD(t, emDBPath, "classic", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	f1 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		if cfg.Spec.Mode != nmd.NaplesMode_CLASSIC_MODE && nm.GetListenURL() != "" &&
			nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
			return true, nil
		}
		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify mode is in Classic")

	// stop NMD, don't clean up DB
	stopNMD(t, nm, false)

	// start/create NMD again, simulating restart
	nm, _, _ = createNMD(t, emDBPath, "classic", nicKey1)
	defer stopNMD(t, nm, true)

	Assert(t, (nm != nil), "Failed to create nmd", nm)
	AssertEventually(t, f1, "Failed to verify Classic mode, after Restart")
}

func TestNaplesManagedMode(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// Start NMD in managed mode
	nm, _, _ := createNMD(t, emDBPath, "managed", nicKey1)
	defer stopNMD(t, nm, true)
	Assert(t, (nm != nil), "Failed to start NMD in managed mode", nm)

	f1 := func() (bool, interface{}) {

		// Verify mode
		cfg := nm.GetNaplesConfig()
		log.Infof("NaplesConfig: %v", cfg)
		if cfg.Spec.Mode != nmd.NaplesMode_MANAGED_MODE {
			log.Errorf("Mode is not managed")
			return false, nil
		}

		// Verify nic state
		nic, err := nm.GetSmartNIC()
		if nic == nil || err != nil {
			log.Errorf("NIC not found in nicDB, mac:%s", nicKey1)
			return false, nil
		}

		// Verify NIC admission
		if nic.Spec.Phase != cmd.SmartNICSpec_ADMITTED.String() {
			log.Errorf("NIC is not admitted")
			return false, nil
		}

		// Verify update task
		if nm.GetUpdStatus() == false {
			log.Errorf("Update NIC is not in progress")
			return false, nil
		}

		// Verify rest server status
		if nm.GetRestServerStatus() == true {
			log.Errorf("REST server is still up")
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify Managed Mode", string("10ms"), string("30s"))
}

// TestNaplesModeTransitions tests the mode transition
// Classic -> Managed -> Classic
func TestNaplesModeTransitions(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _ := createNMD(t, emDBPath, "classic", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, true)

	f1 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		if cfg.Spec.Mode != nmd.NaplesMode_CLASSIC_MODE && nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
			return true, nil
		}
		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify mode is in Classic")

	// Switch to Managed mode
	naplesCfg := &nmd.Naples{
		ObjectMeta: api.ObjectMeta{Name: "NaplesConfig"},
		TypeMeta:   api.TypeMeta{Kind: "Naples"},
		Spec: nmd.NaplesSpec{
			Mode:           nmd.NaplesMode_MANAGED_MODE,
			PrimaryMac:     nicKey1,
			ClusterAddress: []string{"192.168.30.10:9002"},
			NodeName:       "esx-001",
			MgmtIp:         "10.10.10.10",
		},
	}

	log.Infof("Naples config: %+v", naplesCfg)

	var err error
	var resp NaplesConfigResp

	f2 := func() (bool, interface{}) {
		err = netutils.HTTPPost(nm.GetNMDUrl(), naplesCfg, &resp)
		if err != nil {
			log.Errorf("Failed to post naples config, err:%+v resp:%+v", err, resp)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f2, "Failed to post the naples config")

	f3 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		log.Infof("NaplesConfig: %v", cfg)
		if cfg.Spec.Mode != nmd.NaplesMode_MANAGED_MODE {
			log.Errorf("Failed to switch to managed mode")
			return false, nil
		}

		nic, err := nm.GetSmartNIC()
		if nic == nil || err != nil {
			log.Errorf("NIC not found in nicDB, mac:%s", nicKey1)
			return false, nil
		}

		if nic.Spec.Phase != cmd.SmartNICSpec_ADMITTED.String() {
			log.Errorf("NIC is not admitted")
			return false, nil
		}

		if nm.GetUpdStatus() == false {
			log.Errorf("Update NIC is not in progress")
			return false, nil
		}

		if nm.GetRestServerStatus() == true {
			log.Errorf("REST server is still up")
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f3, "Failed to verify mode is in Managed Mode", string("10ms"), string("30s"))

	// Switch to classic mode
	naplesCfg.Spec.Mode = nmd.NaplesMode_CLASSIC_MODE
	AssertEventually(t, f2, "Failed to post the naples config")

	// Verify it is in classic mode
	AssertEventually(t, f1, "Failed to verify mode is in Classic")
}

func TestNaplesManagedModeManualApproval(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _ := createNMD(t, emDBPath, "classic", nicKey2)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, true)

	var err error
	var resp NaplesConfigResp

	// Switch to Managed mode
	naplesCfg := &nmd.Naples{
		ObjectMeta: api.ObjectMeta{Name: "NaplesConfig"},
		TypeMeta:   api.TypeMeta{Kind: "Naples"},
		Spec: nmd.NaplesSpec{
			Mode:       nmd.NaplesMode_MANAGED_MODE,
			PrimaryMac: nicKey2,
			NodeName:   "esx-001",
		},
	}

	f1 := func() (bool, interface{}) {
		err = netutils.HTTPPost(nm.GetNMDUrl(), naplesCfg, &resp)
		if err != nil {
			log.Errorf("Failed to post naples config, err:%+v resp:%+v", err, resp)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f1, "Failed to post the naples config")

	f2 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		if cfg.Spec.Mode != nmd.NaplesMode_MANAGED_MODE {
			log.Errorf("Failed to switch to managed mode")
			return false, nil
		}

		nic, err := nm.GetSmartNIC()
		if nic == nil || err != nil {
			log.Errorf("NIC not found in nicDB")
			return false, nil
		}

		if nic.Spec.Phase != cmd.SmartNICSpec_PENDING.String() {
			log.Errorf("NIC is not admitted")
			return false, nil
		}

		if nm.GetRegStatus() == false {
			log.Errorf("Registration is not in progress")
			return false, nil
		}

		if nm.GetRestServerStatus() == true {
			log.Errorf("REST server is still up")
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify PendingNIC in Managed Mode", string("10ms"), string("30s"))
}

func TestNaplesManagedModeInvalidNIC(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _ := createNMD(t, emDBPath, "classic", nicKey3)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, true)

	var err error
	var resp NaplesConfigResp

	// Switch to Managed mode
	naplesCfg := &nmd.Naples{
		ObjectMeta: api.ObjectMeta{Name: "NaplesConfig"},
		TypeMeta:   api.TypeMeta{Kind: "Naples"},
		Spec: nmd.NaplesSpec{
			Mode:       nmd.NaplesMode_MANAGED_MODE,
			PrimaryMac: nicKey3,
			NodeName:   "esx-001",
		},
	}

	f1 := func() (bool, interface{}) {
		err = netutils.HTTPPost(nm.GetNMDUrl(), naplesCfg, &resp)
		if err != nil {
			log.Errorf("Failed to post naples config, err:%+v resp:%+v", err, resp)
		}
		return true, nil
	}
	AssertEventually(t, f1, "Failed to post the naples config")

	f2 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		log.Infof("CFG: %+v err: %+v", cfg.Spec.Mode, err)
		if cfg.Spec.Mode != nmd.NaplesMode_MANAGED_MODE {
			log.Errorf("Failed to switch to managed mode")
			return false, nil
		}

		nic, err := nm.GetSmartNIC()
		if nic == nil || err != nil {
			log.Errorf("NIC not found in nicDB")
			return false, nil
		}

		if nic.Spec.Phase != cmd.SmartNICSpec_REJECTED.String() {
			log.Errorf("NIC is not rejected")
			return false, nil
		}

		if nm.GetRegStatus() == true {
			log.Errorf("Registration is still in progress")
			return false, nil
		}

		if nm.GetUpdStatus() == true {
			log.Errorf("UpdateNIC is still in progress")
			return false, nil
		}

		if nm.GetRegStatus() == true {
			log.Errorf("REST server is still up")
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify mode RejectedNIC in Managed Mode", string("10ms"), string("30s"))
}

func TestNaplesRestartManagedMode(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _ := createNMD(t, emDBPath, "classic", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	var err error
	var resp NaplesConfigResp

	// Switch to Managed mode
	naplesCfg := &nmd.Naples{
		ObjectMeta: api.ObjectMeta{Name: "NaplesConfig"},
		TypeMeta:   api.TypeMeta{Kind: "Naples"},
		Spec: nmd.NaplesSpec{
			Mode:       nmd.NaplesMode_MANAGED_MODE,
			PrimaryMac: nicKey1,
			NodeName:   "esx-001",
		},
	}

	f1 := func() (bool, interface{}) {
		err = netutils.HTTPPost(nm.GetNMDUrl(), naplesCfg, &resp)
		if err != nil {
			log.Errorf("Failed to post naples config, err:%+v resp:%+v", err, resp)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f1, "Failed to post the naples config")

	f2 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		log.Infof("CFG: %+v err: %+v", cfg.Spec.Mode, err)
		if cfg.Spec.Mode != nmd.NaplesMode_MANAGED_MODE {
			log.Errorf("Failed to switch to managed mode")
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify Managed Mode", string("10ms"), string("30s"))

	// stop NMD, don't clean up DB file
	stopNMD(t, nm, false)

	// create NMD again, simulating restart
	nm, _, _ = createNMD(t, emDBPath, "classic", "")
	defer stopNMD(t, nm, true)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	AssertEventually(t, f2, "Failed to verify Managed Mode after Restart", string("10ms"), string("30s"))
}

// Test invalid mode
func TestNaplesInvalidMode(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// Negative test case, invalid mode
	nm, _, _ := createNMD(t, emDBPath, "unknown", nicKey1)
	Assert(t, (nm == nil), "Invalid mode should have been rejected", nm)
}
