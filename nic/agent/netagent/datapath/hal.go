// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package datapath

import (
	"context"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"os"
	"regexp"
	"strconv"
	"strings"
	"sync"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	halGRPCDefaultBaseURL = "localhost"
	halGRPCDefaultPort    = "50054"
)

// ErrHALNotOK is returned by HAL gRPC Server on failed requests
var ErrHALNotOK = errors.New("hal returned non zero error code")

// ErrIPParse is returned on failing to parse hyphen separated IP Range.
var ErrIPParse = errors.New("hal datapath could not parse the IP")

// ErrInvalidMatchType is returned on an invalid match type
var ErrInvalidMatchType = errors.New("invalid match selector type")

// ErrInvalidNatActionType is returned on an invalid NAT Action
var ErrInvalidNatActionType = errors.New("invalid NAT Action Type")

// ErrInvalidIPSecSAType is returned on an invalid IPSec Policy SA Action
var ErrInvalidIPSecSAType = errors.New("invalid IPSec SA Action")

// ErrIPMissing is returned when the IP field is not specified in an Encrypt SA Action
var ErrIPMissing = errors.New("ipsec encrypt SA needs local and remote gateway IP")

// Kind holds the HAL Datapath kind. It could either be mock HAL or real HAL.
type Kind string

// Hal holds clients to talk to HAL gRPC server. ToDo Remove mock clients, datapath DB and expectations prior to FCS
type Hal struct {
	client      *rpckit.RPCClient
	mockCtrl    *gomock.Controller
	MockClients mockClients
	Epclient    halproto.EndpointClient
	Ifclient    halproto.InterfaceClient
	L2SegClient halproto.L2SegmentClient
	Netclient   halproto.NetworkClient
	Lbclient    halproto.L4LbClient
	Sgclient    halproto.NwSecurityClient
	Sessclient  halproto.SessionClient
	Tnclient    halproto.VrfClient
	Natclient   halproto.NatClient
	IPSecclient halproto.IpsecClient
}

// MockClients stores references for mockclients to be used for setting expectations
type mockClients struct {
	MockEpclient    *halproto.MockEndpointClient
	MockIfclient    *halproto.MockInterfaceClient
	MockL2Segclient *halproto.MockL2SegmentClient
	MockNetClient   *halproto.MockNetworkClient
	MockLbclient    *halproto.MockL4LbClient
	MockSgclient    *halproto.MockNwSecurityClient
	MockSessclient  *halproto.MockSessionClient
	MockTnclient    *halproto.MockVrfClient
	MockNatClient   *halproto.MockNatClient
	MockIPSecClient *halproto.MockIpsecClient
}

// DB holds all the state information.
type DB struct {
	EndpointDB       map[string]*halproto.EndpointRequestMsg
	EndpointUpdateDB map[string]*halproto.EndpointUpdateRequestMsg
	EndpointDelDB    map[string]*halproto.EndpointDeleteRequestMsg
	SgDB             map[string]*halproto.SecurityGroupRequestMsg
	TenantDB         map[string]*halproto.VrfRequestMsg
	TenantDelDB      map[string]*halproto.VrfDeleteRequestMsg
	SgPolicyDB       map[string]*halproto.SecurityGroupPolicyRequestMsg
	InterfaceDB      map[string]*halproto.InterfaceRequestMsg
	LifDB            map[string]*halproto.LifRequestMsg
	InterfaceDelDB   map[string]*halproto.InterfaceDeleteRequestMsg
	LifDelDB         map[string]*halproto.LifDeleteRequestMsg
}

// Datapath contains mock and hal clients.
type Datapath struct {
	sync.Mutex
	Kind Kind
	Hal  Hal
	DB   DB
}

// String returns string value of the datapath kind
func (k *Kind) String() string {
	return string(*k)
}

// Errorf for satisfying gomock
func (hd *Hal) Errorf(format string, args ...interface{}) {
	log.Errorf(format, args...)
}

// Fatalf for satisfying gomock
func (hd *Hal) Fatalf(format string, args ...interface{}) {
	log.Fatalf(format, args...)
}

// NewHalDatapath returns a mock hal datapath
func NewHalDatapath(kind Kind) (*Datapath, error) {
	var err error
	var hal Hal
	haldp := Datapath{}
	haldp.Kind = kind

	db := DB{EndpointDB: make(map[string]*halproto.EndpointRequestMsg),
		EndpointUpdateDB: make(map[string]*halproto.EndpointUpdateRequestMsg),
		EndpointDelDB:    make(map[string]*halproto.EndpointDeleteRequestMsg),
		SgDB:             make(map[string]*halproto.SecurityGroupRequestMsg),
		TenantDB:         make(map[string]*halproto.VrfRequestMsg),
		TenantDelDB:      make(map[string]*halproto.VrfDeleteRequestMsg),
		SgPolicyDB:       make(map[string]*halproto.SecurityGroupPolicyRequestMsg),
		InterfaceDB:      make(map[string]*halproto.InterfaceRequestMsg),
		LifDB:            make(map[string]*halproto.LifRequestMsg),
		InterfaceDelDB:   make(map[string]*halproto.InterfaceDeleteRequestMsg),
		LifDelDB:         make(map[string]*halproto.LifDeleteRequestMsg),
	}
	haldp.DB = db
	if haldp.Kind.String() == "hal" {
		hal.client, err = hal.createNewGRPCClient()
		if err != nil {
			return nil, err
		}
		hal.Epclient = halproto.NewEndpointClient(hal.client.ClientConn)
		hal.Ifclient = halproto.NewInterfaceClient(hal.client.ClientConn)
		hal.L2SegClient = halproto.NewL2SegmentClient(hal.client.ClientConn)
		hal.Netclient = halproto.NewNetworkClient(hal.client.ClientConn)
		hal.Lbclient = halproto.NewL4LbClient(hal.client.ClientConn)
		hal.Sgclient = halproto.NewNwSecurityClient(hal.client.ClientConn)
		hal.Sessclient = halproto.NewSessionClient(hal.client.ClientConn)
		hal.Tnclient = halproto.NewVrfClient(hal.client.ClientConn)
		hal.Natclient = halproto.NewNatClient(hal.client.ClientConn)
		hal.IPSecclient = halproto.NewIpsecClient(hal.client.ClientConn)
		haldp.Hal = hal
		return &haldp, nil
	}
	hal.mockCtrl = gomock.NewController(&hal)
	hal.MockClients = mockClients{
		MockEpclient:    halproto.NewMockEndpointClient(hal.mockCtrl),
		MockIfclient:    halproto.NewMockInterfaceClient(hal.mockCtrl),
		MockL2Segclient: halproto.NewMockL2SegmentClient(hal.mockCtrl),
		MockNetClient:   halproto.NewMockNetworkClient(hal.mockCtrl),
		MockLbclient:    halproto.NewMockL4LbClient(hal.mockCtrl),
		MockSgclient:    halproto.NewMockNwSecurityClient(hal.mockCtrl),
		MockSessclient:  halproto.NewMockSessionClient(hal.mockCtrl),
		MockTnclient:    halproto.NewMockVrfClient(hal.mockCtrl),
		MockNatClient:   halproto.NewMockNatClient(hal.mockCtrl),
		MockIPSecClient: halproto.NewMockIpsecClient(hal.mockCtrl),
	}

	hal.Epclient = hal.MockClients.MockEpclient
	hal.Ifclient = hal.MockClients.MockIfclient
	hal.L2SegClient = hal.MockClients.MockL2Segclient
	hal.Netclient = hal.MockClients.MockNetClient
	hal.Lbclient = hal.MockClients.MockLbclient
	hal.Sgclient = hal.MockClients.MockSgclient
	hal.Sessclient = hal.MockClients.MockSessclient
	hal.Tnclient = hal.MockClients.MockTnclient
	hal.Natclient = hal.MockClients.MockNatClient
	hal.IPSecclient = hal.MockClients.MockIPSecClient
	haldp.Hal = hal
	haldp.Hal.setExpectations()
	return &haldp, nil
}

