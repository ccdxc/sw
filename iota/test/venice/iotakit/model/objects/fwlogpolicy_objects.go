package objects

import (
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

// FwLogPolicy represents FwLogPolicy policy
type FwlogPolicy struct {
	VenicePolicy *monitoring.FwlogPolicy
}

// FwLogPolicyCollection is list of policies
type FwlogPolicyCollection struct {
	CollectionCommon
	Policies []*FwlogPolicy
}

func NewFwlogPolicyCollection(policy *FwlogPolicy, client objClient.ObjClient, testbed *testbed.TestBed) *FwlogPolicyCollection {
	return &FwlogPolicyCollection{
		CollectionCommon: CollectionCommon{Client: client, Testbed: testbed},
		Policies: []*FwlogPolicy{
			policy,
		},
	}
}

// Commit writes the policy to venice
func (spc *FwlogPolicyCollection) Commit() error {
	if spc.HasError() {
		return spc.err
	}

	for _, pol := range spc.Policies {
		err := spc.Client.CreateFwlogPolicy(pol.VenicePolicy)
		if err != nil {
			// try updating it
			err = spc.Client.UpdateFwlogPolicy(pol.VenicePolicy)
			if err != nil {
				spc.err = err
				return err
			}
		}

		log.Debugf("Created policy: %#v", pol.VenicePolicy)

	}

	return nil
}

// Delete deletes all policies in the collection
func (spc *FwlogPolicyCollection) Delete() error {
	if spc == nil {
		return nil
	}
	if spc.err != nil {
		return spc.err
	}

	// walk all policies and delete them
	for _, pol := range spc.Policies {
		err := spc.Client.DeleteFwlogPolicy(pol.VenicePolicy)
		if err != nil {
			return err
		}
	}

	// Added temporarily to avoid race conditions due to back-to-back delete followed by create
	time.Sleep(10 * time.Second)
	return nil
}
