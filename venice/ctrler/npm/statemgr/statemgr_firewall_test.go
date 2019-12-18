// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"errors"
	"fmt"
	"strconv"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/generated/nimbus"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func newStatemgr() (*Statemgr, error) {
	// create nimbus server
	msrv := nimbus.NewMbusServer("npm-test", nil)

	// create network state manager
	stateMgr, err := NewStatemgr(nil, globals.APIServer, nil, msrv, log.GetNewLogger(log.GetDefaultConfig("npm-test")))
	if err != nil {
		log.Errorf("Could not create network manager. Err: %v", err)
		return nil, err
	}

	return stateMgr, nil
}

func startGrpcServer() (*rpckit.RPCServer, error) {
	grpcServer, err := rpckit.NewRPCServer("netctrler", "localhost:1234", rpckit.WithTLSProvider(nil))
	if err != nil {
		return nil, err
	}

	return grpcServer, nil
}

func newStatemgrWithMserver(grpcServer *rpckit.RPCServer) (*Statemgr, error) {

	// create nimbus server
	msrv := nimbus.NewMbusServer("npm-test", grpcServer)

	// create network state manager
	stateMgr, err := NewStatemgr(nil, globals.APIServer, nil, msrv, log.GetNewLogger(log.GetDefaultConfig("npm-test")))
	if err != nil {
		log.Errorf("Could not create network manager. Err: %v", err)
		return nil, err
	}

	if grpcServer != nil {
		grpcServer.Start()
	}
	return stateMgr, nil
}

func stopGrpcServer(grpcServer *rpckit.RPCServer) {
	grpcServer.Stop()
}

func TestSgpolicyCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create sg
	_, err = createSg(stateMgr, "default", "procurement", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")
	_, err = createSg(stateMgr, "default", "catalog", labels.SelectorFromSet(labels.Set{"env": "production", "app": "catalog"}))
	AssertOk(t, err, "Error creating security group")

	// sg policy
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sgpolicy",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachGroups: []string{"procurement"},
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: security.SGRule_PERMIT.String(),
				},
			},
		},
	}

	// create sg policy
	err = stateMgr.ctrler.NetworkSecurityPolicy().Create(&sgp)
	AssertOk(t, err, "Error creating the sg policy")

	// verify we can find the sg policy
	sgps, err := stateMgr.FindSgpolicy("default", "test-sgpolicy")
	AssertOk(t, err, "Could not find the sg policy")
	Assert(t, len(sgps.NetworkSecurityPolicy.Status.RuleStatus) == len(sgps.NetworkSecurityPolicy.Spec.Rules), "Rule status was not updated")
	Assert(t, sgps.NetworkSecurityPolicy.Status.RuleStatus[0].RuleHash != "", "Rule hash was not updated")

	// update sg policy
	newRule := security.SGRule{
		ProtoPorts: []security.ProtoPort{
			{
				Protocol: "tcp",
				Ports:    "8000",
			},
		},
		Action: security.SGRule_PERMIT.String(),
	}
	sgp.Spec.Rules = append(sgp.Spec.Rules, newRule)
	err = stateMgr.ctrler.NetworkSecurityPolicy().Update(&sgp)
	AssertOk(t, err, "Error updating sgpolicy")

	// verify we can not attach a policy to unknown sg
	sgp2 := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "sgpolicy2",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachGroups: []string{"unknown"},
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: security.SGRule_PERMIT.String(),
				},
			},
		},
	}
	err = stateMgr.ctrler.NetworkSecurityPolicy().Create(&sgp2)

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSgpolicy("default", "sgpolicy2")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	// delete the sg policy
	err = stateMgr.ctrler.NetworkSecurityPolicy().Delete(&sgp)
	AssertOk(t, err, "Error deleting security policy")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSgpolicy("default", "test-sgpolicy")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	// verify the sg policy is gone
	_, err = stateMgr.FindSgpolicy("default", "test-sgpolicy")
	Assert(t, (err != nil), "Security policy still found after deleting")
}

