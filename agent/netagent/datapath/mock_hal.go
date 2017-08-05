// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package datapath

import (
	"context"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"regexp"
	"strconv"

	"github.com/Sirupsen/logrus"
	"github.com/golang/mock/gomock"
	"github.com/pensando/sw/agent/netagent"
	"github.com/pensando/sw/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/agent/netagent/datapath/halproto/types"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
)

// MockHalDatapath contains mock hal clients
type MockHalDatapath struct {
	MockCtrl      *gomock.Controller //
	Epclient      *halproto.MockEndpointClient
	Ifclient      *halproto.MockInterfaceClient
	Netclient     *halproto.MockL2SegmentClient
	Lbclient      *halproto.MockL4LbClient
	Sgclient      *halproto.MockNwSecurityClient
	Sessclient    *halproto.MockSessionClient
	Tnclient      *halproto.MockTenantClient
	EndpointDB    map[string]*halproto.EndpointRequestMsg
	EndpointDelDB map[string]*halproto.EndpointDeleteRequestMsg
	SgDB          map[string]*halproto.SecurityGroupMsg
	SgRule        map[string]*halproto.SecurityPolicyRuleMsg
}

// NewMockHalDatapath returns a mock hal datapath
func NewMockHalDatapath() (*MockHalDatapath, error) {
	haldp := MockHalDatapath{}

	// create mock controller
	haldp.MockCtrl = gomock.NewController(&haldp)

	// create mock hal clients
	haldp.Epclient = halproto.NewMockEndpointClient(haldp.MockCtrl)
	haldp.Ifclient = halproto.NewMockInterfaceClient(haldp.MockCtrl)
	haldp.Netclient = halproto.NewMockL2SegmentClient(haldp.MockCtrl)
	haldp.Lbclient = halproto.NewMockL4LbClient(haldp.MockCtrl)
	haldp.Sgclient = halproto.NewMockNwSecurityClient(haldp.MockCtrl)
	haldp.Sessclient = halproto.NewMockSessionClient(haldp.MockCtrl)
	haldp.Tnclient = halproto.NewMockTenantClient(haldp.MockCtrl)

	// init message databases
	haldp.EndpointDB = make(map[string]*halproto.EndpointRequestMsg)
	haldp.EndpointDelDB = make(map[string]*halproto.EndpointDeleteRequestMsg)
	haldp.SgDB = make(map[string]*halproto.SecurityGroupMsg)
	haldp.SgRule = make(map[string]*halproto.SecurityPolicyRuleMsg)

	return &haldp, nil
}

// Errorf for satisfying gomock
func (hd *MockHalDatapath) Errorf(format string, args ...interface{}) {
	logrus.Errorf(format, args...)
}

