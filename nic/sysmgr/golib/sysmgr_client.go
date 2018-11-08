package client

import (
	"os"

	delphisdk "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	proto "github.com/pensando/sw/nic/sysmgr/proto/sysmgr"
)

// Client represents the sysmgr client
type Client struct {
	delphiClient delphisdk.Client
	name         string
}

// NewClient creates a new sysmgr client
func NewClient(delphiClient delphisdk.Client, name string) *Client {
	proto.SysmgrRebootReqMountKey(delphiClient, name, delphi.MountMode_ReadWriteMode)
	return &Client{
		delphiClient: delphiClient,
		name:         name,
	}
}

// InitDone should be called after application initialization is done. *Must
// be called after the delphi client has been connected and mount completed
func (c *Client) InitDone() {
	obj := &proto.SysmgrServiceStatus{
		Key: c.name,
		Pid: (int32)(os.Getpid()),
	}
	c.delphiClient.SetObject(obj)
}
