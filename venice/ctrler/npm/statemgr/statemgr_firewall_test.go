// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"errors"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/netagent/protos/generated/nimbus"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func newStatemgr() (*Statemgr, error) {
	// create nimbus server
	msrv, err := nimbus.NewMbusServer("npm-test", "")
	if err != nil {
		log.Fatalf("Could not start RPC server. Err: %v", err)
	}

	// create network state manager
	stateMgr, err := NewStatemgr(globals.APIServer, nil, msrv)
	if err != nil {
		log.Errorf("Could not create network manager. Err: %v", err)
		return nil, err
	}

	return stateMgr, nil
}

func TestSgpolicyCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create sg
	_, err = createSg(stateMgr, "default", "procurement", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")
	_, err = createSg(stateMgr, "default", "catalog", labels.SelectorFromSet(labels.Set{"env": "production", "app": "catalog"}))
	AssertOk(t, err, "Error creating security group")

	// sg policy
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sgpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachGroups: []string{"procurement"},
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: "PERMIT",
				},
			},
		},
	}

	// create sg policy
	err = stateMgr.ctrler.SGPolicy().Create(&sgp)
	AssertOk(t, err, "Error creating the sg policy")

	// verify we can find the sg policy
	sgps, err := stateMgr.FindSgpolicy("default", "test-sgpolicy")
	AssertOk(t, err, "Could not find the sg policy")
	AssertEquals(t, sgps.SGPolicy.Spec.AttachGroups, sgp.Spec.AttachGroups, "Security policy params did not match")
	Assert(t, (len(sgps.groups) == 1), "Sg was not added to sgpolicy", sgps)
	Assert(t, sgps.groups["procurement"].SecurityGroup.Name == "procurement", "Sgpolicy is not linked to sg", sgps)

	// verify sg has the policy info
	prsg, err := stateMgr.FindSecurityGroup("default", "procurement")
	AssertOk(t, err, "Could not find security group")
	Assert(t, (len(prsg.policies) == 1), "sgpolicy was not added to sg", prsg)
	Assert(t, (prsg.policies[sgps.SGPolicy.Name].SGPolicy.Name == sgps.SGPolicy.Name), "Sg is not linked to sgpolicy", prsg)
	Assert(t, (len(prsg.SecurityGroup.Status.Policies) == 1), "Policy not found in sg status", prsg)
	Assert(t, (prsg.SecurityGroup.Status.Policies[0] == sgps.SGPolicy.Name), "Policy not found in sg status", prsg)

	// update sg policy
	newRule := security.SGRule{
		ProtoPorts: []security.ProtoPort{
			{
				Protocol: "tcp",
				Ports:    "8000",
			},
		},
		Action: "PERMIT",
	}
	sgp.Spec.Rules = append(sgp.Spec.Rules, newRule)
	err = stateMgr.ctrler.SGPolicy().Update(&sgp)
	AssertOk(t, err, "Error updating sgpolicy")

	// verify we can not attach a policy to unknown sg
	sgp2 := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "sgpolicy2",
		},
		Spec: security.SGPolicySpec{
			AttachGroups: []string{"unknown"},
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: "PERMIT",
				},
			},
		},
	}
	err = stateMgr.ctrler.SGPolicy().Create(&sgp2)
	Assert(t, (err != nil), "Policy creation with unknown attachment succeeded")

	// delete the sg policy
	err = stateMgr.ctrler.SGPolicy().Delete(&sgp)
	AssertOk(t, err, "Error deleting security policy")

	// verify the sg policy is gone
	_, err = stateMgr.FindSgpolicy("default", "test-sgpolicy")
	Assert(t, (err != nil), "Security policy still found after deleting")

	// verify sgpolicy is unlinked from sg
	Assert(t, (len(prsg.policies) == 0), "sgpolicy was not removed sg", prsg)
	Assert(t, (len(prsg.SecurityGroup.Status.Policies) == 0), "sgpolicy was not removed sg", prsg)
}

func createPolicy(s *Statemgr, name, version string) error {
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         name,
			GenerationID: version,
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: "PERMIT",
				},
			},
		},
	}

	return s.ctrler.SGPolicy().Create(&sgp)
}