func (hd *Hal) setExpectations() {
	hd.MockClients.MockEpclient.EXPECT().EndpointCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockEpclient.EXPECT().EndpointUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockEpclient.EXPECT().EndpointDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIfclient.EXPECT().InterfaceGet(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().AddL2SegmentOnUplink(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().InterfaceCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().InterfaceUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().InterfaceDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIfclient.EXPECT().LifGet(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().LifCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().LifUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIfclient.EXPECT().LifDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockL2Segclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockL2Segclient.EXPECT().L2SegmentUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockL2Segclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNetClient.EXPECT().NetworkCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().NetworkUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().NetworkDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNetClient.EXPECT().RouteCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().RouteUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().RouteDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNetClient.EXPECT().NexthopCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().NexthopUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNetClient.EXPECT().NexthopDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockSgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockSgclient.EXPECT().SecurityGroupUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockSgclient.EXPECT().SecurityGroupDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockTnclient.EXPECT().VrfUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockTnclient.EXPECT().VrfDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNatClient.EXPECT().NatPoolCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatPoolUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatPoolDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNatClient.EXPECT().NatPolicyCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatPolicyUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatPolicyDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockNatClient.EXPECT().NatMappingCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockNatClient.EXPECT().NatMappingDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIPSecClient.EXPECT().IpsecRuleCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecRuleUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecRuleDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIPSecClient.EXPECT().IpsecSAEncryptCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecSAEncryptUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecSAEncryptDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)

	hd.MockClients.MockIPSecClient.EXPECT().IpsecSADecryptCreate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecSADecryptUpdate(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
	hd.MockClients.MockIPSecClient.EXPECT().IpsecSADecryptDelete(gomock.Any(), gomock.Any()).AnyTimes().Return(nil, nil)
}

func (hd *Hal) createNewGRPCClient() (*rpckit.RPCClient, error) {
	halPort := os.Getenv("HAL_GRPC_PORT")
	if halPort == "" {
		halPort = halGRPCDefaultPort
	}
	srvURL := halGRPCDefaultBaseURL + ":" + halPort
	// create a grpc client
	// ToDo Use AgentID for mysvcName
	rpcClient, err := rpckit.NewRPCClient("hal", srvURL, rpckit.WithTLSProvider(nil))
	if err != nil {
		log.Errorf("Creating gRPC Client failed on HAL Datapath. Server URL: %s", srvURL)
		return nil, err
	}

	return rpcClient, err
}

// objectKey returns object key from meta
func objectKey(ometa *api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", ometa.Tenant, ometa.Name)
}

// utility functions to convert IPv4 address to int and vice versa
func ipv42int(ip net.IP) uint32 {
	if len(ip) == 16 {
		return binary.BigEndian.Uint32(ip[12:])
	}

	return binary.BigEndian.Uint32(ip)
}

// FindEndpoint finds an endpoint in datapath
// used for testing mostly..
func (hd *Datapath) FindEndpoint(epKey string) (*halproto.EndpointRequestMsg, error) {
	hd.Lock()
	epr, ok := hd.DB.EndpointDB[epKey]
	hd.Unlock()
	if !ok {
		return nil, errors.New("Endpoint not found")
	}

	return epr, nil
}

// FindEndpointDel finds an endpoint delete record
// used for testing mostly
func (hd *Datapath) FindEndpointDel(epKey string) (*halproto.EndpointDeleteRequestMsg, error) {
	hd.Lock()
	epdr, ok := hd.DB.EndpointDelDB[epKey]
	hd.Unlock()
	if !ok {
		return nil, errors.New("Endpoint delete record not found")
	}

	return epdr, nil
}

// GetEndpointCount returns number of endpoints in db
func (hd *Datapath) GetEndpointCount() int {
	hd.Lock()
	defer hd.Unlock()
	return len(hd.DB.EndpointDB)
}

// SetAgent sets the agent for this datapath
func (hd *Datapath) SetAgent(ag types.DatapathIntf) error {
	return nil
}

// CreateLocalEndpoint creates an endpoint
func (hd *Datapath) CreateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) (*types.IntfInfo, error) {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
	ipaddr, _, err := net.ParseCIDR(ep.Status.IPv4Address)
	if err != nil {
		log.Errorf("CIDR Parsing failed: %v. Error: %v", ep.Status.IPv4Address, err)
	}
	// convert v4 address
	v4Addr := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// convert v6 address
	v6Addr := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET6,
		V4OrV6: &halproto.IPAddress_V6Addr{
			V6Addr: []byte(net.ParseIP(ep.Status.IPv6Address)),
		},
	}

	// get sg ids
	var sgHandles []*halproto.SecurityGroupKeyHandle
	for _, sg := range sgs {
		sgKey := halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupHandle{
				SecurityGroupHandle: sg.Status.SecurityGroupID,
			},
		}
		sgHandles = append(sgHandles, &sgKey)
	}

	l2Handle := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_L2SegmentHandle{
			L2SegmentHandle: nw.Status.NetworkHandle,
		},
	}

	ifKeyHandle := halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_IfHandle{
			IfHandle: 0, //FIXME
		},
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: &ifKeyHandle, //FIXME
		UsegVlan:           ep.Status.UsegVlan,
		IpAddress:          []*halproto.IPAddress{&v4Addr, &v6Addr},
		SgKeyHandle:        sgHandles,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Handle,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	// build endpoint message
	epinfo := halproto.EndpointSpec{
		KeyOrHandle:   &epHandle,
		Meta:          &halproto.ObjectMeta{},
		EndpointAttrs: &epAttrs,
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// call hal to create the endpoint
	// FIXME: handle response
	_, err = hd.Hal.Epclient.EndpointCreate(context.Background(), &epReq)
	if err != nil {
		log.Errorf("Error creating endpoint. Err: %v", err)
		return nil, err
	}
	// save the endpoint message
	hd.Lock()
	hd.DB.EndpointDB[objectKey(&ep.ObjectMeta)] = &epReq
	hd.Unlock()

	return nil, nil
}

// CreateRemoteEndpoint creates remote endpoint
func (hd *Datapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup, uplink *netproto.Interface, ns *netproto.Namespace) error {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
	ipaddr, _, err := net.ParseCIDR(ep.Status.IPv4Address)

	if err != nil {
		return fmt.Errorf("ipv4 address for endpoint creates should be in CIDR format")
	}

	// convert v4 address
	v4Addr := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// get sg ids
	var sgHandles []*halproto.SecurityGroupKeyHandle
	for _, sg := range sgs {
		sgKey := halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupHandle{
				SecurityGroupHandle: sg.Status.SecurityGroupID,
			},
		}
		sgHandles = append(sgHandles, &sgKey)
	}

	l2Key := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
			SegmentId: nw.Status.NetworkID,
		},
	}

	ifKey := halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
			InterfaceId: uplink.Status.InterfaceID,
		},
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: &ifKey,
		UsegVlan:           ep.Status.UsegVlan,
		IpAddress:          []*halproto.IPAddress{&v4Addr},
		SgKeyHandle:        sgHandles,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Key,
						MacAddress:         macaddr,
					},
				},
			},
		},
	}

	// build endpoint message
	epinfo := halproto.EndpointSpec{
		Meta:          &halproto.ObjectMeta{},
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
		VrfKeyHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: ns.Status.NamespaceID,
			},
		},
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// call hal to create the endpoint
	// FIXME: handle response
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Epclient.EndpointCreate(context.Background(), &epReq)
		if err != nil {
			log.Errorf("Error creating endpoint. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("Error creating endpoint. Err: %v", err)
			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Epclient.EndpointCreate(context.Background(), &epReq)
		if err != nil {
			log.Errorf("Error creating endpoint. Err: %v", err)
			return err
		}
	}

	// save the endpoint message
	hd.Lock()
	hd.DB.EndpointDB[objectKey(&ep.ObjectMeta)] = &epReq
	hd.Unlock()

	return nil
}

// UpdateLocalEndpoint updates the endpoint
func (hd *Datapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// convert mac address
	ipaddr, _, _ := net.ParseCIDR(ep.Status.IPv4Address)

	// convert v4 address
	v4Addr := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// convert v6 address
	v6Addr := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET6,
		V4OrV6: &halproto.IPAddress_V6Addr{
			V6Addr: []byte(net.ParseIP(ep.Status.IPv6Address)),
		},
	}

	// get sg ids
	var sgHandles []*halproto.SecurityGroupKeyHandle
	for _, sg := range sgs {
		sgKey := halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupHandle{
				SecurityGroupHandle: sg.Status.SecurityGroupID,
			},
		}
		sgHandles = append(sgHandles, &sgKey)
	}

	l2Handle := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_L2SegmentHandle{
			L2SegmentHandle: nw.Status.NetworkHandle,
		},
	}

	ifKeyHandle := halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_IfHandle{
			IfHandle: 0, //FIXME
		},
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: &ifKeyHandle, //FIXME
		UsegVlan:           ep.Status.UsegVlan,
		IpAddress:          []*halproto.IPAddress{&v4Addr, &v6Addr},
		SgKeyHandle:        sgHandles,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Handle,
					},
				},
			},
		},
	}

	// build endpoint message
	epUpdateReq := halproto.EndpointUpdateRequest{
		Meta:          &halproto.ObjectMeta{},
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
	}

	epUpdateReqMsg := halproto.EndpointUpdateRequestMsg{
		Request: []*halproto.EndpointUpdateRequest{&epUpdateReq},
	}

	// call hal to update the endpoint
	// FIXME: handle response

	_, err := hd.Hal.Epclient.EndpointUpdate(context.Background(), &epUpdateReqMsg)
	if err != nil {
		log.Errorf("Error updating endpoint. Err: %v", err)
		return err
	}
	// save the endpoint message
	hd.Lock()
	hd.DB.EndpointUpdateDB[objectKey(&ep.ObjectMeta)] = &epUpdateReqMsg
	hd.Unlock()

	return nil
}