func createPolicy(s *Statemgr, name, version string) error {
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         name,
			GenerationID: version,
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: security.SGRule_PERMIT.String(),
				},
			},
		},
	}

	return s.ctrler.NetworkSecurityPolicy().Create(&sgp)
}

func createPolicyWithApps(s *Statemgr, name string, apps []string, version string) error {
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         name,
			GenerationID: version,
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: security.SGRule_PERMIT.String(),
					Apps:   apps,
				},
			},
		},
	}

	return s.ctrler.NetworkSecurityPolicy().Create(&sgp)
}

func createApp(s *Statemgr, name, port string) error {
	numApps := 7
	appSpecs := []*security.ALG{
		&security.ALG{
			Type: security.ALG_DNS.String(),
			Dns:  &security.Dns{DropLargeDomainNamePackets: true},
		},
		&security.ALG{
			Type: security.ALG_ICMP.String(),
			Icmp: &security.Icmp{},
		},

		&security.ALG{
			Type: security.ALG_FTP.String(),
			Ftp:  &security.Ftp{},
		},
		&security.ALG{
			Type:   security.ALG_SunRPC.String(),
			Sunrpc: []*security.Sunrpc{&security.Sunrpc{}},
		},
		&security.ALG{
			Type:  security.ALG_MSRPC.String(),
			Msrpc: []*security.Msrpc{&security.Msrpc{}},
		},
		&security.ALG{
			Type: security.ALG_TFTP.String(),
			//: []*security.ALG_TFTP{},
		},

		&security.ALG{
			Type: security.ALG_RTSP.String(),
		},
	}
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      name,
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				security.ProtoPort{
					Ports:    port,
					Protocol: "tcp",
				},
			},
			ALG: &security.ALG{
				Type: security.ALG_DNS.String(),
				Dns:  &security.Dns{DropLargeDomainNamePackets: true},
			},
		},
	}

	portInt, _ := strconv.Atoi(port)
	app.Spec.ALG = appSpecs[(portInt-100)%numApps]
	return s.ctrler.App().Create(&app)
}

func updateApp(s *Statemgr, name, port string) error {
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         name,
			GenerationID: "2",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				security.ProtoPort{
					Ports:    port,
					Protocol: "tcp",
				},
			},
		},
	}

	return s.ctrler.App().Update(&app)
}

func updatePolicy(s *Statemgr, name, version string) error {
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         name,
			GenerationID: version,
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: security.SGRule_PERMIT.String(),
				},
			},
		},
	}

	return s.ctrler.NetworkSecurityPolicy().Update(&sgp)
}

func deletePolicy(s *Statemgr, name, version string) error {
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         name,
			GenerationID: version,
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: security.SGRule_PERMIT.String(),
				},
			},
		},
	}

	return s.ctrler.NetworkSecurityPolicy().Delete(&sgp)
}

func deleteApp(s *Statemgr, name string) error {
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      name,
		},
	}

	return s.ctrler.App().Delete(&app)
}

func createSmartNic(s *Statemgr, name string) (*cluster.DistributedServiceCard, error) {
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_ADMITTED.String(),
			Conditions: []cluster.DSCCondition{
				{
					Type:   cluster.DSCCondition_HEALTHY.String(),
					Status: cluster.ConditionStatus_TRUE.String(),
				},
			},
		},
	}

	return &snic, s.ctrler.DistributedServiceCard().Create(&snic)
}

func getPolicyVersionForNode(s *Statemgr, policyname, nodename string) (string, error) {
	// Give the watchers a chance to run
	time.Sleep(250 * time.Millisecond)

	p, err := s.FindSgpolicy("default", policyname)
	if err != nil {
		return "", err
	}

	version, ok := p.NodeVersions[nodename]
	if ok != true {
		return "", errors.New("Smartnic not found in versions")
	}

	return version, nil
}

