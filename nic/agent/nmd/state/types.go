// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"path"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	nmdapi "github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	"github.com/pensando/sw/nic/agent/nmd/protos/halproto"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	roprotos "github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certsproxy"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
)

const (
	nicRegistrationWaitTime = time.Duration(time.Minute * 5)
	clusterTrustRootsFile   = "/tmp/clusterTrustRoots.pem"
)

// NMD is the Naples management daemon instance object
type NMD struct {
	sync.Mutex     // Lock for NMD object
	sync.WaitGroup // Wait group

	store          emstore.Emstore       // Embedded DB
	nodeUUID       string                // Node's UUID
	macAddr        string                // Primary MAC addr
	cmdRegURL      string                // The URL for the CMD registration API
	cmd            nmdapi.CmdAPI         // CMD API object
	platform       nmdapi.PlatformAPI    // Platform Agent object
	rollout        nmdapi.RolloutCtrlAPI // Rollout API Object
	upgmgr         nmdapi.UpgMgrAPI      // Upgrade Manager API
	resolverClient resolver.Interface    // Resolver client instance

	config       nmd.Naples    // Naples config received via REST
	nic          *cmd.SmartNIC // SmartNIC object
	DelphiClient clientAPI.Client
	IPClient     *IPClient

	stopNICReg         chan bool     // channel to stop NIC registration
	nicRegInitInterval time.Duration // the initial time interval between nic registration in seconds
	nicRegInterval     time.Duration // time interval between nic registration in seconds. Gets adjusted dynamically with exponential backoff
	isRegOngoing       bool          // status of ongoing nic registration task

	stopNICUpd     chan bool     // channel to stop NIC update
	nicUpdInterval time.Duration // time interval between nic updates in seconds
	isUpdOngoing   bool          // status of ongoing nic update task

	listenURL        string       // URL where http server is listening
	listener         net.Listener // socket listener
	httpServer       *http.Server // HTTP server
	isRestSrvRunning bool         // status of the REST server

	certsListenURL string                            // URL where local processes can request certificates
	remoteCertsURL string                            // URL where local process cert request are forwarder
	certsProxy     *certsproxy.CertsProxy            // the CertsProxy instance
	tlsProvider    *tlsproviders.KeyMgrBasedProvider // TLS provider holding cluster keys
	// Rollout related stuff
	completedOps  map[roprotos.SmartNICOpSpec]bool // the ops that were requested by spec and got completed
	inProgressOps *roprotos.SmartNICOpSpec         // the ops thats currently in progress
	pendingOps    []roprotos.SmartNICOpSpec        // the ops that will be executed after the current op is completed
	opStatus      []roprotos.SmartNICOpStatus
	objectMeta    api.ObjectMeta
	profiles      []*nmd.NaplesProfile
}

// NaplesConfigResp is response to NaplesConfig request nmd.Naples
type NaplesConfigResp struct {
	ErrorMsg string
}

// Setters and Getters for NMD attributes
// Note: Locks are in place for these setter and getters
// to prevent data races reported by go race detector.

// GetRegStatus returns the current status of NIC registration task
func (n *NMD) GetRegStatus() bool {
	n.Lock()
	defer n.Unlock()
	return n.isRegOngoing
}

func (n *NMD) setRegStatus(value bool) {
	n.Lock()
	defer n.Unlock()
	n.isRegOngoing = value
}

// GetUpdStatus returns the current running status of NIC update task
func (n *NMD) GetUpdStatus() bool {
	n.Lock()
	defer n.Unlock()
	return n.isUpdOngoing
}

func (n *NMD) setUpdStatus(value bool) {
	n.Lock()
	defer n.Unlock()
	n.isUpdOngoing = value
}

// GetRestServerStatus returns the current running status of REST server
func (n *NMD) GetRestServerStatus() bool {
	n.Lock()
	defer n.Unlock()
	return n.isRestSrvRunning
}

func (n *NMD) setRestServerStatus(value bool) {
	n.Lock()
	defer n.Unlock()
	n.isRestSrvRunning = value
}

