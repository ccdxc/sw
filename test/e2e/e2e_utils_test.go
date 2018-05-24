// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package e2e_test

import (
	"context"
	"fmt"
	"net"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/orch"
	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	n "github.com/pensando/sw/venice/utils/netutils"
)

// createVM creates a VM using vcsim api
func (s *e2eSuite) createVM(name, ipAddr, usegVlan, nw, hostMac string) (*simapi.NwIFSetResp, error) {
	addr, _, err := net.ParseCIDR(ipAddr)
	if err != nil {
		return nil, err
	}
	macAddr := fmt.Sprintf("00:50:%02x:%02x:%02x:%02x", addr[12], addr[13], addr[14], addr[15])

	nwReq := &simapi.NwIFSetReq{
		Name:      name,
		IPAddr:    ipAddr,
		MacAddr:   macAddr,
		Vlan:      usegVlan,
		PortGroup: nw,
		SmartNIC:  hostMac,
	}
	resp := &simapi.NwIFSetResp{}
	err = n.HTTPPost(defVCSimURL+"/nwifs/create", nwReq, resp)

	return resp, err
}

// deleteVM deletes a VM using vcsim api
func (s *e2eSuite) deleteVM(uuid string) error {
	r := &simapi.NwIFSetReq{}
	del := &simapi.NwIFDelResp{}
	u1 := fmt.Sprintf("%s/nwifs/%s/delete", defVCSimURL, uuid)
	return n.HTTPPost(u1, r, del)
}

// vchubNwifExists checks if nwif exists in vc hub
func (s *e2eSuite) vchubNwifExists(ipAddr, hostMac string) bool {
	filter := &orch.Filter{}
	ifList, err := s.vcHubClient.ListNwIFs(context.Background(), filter)
	if err != nil {
		return false
	}
	nwIfs := ifList.GetItems()
	for _, nwif := range nwIfs {
		if nwif.Status.IpAddress == "10.1.1.1" {
			if hostMac != "" {
				if nwif.Status.SmartNIC_ID == hostMac {
					return true
				}
				return false
			}
			return true
		}
	}
	return false
}

// vchubGetNics gets smartnic list from vchub
func (s *e2eSuite) vchubGetNics() ([]*orch.SmartNIC, error) {
	filter := &orch.Filter{}
	nicList, err := s.vcHubClient.ListSmartNICs(context.Background(), filter)
	if err != nil {
		return nil, err
	}
	nics := nicList.GetItems()
	return nics, nil
}

// createNetwork creates a network using REST api
func (s *e2eSuite) createNetwork(tenant, net, subnet, gw string) (*network.Network, error) {
	// build network object
	nw := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: "",
			Tenant:    tenant,
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
		},
		Status: network.NetworkStatus{},
	}

	ctx, err := s.NewLoggedInContext(context.Background())
	if err != nil {
		return nil, err
	}
	// create it
	return s.restClient.NetworkV1().Network().Create(ctx, &nw)
}

// deleteNetwork deletes a network using REST api
func (s *e2eSuite) deleteNetwork(tenant, net string) (*network.Network, error) {
	// build network object
	ometa := api.ObjectMeta{
		Name:      net,
		Namespace: "",
		Tenant:    tenant,
	}

	ctx, err := s.NewLoggedInContext(context.Background())
	if err != nil {
		return nil, err
	}
	// delete it
	return s.restClient.NetworkV1().Network().Delete(ctx, &ometa)
}

func (s *e2eSuite) NewLoggedInContext(ctx context.Context) (context.Context, error) {
	nctx, err := testutils.NewLoggedInContext(ctx, apigwURL, &auth.PasswordCredential{Username: user, Password: password, Tenant: "default"})
	return nctx, err
}
