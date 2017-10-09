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
	"sync"

	"github.com/golang/mock/gomock"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	halGRPCDefaultBaseURL = "localhost"
	halGRPCDefaultPort    = "50054"
)

// Kind holds the HAL Datapath kind. It could either be mock HAL or real HAL.
type Kind string

// Hal holds clients to talk to HAL gRPC server.
type Hal struct {
	client      *rpckit.RPCClient
	mockCtrl    *gomock.Controller
	MockClients mockClients
	Epclient    halproto.EndpointClient
	Ifclient    halproto.InterfaceClient
	Netclient   halproto.L2SegmentClient
	Lbclient    halproto.L4LbClient
	Sgclient    halproto.NwSecurityClient
	Sessclient  halproto.SessionClient
	Tnclient    halproto.TenantClient
}

// MockClients stores references for mockclients to be used for setting expectations
type mockClients struct {
	MockEpclient   *halproto.MockEndpointClient
	MockIfclient   *halproto.MockInterfaceClient
	MockNetclient  *halproto.MockL2SegmentClient
	MockLbclient   *halproto.MockL4LbClient
	MockSgclient   *halproto.MockNwSecurityClient
	MockSessclient *halproto.MockSessionClient
	MockTnclient   *halproto.MockTenantClient
}

// DB holds all the state information.
type DB struct {
	EndpointDB       map[string]*halproto.EndpointRequestMsg
	EndpointUpdateDB map[string]*halproto.EndpointUpdateRequestMsg
	EndpointDelDB    map[string]*halproto.EndpointDeleteRequestMsg
	SgDB             map[string]*halproto.SecurityGroupRequestMsg
}