// GetConfigMode returns the configured Naples Mode
func (n *NMD) GetConfigMode() string {
	n.Lock()
	defer n.Unlock()
	return n.config.Spec.Mode
}

// SetNaplesConfig sets naples config and status
func (n *NMD) SetNaplesConfig(cfgSpec nmd.NaplesSpec) {
	n.Lock()
	defer n.Unlock()
	c, _ := types.TimestampProto(time.Now())

	ts := api.Timestamp{
		Timestamp: *c,
	}
	n.config.ModTime = ts
	n.config.Spec = cfgSpec
}

// GetNaplesConfig returns the current naples config received via REST
func (n *NMD) GetNaplesConfig() nmd.Naples {
	n.Lock()
	defer n.Unlock()

	return n.config
}

// SetSmartNIC intializes the smartNIC object
func (n *NMD) SetSmartNIC(nic *cmd.SmartNIC) error {
	n.Lock()
	defer n.Unlock()

	n.nic = nic
	return nil
}

// GetSmartNIC returns the smartNIC object
func (n *NMD) GetSmartNIC() (*cmd.SmartNIC, error) {
	n.Lock()
	defer n.Unlock()

	return n.nic, nil
}

// GetListenURL returns the listen URL of the http server
func (n *NMD) GetListenURL() string {
	n.Lock()
	defer n.Unlock()

	if n.listener != nil {
		return n.listener.Addr().String()
	}
	return ""
}

// CreateIPClient creates IPClient to run DHCP
func (n *NMD) CreateIPClient(delphiClient clientAPI.Client) {
	n.Lock()
	defer n.Unlock()

	n.IPClient = NewIPClient(false, n, delphiClient)
}

// CreateMockIPClient creates IPClient in Mock mode to run in venice integ environment
func (n *NMD) CreateMockIPClient(delphiClient clientAPI.Client) {
	n.Lock()
	defer n.Unlock()

	n.IPClient = NewIPClient(true, n, delphiClient)
}

// GetIPClient returns the handle to the ip client
func (n *NMD) GetIPClient() *IPClient {
	n.Lock()
	defer n.Unlock()

	return n.IPClient
}

//PersistHALConfiguration updates feature profile
func (n *NMD) PersistHALConfiguration(profileName string) (err error) {
	if n.config.Spec.Mode == nmd.MgmtMode_HOST.String() {
		fwdMode := device.ForwardingMode_FORWARDING_MODE_CLASSIC
		var featureProfile device.FeatureProfile
		var profile *nmd.NaplesProfile
		var ok bool
		log.Info("Updating feature profile.")
		// Check if the profile exists.
		for _, p := range n.profiles {
			if p.Name == profileName {
				profile = p
				ok = true
				break
			}
		}

		if !ok {
			log.Errorf("could not find profile %v in nmd state, profs: %+v", profileName, n.profiles)
			err = fmt.Errorf("could not find profile %v in nmd state", profileName)
			return
		}

		// Interpret 16 numLifs as scale profile. TODO Remove this when nicmgr can directly read numLifs from device.conf
		if profile.Spec.NumLifs == 16 {
			featureProfile = device.FeatureProfile_FEATURE_PROFILE_CLASSIC_ETH_DEV_SCALE
		} else {
			featureProfile = device.FeatureProfile_FEATURE_PROFILE_CLASSIC_DEFAULT
		}

		err = n.PersistDeviceSpec(fwdMode, featureProfile)
		return
	}

	err = n.PersistDeviceSpec(device.ForwardingMode_FORWARDING_MODE_HOSTPIN, device.FeatureProfile_FEATURE_PROFILE_NONE)
	return
}

// UpdateMgmtIP updates the management IP
func (n *NMD) UpdateMgmtIP() error {
	log.Info("Update Mgmt IP Called.")
	//n.Lock()
	//defer n.Unlock()

	log.Infof("NaplesConfig: %v", n.config)
	return n.IPClient.Update()
}

// GetCMDSmartNICWatcherStatus returns true if the NMD CMD interface has an active watch
func (n *NMD) GetCMDSmartNICWatcherStatus() bool {
	return n.cmd != nil && n.cmd.IsSmartNICWatcherRunning()
}