func TestSmartPolicyNodeVersions(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	createPolicy(stateMgr, "testPolicy1", "1")
	createSmartNic(stateMgr, "testSmartnic1")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	version, err := getPolicyVersionForNode(stateMgr, "testPolicy1", "testSmartnic1")
	AssertOk(t, err, "Couldn't get policy version for node")
	AssertEquals(t, version, "", "Policy version not correct for node")

	stateMgr.UpdateSgpolicyStatus("testSmartnic1", "default", "testPolicy1", "1")

	version, err = getPolicyVersionForNode(stateMgr, "testPolicy1", "testSmartnic1")
	AssertOk(t, err, "Couldn't get policy version for node")
	AssertEquals(t, version, "1", "Policy version not correct for node")

	createSmartNic(stateMgr, "testSmartnic2")
	version, err = getPolicyVersionForNode(stateMgr, "testPolicy1", "testSmartnic2")
	AssertOk(t, err, "Couldn't get policy version for node")
	AssertEquals(t, version, "", "Policy version not correct for node")

	// Wait for the periodic updater to kick in at least once
	time.Sleep(2 * time.Second)
	sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
	AssertOk(t, err, "Error finding sg policy")

	// verify propagation status
	prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
	AssertEquals(t, (int32)(1), prop.Updated, "Incorrect 'updated' propagation status")
	AssertEquals(t, (int32)(1), prop.Pending, "Incorrect 'pending' propagation status")
	AssertEquals(t, "1", prop.GenerationID, "Incorrect 'generation id' propagation status")
	AssertEquals(t, "", prop.MinVersion, "Incorrect 'min version' propagation status")

}

