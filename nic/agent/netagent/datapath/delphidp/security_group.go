// +build ignore

// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/halproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

// CreateSecurityGroup creates a security group
func (dp *DelphiDatapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	// build security group message
	sgs := halproto.SecurityGroupSpec{
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
	}

	err := dp.delphiClient.SetObject(&sgs)
	if err != nil {
		return err
	}

	return nil
}

// UpdateSecurityGroup updates a security group
func (dp *DelphiDatapath) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	// build security group message
	sgs := halproto.SecurityGroupSpec{
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
	}

	err := dp.delphiClient.SetObject(&sgs)
	if err != nil {
		return err
	}

	return nil
}

// DeleteSecurityGroup deletes a security group
func (dp *DelphiDatapath) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	// build security group message
	sgs := halproto.SecurityGroupSpec{
		KeyOrHandle: &halproto.SecurityGroupKeyHandle{
			KeyOrHandle: &halproto.SecurityGroupKeyHandle_SecurityGroupId{
				SecurityGroupId: sg.Status.SecurityGroupID,
			},
		},
	}

	err := dp.delphiClient.DeleteObject(&sgs)
	if err != nil {
		return err
	}

	return nil
}
