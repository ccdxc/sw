// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"bytes"
	"context"
	"crypto"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"mime/multipart"
	"net/http"
	"os"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/nic/agent/nmd/state/ipif"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	nmdapi "github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/certificates/certapi"
	roprotos "github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// Test params
var (
	// Bolt DB file
	emDBPath = "/tmp/nmd.db"

	// NIC to be admitted
	nicKey1 = "00ae.cd01.0203"

	// NIC to be in pending state
	nicKey2 = "4444.4444.4444"

	// NIC to be rejected
	nicKey3 = "6666.6666.6666"

	logger = log.GetNewLogger(log.GetDefaultConfig("nmd_state_test"))

	// NIC registration interval
	nicRegInterval = time.Second

	// NIC update interval
	nicUpdInterval = 100 * time.Millisecond

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("nmd_state_test", logger))
)

// Mock platform agent
type mockAgent struct {
	sync.Mutex
	nicDB map[string]*cmd.DistributedServiceCard
}

// RegisterNMD registers NMD with PlatformAgent
func (m *mockAgent) RegisterNMD(nmdapi nmdapi.NmdPlatformAPI) error {
	return nil
}

// CreateSmartNIC creates a smart NIC object
func (m *mockAgent) CreateSmartNIC(nic *cmd.DistributedServiceCard) error {
	m.Lock()
	defer m.Unlock()

	key := objectKey(nic.ObjectMeta)
	m.nicDB[key] = nic
	return nil
}

// UpdateSmartNIC updates a smart NIC object
func (m *mockAgent) UpdateSmartNIC(nic *cmd.DistributedServiceCard) error {
	m.Lock()
	defer m.Unlock()

	key := objectKey(nic.ObjectMeta)
	m.nicDB[key] = nic
	return nil
}

// DeleteSmartNIC deletes a smart NIC object
func (m *mockAgent) DeleteSmartNIC(nic *cmd.DistributedServiceCard) error {
	m.Lock()
	defer m.Unlock()

	key := objectKey(nic.ObjectMeta)
	delete(m.nicDB, key)
	return nil
}

func (m *mockAgent) GetPlatformCertificate(nic *cmd.DistributedServiceCard) ([]byte, error) {
	return nil, nil
}

func (m *mockAgent) GetPlatformSigner(nic *cmd.DistributedServiceCard) (crypto.Signer, error) {
	return nil, nil
}

type mockCtrler struct {
	sync.Mutex
	nicDB                     map[string]*cmd.DistributedServiceCard
	numUpdateSmartNICReqCalls int
	smartNICWatcherRunning    bool
}

func (m *mockCtrler) RegisterSmartNICReq(nic *cmd.DistributedServiceCard) (grpc.RegisterNICResponse, error) {
	m.Lock()
	defer m.Unlock()

	key := objectKey(nic.ObjectMeta)
	m.nicDB[key] = nic
	if strings.HasPrefix(nic.Spec.ID, nicKey1) {
		// we don't have the actual csr from the NIC request, so we just make up
		// a certificate on the spot
		key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
		if err != nil {
			return grpc.RegisterNICResponse{}, fmt.Errorf("Error generating CA key: %v", err)
		}
		cert, err := certs.SelfSign("nmd_state_test_ca", key, certs.WithValidityDays(1))
		if err != nil {
			return grpc.RegisterNICResponse{}, fmt.Errorf("Error generating CA cert: %v", err)
		}
		resp := grpc.RegisterNICResponse{
			AdmissionResponse: &grpc.NICAdmissionResponse{
				Phase: cmd.DistributedServiceCardStatus_ADMITTED.String(),
				ClusterCert: &certapi.CertificateSignResp{
					Certificate: &certapi.Certificate{
						Certificate: cert.Raw,
					},
				},
				CaTrustChain: &certapi.CaTrustChain{
					Certificates: []*certapi.Certificate{
						{
							Certificate: cert.Raw,
						},
					},
				},
				TrustRoots: &certapi.TrustRoots{
					Certificates: []*certapi.Certificate{
						{
							Certificate: cert.Raw,
						},
					},
				},
			},
		}
		return resp, nil
	}

	if strings.HasPrefix(nic.Spec.ID, nicKey2) {
		return grpc.RegisterNICResponse{
			AdmissionResponse: &grpc.NICAdmissionResponse{
				Phase: cmd.DistributedServiceCardStatus_PENDING.String(),
			},
		}, nil
	}

	return grpc.RegisterNICResponse{
		AdmissionResponse: &grpc.NICAdmissionResponse{
			Phase:  cmd.DistributedServiceCardStatus_REJECTED.String(),
			Reason: string("Invalid Cert"),
		},
	}, nil
}

func (m *mockCtrler) UpdateSmartNICReq(nic *cmd.DistributedServiceCard) error {
	m.Lock()
	defer m.Unlock()
	m.numUpdateSmartNICReqCalls++
	key := objectKey(nic.ObjectMeta)
	m.nicDB[key] = nic
	return nil
}

func (m *mockCtrler) WatchSmartNICUpdates() {
	m.Lock()
	defer m.Unlock()
	m.smartNICWatcherRunning = true
}

func (m *mockCtrler) Stop() {
	m.Lock()
	defer m.Unlock()
	m.smartNICWatcherRunning = false
	m.numUpdateSmartNICReqCalls = 0
}

func (m *mockCtrler) IsSmartNICWatcherRunning() bool {
	return m.smartNICWatcherRunning
}

func (m *mockCtrler) GetNIC(meta *api.ObjectMeta) *cmd.DistributedServiceCard {
	m.Lock()
	defer m.Unlock()

	key := objectKey(*meta)
	return m.nicDB[key]
}

func (m *mockCtrler) GetNumUpdateSmartNICReqCalls() int {
	m.Lock()
	defer m.Unlock()
	return m.numUpdateSmartNICReqCalls
}