func TestNetworkSecurityPolicySmartNICEvents(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	snic1, err := createSmartNic(stateMgr, "testSmartnic1")
	createPolicy(stateMgr, "testPolicy1", "1")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	version, err := getPolicyVersionForNode(stateMgr, "testPolicy1", "testSmartnic1")
	AssertOk(t, err, "Couldn't get policy version for node")
	AssertEquals(t, version, "", "Policy version not correct for node")

	// verify propagation status
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err != nil {
			return false, err
		}
		prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 0 || prop.Pending != 1 || prop.GenerationID != "1" || prop.MinVersion != "" {
			return false, sgp
		}

		return true, nil
	}, "SGPolicy propagation state incorrect", "300ms", "10s")

	// send update from naples
	stateMgr.UpdateSgpolicyStatus("testSmartnic1", "default", "testPolicy1", "1")

	version, err = getPolicyVersionForNode(stateMgr, "testPolicy1", "testSmartnic1")
	AssertOk(t, err, "Couldn't get policy version for node")
	AssertEquals(t, version, "1", "Policy version not correct for node")

	snic2, err := createSmartNic(stateMgr, "testSmartnic2")
	version, err = getPolicyVersionForNode(stateMgr, "testPolicy1", "testSmartnic2")
	AssertOk(t, err, "Couldn't get policy version for node")
	AssertEquals(t, version, "", "Policy version not correct for node")

	// verify propagation status
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err != nil {
			return false, err
		}
		prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 1 || prop.Pending != 1 || prop.GenerationID != "1" || prop.MinVersion != "" {
			return false, sgp
		}

		return true, nil
	}, "NetworkSecurityPolicy propagation state incorrect", "300ms", "10s")

	// mark the smartnic as unhealthy
	snic2.Status.Conditions[0].Status = cluster.ConditionStatus_UNKNOWN.String()
	snic2.TypeMeta = api.TypeMeta{Kind: "DistributedServiceCard"}
	stateMgr.ctrler.DistributedServiceCard().Update(snic2)

	// verify propagation status
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err != nil {
			return false, err
		}
		prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 1 || prop.Pending != 1 || prop.GenerationID != "1" {
			return false, sgp
		}

		return true, nil
	}, "NetworkSecurityPolicy propagation state incorrect", "300ms", "10s")

	// mark the smartnic as healthy
	snic2.Status.Conditions[0].Status = cluster.ConditionStatus_TRUE.String()
	snic2.TypeMeta = api.TypeMeta{Kind: "DistributedServiceCard"}
	stateMgr.ctrler.DistributedServiceCard().Update(snic2)

	// verify propagation status
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err != nil {
			return false, err
		}
		prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 1 || prop.Pending != 1 || prop.GenerationID != "1" || prop.MinVersion != "" {
			return false, sgp
		}

		return true, nil
	}, "NetworkSecurityPolicy propagation state incorrect", "300ms", "10s")

	// send response from new nic
	stateMgr.UpdateSgpolicyStatus("testSmartnic2", "default", "testPolicy1", "1")

	// verify propagation status
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err != nil {
			return false, err
		}
		prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 2 || prop.Pending != 0 || prop.GenerationID != "1" || prop.MinVersion != "" {
			return false, sgp
		}

		return true, nil
	}, "NetworkSecurityPolicy propagation state incorrect", "300ms", "10s")

	// update the policy
	sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
	AssertOk(t, err, "Couldn't find policy")
	nsgp := sgp.NetworkSecurityPolicy.NetworkSecurityPolicy
	nsgp.ObjectMeta.GenerationID = "2"
	err = stateMgr.ctrler.NetworkSecurityPolicy().Update(&nsgp)
	AssertOk(t, err, "Couldn't update policy")

	// send update from first naples
	stateMgr.UpdateSgpolicyStatus("testSmartnic1", "default", "testPolicy1", "2")

	// verify propagation status after policy update
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err != nil {
			return false, err
		}
		prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 1 || prop.Pending != 1 || prop.GenerationID != "2" || prop.MinVersion != "1" {
			return false, sgp
		}
		if len(prop.PendingNaples) != 1 {
			return false, sgp
		}

		return true, nil
	}, "SGPolicy propagation state incorrect", "300ms", "10s")

	// send update from second naples
	stateMgr.UpdateSgpolicyStatus("testSmartnic2", "default", "testPolicy1", "2")

	// verify propagation status after policy update
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err != nil {
			return false, err
		}
		prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 2 || prop.Pending != 0 || prop.GenerationID != "2" || prop.MinVersion != "" {
			return false, sgp
		}
		if len(prop.PendingNaples) != 0 {
			return false, sgp
		}

		return true, nil
	}, "NetworkSecurityPolicy propagation state incorrect", "300ms", "10s")

	// delete smartnic
	err = stateMgr.ctrler.DistributedServiceCard().Delete(snic2)
	AssertOk(t, err, "Couldn't update smartnic")

	// verify propagation status
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err != nil {
			return false, err
		}
		prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 1 || prop.Pending != 0 || prop.GenerationID != "2" || prop.MinVersion != "" {
			return false, sgp
		}

		return true, nil
	}, "NetworkSecurityPolicy propagation state incorrect", "300ms", "10s")

	// delete smartnic
	err = stateMgr.ctrler.DistributedServiceCard().Delete(snic1)
	AssertOk(t, err, "Couldn't delete smartnic")

	// verify propagation status
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindSgpolicy("default", "testPolicy1")
		if err != nil {
			return false, err
		}
		prop := &sgp.NetworkSecurityPolicy.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 0 || prop.Pending != 0 || prop.GenerationID != "2" || prop.MinVersion != "" {
			return false, sgp
		}

		return true, nil
	}, "NetworkSecurityPolicy propagation state incorrect", "300ms", "10s")
}

func getFwProfileVersionForNode(s *Statemgr, fwprofilename, nodename string) (string, error) {
	// Give the watchers a chance to run
	time.Sleep(250 * time.Millisecond)
	p, err := s.FindFirewallProfile("default", fwprofilename)
	if err != nil {
		return "", err
	}

	version, ok := p.NodeVersions[nodename]
	if ok != true {
		return "", errors.New("Smartnic not found in versions")
	}

	return version, nil
}

func createFwProfile(s *Statemgr, name, version string) error {

	// firewall profile
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:         name,
			Namespace:    "default",
			Tenant:       "default",
			GenerationID: version,
		},
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout: "3m",
		},
	}
	return s.ctrler.FirewallProfile().Create(&fwp)
}