// UpdateRemoteEndpoint updates an existing endpoint
func (hd *Datapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// convert mac address
	ipaddr, _, _ := net.ParseCIDR(ep.Status.IPv4Address)

	// convert v4 address
	v4Addr := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// convert v6 address
	v6Addr := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET6,
		V4OrV6: &halproto.IPAddress_V6Addr{
			V6Addr: []byte(net.ParseIP(ep.Status.IPv6Address)),
		},
	}

	// get sg ids
	var sgHandles []*halproto.SecurityGroupKeyHandle
	for _, sg := range sgs {
		sgKey := halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupHandle{
				SecurityGroupHandle: sg.Status.SecurityGroupID,
			},
		}
		sgHandles = append(sgHandles, &sgKey)
	}

	l2Handle := halproto.L2SegmentKeyHandle{
		KeyOrHandle: &halproto.L2SegmentKeyHandle_L2SegmentHandle{
			L2SegmentHandle: nw.Status.NetworkHandle,
		},
	}

	ifKeyHandle := halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_IfHandle{
			IfHandle: 0, //FIXME
		},
	}

	epAttrs := halproto.EndpointAttributes{
		InterfaceKeyHandle: &ifKeyHandle, //FIXME
		UsegVlan:           ep.Status.UsegVlan,
		IpAddress:          []*halproto.IPAddress{&v4Addr, &v6Addr},
		SgKeyHandle:        sgHandles,
	}

	epHandle := halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L2Key{
					L2Key: &halproto.EndpointL2Key{
						L2SegmentKeyHandle: &l2Handle,
					},
				},
			},
		},
	}

	// build endpoint message
	epUpdateReq := halproto.EndpointUpdateRequest{
		Meta:          &halproto.ObjectMeta{},
		KeyOrHandle:   &epHandle,
		EndpointAttrs: &epAttrs,
	}

	epUpdateReqMsg := halproto.EndpointUpdateRequestMsg{
		Request: []*halproto.EndpointUpdateRequest{&epUpdateReq},
	}

	// call hal to update the endpoint
	// FIXME: handle response
	_, err := hd.Hal.Epclient.EndpointUpdate(context.Background(), &epUpdateReqMsg)
	if err != nil {
		log.Errorf("Error updating endpoint. Err: %v", err)
		return err
	}

	// save the endpoint message
	hd.Lock()
	hd.DB.EndpointUpdateDB[objectKey(&ep.ObjectMeta)] = &epUpdateReqMsg
	hd.Unlock()

	return nil
}

// DeleteLocalEndpoint deletes an endpoint
func (hd *Datapath) DeleteLocalEndpoint(ep *netproto.Endpoint) error {
	// convert v4 address
	ipaddr, _, err := net.ParseCIDR(ep.Status.IPv4Address)
	log.Infof("Deleting endpoint: {%+v}, addr: %v/%v. Err: %v", ep, ep.Status.IPv4Address, ipaddr, err)
	v4Addr := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// build endpoint del request
	epdel := halproto.EndpointDeleteRequest{
		Meta: &halproto.ObjectMeta{},
		DeleteBy: &halproto.EndpointDeleteRequest_KeyOrHandle{
			KeyOrHandle: &halproto.EndpointKeyHandle{
				KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
					EndpointKey: &halproto.EndpointKey{
						EndpointL2L3Key: &halproto.EndpointKey_L3Key{
							L3Key: &halproto.EndpointL3Key{
								IpAddress: &v4Addr,
							},
						},
					},
				},
			},
		},
	}
	delReq := halproto.EndpointDeleteRequestMsg{
		Request: []*halproto.EndpointDeleteRequest{&epdel},
	}
	// delete it from hal
	_, err = hd.Hal.Epclient.EndpointDelete(context.Background(), &delReq)
	if err != nil {
		log.Errorf("Error deleting endpoint. Err: %v", err)
		return err
	}

	// save the endpoint delete message
	hd.Lock()
	hd.DB.EndpointDelDB[objectKey(&ep.ObjectMeta)] = &delReq
	delete(hd.DB.EndpointDB, objectKey(&ep.ObjectMeta))
	hd.Unlock()

	return nil
}

// DeleteRemoteEndpoint deletes remote endpoint
func (hd *Datapath) DeleteRemoteEndpoint(ep *netproto.Endpoint) error {
	// convert v4 address
	ipaddr, _, err := net.ParseCIDR(ep.Status.IPv4Address)
	log.Infof("Deleting endpoint: {%+v}, addr: %v/%v. Err: %v", ep, ep.Status.IPv4Address, ipaddr, err)
	v4Addr := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// build endpoint del request
	epdel := halproto.EndpointDeleteRequest{
		Meta: &halproto.ObjectMeta{},
		DeleteBy: &halproto.EndpointDeleteRequest_KeyOrHandle{
			KeyOrHandle: &halproto.EndpointKeyHandle{
				KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
					EndpointKey: &halproto.EndpointKey{
						EndpointL2L3Key: &halproto.EndpointKey_L3Key{
							L3Key: &halproto.EndpointL3Key{
								IpAddress: &v4Addr,
							},
						},
					},
				},
			},
		},
	}
	delReq := halproto.EndpointDeleteRequestMsg{
		Request: []*halproto.EndpointDeleteRequest{&epdel},
	}
	// delete it from hal
	_, err = hd.Hal.Epclient.EndpointDelete(context.Background(), &delReq)
	if err != nil {
		log.Errorf("Error deleting endpoint. Err: %v", err)
		return err
	}

	// save the endpoint delete message
	hd.Lock()
	hd.DB.EndpointDelDB[objectKey(&ep.ObjectMeta)] = &delReq
	delete(hd.DB.EndpointDB, objectKey(&ep.ObjectMeta))
	hd.Unlock()

	return nil
}

// CreateNetwork creates a l2 segment in datapath if vlan id is specified and a network if IPSubnet is specified.
// ToDo Investigate if HAL needs network updates and deletes.
func (hd *Datapath) CreateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error {
	var nwKey halproto.NetworkKeyHandle
	// construct vrf key that gets passed on to hal
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

	gwIP := net.ParseIP(nw.Spec.IPv4Gateway)
	if len(gwIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", nw.Spec.IPv4Gateway)
	}

	halGwIP := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(gwIP),
		},
	}

	if len(nw.Spec.IPv4Subnet) != 0 {
		ip, net, err := net.ParseCIDR(nw.Spec.IPv4Subnet)
		if err != nil {
			return fmt.Errorf("error parsing the subnet mask from {%v}. Err: %v", nw.Spec.IPv4Subnet, err)
		}
		prefixLen, _ := net.Mask.Size()

		nwKey = halproto.NetworkKeyHandle{
			KeyOrHandle: &halproto.NetworkKeyHandle_NwKey{
				NwKey: &halproto.NetworkKey{
					IpPrefix: &halproto.IPPrefix{
						Address: &halproto.IPAddress{
							IpAf: halproto.IPAddressFamily_IP_AF_INET,
							V4OrV6: &halproto.IPAddress_V4Addr{
								V4Addr: ipv4Touint32(ip),
							},
						},
						PrefixLen: uint32(prefixLen),
					},
					VrfKeyHandle: vrfKey,
				},
			},
		}

		halNw := halproto.NetworkSpec{
			KeyOrHandle: &nwKey,
			GatewayIp:   halGwIP,
		}

		halNwReq := halproto.NetworkRequestMsg{
			Request: []*halproto.NetworkSpec{&halNw},
		}
		// create the tenant. Enforce HAL Status == OK for HAL datapath
		if hd.Kind == "hal" {
			resp, err := hd.Hal.Netclient.NetworkCreate(context.Background(), &halNwReq)
			if err != nil {
				log.Errorf("Error creating network. Err: %v", err)
				return err
			}
			if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)
				return ErrHALNotOK
			}
		} else {
			_, err := hd.Hal.Netclient.NetworkCreate(context.Background(), &halNwReq)
			if err != nil {
				log.Errorf("Error creating tenant. Err: %v", err)
				return err
			}
		}
	}

	// build l2 segment data
	seg := halproto.L2SegmentSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		McastFwdPolicy: halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD,
		BcastFwdPolicy: halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD,
		WireEncap: &halproto.EncapInfo{
			EncapType:  halproto.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
		VrfKeyHandle: vrfKey,
		NetworkKeyHandle: []*halproto.NetworkKeyHandle{
			&nwKey,
		},
	}
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// create the tenant. Enforce HAL Status == OK for HAL datapath
	if hd.Kind == "hal" {
		ifL2SegReqMsg := halproto.InterfaceL2SegmentRequestMsg{
			Request: make([]*halproto.InterfaceL2SegmentSpec, 0),
		}

		resp, err := hd.Hal.L2SegClient.L2SegmentCreate(context.Background(), &segReq)
		if err != nil {
			log.Errorf("Error creating L2 Segment. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)
			return ErrHALNotOK
		}
		for _, uplink := range uplinks {
			ifL2SegReq := halproto.InterfaceL2SegmentSpec{
				L2SegmentKeyOrHandle: seg.KeyOrHandle,
				IfKeyHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: uplink.Status.InterfaceID,
					},
				},
			}
			ifL2SegReqMsg.Request = append(ifL2SegReqMsg.Request, &ifL2SegReq)
		}
		// Perform batched Add only if uplinks exist
		if len(uplinks) != 0 {
			l2SegAddResp, err := hd.Hal.Ifclient.AddL2SegmentOnUplink(context.Background(), &ifL2SegReqMsg)
			if err != nil {
				log.Errorf("Error adding l2 segments on uplinks. Err: %v", err)
				return err
			}
			for _, r := range l2SegAddResp.Response {
				if r.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
					log.Errorf("HAL returned non OK status. Err: %v", err)
					return ErrHALNotOK
				}
			}
		} else {
			log.Errorf("Could not find uplinks.")
		}

	} else {
		ifL2SegReqMsg := halproto.InterfaceL2SegmentRequestMsg{
			Request: make([]*halproto.InterfaceL2SegmentSpec, 0),
		}
		_, err := hd.Hal.L2SegClient.L2SegmentCreate(context.Background(), &segReq)
		if err != nil {
			log.Errorf("Error creating tenant. Err: %v", err)
			return err
		}
		for _, uplink := range uplinks {
			ifL2SegReq := halproto.InterfaceL2SegmentSpec{
				L2SegmentKeyOrHandle: seg.KeyOrHandle,
				IfKeyHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: uplink.Status.InterfaceID,
					},
				},
			}
			ifL2SegReqMsg.Request = append(ifL2SegReqMsg.Request, &ifL2SegReq)
		}

		_, err = hd.Hal.Ifclient.AddL2SegmentOnUplink(context.Background(), &ifL2SegReqMsg)
		if err != nil {
			log.Errorf("Error adding l2 segments on uplinks. Err: %v", err)
			return err
		}
	}

	return nil
}