type mockRolloutCtrler struct {
	sync.Mutex
	status                 []roprotos.DSCOpStatus
	smartNICWatcherRunning bool
}

func (f *mockRolloutCtrler) UpdateDSCRolloutStatus(status *roprotos.DSCRolloutStatusUpdate) error {
	f.status = status.Status.OpStatus
	log.Errorf("Got status %#v", f.status)
	return nil
}

func (f *mockRolloutCtrler) WatchDSCRolloutUpdates() error {
	f.Lock()
	defer f.Unlock()
	f.smartNICWatcherRunning = true
	return nil
}

func (f *mockRolloutCtrler) Stop() {
	f.Lock()
	defer f.Unlock()
	f.smartNICWatcherRunning = false
}

func (f *mockRolloutCtrler) IsSmartNICWatcherRunning() bool {
	f.Lock()
	defer f.Unlock()
	return f.smartNICWatcherRunning
}

type mockUpgAgent struct {
	n         nmdapi.NmdRolloutAPI
	forceFail bool
}

func (f *mockUpgAgent) RegisterNMD(n nmdapi.NmdRolloutAPI) error {
	f.n = n
	return nil
}
func (f *mockUpgAgent) StartDisruptiveUpgrade(firmwarePkgName string) error {
	if f.forceFail {
		go f.n.UpgFailed(&[]string{"ForceFailDisruptive"})
	} else {
		go f.n.UpgSuccessful()
	}
	return nil
}
func (f *mockUpgAgent) StartUpgOnNextHostReboot(firmwarePkgName string) error {
	if f.forceFail {
		go f.n.UpgFailed(&[]string{"ForceFailUpgOnNextHostReboot"})
	} else {
		go f.n.UpgSuccessful()
	}
	return nil
}
func (f *mockUpgAgent) IsUpgClientRegistered() error {
	return nil
}
func (f *mockUpgAgent) IsUpgradeInProgress() bool {
	return false
}
func (f *mockUpgAgent) StartPreCheckDisruptive(version string) error {
	if f.forceFail {
		go f.n.UpgNotPossible(&[]string{"ForceFailpreCheckDisruptive"})
	} else {
		go f.n.UpgPossible()
	}
	return nil
}
func (f *mockUpgAgent) StartPreCheckForUpgOnNextHostReboot(version string) error {
	if f.forceFail {
		go f.n.UpgNotPossible(&[]string{"ForceFailpreCheckForUpgOnNextHostReboot"})
	} else {
		go f.n.UpgPossible()
	}
	return nil
}

// createNMD creates a NMD server
func createNMD(t *testing.T, dbPath, mode, nodeID string) (*NMD, *mockAgent, *mockCtrler, *mockUpgAgent, *mockRolloutCtrler) {
	// Start a fake delphi hub
	log.Info("Creating NMD")
	// Hardcode the MacAddress for now. If we want multiple instances of fru.json with unique MAC, then a getMac() function can be written.
	err := CreateFruJSON("00:AE:CD:01:02:03")
	if err != nil {
		log.Errorf("Error creating /tmp/fru.json file. Err: %v", err)
		return nil, nil, nil, nil, nil
	}

	ag := &mockAgent{
		nicDB: make(map[string]*cmd.DistributedServiceCard),
	}
	ct := &mockCtrler{
		nicDB: make(map[string]*cmd.DistributedServiceCard),
	}
	roC := &mockRolloutCtrler{}
	upgAgt := &mockUpgAgent{}

	// create new NMD
	nm, err := NewNMD(nil,
		//ag,
		//upgAgt,
		//nil, // no resolver
		dbPath,
		//nodeID,
		"localhost:0",
		"", // no revproxy endpoint
		//"", // no local certs endpoint
		//"", // no remote certs endpoint
		//"", // no cmd registration endpoint
		//mode,
		nicRegInterval,
		nicUpdInterval,
		WithCMDAPI(ct),
		WithRolloutAPI(roC))

	if err != nil {
		log.Errorf("Error creating NMD. Err: %v", err)
		return nil, nil, nil, nil, nil
	}
	Assert(t, nm.GetAgentID() == nodeID, "Failed to match nodeUUID", nm)

	// Ensure the NMD's rest server is started
	nm.CreateMockIPClient()
	cfg := nm.GetNaplesConfig()

	if cfg.Spec.IPConfig == nil {
		cfg.Spec.IPConfig = &cmd.IPConfig{}
	}

	if mode == "network" {
		// Add a fake controller to the spec so that mock IPClient starts managed mode
		cfg.Spec.Controllers = []string{"127.0.0.1"}
	}

	nm.SetNaplesConfig(cfg.Spec)
	err = nm.UpdateNaplesConfig(nm.GetNaplesConfig())

	nm.IPClient, _ = ipif.NewMockIPClient(nm, "mock")

	nm.Upgmgr = upgAgt
	nm.cmd = ct
	nm.Platform = ag

	return nm, ag, ct, upgAgt, roC
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
	nm, _, _, _, _ := createNMD(t, "", "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, false)

	// NIC message
	nic := cmd.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name: nicKey1,
		},
	}

	// create smartNIC
	err := nm.CreateSmartNIC(&nic)
	AssertOk(t, err, "Error creating nic")
	n, err := nm.GetSmartNIC()
	AssertOk(t, err, "NIC was not found in DB")
	Assert(t, n.ObjectMeta.Name == nicKey1, "NIC name did not match", n)

	// update smartNIC
	nic.Status = cmd.DistributedServiceCardStatus{
		Conditions: []cmd.DSCCondition{
			{
				Type:   cmd.DSCCondition_HEALTHY.String(),
				Status: cmd.ConditionStatus_TRUE.String(),
			},
		},
	}
	err = nm.UpdateSmartNIC(&nic)
	AssertOk(t, err, "Error updating nic")
	n, err = nm.GetSmartNIC()
	AssertOk(t, err, "NIC was not found in DB")
	Assert(t, n.Status.Conditions[0].Status == cmd.ConditionStatus_TRUE.String() && nic.ObjectMeta.Name == "00ae.cd01.0203", "NIC status did not match", n)

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
	nm, _, ct, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, true)

	// NIC message
	nic := cmd.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: nicKey1,
		},
		Spec: cmd.DistributedServiceCardSpec{
			ID: nicKey1,
		},
	}

	// create smartNIC
	resp, err := nm.RegisterSmartNICReq(&nic)
	AssertOk(t, err, "Error registering nic")
	Assert(t, resp.AdmissionResponse.Phase == cmd.DistributedServiceCardStatus_ADMITTED.String(), "NIC is not admitted", nic)

	// update smartNIC
	nic.Status = cmd.DistributedServiceCardStatus{
		Conditions: []cmd.DSCCondition{
			{
				Type:   cmd.DSCCondition_HEALTHY.String(),
				Status: cmd.ConditionStatus_TRUE.String(),
			},
		},
	}
	err = nm.UpdateSmartNICReq(&nic)
	AssertOk(t, err, "Error updating nic")
	updNIC := ct.GetNIC(&nic.ObjectMeta)
	Assert(t, updNIC.Status.Conditions[0].Status == cmd.ConditionStatus_TRUE.String() &&
		updNIC.ObjectMeta.Name == "00ae.cd01.0203", "NIC status did not match", updNIC)
}

