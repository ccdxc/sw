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
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestSgpolicyCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := NewStatemgr(newDummyWriter(nil))
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
			Tenant: "default",
			Name:   "test-sgpolicy",
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
	err = stateMgr.CreateSgpolicy(&sgp)
	AssertOk(t, err, "Error creating the sg policy")

	// verify we can find the sg policy
	sgps, err := stateMgr.FindSgpolicy("default", "test-sgpolicy")
	AssertOk(t, err, "Could not find the sg policy")
	AssertEquals(t, sgps.Spec.AttachGroups, sgp.Spec.AttachGroups, "Security policy params did not match")
	Assert(t, (len(sgps.groups) == 1), "Sg was not added to sgpolicy", sgps)
	Assert(t, sgps.groups["procurement"].Name == "procurement", "Sgpolicy is not linked to sg", sgps)

	// verify sg has the policy info
	prsg, err := stateMgr.FindSecurityGroup("default", "procurement")
	AssertOk(t, err, "Could not find security group")
	Assert(t, (len(prsg.policies) == 1), "sgpolicy was not added to sg", prsg)
	Assert(t, (prsg.policies[sgps.Name].Name == sgps.Name), "Sg is not linked to sgpolicy", prsg)
	Assert(t, (len(prsg.Status.Policies) == 1), "Policy not found in sg status", prsg)
	Assert(t, (prsg.Status.Policies[0] == sgps.Name), "Policy not found in sg status", prsg)

	// verify we can not attach a policy to unknown sg
	sgp2 := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "sgpolicy2",
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
	err = stateMgr.CreateSgpolicy(&sgp2)
	Assert(t, (err != nil), "Policy creation with unknown attachment succeeded")

	// delete the sg policy
	err = stateMgr.DeleteSgpolicy("default", "test-sgpolicy")
	AssertOk(t, err, "Error deleting security policy")

	// verify the sg policy is gone
	_, err = stateMgr.FindSgpolicy("default", "test-sgpolicy")
	Assert(t, (err != nil), "Security policy still found after deleting")

	// verify sgpolicy is unlinked from sg
	Assert(t, (len(prsg.policies) == 0), "sgpolicy was not removed sg", prsg)
	Assert(t, (len(prsg.Status.Policies) == 0), "sgpolicy was not removed sg", prsg)
}

func createPolicy(s *Statemgr, name, version string) error {
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Name:         name,
			GenerationID: version,
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

	return s.CreateSgpolicy(&sgp)
}

func createSmartNic(s *Statemgr, name string) error {
	snic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "",
			Tenant:    "default",
		},
		Spec: cluster.SmartNICSpec{},
	}

	return s.smartNicReactor.CreateSmartNIC(snic)
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

func TestSmartPolicNodeVersions(t *testing.T) {
	// create network state manager
	writerObjects := make(map[interface{}]struct{})
	stateMgr, err := NewStatemgr(newDummyWriter(writerObjects))
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
	time.Sleep(6 * time.Second)
	AssertEquals(t, len(writerObjects), 1, "Too many writer objects")
	for obj := range writerObjects {
		sgp, ok := obj.(*security.SGPolicy)
		AssertEquals(t, ok, true, "Incorrect type of object found in writer objects")
		prop := &sgp.Status.PropagationStatus
		AssertEquals(t, prop.Updated, (int32)(1), "Incorrect 'updated' propagation status")
		AssertEquals(t, prop.Pending, (int32)(1), "Incorrect 'pending' propagation status")
		AssertEquals(t, prop.GenerationID, "1", "Incorrect 'generation id' propagation status")
		AssertEquals(t, prop.MinVersion, "", "Incorrect 'min version' propagation status")
	}
}

func TestFirewallProfile(t *testing.T) {
	// create network state manager
	writerObjects := make(map[interface{}]struct{})
	stateMgr, err := NewStatemgr(newDummyWriter(writerObjects))
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// firewall profile
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testProfile",
			Namespace: "",
			Tenant:    "testTenant",
		},
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout:    "3m",
			IPNormalizationEnable: true,
		},
	}

	// create firewall profile
	err = stateMgr.FirewallProfileReactor().CreateFirewallProfile(fwp)
	AssertOk(t, err, "Error creating firewall profile")

	// verify we can find the firewall profile
	tmpFwp, err := stateMgr.FindFirewallProfile("testTenant", "testProfile")
	AssertOk(t, err, "Error finding firewall profile")
	AssertEquals(t, fwp.Spec.SessionIdleTimeout, tmpFwp.Spec.SessionIdleTimeout, "firewall profile params did not match")

	// delete firewall profile
	err = stateMgr.FirewallProfileReactor().DeleteFirewallProfile(fwp)
	AssertOk(t, err, "Error creating firewall profile")

	// verify firewll profile is gone
	_, err = stateMgr.FindFirewallProfile("testTenant", "testProfile")
	Assert(t, (err != nil), "Firewall profile still found after deleting")
}

func TestAlgPolicy(t *testing.T) {
	// create network state manager
	writerObjects := make(map[interface{}]struct{})
	stateMgr, err := NewStatemgr(newDummyWriter(writerObjects))
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "ftpApp",
			Namespace: "",
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
				FtpAlg: &security.FtpAlg{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	// create the app
	err = stateMgr.AppReactor().CreateApp(app)
	AssertOk(t, err, "Error creating the app")

	// create a policy using this alg
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "test-sgpolicy",
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
	err = stateMgr.CreateSgpolicy(&sgp)
	AssertOk(t, err, "Error creating the sg policy")

	// verify we can find the app
	tmapp, err := stateMgr.FindApp("testTenant", "ftpApp")
	AssertOk(t, err, "Error finding the app")
	AssertEquals(t, app.Spec.Timeout, tmapp.Spec.Timeout, "app params did not match")

	// verify app has this policy in its attached policy list
	Assert(t, (len(tmapp.Status.AttachedPolicies) == 1), "Invalid number of attached policies")
	Assert(t, (tmapp.Status.AttachedPolicies[0] == "test-sgpolicy"), "Invalid attached policy")

	// delete the policy
	err = stateMgr.DeleteSgpolicy("testTenant", "test-sgpolicy")
	AssertOk(t, err, "Error deleting sgpolicy")

	// verify app has no attached policy
	Assert(t, (len(tmapp.Status.AttachedPolicies) == 0), "Invalid number of attached policies")

	// finally delete the app
	err = stateMgr.AppReactor().DeleteApp(app)
	AssertOk(t, err, "Error deleting app")

	_, err = stateMgr.FindApp("testTenant", "ftpApp")
	Assert(t, (err != nil), "App still found after deleting")
}