// UpdateNetwork updates a network in datapath
func (hd *Datapath) UpdateNetwork(nw *netproto.Network, ns *netproto.Namespace) error {
	// build l2 segment data
	seg := halproto.L2SegmentSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		McastFwdPolicy: halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD,
		BcastFwdPolicy: halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD,
		WireEncap: &halproto.EncapInfo{
			EncapType:  halproto.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
		TunnelEncap: &halproto.EncapInfo{
			EncapType:  halproto.EncapType_ENCAP_TYPE_VXLAN,
			EncapValue: nw.Spec.VlanID,
		},
	}
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// update the l2 segment
	_, err := hd.Hal.L2SegClient.L2SegmentUpdate(context.Background(), &segReq)
	if err != nil {
		log.Errorf("Error updating network. Err: %v", err)
		return err
	}

	return nil
}

// DeleteNetwork deletes a network from datapath
func (hd *Datapath) DeleteNetwork(nw *netproto.Network, ns *netproto.Namespace) error {
	// build the segment message
	seg := halproto.L2SegmentDeleteRequest{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
	}

	segDelReqMsg := halproto.L2SegmentDeleteRequestMsg{
		Request: []*halproto.L2SegmentDeleteRequest{&seg},
	}

	// delete the l2 segment
	_, err := hd.Hal.L2SegClient.L2SegmentDelete(context.Background(), &segDelReqMsg)
	if err != nil {
		log.Errorf("Error deleting network. Err: %v", err)
		return err
	}

	return nil
}

// CreateSecurityGroup creates a security group
func (hd *Datapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	var secGroupPolicyRequests []*halproto.SecurityGroupPolicySpec

	// convert the rules
	for _, rl := range sg.Spec.Rules {
		policySpec, err := hd.convertRule(sg, &rl)
		if err != nil {
			return err
		}
		secGroupPolicyRequests = append(secGroupPolicyRequests, policySpec)
	}

	sgPolicyRequestMsg := halproto.SecurityGroupPolicyRequestMsg{
		Request: secGroupPolicyRequests,
	}

	// add security group policies
	_, err := hd.Hal.Sgclient.SecurityGroupPolicyCreate(context.Background(), &sgPolicyRequestMsg)
	if err != nil {
		log.Errorf("Could not create SG Policy. %v", err)
		return err
	}

	hd.Lock()
	hd.DB.SgPolicyDB[objectKey(&sg.ObjectMeta)] = &sgPolicyRequestMsg
	hd.Unlock()

	// build security group message
	sgs := halproto.SecurityGroupSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
	}
	sgmsg := halproto.SecurityGroupRequestMsg{
		Request: []*halproto.SecurityGroupSpec{&sgs},
	}

	// add security group
	_, err = hd.Hal.Sgclient.SecurityGroupCreate(context.Background(), &sgmsg)
	if err != nil {
		log.Errorf("Error creating security group. Err: %v", err)
		return err
	}

	// save the sg message
	hd.Lock()
	hd.DB.SgDB[objectKey(&sg.ObjectMeta)] = &sgmsg
	hd.Unlock()

	return nil
}

// UpdateSecurityGroup updates a security group
func (hd *Datapath) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	var secGroupPolicyRequests []*halproto.SecurityGroupPolicySpec
	// convert the rules
	for _, rl := range sg.Spec.Rules {
		policySpec, err := hd.convertRule(sg, &rl)
		if err != nil {
			return err
		}
		secGroupPolicyRequests = append(secGroupPolicyRequests, policySpec)
	}

	sgPolicyRequestMsg := halproto.SecurityGroupPolicyRequestMsg{
		Request: secGroupPolicyRequests,
	}
	// add security group policies
	_, err := hd.Hal.Sgclient.SecurityGroupPolicyUpdate(context.Background(), &sgPolicyRequestMsg)
	if err != nil {
		log.Errorf("Could not create SG Policy. %v", err)
		return err
	}

	hd.Lock()
	hd.DB.SgPolicyDB[objectKey(&sg.ObjectMeta)] = &sgPolicyRequestMsg
	hd.Unlock()

	// build security group message
	sgs := halproto.SecurityGroupSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
	}
	sgmsg := halproto.SecurityGroupRequestMsg{
		Request: []*halproto.SecurityGroupSpec{&sgs},
	}

	// update security group
	_, err = hd.Hal.Sgclient.SecurityGroupUpdate(context.Background(), &sgmsg)
	if err != nil {
		log.Errorf("Error updating security group. Err: %v", err)
		return err
	}

	// save the sg message
	hd.Lock()
	hd.DB.SgDB[objectKey(&sg.ObjectMeta)] = &sgmsg
	hd.Unlock()

	return nil
}

// DeleteSecurityGroup deletes a security group
func (hd *Datapath) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	// build security group message
	sgdel := halproto.SecurityGroupDeleteRequest{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
	}
	sgmsg := halproto.SecurityGroupDeleteRequestMsg{
		Request: []*halproto.SecurityGroupDeleteRequest{&sgdel},
	}

	// delete security group
	_, err := hd.Hal.Sgclient.SecurityGroupDelete(context.Background(), &sgmsg)
	if err != nil {
		log.Errorf("Error deleting security group. Err: %v", err)
		return err
	}

	// delete the sg message
	hd.Lock()
	delete(hd.DB.SgDB, objectKey(&sg.ObjectMeta))
	hd.Unlock()

	return nil
}

// CreateVrf creates a vrf
func (hd *Datapath) CreateVrf(vrfID uint64) error {
	vrfSpec := halproto.VrfSpec{
		Meta: &halproto.ObjectMeta{
			VrfId: vrfID,
		},
		KeyOrHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: vrfID,
			},
		},
		// All tenant creates are currently customer type as we don't intend to expose infra vrf creates to the user.
		VrfType: halproto.VrfType_VRF_TYPE_CUSTOMER,
	}
	vrfReqMsg := halproto.VrfRequestMsg{
		Request: []*halproto.VrfSpec{&vrfSpec},
	}

	// create the tenant. Enforce HAL Status == OK for HAL datapath
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Tnclient.VrfCreate(context.Background(), &vrfReqMsg)
		if err != nil {
			log.Errorf("Error creating tenant. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)
			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Tnclient.VrfCreate(context.Background(), &vrfReqMsg)
		if err != nil {
			log.Errorf("Error creating tenant. Err: %v", err)
			return err
		}
	}

	return nil
}

// DeleteVrf deletes a vrf
func (hd *Datapath) DeleteVrf(vrfID uint64) error {

	vrfDelReq := halproto.VrfDeleteRequest{
		Meta: &halproto.ObjectMeta{
			VrfId: vrfID,
		},
	}

	vrfDelReqMsg := halproto.VrfDeleteRequestMsg{
		Request: []*halproto.VrfDeleteRequest{&vrfDelReq},
	}

	// delete the tenant
	_, err := hd.Hal.Tnclient.VrfDelete(context.Background(), &vrfDelReqMsg)
	if err != nil {
		log.Errorf("Error deleting tenant. Err: %v", err)
		return err
	}

	return nil
}

// UpdateVrf deletes a tenant
func (hd *Datapath) UpdateVrf(vrfID uint64) error {
	vrfSpec := halproto.VrfSpec{
		Meta: &halproto.ObjectMeta{
			VrfId: vrfID,
		},
	}
	vrfReqMsg := halproto.VrfRequestMsg{
		Request: []*halproto.VrfSpec{&vrfSpec},
	}

	// update the tenant
	_, err := hd.Hal.Tnclient.VrfUpdate(context.Background(), &vrfReqMsg)
	if err != nil {
		log.Errorf("Error deleting tenant. Err: %v", err)
		return err
	}

	return nil
}