func createSmartNic(s *Statemgr, name string) error {
	snic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: cluster.SmartNICSpec{},
	}

	return s.ctrler.SmartNIC().Create(&snic)
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

	createPolicy(stateMgr, "testPolicy1", "1")
	createSmartNic(stateMgr, "testSmartnic1")
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
	prop := &sgp.SGPolicy.Status.PropagationStatus
	AssertEquals(t, (int32)(1), prop.Updated, "Incorrect 'updated' propagation status")
	AssertEquals(t, (int32)(1), prop.Pending, "Incorrect 'pending' propagation status")
	AssertEquals(t, "1", prop.GenerationID, "Incorrect 'generation id' propagation status")
	AssertEquals(t, "", prop.MinVersion, "Incorrect 'min version' propagation status")

}

func TestFirewallProfile(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// firewall profile
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testProfile",
			Namespace: "default",
			Tenant:    "testTenant",
		},
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout: "3m",
		},
	}

	// create firewall profile
	err = stateMgr.ctrler.FirewallProfile().Create(&fwp)
	AssertOk(t, err, "Error creating firewall profile")

	// verify we can find the firewall profile
	tmpFwp, err := stateMgr.FindFirewallProfile("testTenant", "testProfile")
	AssertOk(t, err, "Error finding firewall profile")
	AssertEquals(t, fwp.Spec.SessionIdleTimeout, tmpFwp.FirewallProfile.Spec.SessionIdleTimeout, "firewall profile params did not match")

	// updat ethe firewall profile
	fwp.Spec.SessionIdleTimeout = "5m"
	err = stateMgr.ctrler.FirewallProfile().Update(&fwp)
	AssertOk(t, err, "Error updating fw profile")

	// verify fw profile got updated
	fwlist, err := stateMgr.ListFirewallProfiles()
	AssertOk(t, err, "Error getting list of firewall profiles")
	Assert(t, (len(fwlist) == 1), "invalid number of fw profiles")
	Assert(t, fwlist[0].FirewallProfile.Spec.SessionIdleTimeout == "5m", "fw profile was not updated")

	// delete firewall profile
	err = stateMgr.ctrler.FirewallProfile().Delete(&fwp)
	AssertOk(t, err, "Error creating firewall profile")

	// verify firewll profile is gone
	_, err = stateMgr.FindFirewallProfile("testTenant", "testProfile")
	Assert(t, (err != nil), "Firewall profile still found after deleting")
}

func TestAlgPolicy(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "ftpApp",
			Namespace: "default",
			Tenant:    "testTenant",
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
	err = stateMgr.ctrler.App().Create(&app)
	AssertOk(t, err, "Error creating the app")

	// create a policy using this alg
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "default",
			Name:      "test-sgpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					Apps:   []string{"ftpApp"},
					Action: "PERMIT",
				},
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "8000",
						},
					},
					Action: "PERMIT",
				},
			},
		},
	}

	// create sg policy
	err = stateMgr.ctrler.SGPolicy().Create(&sgp)
	AssertOk(t, err, "Error creating the sg policy")

	// verify we can find the app
	tmapp, err := stateMgr.FindApp("testTenant", "ftpApp")
	AssertOk(t, err, "Error finding the app")
	AssertEquals(t, app.Spec.Timeout, tmapp.App.Spec.Timeout, "app params did not match")

	// verify app has this policy in its attached policy list
	Assert(t, (len(tmapp.App.Status.AttachedPolicies) == 1), "Invalid number of attached policies")
	Assert(t, (tmapp.App.Status.AttachedPolicies[0] == "test-sgpolicy"), "Invalid attached policy")

	// delete the policy
	err = stateMgr.ctrler.SGPolicy().Delete(&sgp)
	AssertOk(t, err, "Error deleting sgpolicy")

	// verify app has no attached policy
	Assert(t, (len(tmapp.App.Status.AttachedPolicies) == 0), "Invalid number of attached policies")

	// list all apps
	applist, err := stateMgr.ListApps()
	AssertOk(t, err, "error listing apps")
	Assert(t, (len(applist) == 1), "Invalid number of apps in the list")

	// finally delete the app
	err = stateMgr.ctrler.App().Delete(&app)
	AssertOk(t, err, "Error deleting app")

	_, err = stateMgr.FindApp("testTenant", "ftpApp")
	Assert(t, (err != nil), "App still found after deleting")
}
