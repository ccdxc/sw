// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"errors"
	"fmt"
	"strconv"
	"strings"

	"github.com/Sirupsen/logrus"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/ctrler/npm/statemgr"
	"github.com/pensando/sw/utils/memdb"
	"golang.org/x/net/context"
)

// SecurityRPCServer serves security rpc requests
type SecurityRPCServer struct {
	stateMgr *statemgr.Statemgr // reference to network manager
}

// convertRule converts a single rule
func convertRule(rl *network.SGRule, dir string) netproto.SecurityRule {
	var proto = "tcp"
	var port int
	svcs := strings.Split(rl.Ports, "/")
	if len(svcs) == 2 {
		proto = svcs[0]
		port, _ = strconv.Atoi(svcs[1])
	} else if _, err := strconv.Atoi(svcs[0]); err == nil {
		port, _ = strconv.Atoi(svcs[0])
	}

	return netproto.SecurityRule{
		Direction: dir,
		PeerGroup: rl.PeerGroup,
		Services: []netproto.SecurityRule_Service{
			{
				Protocol: proto,
				Port:     uint32(port),
			},
		},
		Action: rl.Action,
		Log:    false, // FIXME: where to get the log params?
	}
}

// convertAllRules converts all rules attached to a security group
func convertAllRules(stateMgr *statemgr.Statemgr, sgs *statemgr.SecurityGroupState) ([]netproto.SecurityRule, error) {
	// walk all attached policies
	var rules []netproto.SecurityRule
	for _, pname := range sgs.Status.Policies {
		sgp, err := stateMgr.FindSgpolicy(sgs.Tenant, pname)
		if err == nil {
			for _, rl := range sgp.Spec.InRules {
				nrule := convertRule(&rl, "incoming")
				rules = append(rules, nrule)
			}
			for _, rl := range sgp.Spec.OutRules {
				nrule := convertRule(&rl, "outgoing")
				rules = append(rules, nrule)
			}
		} else {
			logrus.Errorf("Error finding sg policy %s. Err: %v", pname, err)
		}
	}

	return rules, nil
}

// GetSecurityGroup gets security group
func (srs *SecurityRPCServer) GetSecurityGroup(ctx context.Context, ometa *api.ObjectMeta) (*netproto.SecurityGroup, error) {
	// find the sg
	sgs, err := srs.stateMgr.FindSecurityGroup(ometa.Tenant, ometa.Name)
	if err != nil {
		logrus.Errorf("Could not find the sg %s|%s", ometa.Tenant, ometa.Name)
		return nil, fmt.Errorf("Could not find the sg")
	}

	// build sg message
	sg := netproto.SecurityGroup{
		TypeMeta:   sgs.TypeMeta,
		ObjectMeta: sgs.ObjectMeta,
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "",                        // FIXME: fill in security profile
			Rules:           []netproto.SecurityRule{}, // FIXME: fill in security rules from sgPolicy
		},
	}

	return &sg, nil
}

// ListSecurityGroups returns a list of security groups
func (srs *SecurityRPCServer) ListSecurityGroups(context.Context, *api.ObjectMeta) (*netproto.SecurityGroupList, error) {
	var sgList netproto.SecurityGroupList
	// get all sgs
	sgl, err := srs.stateMgr.ListSecurityGroups()
	if err != nil {
		return nil, err
	}

	// walk all sgs in the list
	for _, sgs := range sgl {
		// build rule list by walking all attached sgpolicies
		rules, err := convertAllRules(srs.stateMgr, sgs)
		if err != nil {
			logrus.Errorf("Error converting rules. Err: %v", err)
			return nil, err
		}

		// build sg message
		sg := netproto.SecurityGroup{
			TypeMeta:   sgs.TypeMeta,
			ObjectMeta: sgs.ObjectMeta,
			Spec: netproto.SecurityGroupSpec{
				SecurityProfile: "", // FIXME: fill in security profile
				Rules:           rules,
			},
		}
		sgList.SecurityGroups = append(sgList.SecurityGroups, &sg)
	}

	return &sgList, nil
}

// WatchSecurityGroups watches for sg changes
func (srs *SecurityRPCServer) WatchSecurityGroups(sel *api.ObjectMeta, stream netproto.SecurityApi_WatchSecurityGroupsServer) error {
	// watch for changes
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	srs.stateMgr.WatchObjects("SecurityGroup", watchChan)

	// first get a list of all networks
	sgl, err := srs.ListSecurityGroups(context.Background(), sel)
	if err != nil {
		logrus.Errorf("Error getting a list of sgs. Err: %v", err)
		return err
	}

	// send the objects out as a stream
	for _, sg := range sgl.SecurityGroups {
		watchEvt := netproto.SecurityGroupEvent{
			EventType:     api.EventType_CreateEvent,
			SecurityGroup: *sg,
		}
		err = stream.Send(&watchEvt)
		if err != nil {
			logrus.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	// loop forever on watch channel
	for {
		select {
		// read from channel
		case evt, ok := <-watchChan:
			if !ok {
				logrus.Errorf("Error reading from channel. Closing watch")
				close(watchChan)
				return errors.New("Error reading from channel")
			}

			// get event type from memdb event
			var etype api.EventType
			switch evt.EventType {
			case memdb.CreateEvent:
				etype = api.EventType_CreateEvent
			case memdb.UpdateEvent:
				etype = api.EventType_UpdateEvent
			case memdb.DeleteEvent:
				etype = api.EventType_DeleteEvent
			}

			// convert to network object
			sgs, err := statemgr.SecurityGroupStateFromObj(evt.Obj)
			if err != nil {
				continue
			}

			// build rule list by walking all attached sgpolicies
			rules, err := convertAllRules(srs.stateMgr, sgs)
			if err != nil {
				logrus.Errorf("Error converting rules. Err: %v", err)
				continue
			}

			// construct the netproto object
			watchEvt := netproto.SecurityGroupEvent{
				EventType: etype,
				SecurityGroup: netproto.SecurityGroup{
					TypeMeta:   sgs.TypeMeta,
					ObjectMeta: sgs.ObjectMeta,
					Spec: netproto.SecurityGroupSpec{
						SecurityProfile: "", // FIXME: fill in security profile
						Rules:           rules,
					},
				},
			}

			err = stream.Send(&watchEvt)
			if err != nil {
				logrus.Errorf("Error sending stream. Err: %v", err)
				close(watchChan)
				return err
			}
		}
	}

	// done
}

// NewSecurityRPCServer returns a security RPC server
func NewSecurityRPCServer(stateMgr *statemgr.Statemgr) (*SecurityRPCServer, error) {
	return &SecurityRPCServer{stateMgr: stateMgr}, nil
}