// CreateInterface creates an interface
func (hd *Datapath) CreateInterface(intf *netproto.Interface, lif *netproto.Interface, ns *netproto.Namespace) error {
	var ifSpec *halproto.InterfaceSpec
	switch intf.Spec.Type {
	case "LIF":
		lifReqMsg := &halproto.LifRequestMsg{
			Request: []*halproto.LifSpec{
				{
					Meta: &halproto.ObjectMeta{
						VrfId: ns.Status.NamespaceID,
					},
					KeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: intf.Status.InterfaceID,
						},
					},
				},
			},
		}
		_, err := hd.Hal.Ifclient.LifCreate(context.Background(), lifReqMsg)
		if err != nil {
			log.Errorf("Error creating lif. Err: %v", err)
			return err
		}
		hd.Lock()
		hd.DB.LifDB[objectKey(&ns.ObjectMeta)] = lifReqMsg
		hd.Unlock()
		return nil

	case "UPLINK":
		ifSpec = &halproto.InterfaceSpec{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_UPLINK,
		}

	case "ENIC":
		ifSpec = &halproto.InterfaceSpec{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_ENIC,
			// associate the lif id
			IfInfo: &halproto.InterfaceSpec_IfEnicInfo{
				IfEnicInfo: &halproto.IfEnicInfo{
					EnicType: halproto.IfEnicType_IF_ENIC_TYPE_USEG,
					LifKeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: lif.Status.InterfaceID,
						},
					},
				},
			},
		}
	default:
		return errors.New("invalid interface type")
	}

	ifReqMsg := &halproto.InterfaceRequestMsg{
		Request: []*halproto.InterfaceSpec{
			ifSpec,
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), ifReqMsg)
		if err != nil {
			log.Errorf("Error creating interface. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Ifclient.InterfaceCreate(context.Background(), ifReqMsg)
		if err != nil {
			log.Errorf("Error creating inteface. Err: %v", err)
			return err
		}
	}

	hd.Lock()
	hd.DB.InterfaceDB[objectKey(&ns.ObjectMeta)] = ifReqMsg
	hd.Unlock()
	return nil
}

// DeleteInterface deletes an interface
func (hd *Datapath) DeleteInterface(intf *netproto.Interface, ns *netproto.Namespace) error {
	var ifDelReq *halproto.InterfaceDeleteRequest
	switch intf.Spec.Type {
	case "LIF":
		lifDelReqMsg := &halproto.LifDeleteRequestMsg{
			Request: []*halproto.LifDeleteRequest{
				{
					Meta: &halproto.ObjectMeta{
						VrfId: ns.Status.NamespaceID,
					},
					KeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: intf.Status.InterfaceID,
						},
					},
				},
			},
		}
		_, err := hd.Hal.Ifclient.LifDelete(context.Background(), lifDelReqMsg)
		if err != nil {
			log.Errorf("Error creating lif. Err: %v", err)
			return err
		}
		// save the lif delete message
		hd.Lock()
		hd.DB.LifDelDB[objectKey(&intf.ObjectMeta)] = lifDelReqMsg
		delete(hd.DB.LifDB, objectKey(&intf.ObjectMeta))
		hd.Unlock()

		return nil
	case "ENIC":
		fallthrough

	case "UPLINK":
		ifDelReq = &halproto.InterfaceDeleteRequest{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
		}
	default:
		return errors.New("invalid interface type")
	}

	ifDelReqMsg := &halproto.InterfaceDeleteRequestMsg{
		Request: []*halproto.InterfaceDeleteRequest{
			ifDelReq,
		},
	}
	_, err := hd.Hal.Ifclient.InterfaceDelete(context.Background(), ifDelReqMsg)
	if err != nil {
		log.Errorf("Error creating lif. Err: %v", err)
		return err
	}
	// save the lif delete message
	hd.Lock()
	hd.DB.InterfaceDelDB[objectKey(&intf.ObjectMeta)] = ifDelReqMsg
	delete(hd.DB.LifDB, objectKey(&intf.ObjectMeta))
	hd.Unlock()
	return nil

}

// UpdateInterface updates an interface
func (hd *Datapath) UpdateInterface(intf *netproto.Interface, ns *netproto.Namespace) error {
	var ifSpec *halproto.InterfaceSpec
	switch intf.Spec.Type {
	case "LIF":
		lifReqMsg := &halproto.LifRequestMsg{
			Request: []*halproto.LifSpec{
				{
					Meta: &halproto.ObjectMeta{
						VrfId: ns.Status.NamespaceID,
					},
					KeyOrHandle: &halproto.LifKeyHandle{
						KeyOrHandle: &halproto.LifKeyHandle_LifId{
							LifId: intf.Status.InterfaceID,
						},
					},
				},
			},
		}
		_, err := hd.Hal.Ifclient.LifUpdate(context.Background(), lifReqMsg)
		if err != nil {
			log.Errorf("Error creating lif. Err: %v", err)
			return err
		}
		hd.Lock()
		hd.DB.LifDB[objectKey(&ns.ObjectMeta)] = lifReqMsg
		hd.Unlock()
		return nil

	case "UPLINK":
		ifSpec = &halproto.InterfaceSpec{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_UPLINK,
		}

	case "ENIC":
		ifSpec = &halproto.InterfaceSpec{
			Meta: &halproto.ObjectMeta{
				VrfId: ns.Status.NamespaceID,
			},
			KeyOrHandle: &halproto.InterfaceKeyHandle{
				KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
					InterfaceId: intf.Status.InterfaceID,
				},
			},
			Type: halproto.IfType_IF_TYPE_ENIC,
		}
	default:
		return errors.New("invalid interface type")
	}

	ifReqMsg := &halproto.InterfaceRequestMsg{
		Request: []*halproto.InterfaceSpec{
			ifSpec,
		},
	}
	_, err := hd.Hal.Ifclient.InterfaceUpdate(context.Background(), ifReqMsg)
	if err != nil {
		log.Errorf("Error updating interface. Err: %v", err)
		return err
	}
	hd.Lock()
	hd.DB.InterfaceDB[objectKey(&ns.ObjectMeta)] = ifReqMsg
	hd.Unlock()
	return nil
}