func TestFirewallProfileNodeVersions(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	createFwProfile(stateMgr, "testFwProfile1", "1")
	createSmartNic(stateMgr, "testSmartnic1")

	// verify propagation status
	AssertEventually(t, func() (bool, interface{}) {
		sgp, err := stateMgr.FindFirewallProfile("default", "testFwProfile1")
		if err != nil {
			return false, err
		}
		prop := &sgp.FirewallProfile.Status.PropagationStatus
		log.Infof("Got propagation status: %#v", prop)
		if prop.Updated != 0 || prop.Pending != 1 || prop.GenerationID != "1" || prop.MinVersion != "" {
			fmt.Printf("Prop failed..%v\n", prop)
			return false, sgp
		}

		return true, nil
	}, "Firewall propagation state incorrect", "300ms", "10s")

	version, err := getFwProfileVersionForNode(stateMgr, "testFwProfile1", "testSmartnic1")
	AssertOk(t, err, "Couldn't get fwprofile version for node")
	AssertEquals(t, version, "", "FwProfile version not correct for node")

	stateMgr.UpdateFirewallProfileStatus("testSmartnic1", "default", "testFwProfile1", "1")

	version, err = getFwProfileVersionForNode(stateMgr, "testFwProfile1", "testSmartnic1")
	AssertOk(t, err, "Couldn't get fwprofile version for node")
	AssertEquals(t, version, "1", "Fwprofile version not correct for node")

	createSmartNic(stateMgr, "testSmartnic2")
	version, err = getFwProfileVersionForNode(stateMgr, "testFwProfile1", "testSmartnic2")
	AssertOk(t, err, "Smartnic not found in versions")
	AssertEquals(t, version, "", "Fwprofile version not correct for node")

	// Wait for the periodic updater to kick in at least once
	time.Sleep(2 * time.Second)
	fwp, err := stateMgr.FindFirewallProfile("default", "testFwProfile1")
	AssertOk(t, err, "Error finding FwProfile")

	// verify propagation status
	prop := &fwp.FirewallProfile.Status.PropagationStatus
	AssertEquals(t, (int32)(1), prop.Updated, "Incorrect 'updated' propagation status")
	AssertEquals(t, (int32)(1), prop.Pending, "Incorrect 'pending' propagation status")
	AssertEquals(t, "1", prop.GenerationID, "Incorrect 'generation id' propagation status")
	AssertEquals(t, "", prop.MinVersion, "Incorrect 'min version' propagation status")

}

func TestFirewallProfile(t *testing.T) {
	// create network state manazger
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)

	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// firewall profile
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testProfile",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout: "3m",
		},
	}

	// create firewall profile
	err = stateMgr.ctrler.FirewallProfile().Create(&fwp)
	AssertOk(t, err, "Error creating firewall profile")

	// verify propagation status
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindFirewallProfile("default", "testProfile")
		if err != nil {
			return false, err
		}

		return true, nil
	}, "Firewall propagation state incorrect", "300ms", "1s")

	// verify we can find the firewall profile
	tmpFwp, err := stateMgr.FindFirewallProfile("default", "testProfile")
	AssertOk(t, err, "Error finding firewall profile")
	AssertEquals(t, fwp.Spec.SessionIdleTimeout, tmpFwp.FirewallProfile.Spec.SessionIdleTimeout, "firewall profile params did not match")

	// updat ethe firewall profile
	fwp.Spec.SessionIdleTimeout = "5m"
	err = stateMgr.ctrler.FirewallProfile().Update(&fwp)
	AssertOk(t, err, "Error updating fw profile")

	AssertEventually(t, func() (bool, interface{}) {
		p, err := stateMgr.FindFirewallProfile("default", "testProfile")
		if err == nil && p.FirewallProfile.Spec.SessionIdleTimeout == "5m" {
			return true, err
		}
		return false, nil
	}, "Firewall propagation state incorrect", "300ms", "1s")

	// verify fw profile got updated
	fwlist, err := stateMgr.ListFirewallProfiles()
	AssertOk(t, err, "Error getting list of firewall profiles")
	Assert(t, (len(fwlist) == 1), "invalid number of fw profiles")
	Assert(t, fwlist[0].FirewallProfile.Spec.SessionIdleTimeout == "5m", "fw profile was not updated")

	// delete firewall profile
	err = stateMgr.ctrler.FirewallProfile().Delete(&fwp)
	AssertOk(t, err, "Error creating firewall profile")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindFirewallProfile("default", "testProfile")
		if err != nil {
			return true, err
		}
		return false, nil
	}, "Firewall propagation state incorrect", "300ms", "1s")

	// verify firewll profile is gone
	_, err = stateMgr.FindFirewallProfile("default", "testProfile")
	Assert(t, (err != nil), "Firewall profile still found after deleting")
}

