package mock

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"fmt"
	"sync"

	nmdAPI "github.com/pensando/sw/nic/agent/nmd/api"

	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/keymgr"
)

// NMD implements nmd api for mocking
type NMD struct {
	Name string
	sync.Mutex
	Nic           *cmd.DistributedServiceCard
	NicAdded      map[string]*cmd.DistributedServiceCard
	NicUpdated    map[string]*cmd.DistributedServiceCard
	NicDeleted    map[string]*cmd.DistributedServiceCard
	Naples        *nmd.DistributedServiceCard
	IPConfig      *cmd.IPConfig
	keyMgr        *keymgr.KeyMgr
	Rollout       *protos.DSCRollout
	RolloutStatus *protos.DSCRolloutStatusUpdate
	Profiles      []*nmd.DSCProfile
}

// RegisterCMD registers CMD stubbed out
func (ag *NMD) RegisterCMD(cmd nmdAPI.CmdAPI) error {
	return nil
}

// UnRegisterCMD unregisters CMD stubbed out
func (ag *NMD) UnRegisterCMD() error {
	return nil
}

// UpdateCMDClient updates cmd client stubbed out
func (ag *NMD) UpdateCMDClient(resolvers []string) error {
	return nil
}

// GetControllerIps returns the controller IPs
func (ag *NMD) GetControllerIps() []string {
	return ag.Naples.Status.Controllers
}