// Fatalf for satisfying gomock
func (hd *MockHalDatapath) Fatalf(format string, args ...interface{}) {
	logrus.Fatalf(format, args...)
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

// CreateLocalEndpoint creates an endpoint
func (hd *MockHalDatapath) CreateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) (*netagent.IntfInfo, error) {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
	ipaddr, _, _ := net.ParseCIDR(ep.Status.IPv4Address)

	// convert v4 address
	v4Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET,
		V4OrV6: &types.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// convert v6 address
	v6Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET6,
		V4OrV6: &types.IPAddress_V6Addr{
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
		Meta:            &types.ObjectMeta{},
		L2SegmentHandle: nw.Status.NetworkHandle,
		MacAddress:      macaddr,
		InterfaceHandle: 0, // FIXME
		UsegVlan:        ep.Status.UsegVlan,
		IpAddress:       []*types.IPAddress{&v4Addr, &v6Addr},
		SecurityGroup:   sgids,
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// call hal to create the endpoint
	// FIXME: handle response
	_, err := hd.Epclient.EndpointCreate(context.Background(), &epReq)
	if err != nil {
		logrus.Errorf("Error creating endpoint. Err: %v", err)
		return nil, err
	}

	// save the endpoint message
	hd.EndpointDB[objectKey(&ep.ObjectMeta)] = &epReq

	return nil, nil
}

// CreateRemoteEndpoint creates remote endpoint
func (hd *MockHalDatapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
	ipaddr, _, _ := net.ParseCIDR(ep.Status.IPv4Address)

	// convert v4 address
	v4Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET,
		V4OrV6: &types.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// convert v6 address
	v6Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET6,
		V4OrV6: &types.IPAddress_V6Addr{
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
		Meta:            &types.ObjectMeta{},
		L2SegmentHandle: nw.Status.NetworkHandle,
		MacAddress:      macaddr,
		InterfaceHandle: 0, // FIXME
		UsegVlan:        ep.Status.UsegVlan,
		IpAddress:       []*types.IPAddress{&v4Addr, &v6Addr},
		SecurityGroup:   sgids,
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// call hal to create the endpoint
	// FIXME: handle response
	_, err := hd.Epclient.EndpointCreate(context.Background(), &epReq)
	if err != nil {
		logrus.Errorf("Error creating endpoint. Err: %v", err)
		return err
	}

	// save the endpoint message
	hd.EndpointDB[objectKey(&ep.ObjectMeta)] = &epReq

	return nil
}

// UpdateLocalEndpoint updates the endpoint
func (hd *MockHalDatapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
	ipaddr, _, _ := net.ParseCIDR(ep.Status.IPv4Address)

	// convert v4 address
	v4Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET,
		V4OrV6: &types.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// convert v6 address
	v6Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET6,
		V4OrV6: &types.IPAddress_V6Addr{
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
		Meta:            &types.ObjectMeta{},
		L2SegmentHandle: nw.Status.NetworkHandle,
		MacAddress:      macaddr,
		InterfaceHandle: 0, // FIXME
		UsegVlan:        ep.Status.UsegVlan,
		IpAddress:       []*types.IPAddress{&v4Addr, &v6Addr},
		SecurityGroup:   sgids,
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// call hal to update the endpoint
	// FIXME: handle response
	_, err := hd.Epclient.EndpointUpdate(context.Background(), &epReq)
	if err != nil {
		logrus.Errorf("Error creating endpoint. Err: %v", err)
		return err
	}

	// save the endpoint message
	hd.EndpointDB[objectKey(&ep.ObjectMeta)] = &epReq

	return nil
}

// UpdateRemoteEndpoint updates an existing endpoint
func (hd *MockHalDatapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network, sgs []*netproto.SecurityGroup) error {
	// convert mac address
	var macStripRegexp = regexp.MustCompile(`[^a-fA-F0-9]`)
	hex := macStripRegexp.ReplaceAllLiteralString(ep.Status.MacAddress, "")
	macaddr, _ := strconv.ParseUint(hex, 16, 64)
	ipaddr, _, _ := net.ParseCIDR(ep.Status.IPv4Address)

	// convert v4 address
	v4Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET,
		V4OrV6: &types.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// convert v6 address
	v6Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET6,
		V4OrV6: &types.IPAddress_V6Addr{
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
		Meta:            &types.ObjectMeta{},
		L2SegmentHandle: nw.Status.NetworkHandle,
		MacAddress:      macaddr,
		InterfaceHandle: 0, // FIXME
		UsegVlan:        ep.Status.UsegVlan,
		IpAddress:       []*types.IPAddress{&v4Addr, &v6Addr},
		SecurityGroup:   sgids,
	}
	epReq := halproto.EndpointRequestMsg{
		Request: []*halproto.EndpointSpec{&epinfo},
	}

	// call hal to update the endpoint
	// FIXME: handle response
	_, err := hd.Epclient.EndpointUpdate(context.Background(), &epReq)
	if err != nil {
		logrus.Errorf("Error creating endpoint. Err: %v", err)
		return err
	}

	// save the endpoint message
	hd.EndpointDB[objectKey(&ep.ObjectMeta)] = &epReq

	return nil
}

// DeleteLocalEndpoint deletes an endpoint
func (hd *MockHalDatapath) DeleteLocalEndpoint(ep *netproto.Endpoint) error {
	// convert v4 address
	ipaddr, _, err := net.ParseCIDR(ep.Status.IPv4Address)
	logrus.Infof("Deleting endpoint: {%+v}, addr: %v/%v. Err: %v", ep, ep.Status.IPv4Address, ipaddr, err)
	v4Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET,
		V4OrV6: &types.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// build endpoint del request
	epdel := halproto.EndpointDeleteRequest{
		Meta: &types.ObjectMeta{},
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
	_, err = hd.Epclient.EndpointDelete(context.Background(), &delReq)
	if err != nil {
		logrus.Errorf("Error deleting endpoint. Err: %v", err)
		return err
	}

	// save the endpoint delete message
	hd.EndpointDelDB[objectKey(&ep.ObjectMeta)] = &delReq
	delete(hd.EndpointDB, objectKey(&ep.ObjectMeta))

	return nil
}

// DeleteRemoteEndpoint deletes remote endpoint
func (hd *MockHalDatapath) DeleteRemoteEndpoint(ep *netproto.Endpoint) error {
	// convert v4 address
	ipaddr, _, err := net.ParseCIDR(ep.Status.IPv4Address)
	logrus.Infof("Deleting endpoint: {%+v}, addr: %v/%v. Err: %v", ep, ep.Status.IPv4Address, ipaddr, err)
	v4Addr := types.IPAddress{
		IpAf: types.IPAddressFamily_IP_AF_INET,
		V4OrV6: &types.IPAddress_V4Addr{
			V4Addr: ipv42int(ipaddr),
		},
	}

	// build endpoint del request
	epdel := halproto.EndpointDeleteRequest{
		Meta: &types.ObjectMeta{},
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
	_, err = hd.Epclient.EndpointDelete(context.Background(), &delReq)
	if err != nil {
		logrus.Errorf("Error deleting endpoint. Err: %v", err)
		return err
	}

	// save the endpoint delete message
	hd.EndpointDelDB[objectKey(&ep.ObjectMeta)] = &delReq
	delete(hd.EndpointDB, objectKey(&ep.ObjectMeta))

	return nil
}

// CreateNetwork creates a network in datapath
func (hd *MockHalDatapath) CreateNetwork(nw *netproto.Network) error {
	// build l2 segment data
	seg := halproto.L2SegmentSpec{
		Meta: &types.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		SegmentType:    types.L2SegmentType_L2_SEGMENT_TYPE_TENANT,
		McastFwdPolicy: halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD,
		BcastFwdPolicy: halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD,
		AccessEncap: &types.EncapInfo{
			EncapType:  types.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
		FabricEncap: &types.EncapInfo{
			EncapType:  types.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
		L4ProfileHandle: 0, //FIXME: what should go here?
	}
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// create the l2 segment
	_, err := hd.Netclient.L2SegmentCreate(context.Background(), &segReq)
	if err != nil {
		logrus.Errorf("Error creating network. Err: %v", err)
		return err
	}

	return nil
}

// UpdateNetwork updates a network in datapath
func (hd *MockHalDatapath) UpdateNetwork(nw *netproto.Network) error {
	// build l2 segment data
	seg := halproto.L2SegmentSpec{
		Meta: &types.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
		SegmentType:    types.L2SegmentType_L2_SEGMENT_TYPE_TENANT,
		McastFwdPolicy: halproto.MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD,
		BcastFwdPolicy: halproto.BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD,
		AccessEncap: &types.EncapInfo{
			EncapType:  types.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
		FabricEncap: &types.EncapInfo{
			EncapType:  types.EncapType_ENCAP_TYPE_DOT1Q,
			EncapValue: nw.Spec.VlanID,
		},
		L4ProfileHandle: 0, //FIXME: what should go here?
	}
	segReq := halproto.L2SegmentRequestMsg{
		Request: []*halproto.L2SegmentSpec{&seg},
	}

	// update the l2 segment
	_, err := hd.Netclient.L2SegmentUpdate(context.Background(), &segReq)
	if err != nil {
		logrus.Errorf("Error creating network. Err: %v", err)
		return err
	}

	return nil
}

// DeleteNetwork deletes a network from datapath
func (hd *MockHalDatapath) DeleteNetwork(nw *netproto.Network) error {
	// build the segment message
	seg := halproto.L2SegmentDeleteRequest{
		Meta: &types.ObjectMeta{},
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: nw.Status.NetworkID,
			},
		},
	}
	segReq := halproto.L2SegmentDeleteRequestMsg{
		SegmentId: []*halproto.L2SegmentDeleteRequest{&seg},
	}

	// delete the l2 segment
	_, err := hd.Netclient.L2SegmentDelete(context.Background(), &segReq)
	if err != nil {
		logrus.Errorf("Error creating network. Err: %v", err)
		return err
	}

	return nil
}

// CreateSecurityGroup creates a security group
func (hd *MockHalDatapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	// build security group message
	sgs := halproto.SecurityGroupSpec{
		Meta: &types.ObjectMeta{},
		SgId: sg.Status.SecurityGroupID,
	}
	sgmsg := halproto.SecurityGroupMsg{
		Request: []*halproto.SecurityGroupSpec{&sgs},
	}

	// add security group
	_, err := hd.Sgclient.SecurityGroupCreate(context.Background(), &sgmsg)
	if err != nil {
		logrus.Errorf("Error creating network. Err: %v", err)
		return err
	}

	// save the sg message
	hd.SgDB[objectKey(&sg.ObjectMeta)] = &sgmsg

	return nil
}

// UpdateSecurityGroup updates a security group
func (hd *MockHalDatapath) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	// build security group message
	sgs := halproto.SecurityGroupSpec{
		Meta: &types.ObjectMeta{},
		SgId: sg.Status.SecurityGroupID,
	}
	sgmsg := halproto.SecurityGroupMsg{
		Request: []*halproto.SecurityGroupSpec{&sgs},
	}

	// update security group
	_, err := hd.Sgclient.SecurityGroupUpdate(context.Background(), &sgmsg)
	if err != nil {
		logrus.Errorf("Error creating network. Err: %v", err)
		return err
	}

	// save the sg message
	hd.SgDB[objectKey(&sg.ObjectMeta)] = &sgmsg

	return nil
}

// DeleteSecurityGroup deletes a security group
func (hd *MockHalDatapath) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	// build security group message
	sgs := halproto.SecurityGroupSpec{
		Meta: &types.ObjectMeta{},
		SgId: sg.Status.SecurityGroupID,
	}
	sgmsg := halproto.SecurityGroupMsg{
		Request: []*halproto.SecurityGroupSpec{&sgs},
	}

	// delete security group
	_, err := hd.Sgclient.SecurityGroupDelete(context.Background(), &sgmsg)
	if err != nil {
		logrus.Errorf("Error creating network. Err: %v", err)
		return err
	}

	// delete the sg message
	delete(hd.SgDB, objectKey(&sg.ObjectMeta))

	return nil
}

// AddSecurityRule adds a security rule
func (hd *MockHalDatapath) AddSecurityRule(sg *netproto.SecurityGroup, rule *netproto.SecurityRule, peersg *netproto.SecurityGroup) error {
	// convert the action
	act := halproto.Action_SECURITY_POLICY_ACTION_NONE
	switch rule.Action {
	case "Allow":
		act = halproto.Action_SECURITY_POLICY_ACTION_ALLOW
	case "Deny":
		act = halproto.Action_SECURITY_POLICY_ACTION_DENY
	case "Reject":
		act = halproto.Action_SECURITY_POLICY_ACTION_REJECT
	default:
		logrus.Errorf("Unknown action %s in rule {%+v}", rule.Action, rule)
		return errors.New("Unknown action")
	}

	// build service list
	srvs := []*halproto.Service{}
	for _, svc := range rule.Services {
		proto := types.IPProtocol_IP_PROTO_NONE
		switch svc.Protocol {
		case "tcp":
			proto = types.IPProtocol_IP_PROTO_TCP
		case "udp":
			proto = types.IPProtocol_IP_PROTO_UDP
		case "icmp":
			proto = types.IPProtocol_IP_PROTO_ICMP
		default:
			logrus.Errorf("Unknown protocol %s in rule {%+v}", svc.Protocol, rule)
			return errors.New("Unknown protocol")
		}

		sr := halproto.Service{
			IpProtocol: proto,
			Port:       svc.Port,
		}

		srvs = append(srvs, &sr)
	}

	// check direction
	var srcSgID, dstSgID uint32
	switch rule.Direction {
	case "incoming":
		srcSgID = peersg.Status.SecurityGroupID
		dstSgID = sg.Status.SecurityGroupID
	case "outgoing":
		dstSgID = peersg.Status.SecurityGroupID
		srcSgID = sg.Status.SecurityGroupID
	default:
		logrus.Errorf("Unknown direction %s", rule.Direction)
		return errors.New("Unknown direction")
	}

	// build security rule
	rl := halproto.SecurityPolicyRuleSpec{
		Meta:      &types.ObjectMeta{},
		SrcSgId:   srcSgID,
		DstSgId:   dstSgID,
		Svc:       srvs,
		Action:    act,
		RuleLogEn: rule.Log,
	}
	rulemsg := halproto.SecurityPolicyRuleMsg{
		Request: []*halproto.SecurityPolicyRuleSpec{&rl},
	}
	_, err := hd.Sgclient.SecurityPolicyRuleCreate(context.Background(), &rulemsg)
	if err != nil {
		logrus.Errorf("Error creating network. Err: %v", err)
		return err
	}

	// save the rule in db
	hd.SgRule[rl.String()] = &rulemsg

	return nil
}

// DeleteSecurityRule deletes a security rule
func (hd *MockHalDatapath) DeleteSecurityRule(sg *netproto.SecurityGroup, rule *netproto.SecurityRule, peersg *netproto.SecurityGroup) error {
	// convert the action
	act := halproto.Action_SECURITY_POLICY_ACTION_NONE
	switch rule.Action {
	case "Allow":
		act = halproto.Action_SECURITY_POLICY_ACTION_ALLOW
	case "Deny":
		act = halproto.Action_SECURITY_POLICY_ACTION_DENY
	case "Reject":
		act = halproto.Action_SECURITY_POLICY_ACTION_REJECT
	default:
		logrus.Errorf("Unknown action %s in rule {%+v}", rule.Action, rule)
		return errors.New("Unknown action")
	}

	// build service list
	srvs := []*halproto.Service{}
	for _, svc := range rule.Services {
		proto := types.IPProtocol_IP_PROTO_NONE
		switch svc.Protocol {
		case "tcp":
			proto = types.IPProtocol_IP_PROTO_TCP
		case "udp":
			proto = types.IPProtocol_IP_PROTO_UDP
		case "icmp":
			proto = types.IPProtocol_IP_PROTO_ICMP
		default:
			logrus.Errorf("Unknown protocol %s in rule {%+v}", svc.Protocol, rule)
			return errors.New("Unknown protocol")
		}

		sr := halproto.Service{
			IpProtocol: proto,
			Port:       svc.Port,
		}

		srvs = append(srvs, &sr)
	}

	// check direction
	var srcSgID, dstSgID uint32
	switch rule.Direction {
	case "incoming":
		srcSgID = peersg.Status.SecurityGroupID
		dstSgID = sg.Status.SecurityGroupID
	case "outgoing":
		dstSgID = peersg.Status.SecurityGroupID
		srcSgID = sg.Status.SecurityGroupID
	default:
		logrus.Errorf("Unknown direction %s", rule.Direction)
		return errors.New("Unknown direction")
	}

	// build security rule
	rl := halproto.SecurityPolicyRuleSpec{
		Meta:      &types.ObjectMeta{},
		SrcSgId:   srcSgID,
		DstSgId:   dstSgID,
		Svc:       srvs,
		Action:    act,
		RuleLogEn: rule.Log,
	}
	rulemsg := halproto.SecurityPolicyRuleMsg{
		Request: []*halproto.SecurityPolicyRuleSpec{&rl},
	}

	// delete the security rule
	_, err := hd.Sgclient.SecurityPolicyRuleDelete(context.Background(), &rulemsg)
	if err != nil {
		logrus.Errorf("Error creating network. Err: %v", err)
		return err
	}

	// verify we had the rule
	_, ok := hd.SgRule[rl.String()]
	if !ok {
		logrus.Errorf("Rule %s was not present in db", rl.String())
	}

	// delete from rule db
	delete(hd.SgRule, rl.String())

	return nil
}