func TestAlgPolicy(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "ftpApp",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "21",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: security.ALG_FTP.String(),
				Ftp: &security.Ftp{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	// create the app
	err = stateMgr.ctrler.App().Create(&app)
	AssertOk(t, err, "Error creating the app")

	// create a policy using this alg
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sgpolicy",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					Apps:   []string{"ftpApp"},
					Action: security.SGRule_PERMIT.String(),
				},
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "8000",
						},
					},
					Action: security.SGRule_PERMIT.String(),
				},
			},
		},
	}

	// create sg policy
	err = stateMgr.ctrler.NetworkSecurityPolicy().Create(&sgp)
	AssertOk(t, err, "Error creating the sg policy")

	AssertEventually(t, func() (bool, interface{}) {
		tmapp, err := stateMgr.FindApp("default", "ftpApp")
		if err == nil && tmapp.App.Spec.Timeout == app.Spec.Timeout {
			return true, err
		}
		return false, nil
	}, "Firewall propagation state incorrect", "300ms", "1s")

	// verify app has this policy in its attached policy list
	//Assert(t, (len(tmapp.App.Status.AttachedPolicies) == 1), "Invalid number of attached policies")
	//	Assert(t, (tmapp.App.Status.AttachedPolicies[0] == "test-sgpolicy"), "Invalid attached policy")

	// delete the policy
	err = stateMgr.ctrler.NetworkSecurityPolicy().Delete(&sgp)
	AssertOk(t, err, "Error deleting sgpolicy")

	// verify app has no attached policy

	// list all apps
	AssertEventually(t, func() (bool, interface{}) {
		tmapp, err := stateMgr.FindApp("default", "ftpApp")
		if err == nil && len(tmapp.App.Status.AttachedPolicies) == 0 {
			return true, err
		}
		return false, nil
	}, "Firewall propagation state incorrect", "300ms", "1s")
}

func TestNetworkSecurityPolicyMultiApp(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// app
	ftpApp := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "ftpApp",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "21",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: "FTP",
				Ftp: &security.Ftp{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	// create the app
	err = stateMgr.ctrler.App().Create(&ftpApp)
	AssertOk(t, err, "Error creating the app")

	// ICMP app
	icmpApp := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "icmpApp",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			Timeout: "5m",
			ALG: &security.ALG{
				Type: "ICMP",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "2",
				},
			},
		},
	}
	err = stateMgr.ctrler.App().Create(&icmpApp)
	AssertOk(t, err, "Error creating the app")

	// create a policy using this alg
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sgpolicy",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"2.101.0.0/22"},
					ToIPAddresses:   []string{"2.101.0.0/24"},
					Apps:            []string{"ftpApp", "icmpApp"},
					Action:          "PERMIT",
				},
			},
		},
	}

	// create sg policy
	err = stateMgr.ctrler.NetworkSecurityPolicy().Create(&sgp)
	AssertOk(t, err, "Error creating the sg policy")

	// find sgp in mbus
	//nsgp, err := stateMgr.mbus.FindNetworkSecurityPolicy(&sgp.ObjectMeta)
	//AssertOk(t, err, "Error finding endpoint in mbus")
	//Assert(t, len(nsgp.Spec.Rules) == 2, "Invalid number of rules in mbus")
	//Assert(t, (nsgp.Spec.Rules[0].ID == nsgp.Spec.Rules[1].ID), "Invalid rule ids in mbus")

}