// CreateMockNMD creates a mock nmd
func CreateMockNMD(name string) *NMD {
	return &NMD{
		Name:       name,
		NicAdded:   make(map[string]*cmd.DistributedServiceCard),
		NicUpdated: make(map[string]*cmd.DistributedServiceCard),
		NicDeleted: make(map[string]*cmd.DistributedServiceCard),
		Naples: &nmd.DistributedServiceCard{
			TypeMeta: api.TypeMeta{Kind: "DistributedServiceCardConfig"},
			ObjectMeta: api.ObjectMeta{
				Name: "DistributedServiceCardConfig",
			},
			Spec: nmd.DistributedServiceCardSpec{
				PrimaryMAC:  "42:42:42:42:42:42:42",
				Mode:        nmd.MgmtMode_HOST.String(),
				DSCProfile:  "default",
				Controllers: []string{"1.1.1.1"},
			},
		},
	}
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// GetDSCRolloutStatus gets the rollout status
func (ag *NMD) GetDSCRolloutStatus() (ro protos.DSCRolloutStatusUpdate, err error) {
	return
}

// CreateUpdateDSCRollout creates the rollout object
func (ag *NMD) CreateUpdateDSCRollout(sro *protos.DSCRollout) error {
	return nil
}

// DeleteDSCRollout deletes the smart nic rollout
func (ag *NMD) DeleteDSCRollout(sro *protos.DSCRollout) error {
	return nil
}

// GetAgentID returns the agent id
func (ag *NMD) GetAgentID() string {
	return "mockAgent_" + ag.Name
}

// GetVeniceIPs returns the venice coordinates
func (ag *NMD) GetVeniceIPs() []string {
	return ag.Naples.Spec.Controllers
}

// SetVeniceIPs sets venice coordinates
func (ag *NMD) SetVeniceIPs(veniceIPs []string) {

}

// CreateSmartNIC creates smart NIC
func (ag *NMD) CreateSmartNIC(n *cmd.DistributedServiceCard) error {
	ag.Lock()
	defer ag.Unlock()
	ag.NicAdded[objectKey(n.ObjectMeta)] = n
	return nil
}

// UpdateSmartNIC updates smart NIC
func (ag *NMD) UpdateSmartNIC(n *cmd.DistributedServiceCard) error {
	ag.Lock()
	defer ag.Unlock()
	ag.NicUpdated[objectKey(n.ObjectMeta)] = n
	return nil
}

// DeleteSmartNIC deltes the smart NIC
func (ag *NMD) DeleteSmartNIC(n *cmd.DistributedServiceCard) error {
	ag.Lock()
	defer ag.Unlock()
	ag.NicDeleted[objectKey(n.ObjectMeta)] = n
	return nil
}

// GetSmartNIC returns the smart NIC
func (ag *NMD) GetSmartNIC() (*cmd.DistributedServiceCard, error) {
	ag.Lock()
	defer ag.Unlock()
	return ag.Nic, nil
}

// GetNaplesConfig gets the current naples config
func (ag *NMD) GetNaplesConfig() (*nmd.DistributedServiceCard, error) {
	ag.Lock()
	defer ag.Unlock()
	return ag.Naples, nil
}

// GetNaplesRollout gets the current rollout
func (ag *NMD) GetNaplesRollout() (rollout *protos.DSCRolloutStatusUpdate, err error) {
	ag.Lock()
	defer ag.Unlock()
	return ag.RolloutStatus, nil
}

// CreateNaplesRollout creates naples rollout
func (ag *NMD) CreateNaplesRollout(rollout *protos.DSCRollout) (err error) {
	ag.Lock()
	defer ag.Unlock()
	ag.Rollout = rollout
	return nil
}

// GetPrimaryMAC gets the primary mac
func (ag *NMD) GetPrimaryMAC() string {
	ag.Lock()
	defer ag.Unlock()
	return ag.Naples.Spec.ID
}

// DeleteNaplesRollout deletes naples rollout
func (ag *NMD) DeleteNaplesRollout(rollout *protos.DSCRollout) (err error) {
	ag.Lock()
	defer ag.Unlock()
	ag.Rollout = nil
	return nil
}

// GetNaplesProfiles gets naples profiles
func (ag *NMD) GetNaplesProfiles() (profiles []*nmd.DSCProfile, err error) {
	ag.Lock()
	defer ag.Unlock()
	return ag.Profiles, nil
}

// CreateNaplesProfile create naples profiles
func (ag *NMD) CreateNaplesProfile(profile *nmd.DSCProfile) (err error) {
	ag.Lock()
	defer ag.Unlock()
	ag.Profiles = append(ag.Profiles, profile)
	return nil
}

// AdmitNaples admits naples
func (ag *NMD) AdmitNaples() error {
	return nil
}

// UpdateNaplesProfile updates naples profile
func (ag *NMD) UpdateNaplesProfile(profile *nmd.DSCProfile) (err error) {
	ag.Lock()
	defer ag.Unlock()
	var found bool
	for idx, p := range ag.Profiles {
		if p.Name == profile.Name {
			found = true
			ag.Profiles[idx] = profile
			break
		}
	}

	if found {
		err = nil
		return
	}

	return
}

// DeleteNaplesProfile deletes naples profile
func (ag *NMD) DeleteNaplesProfile(profileName string) (err error) {
	ag.Lock()
	defer ag.Unlock()
	var found bool
	for idx, p := range ag.Profiles {
		if p.Name == profileName {
			found = true
			ag.Profiles[idx] = ag.Profiles[len(ag.Profiles)-1]
			ag.Profiles = ag.Profiles[:len(ag.Profiles)-1]
			break
		}
	}

	if found {
		err = nil
		return
	}

	return
}

// UpdateNaplesConfig updates naples config
func (ag *NMD) UpdateNaplesConfig(naples *nmd.DistributedServiceCard) error {
	ag.Lock()
	defer ag.Unlock()
	ag.Naples = naples
	return nil
}

// GetIPConfig gets the current IP Config
func (ag *NMD) GetIPConfig() *cmd.IPConfig {
	return ag.IPConfig
}

// SetIPConfig sets the current IPConfig
func (ag *NMD) SetIPConfig(cfg *cmd.IPConfig) {
	ag.IPConfig = cfg
}

// SetSmartNIC sets the smart nic
func (ag *NMD) SetSmartNIC(nic *cmd.DistributedServiceCard) error {
	ag.Lock()
	defer ag.Unlock()
	ag.Nic = nic
	return nil
}

// GenClusterKeyPair generates the cluster key pair
func (ag *NMD) GenClusterKeyPair() (*keymgr.KeyPair, error) {
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	return keymgr.NewKeyPairObject("mock-agent-key", key), err
}

// GetPlatformCertificate gets the platform certificate
func (ag *NMD) GetPlatformCertificate(nic *cmd.DistributedServiceCard) ([]byte, error) {
	return nil, nil
}

// GenChallengeResponse generates a challenge response
func (ag *NMD) GenChallengeResponse(nic *cmd.DistributedServiceCard, challenge []byte) ([]byte, []byte, error) {
	return nil, nil, nil
}

// PersistState persists Naples object
func (ag *NMD) PersistState(updateDelphi bool) error {
	return nil
}

// SyncDHCPState syncs the DHCP information received with NMD
func (ag *NMD) SyncDHCPState() {

}