// GetRoSmartNICWatcherStatus returns true if the NMD rollout interface has an active watch
func (n *NMD) GetRoSmartNICWatcherStatus() bool {
	return n.rollout != nil && n.rollout.IsSmartNICWatcherRunning()
}

// UpdateCurrentManagementMode fixes the currently active Management mode
func (n *NMD) UpdateCurrentManagementMode() {
	appStartConfFilePath := fmt.Sprintf("%s/app-start.conf", path.Dir(globals.NaplesModeConfigFile))
	if appStartSpec, err := ioutil.ReadFile(appStartConfFilePath); err != nil {
		n.config.Status.Mode = nmd.MgmtMode_HOST.String()
	} else {
		if string(appStartSpec) == "classic" {
			n.config.Status.Mode = nmd.MgmtMode_HOST.String()
		} else {
			n.config.Status.Mode = nmd.MgmtMode_NETWORK.String()
		}
	}
}

// PersistDeviceSpec accepts forwarding mode and feature profile and persists this in device.conf
func (n *NMD) PersistDeviceSpec(fwdMode device.ForwardingMode, featureProfile device.FeatureProfile) (err error) {
	log.Infof("Setting forwarding mode to : %v", fwdMode)
	deviceSpec := device.SystemSpec{
		FwdMode:        fwdMode,
		FeatureProfile: featureProfile,
	}

	// Create the /sysconfig/config0 if it doesn't exist. Needed for non naples nmd test environments
	if _, err := os.Stat(globals.NaplesModeConfigFile); os.IsNotExist(err) {
		os.MkdirAll(path.Dir(globals.NaplesModeConfigFile), 0664)
	}

	data, err := json.MarshalIndent(deviceSpec, "", "  ")
	if err != nil {
		log.Errorf("Failed to marshal device spec. Err: %v", err)
		return err
	}

	if err = ioutil.WriteFile(globals.NaplesModeConfigFile, data, 0664); err != nil {
		log.Errorf("Failed to write feature profile to %s. Err: %v", globals.NaplesModeConfigFile, err)
	}

	// Update app-start.conf file. TODO Remove this workaround when all the processes are migrated to read from device.conf
	var appStartSpec []byte
	switch fwdMode {
	case device.ForwardingMode_FORWARDING_MODE_HOSTPIN:
		appStartSpec = []byte("hostpin")
	default:
		appStartSpec = []byte("classic")
	}

	log.Infof("App Start Spec is : %v", appStartSpec)
	appStartConfFilePath := fmt.Sprintf("%s/app-start.conf", path.Dir(globals.NaplesModeConfigFile))
	if err := ioutil.WriteFile(appStartConfFilePath, appStartSpec, 0755); err != nil {
		log.Errorf("Failed to write app start conf. Err: %v", err)
	}

	return
}

// ReadFruFromJSON reads the fru.json file created at Startup
func ReadFruFromJSON() *nmd.NaplesFru {
	log.Infof("Updating FRU from /tmp/fru.json")
	fruJSON, err := os.Open("/tmp/fru.json")
	if err != nil {
		log.Errorf("Failed to open /tmp/fru.json.")
		return nil
	}
	defer fruJSON.Close()
	var dat map[string]interface{}

	byt, err := ioutil.ReadFile("/tmp/fru.json")
	if err != nil {
		log.Errorf("Failed to read contents of fru.json")
		return nil
	}

	if err := json.Unmarshal(byt, &dat); err != nil {
		log.Errorf("Failed to unmarshal fru.json.")
		return nil
	}

	return &nmd.NaplesFru{
		ManufacturingDate: dat["Manufacturing date"].(string),
		Manufacturer:      dat["Manufacturer"].(string),
		ProductName:       dat["Product Name"].(string),
		SerialNum:         dat["Serial Number"].(string),
		PartNum:           dat["Part Number"].(string),
		BoardId:           dat["Engineering Change level"].(string),
		EngChangeLevel:    dat["Board Id Number"].(string),
		NumMacAddr:        dat["NumMac Address"].(string),
		MacStr:            dat["Mac Address"].(string),
	}
}