func TestNaplesDefaultNetworkMode(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	defer stopNMD(t, nm, true)

	f1 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		if cfg.Spec.Mode == nmd.MgmtMode_NETWORK.String() && nm.GetListenURL() != "" &&
			nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
			return true, nil
		}
		return false, nil
	}
	AssertEventually(t, f1, "Failed to verify mode is in host")

	var naplesCfg nmd.DistributedServiceCard

	f2 := func() (bool, interface{}) {
		err := netutils.HTTPGet(nm.GetNMDUrl()+"/", &naplesCfg)
		if err != nil {
			log.Errorf("Failed to get naples config, err:%+v", err)
			return false, nil
		}

		if naplesCfg.Spec.Mode != nmd.MgmtMode_NETWORK.String() {
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f2, "Failed to get the default naples config")
}

func TestNaplesNetworkMode(t *testing.T) {
	t.Skip("Temporarily Skipped.")
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// Start NMD in network mode
	nm, _, cm, _, ro := createNMD(t, emDBPath, "network", nicKey1)
	defer stopNMD(t, nm, true)
	Assert(t, (nm != nil), "Failed to start NMD", nm)

	cfg := nmd.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{
			Name: "DistributedServiceCardConfig",
		},
		TypeMeta: api.TypeMeta{
			Kind: "DistributedServiceCard",
		},
		Spec: nmd.DistributedServiceCardSpec{
			Mode:        nmd.MgmtMode_NETWORK.String(),
			NetworkMode: nmd.NetworkMode_INBAND.String(),
			PrimaryMAC:  "42:42:42:42:42:42",
			ID:          "42:42:42:42:42:42",
			DSCProfile:  "default",
			IPConfig: &cmd.IPConfig{
				IPAddress:  "4.4.4.4/16",
				DefaultGW:  "",
				DNSServers: nil,
			},
		},
	}

	err := nm.UpdateNaplesConfig(cfg)
	AssertOk(t, err, "Failed to update naples config")
	err = nm.UpdateNaplesInfoFromConfig()
	AssertOk(t, err, "Failed to update naples info")

	f1 := func() (bool, interface{}) {

		// Verify mode
		cfg := nm.GetNaplesConfig()
		log.Infof("NaplesConfig: %v", cfg)
		if cfg.Spec.Mode != nmd.MgmtMode_NETWORK.String() {
			log.Errorf("Mode is not network")
			return false, nil
		}

		// Verify nic state
		nic, err := nm.GetSmartNIC()
		if nic == nil || err != nil {
			log.Errorf("NIC %s not found in nicDB, nic: %v err: %v", nicKey1, nic, err)
			return false, nil
		}
		log.Infof("NIC: %v", nic)

		//// Verify NIC admission
		//if nic.Status.AdmissionPhase != cmd.SmartNICStatus_ADMITTED.String() {
		//	log.Errorf("NIC is not admitted")
		//	return false, nil
		//}

		// Verify registration status
		if nm.GetRegStatus() == true {
			log.Errorf("Registration is still in progress")
			return false, nil
		}

		// Verify update task
		if nm.GetUpdStatus() == false {
			log.Errorf("Update NIC is not in progress")
			return false, nil
		}

		// Verify watcher is active
		if nm.GetCMDSmartNICWatcherStatus() == false {
			log.Errorf("CMD SmartNIC watcher is not active")
			return false, nil
		}

		// Verify watcher is active
		if nm.GetRoSmartNICWatcherStatus() == false {
			log.Errorf("Rollout SmartNIC watcher is not active")
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify network Mode", string("50ms"), string("30s"))

	// Verify updates are sent
	f2 := func() (bool, interface{}) {
		if cm.GetNumUpdateSmartNICReqCalls() < 3 {
			log.Errorf("Received %d update calls, want 3", cm.GetNumUpdateSmartNICReqCalls())
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify network Mode", string("500ms"), string("30s"))

	// Simulate de-admit re-admit
	nic, err := nm.GetSmartNIC()
	AssertOk(t, err, "NIC not found in nicDB")
	nic.Status.AdmissionPhase = cmd.DistributedServiceCardStatus_PENDING.String()
	nm.StopManagedMode()
	nm.RegisterCMD(cm)
	nm.RegisterROCtrlClient(ro)
	nm.AdmitNaples()

	f3 := func() (bool, interface{}) {
		nic, err := nm.GetSmartNIC()
		AssertOk(t, err, "NIC not found in nicDB")

		// Verify NIC admission
		if nic.Status.AdmissionPhase != cmd.DistributedServiceCardStatus_ADMITTED.String() {
			log.Errorf("NIC is not admitted")
			return false, nil
		}

		// Verify update task
		if nm.GetUpdStatus() == false {
			log.Errorf("Update NIC is not in progress")
			return false, nil
		}

		// Verify CMD watcher is active
		if nm.GetCMDSmartNICWatcherStatus() == false {
			log.Errorf("CMD SmartNIC watcher is not active")
			return false, nil
		}

		// Verify Rollout watcher is active
		if nm.GetRoSmartNICWatcherStatus() == false {
			log.Errorf("Rollout SmartNIC watcher is not active")
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f3, "Failed to verify network Mode", string("50ms"), string("30s"))
}

// TestNaplesModeTransitions tests the mode transition
// host -> network -> host
func TestNaplesModeTransitions(t *testing.T) {
	t.Skip("Skipped temporarily")
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, true)

	f1 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		if cfg.Spec.Mode != nmd.MgmtMode_HOST.String() && nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
			return true, nil
		}
		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify mode is in host")

	// Switch to network mode
	naplesCfg := &nmd.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{Name: "DistributedServiceCardConfig"},
		TypeMeta:   api.TypeMeta{Kind: "DistributedServiceCard"},
		Spec: nmd.DistributedServiceCardSpec{
			Mode:        nmd.MgmtMode_NETWORK.String(),
			NetworkMode: nmd.NetworkMode_INBAND.String(),
			//Controllers: []string{"192.168.30.10"},
			ID: nicKey1,
			IPConfig: &cmd.IPConfig{
				IPAddress: "10.10.10.10/24",
			},
			PrimaryMAC: nicKey1,
		},
		Status: nmd.DistributedServiceCardStatus{
			AdmissionPhase: cmd.DistributedServiceCardStatus_ADMITTED.String(),
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
		if cfg.Spec.Mode != nmd.MgmtMode_NETWORK.String() {
			log.Errorf("Failed to switch to network mode")
			return false, nil
		}

		nic, err := nm.GetSmartNIC()
		if nic == nil || err != nil {
			log.Errorf("NIC not found in nicDB, mac:%s", nicKey1)
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f3, "Failed to verify mode is in network Mode", string("10ms"), string("30s"))

	// Switch to host mode
	naplesCfg.Spec.Mode = nmd.MgmtMode_HOST.String()
	naplesCfg.Spec.DSCProfile = "default"

	AssertEventually(t, f2, "Failed to post the naples config")

	// Verify it is in host mode
	AssertEventually(t, f1, "Failed to verify mode is in host")
}

func TestNaplesNetworkModeManualApproval(t *testing.T) {
	t.Skip("Skipping temporarily")
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey2)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, true)

	var err error
	var resp NaplesConfigResp

	// Switch to network mode
	naplesCfg := &nmd.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{Name: "DistributedServiceCardConfig"},
		TypeMeta:   api.TypeMeta{Kind: "DistributedServiceCard"},
		Spec: nmd.DistributedServiceCardSpec{
			Mode:        nmd.MgmtMode_NETWORK.String(),
			PrimaryMAC:  nicKey2,
			ID:          nicKey2,
			Controllers: []string{"localhost"},
			IPConfig: &cmd.IPConfig{
				IPAddress: "10.10.10.10/24",
			},
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
		if cfg.Spec.Mode != nmd.MgmtMode_NETWORK.String() {
			log.Errorf("Failed to switch to network mode")
			return false, nil
		}

		nic, err := nm.GetSmartNIC()
		if nic == nil || err != nil {
			log.Errorf("NIC not found in nicDB")
			return false, nil
		}

		if nic.Status.AdmissionPhase != cmd.DistributedServiceCardStatus_PENDING.String() {
			log.Errorf("NIC is not pending, expected %v, found %v", cmd.DistributedServiceCardStatus_PENDING.String(), nic.Status.AdmissionPhase)
			return false, nil
		}

		if nm.GetRegStatus() == false {
			log.Errorf("Registration is not in progress")
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify PendingNIC in network Mode", string("10ms"), string("30s"))
}

func TestNaplesNetworkModeInvalidNIC(t *testing.T) {
	t.Skip("Skipping temporarily")
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey3)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	defer stopNMD(t, nm, true)

	var err error
	var resp NaplesConfigResp

	// Switch to network mode
	naplesCfg := &nmd.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{Name: "DistributedServiceCardConfig"},
		TypeMeta:   api.TypeMeta{Kind: "DistributedServiceCard"},
		Spec: nmd.DistributedServiceCardSpec{
			Mode:        nmd.MgmtMode_NETWORK.String(),
			PrimaryMAC:  nicKey3,
			ID:          nicKey3,
			Controllers: []string{"localhost"},
			IPConfig: &cmd.IPConfig{
				IPAddress: "10.10.10.10/24",
			},
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
		log.Infof("CFG: %+v Status : %+v err: %+v", cfg.Spec.Mode, cfg.Status, err)
		if cfg.Spec.Mode != nmd.MgmtMode_NETWORK.String() {
			log.Errorf("Failed to switch to network mode")
			return false, nil
		}

		nic, err := nm.GetSmartNIC()
		if nic == nil || err != nil {
			log.Errorf("NIC not found in nicDB")
			return false, nil
		}

		if nic.Status.AdmissionPhase != cmd.DistributedServiceCardStatus_REJECTED.String() {
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

		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify mode RejectedNIC in network Mode", string("10ms"), string("30s"))
}

func TestNaplesRestartNetworkMode(t *testing.T) {
	t.Skip("Skipped temporarily")
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &mock.ResolverClient{}})
	defer cancel()

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	var err error
	var resp NaplesConfigResp

	// Switch to network mode
	naplesCfg := &nmd.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{Name: "DistributedServiceCardConfig"},
		TypeMeta:   api.TypeMeta{Kind: "DistributedServiceCard"},
		Spec: nmd.DistributedServiceCardSpec{
			Mode:        nmd.MgmtMode_NETWORK.String(),
			NetworkMode: nmd.NetworkMode_OOB.String(),
			PrimaryMAC:  nicKey1,
			Controllers: []string{"localhost"},
			ID:          nicKey1,
			IPConfig: &cmd.IPConfig{
				IPAddress: "10.10.10.10/24",
			},
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
		log.Infof("CFG: %+v err: %+v EXPECTED : %+v", cfg.Spec.Mode, err, nmd.MgmtMode_NETWORK.String())
		if cfg.Spec.Mode != nmd.MgmtMode_NETWORK.String() {
			log.Errorf("Failed to switch to network mode")
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify network Mode", string("10ms"), string("30s"))

	// stop NMD, don't clean up DB file
	stopNMD(t, nm, false)

	// create NMD again, simulating restart
	nm, _, _, _, _ = createNMD(t, emDBPath, "host", "")
	defer stopNMD(t, nm, true)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	AssertEventually(t, f2, "Failed to verify network Mode after Restart", string("10ms"), string("30s"))
}

// Test invalid mode
func TestNaplesInvalidMode(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// Start NMD in network mode
	nm, _, _, _, _ := createNMD(t, emDBPath, "network", nicKey1)
	defer stopNMD(t, nm, true)
	Assert(t, (nm != nil), "Failed to start NMD", nm)

	cfg := nmd.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{
			Name: "DistributedServiceCardConfig",
		},
		TypeMeta: api.TypeMeta{
			Kind: "DistributedServiceCard",
		},
		Spec: nmd.DistributedServiceCardSpec{
			Mode:        "Invalid Mode",
			NetworkMode: nmd.NetworkMode_INBAND.String(),
			PrimaryMAC:  "42:42:42:42:42:42",
			ID:          "42:42:42:42:42:42",
			DSCProfile:  "default",
			IPConfig: &cmd.IPConfig{
				IPAddress:  "4.4.4.4/16",
				DefaultGW:  "",
				DNSServers: nil,
			},
		},
	}

	err := nm.UpdateNaplesConfig(cfg)
	Assert(t, err != nil, "Invalid mode should have been rejected")
}

func TestNaplesRollout(t *testing.T) {
	t.Skip("Temporarily skipped")
	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	t.Log("Create nmd")
	nm, _, _, upgAg, roCtrl := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)
	Assert(t, (upgAg != nil), "Failed to create nmd", nm)
	Assert(t, (roCtrl != nil), "Failed to create nmd", nm)

	sro := roprotos.DSCRollout{
		TypeMeta: api.TypeMeta{
			Kind: "DSCRollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: nm.GetPrimaryMAC(),
		},
		Spec: roprotos.DSCRolloutSpec{
			Ops: []roprotos.DSCOpSpec{
				{
					Op:      roprotos.DSCOp_DSCPreCheckForDisruptive,
					Version: "ver1",
				},
			},
		},
	}

	t.Log("Create ver1 PreCheckForDisruptive")
	err := nm.CreateUpdateDSCRollout(&sro)
	Assert(t, (err == nil), "CreateDSCRollout Failed")

	// When venice asks for one Op, we expect that status should reflect that Op to be successful
	f1 := func() (bool, interface{}) {
		if len(roCtrl.status) == 1 && roCtrl.status[0].Op == roprotos.DSCOp_DSCPreCheckForDisruptive &&
			roCtrl.status[0].Version == "ver1" {
			return true, nil
		}
		return false, nil
	}
	AssertEventually(t, f1, "PreCheckForDisruptive failed to succeed")

	// venice Adds another Op (typically doUpgrade) to existing one(PreCheck),
	// we expect that status should reflect the second Op to be successful
	// and the status should contain both the Ops as success
	t.Log("ver1 DoDisruptive")
	sro.Spec.Ops = append(sro.Spec.Ops, roprotos.DSCOpSpec{
		Op:      roprotos.DSCOp_DSCDisruptiveUpgrade,
		Version: "ver1",
	})
	err = nm.CreateUpdateDSCRollout(&sro)
	Assert(t, (err == nil), "CreateUpdateDSCRollout with Op: DSCOp_DSCDisruptiveUpgrade Failed")

	f2 := func() (bool, interface{}) {
		if len(roCtrl.status) == 2 &&
			roCtrl.status[0].Op == roprotos.DSCOp_DSCPreCheckForDisruptive && roCtrl.status[0].Version == "ver1" &&
			roCtrl.status[1].Op == roprotos.DSCOp_DSCDisruptiveUpgrade && roCtrl.status[1].Version == "ver1" {
			return true, nil
		}
		return false, nil
	}
	AssertEventually(t, f2, "DisruptiveUpgrade failed")

	// venice can always update with the same Spec as already informed before. This can happen say when the controller
	// restarts before persisting status update from NIC. In such a case we expect the status to continue to succeed
	t.Log("Updated spec with same contents again")
	err = nm.CreateUpdateDSCRollout(&sro)
	Assert(t, (err == nil), "CreateUpdateDSCRollout Spec with Same contents Failed")

	f3 := func() (bool, interface{}) {
		if len(roCtrl.status) == 2 &&
			roCtrl.status[0].Op == roprotos.DSCOp_DSCPreCheckForDisruptive && roCtrl.status[0].Version == "ver1" &&
			roCtrl.status[1].Op == roprotos.DSCOp_DSCDisruptiveUpgrade && roCtrl.status[1].Version == "ver1" {
			return true, nil
		}
		return false, nil
	}
	AssertConsistently(t, f3, "DSCRollout second time with same Spec failed during NonDisruptive Upgrade", "100ms", "500ms")

	t.Log("Update with ver2 Precheck and doUpgrade")
	sro.Spec.Ops[0].Version = "ver2"
	sro.Spec.Ops[1].Version = "ver2"
	err = nm.CreateUpdateDSCRollout(&sro)
	Assert(t, (err == nil), "Failed to update DSCRollout with new version in Spec")

	f5 := func() (bool, interface{}) {
		if len(roCtrl.status) == 2 &&
			roCtrl.status[0].Op == roprotos.DSCOp_DSCPreCheckForDisruptive && roCtrl.status[0].Version == "ver2" && roCtrl.status[0].OpStatus == "success" &&
			roCtrl.status[1].Op == roprotos.DSCOp_DSCDisruptiveUpgrade && roCtrl.status[1].Version == "ver2" && roCtrl.status[1].OpStatus == "success" {
			return true, nil
		}
		return false, nil
	}
	AssertEventually(t, f5, "Version change and issuing 2 ops together caused failure")

	upgAg.forceFail = true // Failure cases now
	t.Log("Forcefail set. Updating  with ver3 Precheck and doUpgrade")

	sro.Spec.Ops[0].Version = "ver3"
	sro.Spec.Ops[1].Version = "ver3"
	err = nm.CreateUpdateDSCRollout(&sro)
	Assert(t, (err == nil), "Failed to update DSCRollout with new version in Spec")

	f6 := func() (bool, interface{}) {
		if len(roCtrl.status) == 2 &&
			roCtrl.status[0].Op == roprotos.DSCOp_DSCPreCheckForDisruptive && roCtrl.status[0].Version == "ver3" && roCtrl.status[0].OpStatus == "failure" && roCtrl.status[0].Message == "ForceFailpreCheckDisruptive" &&
			roCtrl.status[1].Op == roprotos.DSCOp_DSCDisruptiveUpgrade && roCtrl.status[1].Version == "ver3" && roCtrl.status[1].OpStatus == "failure" && roCtrl.status[1].Message == "ForceFailDisruptive" {
			return true, nil
		}
		return false, nil
	}
	AssertEventually(t, f6, "Expecting fail but still succeeded")
	// Even after this state is reached there should not be any further transitions
	AssertConsistently(t, f6, "Expecting fail but still succeeded", "100ms", "3s")

	// Finally a delete of Smartnic Rollout object should succeed
	t.Log("Delete VeniceRollout")

	err = nm.DeleteDSCRollout()
	Assert(t, (err == nil), "DeleteDSCRollout Failed")

}

func TestNaplesCmdExec(t *testing.T) {

	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "ls",
		Opts:       "-al /",
	}

	f1 := func() (bool, interface{}) {
		payloadBytes, err := json.Marshal(v)
		if err != nil {
			log.Errorf("Failed to marshal data, err:%+v", err)
			return false, nil
		}
		body := bytes.NewReader(payloadBytes)
		getReq, err := http.NewRequest("GET", nm.GetNMDCmdExecURL(), body)
		if err != nil {
			log.Errorf("Failed to create new request, err:%+v", err)
			return false, nil
		}
		getReq.Header.Set("Content-Type", "application/json")

		getResp, err := http.DefaultClient.Do(getReq)
		if err != nil {
			log.Errorf("Failed to get response, err:%+v", err)
			return false, nil
		}
		defer getResp.Body.Close()
		_, err = ioutil.ReadAll(getResp.Body)
		if err != nil {
			log.Errorf("Failed to read body bytes, err:%+v", err)
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f1, "Failed to post exec cmd")

	// stop NMD, don't clean up DB file
	stopNMD(t, nm, false)
}

func mustOpen(f string) *os.File {
	r, err := os.Open(f)
	if err != nil {
		panic(err)
	}
	return r
}

func TestNaplesFileUpload(t *testing.T) {
	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	f1 := func() (bool, interface{}) {
		path, err := ioutil.TempDir("", "nmd-upload-")
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		d1 := []byte("hello\ngo\n")
		err = ioutil.WriteFile(path+"/dat1", d1, 0644)
		AssertOk(t, err, "Error writing upload file")

		values := map[string]io.Reader{
			"uploadFile": mustOpen(path + "/dat1"),
			"uploadPath": strings.NewReader(path),
		}
		var b bytes.Buffer
		w := multipart.NewWriter(&b)
		for key, r := range values {
			var fw io.Writer
			if x, ok := r.(io.Closer); ok {
				defer x.Close()
			}
			if x, ok := r.(*os.File); ok {
				if fw, err = w.CreateFormFile(key, x.Name()); err != nil {
					return false, err
				}
			} else {
				if fw, err = w.CreateFormField(key); err != nil {
					return false, err
				}
			}
			if _, err = io.Copy(fw, r); err != nil {
				return false, err
			}

		}
		w.Close()
		if err != nil {
			log.Errorf("Failed to marshal data, err:%+v", err)
			return false, nil
		}
		getReq, err := http.NewRequest("POST", nm.GetGetNMDUploadURL(), &b)
		if err != nil {
			log.Errorf("Failed to create new request, err:%+v", err)
			return false, nil
		}
		getReq.Header.Set("Content-Type", w.FormDataContentType())

		getResp, err := http.DefaultClient.Do(getReq)
		if err != nil {
			log.Errorf("Failed to get response, err:%+v", err)
			return false, nil
		}
		defer getResp.Body.Close()
		respBody, err := ioutil.ReadAll(getResp.Body)
		if err != nil {
			log.Errorf("Failed to read body bytes, err:%+v", err)
			return false, nil
		}
		if strings.Compare(string(respBody), "File Copied Successfully\n") != 0 {
			log.Errorf("respBody not as expected, got:%s", string(respBody))
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f1, "Failed to upload file")

	// stop NMD, don't clean up DB file
	stopNMD(t, nm, false)
}

func TestNaplesFileUploadNoUploadFile(t *testing.T) {
	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	f1 := func() (bool, interface{}) {
		path, err := ioutil.TempDir("", "nmd-upload-")
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		d1 := []byte("hello\ngo\n")
		err = ioutil.WriteFile(path+"/dat1", d1, 0644)
		AssertOk(t, err, "Error writing upload file")

		values := map[string]io.Reader{
			"uploadPath": strings.NewReader(path),
		}
		var b bytes.Buffer
		w := multipart.NewWriter(&b)
		for key, r := range values {
			var fw io.Writer
			if x, ok := r.(io.Closer); ok {
				defer x.Close()
			}
			if x, ok := r.(*os.File); ok {
				if fw, err = w.CreateFormFile(key, x.Name()); err != nil {
					return false, err
				}
			} else {
				if fw, err = w.CreateFormField(key); err != nil {
					return false, err
				}
			}
			if _, err = io.Copy(fw, r); err != nil {
				return false, err
			}

		}
		w.Close()
		if err != nil {
			log.Errorf("Failed to marshal data, err:%+v", err)
			return false, nil
		}
		getReq, err := http.NewRequest("POST", nm.GetGetNMDUploadURL(), &b)
		if err != nil {
			log.Errorf("Failed to create new request, err:%+v", err)
			return false, nil
		}
		getReq.Header.Set("Content-Type", w.FormDataContentType())

		getResp, err := http.DefaultClient.Do(getReq)
		if err != nil {
			log.Errorf("Failed to get response, err:%+v", err)
			return false, nil
		}
		defer getResp.Body.Close()
		respBody, err := ioutil.ReadAll(getResp.Body)
		if err != nil {
			log.Errorf("Failed to read body bytes, err:%+v", err)
			return false, nil
		}
		if strings.Compare(string(respBody), "http: no such file") != 0 {
			log.Errorf("respBody not as expected, got:%s", string(respBody))
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f1, "Failed to upload file")

	// stop NMD, don't clean up DB file
	stopNMD(t, nm, false)
}

func TestNaplesFileUploadNoUploadPath(t *testing.T) {
	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	f1 := func() (bool, interface{}) {
		path, err := ioutil.TempDir("", "nmd-upload-")
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		d1 := []byte("hello\ngo\n")
		err = ioutil.WriteFile(path+"/dat1", d1, 0644)
		AssertOk(t, err, "Error writing upload file")

		values := map[string]io.Reader{
			"uploadFile": mustOpen(path + "/dat1"),
		}
		var b bytes.Buffer
		w := multipart.NewWriter(&b)
		for key, r := range values {
			var fw io.Writer
			if x, ok := r.(io.Closer); ok {
				defer x.Close()
			}
			if x, ok := r.(*os.File); ok {
				if fw, err = w.CreateFormFile(key, x.Name()); err != nil {
					return false, err
				}
			} else {
				if fw, err = w.CreateFormField(key); err != nil {
					return false, err
				}
			}
			if _, err = io.Copy(fw, r); err != nil {
				return false, err
			}

		}
		w.Close()
		if err != nil {
			log.Errorf("Failed to marshal data, err:%+v", err)
			return false, nil
		}
		getReq, err := http.NewRequest("POST", nm.GetGetNMDUploadURL(), &b)
		if err != nil {
			log.Errorf("Failed to create new request, err:%+v", err)
			return false, nil
		}
		getReq.Header.Set("Content-Type", w.FormDataContentType())

		getResp, err := http.DefaultClient.Do(getReq)
		if err != nil {
			log.Errorf("Failed to get response, err:%+v", err)
			return false, nil
		}
		defer getResp.Body.Close()
		respBody, err := ioutil.ReadAll(getResp.Body)
		if err != nil {
			log.Errorf("Failed to read body bytes, err:%+v", err)
			return false, nil
		}
		if strings.Compare(string(respBody), "Upload Path Not Specified\n") != 0 {
			log.Errorf("respBody not as expected, got:%s", string(respBody))
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f1, "Failed to upload file")

	// stop NMD, don't clean up DB file
	stopNMD(t, nm, false)
}

func TestNaplesPkgVerify(t *testing.T) {
	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	os.Remove("/tmp/fwupdate")

	f1 := func() (bool, interface{}) {
		path, err := ioutil.TempDir("/tmp", "update")
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		d1 := []byte("hello\ngo\n")
		err = ioutil.WriteFile(path+"/dat1", d1, 0644)
		AssertOk(t, err, "Error writing upload file")

		values := map[string]io.Reader{
			"uploadFile": mustOpen(path + "/dat1"),
			"uploadPath": strings.NewReader(path),
		}
		var b bytes.Buffer
		w := multipart.NewWriter(&b)
		for key, r := range values {
			var fw io.Writer
			if x, ok := r.(io.Closer); ok {
				defer x.Close()
			}
			if x, ok := r.(*os.File); ok {
				if fw, err = w.CreateFormFile(key, x.Name()); err != nil {
					return false, err
				}
			} else {
				if fw, err = w.CreateFormField(key); err != nil {
					return false, err
				}
			}
			if _, err = io.Copy(fw, r); err != nil {
				return false, err
			}

		}
		w.Close()
		if err != nil {
			log.Errorf("Failed to marshal data, err:%+v", err)
			return false, nil
		}
		getReq, err := http.NewRequest("POST", nm.GetGetNMDUploadURL(), &b)
		if err != nil {
			log.Errorf("Failed to create new request, err:%+v", err)
			return false, nil
		}
		getReq.Header.Set("Content-Type", w.FormDataContentType())

		getResp, err := http.DefaultClient.Do(getReq)
		if err != nil {
			log.Errorf("Failed to get response, err:%+v", err)
			return false, nil
		}
		defer getResp.Body.Close()
		respBody, err := ioutil.ReadAll(getResp.Body)
		if err != nil {
			log.Errorf("Failed to read body bytes, err:%+v", err)
			return false, nil
		}
		if strings.Compare(string(respBody), "File Copied Successfully\n") != 0 {
			log.Errorf("respBody not as expected, got:%s", string(respBody))
			return false, nil
		}
		resp, err := naplesPkgVerify("dot1")
		if err == nil {
			log.Errorf("Verified invalid package, err:%+v resp:%s", err, resp)
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify package")

	// stop NMD, don't clean up DB file
	stopNMD(t, nm, false)
}

func TestNaplesSetBootImg(t *testing.T) {
	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	f1 := func() (bool, interface{}) {
		path, err := ioutil.TempDir("/tmp", "update")
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		d1 := []byte("hello\ngo\n")
		err = ioutil.WriteFile(path+"/dat1", d1, 0644)
		AssertOk(t, err, "Error writing upload file")

		values := map[string]io.Reader{
			"uploadFile": mustOpen(path + "/dat1"),
			"uploadPath": strings.NewReader(path),
		}
		var b bytes.Buffer
		w := multipart.NewWriter(&b)
		for key, r := range values {
			var fw io.Writer
			if x, ok := r.(io.Closer); ok {
				defer x.Close()
			}
			if x, ok := r.(*os.File); ok {
				if fw, err = w.CreateFormFile(key, x.Name()); err != nil {
					return false, err
				}
			} else {
				if fw, err = w.CreateFormField(key); err != nil {
					return false, err
				}
			}
			if _, err = io.Copy(fw, r); err != nil {
				return false, err
			}

		}
		w.Close()
		if err != nil {
			log.Errorf("Failed to marshal data, err:%+v", err)
			return false, nil
		}
		getReq, err := http.NewRequest("POST", nm.GetGetNMDUploadURL(), &b)
		if err != nil {
			log.Errorf("Failed to create new request, err:%+v", err)
			return false, nil
		}
		getReq.Header.Set("Content-Type", w.FormDataContentType())

		getResp, err := http.DefaultClient.Do(getReq)
		if err != nil {
			log.Errorf("Failed to get response, err:%+v", err)
			return false, nil
		}
		defer getResp.Body.Close()
		respBody, err := ioutil.ReadAll(getResp.Body)
		if err != nil {
			log.Errorf("Failed to read body bytes, err:%+v", err)
			return false, nil
		}
		if strings.Compare(string(respBody), "File Copied Successfully\n") != 0 {
			log.Errorf("respBody not as expected, got:%s", string(respBody))
			return false, nil
		}
		resp, err := naplesSetBootImg()
		if err == nil {
			log.Errorf("Verified invalid package, err:%+v resp:%s", err, resp)
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify package")

	// stop NMD, don't clean up DB file
	stopNMD(t, nm, false)
}

func TestNaplesPkgInstall(t *testing.T) {
	// Cleanup any prior DB file
	os.Remove(emDBPath)

	// create nmd
	nm, _, _, _, _ := createNMD(t, emDBPath, "host", nicKey1)
	Assert(t, (nm != nil), "Failed to create nmd", nm)

	f1 := func() (bool, interface{}) {
		path, err := ioutil.TempDir("/tmp", "update")
		AssertOk(t, err, "Error creating tmp dir")
		defer os.RemoveAll(path)

		d1 := []byte("hello\ngo\n")
		err = ioutil.WriteFile(path+"/dat1", d1, 0644)
		AssertOk(t, err, "Error writing upload file")

		values := map[string]io.Reader{
			"uploadFile": mustOpen(path + "/dat1"),
			"uploadPath": strings.NewReader(path),
		}
		var b bytes.Buffer
		w := multipart.NewWriter(&b)
		for key, r := range values {
			var fw io.Writer
			if x, ok := r.(io.Closer); ok {
				defer x.Close()
			}
			if x, ok := r.(*os.File); ok {
				if fw, err = w.CreateFormFile(key, x.Name()); err != nil {
					return false, err
				}
			} else {
				if fw, err = w.CreateFormField(key); err != nil {
					return false, err
				}
			}
			if _, err = io.Copy(fw, r); err != nil {
				return false, err
			}

		}
		w.Close()
		if err != nil {
			log.Errorf("Failed to marshal data, err:%+v", err)
			return false, nil
		}
		getReq, err := http.NewRequest("POST", nm.GetGetNMDUploadURL(), &b)
		if err != nil {
			log.Errorf("Failed to create new request, err:%+v", err)
			return false, nil
		}
		getReq.Header.Set("Content-Type", w.FormDataContentType())

		getResp, err := http.DefaultClient.Do(getReq)
		if err != nil {
			log.Errorf("Failed to get response, err:%+v", err)
			return false, nil
		}
		defer getResp.Body.Close()
		respBody, err := ioutil.ReadAll(getResp.Body)
		if err != nil {
			log.Errorf("Failed to read body bytes, err:%+v", err)
			return false, nil
		}
		if strings.Compare(string(respBody), "File Copied Successfully\n") != 0 {
			log.Errorf("respBody not as expected, got:%s", string(respBody))
			return false, nil
		}
		resp, err := naplesPkgInstall("dot1")
		if err == nil {
			log.Errorf("Verified invalid package, err:%+v resp:%s", err, resp)
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify package")

	// stop NMD, don't clean up DB file
	stopNMD(t, nm, false)
}