// CreateNatPool creates a NAT Pool in the datapath
func (hd *Datapath) CreateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error {
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

	ipRange := strings.Split(np.Spec.IPRange, "-")
	if len(ipRange) != 2 {
		return fmt.Errorf("could not parse IP Range from the NAT Pool IPRange. {%v}", np.Spec.IPRange)
	}

	startIP := net.ParseIP(strings.TrimSpace(ipRange[0]))
	if len(startIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", startIP)
	}
	endIP := net.ParseIP(strings.TrimSpace(ipRange[1]))
	if len(endIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", endIP)
	}

	lowIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(startIP),
		},
	}

	highIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(endIP),
		},
	}

	addrRange := &halproto.Address_Range{
		Range: &halproto.AddressRange{
			Range: &halproto.AddressRange_Ipv4Range{
				Ipv4Range: &halproto.IPRange{
					LowIpaddr:  &lowIP,
					HighIpaddr: &highIP,
				},
			},
		},
	}

	natPoolReqMsg := &halproto.NatPoolRequestMsg{
		Request: []*halproto.NatPoolSpec{
			{
				KeyOrHandle: &halproto.NatPoolKeyHandle{
					KeyOrHandle: &halproto.NatPoolKeyHandle_PoolKey{
						PoolKey: &halproto.NatPoolKey{
							VrfKh:  vrfKey,
							PoolId: np.Status.NatPoolID,
						},
					},
				},
				Address: []*halproto.Address{
					{
						addrRange,
					},
				},
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Natclient.NatPoolCreate(context.Background(), natPoolReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Natclient.NatPoolCreate(context.Background(), natPoolReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
	}
	return nil
}

// UpdateNatPool updates a NAT Pool in the datapath
func (hd *Datapath) UpdateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error {

	return nil
}

// DeleteNatPool deletes a NAT Pool in the datapath
func (hd *Datapath) DeleteNatPool(np *netproto.NatPool, ns *netproto.Namespace) error {

	return nil
}

// CreateNatPolicy creates a NAT Policy in the datapath
func (hd *Datapath) CreateNatPolicy(np *netproto.NatPolicy, natPoolLUT map[string]*types.NatPoolRef, ns *netproto.Namespace) error {
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

	var natRules []*halproto.NatRuleSpec

	for _, r := range np.Spec.Rules {
		ruleMatch, err := hd.convertMatchCriteria(r.Src, r.Dst)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}
		npRef, ok := natPoolLUT[r.NatPool]
		if !ok {
			return fmt.Errorf("nat pool not found. {%v}", r.NatPool)
		}

		natAction, err := hd.convertNatRuleAction(r.Action, npRef.NamespaceID, npRef.PoolID)
		if err != nil {
			log.Errorf("Could not convert NAT Action. Action: %v. Err: %v", r.Action, err)
		}

		rule := &halproto.NatRuleSpec{
			RuleId: r.ID,
			Match:  ruleMatch,
			Action: natAction,
		}
		natRules = append(natRules, rule)
	}

	natPolicyReqMsg := &halproto.NatPolicyRequestMsg{
		Request: []*halproto.NatPolicySpec{
			{
				KeyOrHandle: &halproto.NatPolicyKeyHandle{
					KeyOrHandle: &halproto.NatPolicyKeyHandle_PolicyKey{
						PolicyKey: &halproto.NATPolicyKey{
							NatPolicyId:    np.Status.NatPolicyID,
							VrfKeyOrHandle: vrfKey,
						},
					},
				},
				Rules: natRules,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Natclient.NatPolicyCreate(context.Background(), natPolicyReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Natclient.NatPolicyCreate(context.Background(), natPolicyReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
	}
	return nil
}

// UpdateNatPolicy updates a NAT Policy in the datapath
func (hd *Datapath) UpdateNatPolicy(np *netproto.NatPolicy, ns *netproto.Namespace) error {

	return nil
}

// DeleteNatPolicy deletes a NAT Policy in the datapath
func (hd *Datapath) DeleteNatPolicy(np *netproto.NatPolicy, ns *netproto.Namespace) error {

	return nil
}

// CreateRoute creates a Route in the datapath
func (hd *Datapath) CreateRoute(rt *netproto.Route, ns *netproto.Namespace) error {
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

	// Build next hop object
	nextHopKey := &halproto.NexthopKeyHandle{
		KeyOrHandle: &halproto.NexthopKeyHandle_NexthopId{
			NexthopId: rt.Status.RouteID,
		},
	}

	epIP := net.ParseIP(rt.Spec.GatewayIP)
	if len(epIP) == 0 {
		return fmt.Errorf("could not parse IP from %v", rt.Spec.GatewayIP)
	}

	gwIPAddr := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(epIP),
		},
	}

	epKey := &halproto.EndpointKeyHandle{
		KeyOrHandle: &halproto.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halproto.EndpointKey{
				EndpointL2L3Key: &halproto.EndpointKey_L3Key{
					L3Key: &halproto.EndpointL3Key{
						VrfKeyHandle: vrfKey,
						IpAddress:    gwIPAddr,
					},
				},
			},
		},
	}

	nextHopReqMsg := &halproto.NexthopRequestMsg{
		Request: []*halproto.NexthopSpec{
			{
				KeyOrHandle: nextHopKey,
				IfOrEp: &halproto.NexthopSpec_EpKeyOrHandle{
					EpKeyOrHandle: epKey,
				},
			},
		},
	}

	// create nexthop object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Netclient.NexthopCreate(context.Background(), nextHopReqMsg)
		if err != nil {
			log.Errorf("Error creating next hop. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Netclient.NexthopCreate(context.Background(), nextHopReqMsg)
		if err != nil {
			log.Errorf("Error creating next Hop Err: %v", err)
			return err
		}
	}

	// build route object
	ip, net, err := net.ParseCIDR(rt.Spec.IPPrefix)
	if err != nil {
		return fmt.Errorf("error parsing the IP Prefix mask from %v. Err: %v", rt.Spec.IPPrefix, err)

	}
	prefixLen, _ := net.Mask.Size()
	ipPrefix := &halproto.IPPrefix{
		Address: &halproto.IPAddress{
			IpAf: halproto.IPAddressFamily_IP_AF_INET,
			V4OrV6: &halproto.IPAddress_V4Addr{
				V4Addr: ipv4Touint32(ip),
			},
		},
		PrefixLen: uint32(prefixLen),
	}

	routeReqMsg := &halproto.RouteRequestMsg{
		Request: []*halproto.RouteSpec{
			{
				KeyOrHandle: &halproto.RouteKeyHandle{
					KeyOrHandle: &halproto.RouteKeyHandle_RouteKey{
						RouteKey: &halproto.RouteKey{
							VrfKeyHandle: vrfKey,
							IpPrefix:     ipPrefix,
						},
					},
				},
				NhKeyOrHandle: nextHopKey,
			},
		},
	}

	// create route object
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Netclient.RouteCreate(context.Background(), routeReqMsg)
		if err != nil {
			log.Errorf("Error creating route. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.Netclient.RouteCreate(context.Background(), routeReqMsg)
		if err != nil {
			log.Errorf("Error creating route. Err: %v", err)
			return err
		}
	}
	return nil
}

// CreateNatBinding creates a NAT Binding in the datapath
func (hd *Datapath) CreateNatBinding(nb *netproto.NatBinding, np *netproto.NatPool, natPoolVrfID uint64, ns *netproto.Namespace) (*netproto.NatBinding, error) {
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

	natBindingIP := net.ParseIP(nb.Spec.IPAddress)
	if len(natBindingIP) == 0 {
		log.Errorf("could not parse IP from {%v}", natBindingIP)
		return nil, ErrIPParse
	}

	ipAddr := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(natBindingIP),
		},
	}

	natPoolVrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: natPoolVrfID,
		},
	}

	natPoolKey := &halproto.NatPoolKeyHandle{
		KeyOrHandle: &halproto.NatPoolKeyHandle_PoolKey{
			PoolKey: &halproto.NatPoolKey{
				VrfKh:  natPoolVrfKey,
				PoolId: np.Status.NatPoolID,
			},
		},
	}

	natBindingReqMsg := &halproto.NatMappingRequestMsg{
		Request: []*halproto.NatMappingSpec{
			{
				KeyOrHandle: &halproto.NatMappingKeyHandle{
					KeyOrHandle: &halproto.NatMappingKeyHandle_Svc{
						Svc: &halproto.Svc{
							VrfKh:  vrfKey,
							IpAddr: ipAddr,
						},
					},
				},
				NatPool: natPoolKey,
				Bidir:   true, // Set Bidirectional to true inorder to create forward and reverse mappings.
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Natclient.NatMappingCreate(context.Background(), natBindingReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return nb, err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return nb, ErrHALNotOK
		}
		ipv4Int := resp.Response[0].Status.MappedIp.GetV4Addr()
		ip := intToIPv4(ipv4Int)
		nb.Status.NatIP = ip.String()
		return nb, nil
	}
	_, err := hd.Hal.Natclient.NatMappingCreate(context.Background(), natBindingReqMsg)
	if err != nil {
		log.Errorf("Error creating nat pool. Err: %v", err)
		return nb, err
	}
	return nb, nil

}

// UpdateRoute updates a Route in the datapath
func (hd *Datapath) UpdateRoute(rt *netproto.Route, ns *netproto.Namespace) error {

	return nil
}

// UpdateNatBinding updates a NAT Binding in the datapath
func (hd *Datapath) UpdateNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error {

	return nil
}

// DeleteRoute deletes a Route in the datapath
func (hd *Datapath) DeleteRoute(rt *netproto.Route, ns *netproto.Namespace) error {

	return nil
}

// DeleteNatBinding deletes a NAT Binding in the datapath
func (hd *Datapath) DeleteNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error {

	return nil
}

// CreateIPSecPolicy creates an IPSec Policy in the datapath
func (hd *Datapath) CreateIPSecPolicy(ipSec *netproto.IPSecPolicy, ns *netproto.Namespace, ipSecLUT map[string]*types.IPSecRuleRef) error {
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: ns.Status.NamespaceID,
		},
	}

	var ipSecRules []*halproto.IpsecRuleMatchSpec

	for _, r := range ipSec.Spec.Rules {
		// Match source and dest attributes
		ruleMatch, err := hd.convertMatchCriteria(r.Src, r.Dst)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}

		// Populate esp info in the match selector.
		appInfo := []*halproto.RuleMatch_AppMatchInfo{
			{
				App: &halproto.RuleMatch_AppMatchInfo_EspInfo{
					EspInfo: &halproto.RuleMatch_ESPInfo{
						Spi: r.SPI,
					},
				},
			},
		}
		ruleMatch.AppMatch = appInfo

		// Lookup corresponding SA
		lookupKey := fmt.Sprintf("%s|%s", r.SAType, r.SAName)

		ipSecRuleRef, ok := ipSecLUT[lookupKey]
		if !ok {
			return fmt.Errorf("IPSec SA Rule not found. {%v}", r.SAName)
		}

		ipSecAction, err := hd.convertIPSecRuleAction(r.SAType, ipSecRuleRef.NamespaceID, ipSecRuleRef.RuleID)
		if err != nil {
			log.Errorf("Could not convert IPSec rule action. Rule: %v. Err: %v", r, err)
		}

		rule := &halproto.IpsecRuleMatchSpec{
			RuleId:   r.ID,
			Match:    ruleMatch,
			SaAction: ipSecAction,
		}
		ipSecRules = append(ipSecRules, rule)
	}

	ipSecPolicyReqMsg := &halproto.IpsecRuleRequestMsg{
		Request: []*halproto.IpsecRuleSpec{
			{
				KeyOrHandle: &halproto.IpsecRuleKeyHandle{
					KeyOrHandle: &halproto.IpsecRuleKeyHandle_RuleKey{
						RuleKey: &halproto.IPSecRuleKey{
							IpsecRuleId:    ipSec.Status.IPSecPolicyID,
							VrfKeyOrHandle: vrfKey,
						},
					},
				},
				VrfKeyHandle: vrfKey,
				Rules:        ipSecRules,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.IPSecclient.IpsecRuleCreate(context.Background(), ipSecPolicyReqMsg)
		if err != nil {
			log.Errorf("EError creating IPSec Policy. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.IPSecclient.IpsecRuleCreate(context.Background(), ipSecPolicyReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Policy. Err: %v", err)
			return err
		}
	}
	return nil
}

// UpdateIPSecPolicy updates an IPSec Policy in the datapath
func (hd *Datapath) UpdateIPSecPolicy(ipSec *netproto.IPSecPolicy, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecPolicy deletes an IPSec Policy in the datapath
func (hd *Datapath) DeleteIPSecPolicy(ipSec *netproto.IPSecPolicy, ns *netproto.Namespace) error {
	return nil
}

// CreateIPSecSAEncrypt creates an IPSecSA encrypt rule in the datapath
func (hd *Datapath) CreateIPSecSAEncrypt(sa *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	localGwIP := net.ParseIP(strings.TrimSpace(sa.Spec.LocalGwIP))
	if len(localGwIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", localGwIP)
	}
	remoteGwIP := net.ParseIP(strings.TrimSpace(sa.Spec.RemoteGwIP))
	if len(remoteGwIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", remoteGwIP)
	}

	localGw := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(localGwIP),
		},
	}

	remoteGw := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(remoteGwIP),
		},
	}

	ipSecSAEncryptReqMsg := &halproto.IpsecSAEncryptRequestMsg{
		Request: []*halproto.IpsecSAEncrypt{
			{
				KeyOrHandle: &halproto.IpsecSAEncryptKeyHandle{
					KeyOrHandle: &halproto.IpsecSAEncryptKeyHandle_CbId{
						CbId: sa.Status.IPSecSAEncryptID,
					},
				},
				Protocol:                halproto.IpsecProtocol_IPSEC_PROTOCOL_ESP,
				AuthenticationAlgorithm: convertAuthAlgorithm(sa.Spec.AuthAlgo),
				AuthenticationKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.AuthKey),
					},
				},
				EncryptionAlgorithm: convertEncryptionAlgorithm(sa.Spec.EncryptAlgo),
				EncryptionKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.EncryptionKey),
					},
				},
				LocalGatewayIp:  localGw,
				RemoteGatewayIp: remoteGw,
				Spi:             sa.Spec.SPI,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.IPSecclient.IpsecSAEncryptCreate(context.Background(), ipSecSAEncryptReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Encrypt SA Rule. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.IPSecclient.IpsecSAEncryptCreate(context.Background(), ipSecSAEncryptReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Encrypt SA Rule. Err: %v", err)
			return err
		}
	}

	return nil
}