// HalDatapath contains mock and hal clients.
type HalDatapath struct {
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
func NewHalDatapath(kind Kind) (*HalDatapath, error) {
	var err error
	var hal Hal
	haldp := HalDatapath{}
	haldp.Kind = kind

	db := DB{EndpointDB: make(map[string]*halproto.EndpointRequestMsg),
		EndpointUpdateDB: make(map[string]*halproto.EndpointUpdateRequestMsg),
		EndpointDelDB:    make(map[string]*halproto.EndpointDeleteRequestMsg),
		SgDB:             make(map[string]*halproto.SecurityGroupRequestMsg),
	}
	haldp.DB = db
	if haldp.Kind.String() == "hal" {
		hal.client, err = hal.createNewGRPCClient()
		if err != nil {
			return nil, err
		}
		hal.Epclient = halproto.NewEndpointClient(hal.client.ClientConn)
		hal.Ifclient = halproto.NewInterfaceClient(hal.client.ClientConn)
		hal.Netclient = halproto.NewL2SegmentClient(hal.client.ClientConn)
		hal.Lbclient = halproto.NewL4LbClient(hal.client.ClientConn)
		hal.Sgclient = halproto.NewNwSecurityClient(hal.client.ClientConn)
		hal.Sessclient = halproto.NewSessionClient(hal.client.ClientConn)
		hal.Tnclient = halproto.NewTenantClient(hal.client.ClientConn)
		haldp.Hal = hal
		return &haldp, nil
	}
	hal.mockCtrl = gomock.NewController(&hal)
	hal.MockClients = mockClients{
		MockEpclient:   halproto.NewMockEndpointClient(hal.mockCtrl),
		MockIfclient:   halproto.NewMockInterfaceClient(hal.mockCtrl),
		MockNetclient:  halproto.NewMockL2SegmentClient(hal.mockCtrl),
		MockLbclient:   halproto.NewMockL4LbClient(hal.mockCtrl),
		MockSgclient:   halproto.NewMockNwSecurityClient(hal.mockCtrl),
		MockSessclient: halproto.NewMockSessionClient(hal.mockCtrl),
		MockTnclient:   halproto.NewMockTenantClient(hal.mockCtrl),
	}

	hal.Epclient = hal.MockClients.MockEpclient
	hal.Ifclient = hal.MockClients.MockIfclient
	hal.Netclient = hal.MockClients.MockNetclient
	hal.Lbclient = hal.MockClients.MockLbclient
	hal.Sgclient = hal.MockClients.MockSgclient
	hal.Sessclient = hal.MockClients.MockSessclient
	hal.Tnclient = hal.MockClients.MockTnclient
	haldp.Hal = hal
	return &haldp, nil
}

func (hd *Hal) createNewGRPCClient() (*rpckit.RPCClient, error) {
	halPort := os.Getenv("HAL_GRPC_PORT")
	if halPort == "" {
		halPort = halGRPCDefaultPort
	}
	srvURL := halGRPCDefaultBaseURL + ":" + halPort
	// create a grpc client
	// ToDo Use AgentID for mysvcName
	rpcClient, err := rpckit.NewRPCClient("hal", srvURL)
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
func (hd *HalDatapath) FindEndpoint(epKey string) (*halproto.EndpointRequestMsg, error) {
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
func (hd *HalDatapath) FindEndpointDel(epKey string) (*halproto.EndpointDeleteRequestMsg, error) {
	hd.Lock()
	epdr, ok := hd.DB.EndpointDelDB[epKey]
	hd.Unlock()
	if !ok {
		return nil, errors.New("Endpoint delete record not found")
	}

	return epdr, nil
}

// GetEndpointCount returns number of endpoints in db
func (hd *HalDatapath) GetEndpointCount() int {
	hd.Lock()
	defer hd.Unlock()
	return len(hd.DB.EndpointDB)
}

// SetAgent sets the agent for this datapath
func (hd *HalDatapath) SetAgent(ag state.DatapathIntf) error {
	return nil
}

// CreateLocalEndpoint creates an endpoint
func (hd *HalDatapath) CreateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) (*state.IntfInfo, error) {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
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
	var sgids []uint32
	for _, sg := range sgs {
		sgids = append(sgids, sg.Status.SecurityGroupID)
	}

	// build endpoint message
	epinfo := halproto.EndpointSpec{
		Meta:            &halproto.ObjectMeta{},
		L2SegmentHandle: nw.Status.NetworkHandle,
		MacAddress:      macaddr,
		InterfaceHandle: 0, // FIXME
		UsegVlan:        ep.Status.UsegVlan,
		IpAddress:       []*halproto.IPAddress{&v4Addr, &v6Addr},
		SecurityGroup:   sgids,
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// call hal to create the endpoint
	// FIXME: handle response
	_, err := hd.Hal.Epclient.EndpointCreate(context.Background(), &epReq)
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
func (hd *HalDatapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
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
	var sgids []uint32
	for _, sg := range sgs {
		sgids = append(sgids, sg.Status.SecurityGroupID)
	}

	// build endpoint message
	epinfo := halproto.EndpointSpec{
		Meta:            &halproto.ObjectMeta{},
		L2SegmentHandle: nw.Status.NetworkHandle,
		MacAddress:      macaddr,
		InterfaceHandle: 0, // FIXME
		UsegVlan:        ep.Status.UsegVlan,
		IpAddress:       []*halproto.IPAddress{&v4Addr, &v6Addr},
		SecurityGroup:   sgids,
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// call hal to create the endpoint
	// FIXME: handle response
	_, err := hd.Hal.Epclient.EndpointCreate(context.Background(), &epReq)
	if err != nil {
		log.Errorf("Error creating endpoint. Err: %v", err)
		return err
	}

	// save the endpoint message
	hd.Lock()
	hd.DB.EndpointDB[objectKey(&ep.ObjectMeta)] = &epReq
	hd.Unlock()

	return nil
}

// UpdateLocalEndpoint updates the endpoint
func (hd *HalDatapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
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
	var sgids []uint32
	for _, sg := range sgs {
		sgids = append(sgids, sg.Status.SecurityGroupID)
	}

	// build endpoint message
	epUpdateReq := halproto.EndpointUpdateRequest{
		Meta:            &halproto.ObjectMeta{},
		L2SegmentHandle: nw.Status.NetworkHandle,
		MacAddress:      macaddr,
		InterfaceHandle: 0, // FIXME
		UsegVlan:        ep.Status.UsegVlan,
		IpAddress:       []*halproto.IPAddress{&v4Addr, &v6Addr},
		SecurityGroup:   sgids,
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
func (hd *HalDatapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
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
	var sgids []uint32
	for _, sg := range sgs {
		sgids = append(sgids, sg.Status.SecurityGroupID)
	}

	// build endpoint message
	epUpdateReq := halproto.EndpointUpdateRequest{
		Meta:            &halproto.ObjectMeta{},
		L2SegmentHandle: nw.Status.NetworkHandle,
		MacAddress:      macaddr,
		InterfaceHandle: 0, // FIXME
		UsegVlan:        ep.Status.UsegVlan,
		IpAddress:       []*halproto.IPAddress{&v4Addr, &v6Addr},
		SecurityGroup:   sgids,
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
func (hd *HalDatapath) DeleteLocalEndpoint(ep *netproto.Endpoint) error {
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
func (hd *HalDatapath) DeleteRemoteEndpoint(ep *netproto.Endpoint) error {
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

// CreateNetwork creates a network in datapath
func (hd *HalDatapath) CreateNetwork(nw *netproto.Network) error {
	// build l2 segment data
	seg := halproto.L2SegmentSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		SegmentType:    halproto.L2SegmentType_L2_SEGMENT_TYPE_TENANT,
		McastFwdPolicy: halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD,
		BcastFwdPolicy: halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD,
		AccessEncap: &halproto.EncapInfo{
			EncapType:  halproto.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
		FabricEncap: &halproto.EncapInfo{
			EncapType:  halproto.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
	}
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// create the l2 segment
	_, err := hd.Hal.Netclient.L2SegmentCreate(context.Background(), &segReq)
	if err != nil {
		log.Errorf("Error creating network. Err: %v", err)
		return err
	}

	return nil
}

// UpdateNetwork updates a network in datapath
func (hd *HalDatapath) UpdateNetwork(nw *netproto.Network) error {
	// build l2 segment data
	seg := halproto.L2SegmentSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		SegmentType:    halproto.L2SegmentType_L2_SEGMENT_TYPE_TENANT,
		McastFwdPolicy: halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD,
		BcastFwdPolicy: halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD,
		AccessEncap: &halproto.EncapInfo{
			EncapType:  halproto.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
		FabricEncap: &halproto.EncapInfo{
			EncapType:  halproto.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
	}
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// update the l2 segment
	_, err := hd.Hal.Netclient.L2SegmentUpdate(context.Background(), &segReq)
	if err != nil {
		log.Errorf("Error updating network. Err: %v", err)
		return err
	}

	return nil
}

// DeleteNetwork deletes a network from datapath
func (hd *HalDatapath) DeleteNetwork(nw *netproto.Network) error {
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
	_, err := hd.Hal.Netclient.L2SegmentDelete(context.Background(), &segDelReqMsg)
	if err != nil {
		log.Errorf("Error deleting network. Err: %v", err)
		return err
	}

	return nil
}

// CreateSecurityGroup creates a security group
func (hd *HalDatapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	var inrules []*halproto.FirewallRuleSpec
	var outrules []*halproto.FirewallRuleSpec

	// convert the rules
	for _, rl := range sg.Spec.Rules {
		hrule, err := hd.convertRule(sg, &rl)
		if err != nil {
			return err
		}

		// append it to the list
		if rl.Direction == "outgoing" {
			outrules = append(outrules, hrule)
		} else {
			inrules = append(inrules, hrule)
		}
	}

	// build security group message
	sgs := halproto.SecurityGroupSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
		IngressPolicy: &halproto.IngressSGPolicy{
			FwRules: inrules,
		},
		EgressPolicy: &halproto.EgressSGPolicy{
			FwRules: outrules,
		},
	}
	sgmsg := halproto.SecurityGroupRequestMsg{
		Request: []*halproto.SecurityGroupSpec{&sgs},
	}

	// add security group
	_, err := hd.Hal.Sgclient.SecurityGroupCreate(context.Background(), &sgmsg)
	if err != nil {
		log.Errorf("Error creating security group. Err: %v", err)
		//return nil
	}

	// save the sg message
	hd.Lock()
	hd.DB.SgDB[objectKey(&sg.ObjectMeta)] = &sgmsg
	hd.Unlock()

	return nil
}

// UpdateSecurityGroup updates a security group
func (hd *HalDatapath) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	var inrules []*halproto.FirewallRuleSpec
	var outrules []*halproto.FirewallRuleSpec

	// convert the rules
	for _, rl := range sg.Spec.Rules {
		hrule, err := hd.convertRule(sg, &rl)
		if err != nil {
			return err
		}

		// append it to the list
		if rl.Direction == "outgoing" {
			outrules = append(outrules, hrule)
		} else {
			inrules = append(inrules, hrule)
		}
	}

	// build security group message
	sgs := halproto.SecurityGroupSpec{
		Meta: &halproto.ObjectMeta{},
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
		IngressPolicy: &halproto.IngressSGPolicy{
			FwRules: inrules,
		},
		EgressPolicy: &halproto.EgressSGPolicy{
			FwRules: outrules,
		},
	}
	sgmsg := halproto.SecurityGroupRequestMsg{
		Request: []*halproto.SecurityGroupSpec{&sgs},
	}

	// update security group
	_, err := hd.Hal.Sgclient.SecurityGroupUpdate(context.Background(), &sgmsg)
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
func (hd *HalDatapath) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
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

func (hd *HalDatapath) convertRule(sg *netproto.SecurityGroup, rule *netproto.SecurityRule) (*halproto.FirewallRuleSpec, error) {
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

	// build security rule
	rl := halproto.FirewallRuleSpec{
		PeerSecurityGroup: []uint32{rule.PeerGroupID},
		Svc:               srvs,
		Action:            act,
		Log:               rule.Log,
	}

	return &rl, nil
}
