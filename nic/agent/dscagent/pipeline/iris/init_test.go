package iris

import (
	"errors"
	"io/ioutil"
	"math"
	"net"
	"os"
	"testing"

	"github.com/mdlayher/arp"
	"github.com/pensando/netlink"

	"github.com/pensando/sw/venice/utils/emstore"

	"github.com/pensando/sw/nic/agent/dscagent/infra"

	"github.com/pensando/sw/nic/agent/dscagent/types"

	"google.golang.org/grpc"

	"github.com/pensando/sw/nic/agent/cmd/fakehal/hal"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

var (
	infraAPI        types.InfraAPI
	mockHal         *fakehal.Hal
	lis             netutils.TestListenAddr
	epClient        halapi.EndpointClient
	intfClient      halapi.InterfaceClient
	l2SegClient     halapi.L2SegmentClient
	nwSecClient     halapi.NwSecurityClient
	vrfClient       halapi.VrfClient
	telemetryClient halapi.TelemetryClient
	systemClient    halapi.SystemClient
)

// Implements InfraAPI that return errors for testing
type badInfraAPI struct{}

func findMgmtIP(destIP string) (mgmtIP string, mgmtLink netlink.Link, mgmtIntf *net.Interface, err error) {
	rts, err := netlink.RouteGet(net.ParseIP(destIP))
	if err != nil {
		return
	}
	// Associate mgmt intf to the default route
	mgmtLink, err = netlink.LinkByIndex(rts[0].LinkIndex)
	if err != nil {
		return
	}
	addrs, err := netlink.AddrList(mgmtLink, netlink.FAMILY_V4)
	if err != nil {
		return
	}
	mgmtIP = addrs[0].IP.String()
	mgmtIntf, err = net.InterfaceByName(mgmtLink.Attrs().Name)
	return
}

// Sets up the grpc client handlers for the package
func TestMain(m *testing.M) {
	primaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	secondaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	infraAPI, err = infra.NewInfraAPI(primaryDB.Name(), secondaryDB.Name())
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	// Set initial bootstrap config.
	fakeCfg := types.DistributedServiceCardStatus{
		DSCMode:     "network_managed_inband",
		MgmtIP:      "42.42.42.42/24",
		Controllers: []string{"4.4.4.4"},
		MgmtIntf:    "mock",
	}
	infraAPI.StoreConfig(fakeCfg)

	_ = lis.GetAvailablePort()
	mockHal = fakehal.NewFakeHalServer(lis.ListenURL.String())
	conn, err := grpc.Dial(lis.ListenURL.String(), grpc.WithMaxMsgSize(math.MaxInt32-1), grpc.WithInsecure())
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		mockHal.Stop()
		os.Exit(1)
	}

	_, mgmtLink, mgmtIntf, err := findMgmtIP("42.42.42.42")
	if err != nil {
		log.Errorf("Failed to find Mgmt IP %v", err)
		mockHal.Stop()
		os.Exit(1)
	}

	client, err := arp.Dial(mgmtIntf)
	if err != nil {
		log.Error("Failed to initiate an ARP client")
		mockHal.Stop()
		os.Exit(1)
	}
	ArpClient = client
	MgmtLink = mgmtLink
	go ResolveWatch()

	epClient = halapi.NewEndpointClient(conn)
	intfClient = halapi.NewInterfaceClient(conn)
	l2SegClient = halapi.NewL2SegmentClient(conn)
	nwSecClient = halapi.NewNwSecurityClient(conn)
	vrfClient = halapi.NewVrfClient(conn)
	telemetryClient = halapi.NewTelemetryClient(conn)
	systemClient = halapi.NewSystemClient(conn)
	code := m.Run()
	mockHal.Stop()
	infraAPI.Close()
	os.Remove(primaryDB.Name())
	os.Remove(secondaryDB.Name())
	os.Exit(code)
}

// newBadInfraAPI returns a new instance of InfraAPI
func newBadInfraAPI() *badInfraAPI {
	i := badInfraAPI{}
	return &i
}

// List returns a list of objects by kind
func (i *badInfraAPI) List(kind string) ([][]byte, error) {
	return nil, errors.New("failed to list")
}

// Read returns a specific object based on key and kind
func (i *badInfraAPI) Read(kind, key string) ([]byte, error) {
	return nil, errors.New("failed to read")
}

// Store stores a specific object based on key and kind
func (i *badInfraAPI) Store(kind, key string, data []byte) error {
	return errors.New("failed to store")
}

// Delete deletes a specific object based on key and kind
func (i *badInfraAPI) Delete(kind, key string) error {
	return errors.New("failed to delete")
}

// AllocateID allocates a uint64 ID based on an offset.
func (i *badInfraAPI) AllocateID(kind emstore.ResourceType, offset int) uint64 {
	return 0
}

// getDscName returns DSC Name
func (i *badInfraAPI) GetDscName() string {
	return "dsc"
}

// StoreConfig updates config
func (i *badInfraAPI) StoreConfig(cfg types.DistributedServiceCardStatus) {

}

// NotifyVeniceConnection notifies venice connection
func (i *badInfraAPI) NotifyVeniceConnection() {

}

// Purge stubbed out
func (i *badInfraAPI) Purge() {

}

func (i *badInfraAPI) GetConfig() (cfg types.DistributedServiceCardStatus) {
	return types.DistributedServiceCardStatus{
		DSCMode:     "",
		DSCName:     "",
		MgmtIP:      "",
		Controllers: nil,
	}
}

// UpdateIfChannel updates the intf update channel
func (i *badInfraAPI) UpdateIfChannel(evt types.UpdateIfEvent) {
}

// IfUpdateChannel returns a channel for propogating interface state to the netagent
func (i *badInfraAPI) IfUpdateChannel() chan types.UpdateIfEvent {
	return nil
}

func (i *badInfraAPI) Close() error {
	return errors.New("failed to close")
}