// UpdateIPSecSAEncrypt updates an IPSecSA encrypt rule in the datapath
func (hd *Datapath) UpdateIPSecSAEncrypt(sa *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecSAEncrypt deletes an IPSecSA encrypt rule in the datapath
func (hd *Datapath) DeleteIPSecSAEncrypt(sa *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	return nil
}

// CreateIPSecSADecrypt creates an IPSecSA decrypt rule in the datapath
func (hd *Datapath) CreateIPSecSADecrypt(sa *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	ipSecSADecryptReqMsg := &halproto.IpsecSADecryptRequestMsg{
		Request: []*halproto.IpsecSADecrypt{
			{
				KeyOrHandle: &halproto.IpsecSADecryptKeyHandle{
					KeyOrHandle: &halproto.IpsecSADecryptKeyHandle_CbId{
						CbId: sa.Status.IPSecSADecryptID,
					},
				},
				Protocol:                halproto.IpsecProtocol_IPSEC_PROTOCOL_ESP,
				AuthenticationAlgorithm: convertAuthAlgorithm(sa.Spec.AuthAlgo),
				AuthenticationKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.AuthKey),
					},
				},
				DecryptionAlgorithm: convertEncryptionAlgorithm(sa.Spec.DecryptAlgo),
				DecryptionKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.DecryptionKey),
					},
				},
				RekeyDecAlgorithm: convertEncryptionAlgorithm(sa.Spec.RekeyDecryptAlgo),
				RekeyDecryptionKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.RekeyDecryptionKey),
					},
				},
				RekeyAuthenticationKey: &halproto.Key{
					KeyInfo: &halproto.Key_Key{
						Key: []byte(sa.Spec.RekeyAuthKey),
					},
				},

				Spi: sa.Spec.SPI,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.IPSecclient.IpsecSADecryptCreate(context.Background(), ipSecSADecryptReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Decrypt SA Rule. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus)

			return ErrHALNotOK
		}
	} else {
		_, err := hd.Hal.IPSecclient.IpsecSADecryptCreate(context.Background(), ipSecSADecryptReqMsg)
		if err != nil {
			log.Errorf("Error creating IPSec Decrypt SA Rule. Err: %v", err)
			return err
		}
	}

	return nil
}

// UpdateIPSecSADecrypt updates an IPSecSA decrypt rule in the datapath
func (hd *Datapath) UpdateIPSecSADecrypt(sa *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecSADecrypt deletes an IPSecSA decrypt rule in the datapath
func (hd *Datapath) DeleteIPSecSADecrypt(sa *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	return nil
}

// ListInterfaces returns the lisg of lifs and uplinks from the datapath
func (hd *Datapath) ListInterfaces() (*halproto.LifGetResponseMsg, *halproto.InterfaceGetResponseMsg, error) {
	if hd.Kind == "hal" {
		lifReq := &halproto.LifGetRequest{}
		var uplinks halproto.InterfaceGetResponseMsg
		lifReqMsg := &halproto.LifGetRequestMsg{
			Request: []*halproto.LifGetRequest{
				lifReq,
			},
		}

		// ToDo Add lif checks once nic mgr is integrated
		lifs, err := hd.Hal.Ifclient.LifGet(context.Background(), lifReqMsg)
		if err != nil {
			log.Errorf("Error getting lifs from the datapath. Err: %v", err)
			return nil, nil, nil
		}

		// get all interfaces
		ifReq := &halproto.InterfaceGetRequest{}
		ifReqMsg := &halproto.InterfaceGetRequestMsg{
			Request: []*halproto.InterfaceGetRequest{ifReq},
		}
		intfs, err := hd.Hal.Ifclient.InterfaceGet(context.Background(), ifReqMsg)
		if err != nil {
			return nil, nil, err
		}

		// return only the uplinks
		for _, intf := range intfs.Response {
			if intf.Spec.Type == halproto.IfType_IF_TYPE_UPLINK {
				uplinks.Response = append(uplinks.Response, intf)
			}
		}
		return lifs, &uplinks, err
	}

	// ToDo Remove the List Mock prior to FCS
	lifs, uplinks, err := generateMockHwState()
	return lifs, uplinks, err
}

func (hd *Datapath) convertRule(sg *netproto.SecurityGroup, rule *netproto.SecurityRule) (*halproto.SecurityGroupPolicySpec, error) {
	//var policyRules *halproto.SGPolicy
	// convert the action
	act := halproto.FirewallAction_FIREWALL_ACTION_NONE
	switch rule.Action {
	case "Allow":
		act = halproto.FirewallAction_FIREWALL_ACTION_ALLOW
	case "Deny":
		act = halproto.FirewallAction_FIREWALL_ACTION_DENY
	case "Reject":
		act = halproto.FirewallAction_FIREWALL_ACTION_REJECT
	default:
		log.Errorf("Unknown action %s in rule {%+v}", rule.Action, rule)
		return nil, errors.New("Unknown action")
	}

	// build service list
	srvs := []*halproto.Service{}
	for _, svc := range rule.Services {
		proto := halproto.IPProtocol_IPPROTO_NONE
		switch svc.Protocol {
		case "tcp":
			proto = halproto.IPProtocol_IPPROTO_TCP
		case "udp":
			proto = halproto.IPProtocol_IPPROTO_UDP
		case "icmp":
			proto = halproto.IPProtocol_IPPROTO_ICMP
		default:
			log.Errorf("Unknown protocol %s in rule {%+v}", svc.Protocol, rule)
			return nil, errors.New("Unknown protocol")
		}

		sr := halproto.Service{
			IpProtocol: proto,
			L4Info: &halproto.Service_DstPort{
				DstPort: svc.Port,
			},
		}

		srvs = append(srvs, &sr)
	}

	if strings.ToLower(rule.Direction) == "incoming" {
		var policyRules = halproto.SGPolicy{
			InFwRules: []*halproto.FirewallRuleSpec{
				{
					Svc:    srvs,
					Action: act,
					Log:    rule.Log,
				},
			},
		}
		sgPolicy := halproto.SecurityGroupPolicySpec{
			Meta: &halproto.ObjectMeta{},
			KeyOrHandle: &halproto.SecurityGroupPolicyKeyHandle{
				PolicyKeyOrHandle: &halproto.SecurityGroupPolicyKeyHandle_SecurityGroupPolicyId{
					SecurityGroupPolicyId: &halproto.SecurityGroupPolicyId{
						SecurityGroupId:     sg.Status.SecurityGroupID,
						PeerSecurityGroupId: rule.PeerGroupID,
					},
				},
			},
			PolicyRules: &policyRules,
		}
		return &sgPolicy, nil

	}
	var policyRules = halproto.SGPolicy{
		EgFwRules: []*halproto.FirewallRuleSpec{
			{
				Svc:    srvs,
				Action: act,
				Log:    rule.Log,
			},
		},
	}
	sgPolicy := halproto.SecurityGroupPolicySpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupPolicyKeyHandle{
			PolicyKeyOrHandle: &halproto.SecurityGroupPolicyKeyHandle_SecurityGroupPolicyId{
				SecurityGroupPolicyId: &halproto.SecurityGroupPolicyId{
					SecurityGroupId:     sg.Status.SecurityGroupID,
					PeerSecurityGroupId: rule.PeerGroupID,
				},
			},
		},
		PolicyRules: &policyRules,
	}
	return &sgPolicy, nil
}

