package objects

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

// IPAM Policy
type IPAMPolicy struct {
	PolicyObj *network.IPAMPolicy
}

type IPAMPolicyCollection struct {
	CollectionCommon
	err        error
	PolicyObjs []*IPAMPolicy
}

func NewIPAMPolicy(name string, tenant string, vrf string, ip string, client objClient.ObjClient, testbed *testbed.TestBed) *IPAMPolicyCollection {
	return &IPAMPolicyCollection{
		PolicyObjs: []*IPAMPolicy{
			{
				PolicyObj: &network.IPAMPolicy{
					TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
					ObjectMeta: api.ObjectMeta{
						Name:   name,
						Tenant: tenant},
					Spec: network.IPAMPolicySpec{
						Type: "dhcp-relay",
						DHCPRelay: &network.DHCPRelayPolicy{
							Servers: []*network.DHCPServer{
								{
									VirtualRouter: vrf,
									IPAddress:     ip,
								},
							},
						},
					},
				},
			},
		},
		CollectionCommon: CollectionCommon{Testbed: testbed, Client: client},
	}
}

// Commit writes the IPAMPolicies to Venice
func (ipc *IPAMPolicyCollection) Commit() error {
	if ipc.err != nil {
		log.Infof("Hit error on top of Commit itself")
		return ipc.err
	}

	for _, v := range ipc.PolicyObjs {
		err := ipc.Client.CreateIPAMPolicy(v.PolicyObj)
		if err != nil {
			ipc.err = err
			log.Infof("Creating IPAM policy failed %v", err)
			return err
		}
	}
	return nil
}

// Delete deletes all IPAMPolicies in the collection
func (ipc *IPAMPolicyCollection) Delete() error {
	if ipc.err != nil {
		return ipc.err
	}

	for _, v := range ipc.PolicyObjs {
		err := ipc.Client.DeleteIPAMPolicy(v.PolicyObj)
		if err != nil {
			ipc.err = err
			log.Infof("Deleting IPAM policy failed %v", err)
			return err
		}
	}
	return nil
}

func NewIPAMPolicyCollection(client objClient.ObjClient, testbed *testbed.TestBed) *IPAMPolicyCollection {
	return &IPAMPolicyCollection{
		CollectionCommon: CollectionCommon{Client: client, Testbed: testbed},
	}
}

func (ipc *IPAMPolicyCollection) AddServer(name string, server_ip string, vrf string) *IPAMPolicyCollection {

	Server := &network.DHCPServer{VirtualRouter: vrf, IPAddress: server_ip}
	for _, v := range ipc.PolicyObjs {
		if v.PolicyObj.Name == name {
			v.PolicyObj.Spec.DHCPRelay.Servers = append(v.PolicyObj.Spec.DHCPRelay.Servers, Server)
			break
		}
	}
	return ipc
}

func GetIPAMPolicy(client objClient.ObjClient, name string, tenant string) (*IPAMPolicy, error) {
	obj, err := client.GetIPAMPolicy(name, tenant)
	return &IPAMPolicy{PolicyObj: obj}, err
}