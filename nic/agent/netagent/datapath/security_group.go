package datapath

import (
	"context"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateSecurityGroup creates a security group
func (hd *Datapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	var secGroupPolicyRequests []*halproto.SecurityGroupPolicySpec
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