// ToDo Remove Mock code prior to FCS. This is needed only for UT
func generateMockHwState() (*halproto.LifGetResponseMsg, *halproto.InterfaceGetResponseMsg, error) {
	var lifs halproto.LifGetResponseMsg
	var uplinks halproto.InterfaceGetResponseMsg

	mockLifs := []*halproto.LifGetResponse{
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.LifSpec{
				KeyOrHandle: &halproto.LifKeyHandle{
					KeyOrHandle: &halproto.LifKeyHandle_LifId{
						LifId: 1,
					},
				},
			},
		},
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.LifSpec{
				KeyOrHandle: &halproto.LifKeyHandle{
					KeyOrHandle: &halproto.LifKeyHandle_LifId{
						LifId: 2,
					},
				},
			},
		},
	}
	lifs.Response = append(lifs.Response, mockLifs...)

	mockUplinks := []*halproto.InterfaceGetResponse{
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.InterfaceSpec{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: 3,
					},
				},
				Type: halproto.IfType_IF_TYPE_UPLINK,
			},
		},
		{
			ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			Spec: &halproto.InterfaceSpec{
				KeyOrHandle: &halproto.InterfaceKeyHandle{
					KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
						InterfaceId: 4,
					},
				},
				Type: halproto.IfType_IF_TYPE_UPLINK,
			},
		},
	}
	uplinks.Response = append(uplinks.Response, mockUplinks...)

	return &lifs, &uplinks, nil
}

func (hd *Datapath) convertMatchCriteria(src, dst *netproto.MatchSelector) (*halproto.RuleMatch, error) {
	var srcIPRange []*halproto.IPAddressObj
	var dstIPRange []*halproto.IPAddressObj
	var ruleMatch halproto.RuleMatch
	var err error

	// Match source attributes
	if src != nil {
		// ToDo implement IP, Prefix Match address converters.
		srcIPRange, err = hd.convertIPRange(src.Address)
		if err != nil {
			log.Errorf("Could not convert match criteria from Src: {%v}. Err: %v", src, err)
			return nil, err
		}
		ruleMatch.SrcAddress = srcIPRange
	}

	// Match dest attributes
	if dst != nil {
		dstIPRange, err = hd.convertIPRange(dst.Address)
		if err != nil {
			log.Errorf("Could not convert match criteria from Dst: {%v}. Err: %v", dst, err)
			return nil, err
		}
		ruleMatch.DstAddress = dstIPRange

	}

	return &ruleMatch, nil
}

func (hd *Datapath) convertIPRange(sel string) ([]*halproto.IPAddressObj, error) {

	if len(sel) == 0 {
		return nil, ErrIPParse
	}

	ipRange := strings.Split(sel, "-")
	if len(ipRange) != 2 {
		log.Errorf("could not parse IP Range from selector. {%v}", sel)
		return nil, ErrIPParse
	}

	startIP := net.ParseIP(strings.TrimSpace(ipRange[0]))
	if len(startIP) == 0 {
		log.Errorf("could not parse IP from {%v}", ipRange[0])
		return nil, ErrIPParse
	}
	endIP := net.ParseIP(strings.TrimSpace(ipRange[1]))
	if len(endIP) == 0 {
		log.Errorf("could not parse IP from {%v}", endIP)
		return nil, ErrIPParse
	}

	lowIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(startIP),
		},
	}

	highIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(endIP),
		},
	}

	addrRange := &halproto.Address_Range{
		Range: &halproto.AddressRange{
			Range: &halproto.AddressRange_Ipv4Range{
				Ipv4Range: &halproto.IPRange{
					LowIpaddr:  &lowIP,
					HighIpaddr: &highIP,
				},
			},
		},
	}

	addressObj := []*halproto.IPAddressObj{
		{
			Formats: &halproto.IPAddressObj_Address{
				Address: &halproto.Address{
					Address: addrRange,
				},
			},
		},
	}

	return addressObj, nil
}

func (hd *Datapath) convertNatRuleAction(action string, vrfID, poolID uint64) (*halproto.NatRuleAction, error) {
	// Build the NaT Pool Vrf Key
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}

	// Build the Nat Pool Key
	poolKey := &halproto.NatPoolKeyHandle{
		KeyOrHandle: &halproto.NatPoolKeyHandle_PoolKey{
			PoolKey: &halproto.NatPoolKey{
				VrfKh:  vrfKey,
				PoolId: poolID,
			},
		},
	}

	switch action {
	case "SNAT":
		natAction := &halproto.NatRuleAction{
			SrcNatAction: halproto.NatAction_NAT_TYPE_DYNAMIC_ADDRESS,
			SrcNatPool:   poolKey,
		}
		return natAction, nil
	case "DNAT":
		natAction := &halproto.NatRuleAction{
			DstNatAction: halproto.NatAction_NAT_TYPE_DYNAMIC_ADDRESS,
			DstNatPool:   poolKey,
		}
		return natAction, nil
	default:
		log.Errorf("Invalid Nat action. %v", action)
		return nil, ErrInvalidNatActionType
	}
}

func (hd *Datapath) convertIPSecRuleAction(saType string, vrfID, ruleID uint64) (*halproto.IpsecSAAction, error) {
	switch saType {
	case "ENCRYPT":
		encryptAction := &halproto.IpsecSAAction{
			SaActionType: halproto.IpsecSAActionType_IPSEC_SA_ACTION_TYPE_ENCRYPT,
			SaHandle: &halproto.IpsecSAAction_EncHandle{
				EncHandle: &halproto.IpsecSAEncryptKeyHandle{
					KeyOrHandle: &halproto.IpsecSAEncryptKeyHandle_CbId{
						CbId: ruleID,
					},
				},
			},
		}
		return encryptAction, nil
	case "DECRYPT":
		decryptAction := &halproto.IpsecSAAction{
			SaActionType: halproto.IpsecSAActionType_IPSEC_SA_ACTION_TYPE_DECRYPT,
			SaHandle: &halproto.IpsecSAAction_DecHandle{
				DecHandle: &halproto.IpsecSADecryptKeyHandle{
					KeyOrHandle: &halproto.IpsecSADecryptKeyHandle_CbId{
						CbId: ruleID,
					},
				},
			},
		}
		return decryptAction, nil
	default:
		log.Errorf("Invalid IPSec Action type. %s", saType)
		return nil, ErrInvalidIPSecSAType
	}

}

func ipv4Touint32(ip net.IP) uint32 {
	if len(ip) == 16 {
		return binary.BigEndian.Uint32(ip[12:16])
	}
	return binary.BigEndian.Uint32(ip)
}

func intToIPv4(intIP uint32) net.IP {
	ip := make(net.IP, 4)
	binary.BigEndian.PutUint32(ip, intIP)
	return ip
}

func convertAuthAlgorithm(algorithm string) halproto.AuthenticationAlgorithm {
	switch algorithm {
	case "AES_GCM":
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_AES_GCM
	case "AES_CCM":
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_AES_CCM
	case "HMAC":
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_HMAC
	case "AES_CBC_SHA":
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_AES_CBC_SHA
	default:
		return halproto.AuthenticationAlgorithm_AUTHENTICATION_ALGORITHM_NONE
	}
}

func convertEncryptionAlgorithm(algorithm string) halproto.EncryptionAlgorithm {
	switch algorithm {
	case "AES_GCM_128":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_GCM_128
	case "AES_GCM_256":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_GCM_256
	case "AES_CCM_128":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CCM_128
	case "AES_CCM_192":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CCM_192
	case "AES_CCM_256":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CCM_256
	case "AES_CBC_128":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CBC_128
	case "AES_CBC_192":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CBC_192
	case "AES_CBC_256":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_AES_CBC_256
	case "DES3":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_DES3
	case "CHA_CHA":
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_CHA_CHA
	default:
		return halproto.EncryptionAlgorithm_ENCRYPTION_ALGORITHM_NONE
	}
}
