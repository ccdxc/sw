package client

import (
	"context"
	"strconv"
	"strings"

	"github.com/pkg/errors"
	"google.golang.org/grpc"

	pb "github.com/pensando/sw/test/utils/infra/agent/grpc/api/pb"
	node "github.com/pensando/sw/test/utils/infra/agent/service"
)

//NewAppAgentClient returns instance of new app agent client.
func NewAppAgentClient(ip string, port int) (pb.AppAgentClient, error) {
	var conn *grpc.ClientConn
	conn, err := grpc.Dial(ip+":"+strconv.Itoa(port), grpc.WithInsecure())
	if err != nil {
		return nil, errors.Wrap(err, "Agent connection failed!")
	}
	return pb.NewAppAgentClient(conn), nil
}

//NewNaplesAgentClient returns instance of new app agent client.
func NewNaplesAgentClient(ip string, port int) (pb.NaplesSimClient, error) {
	var conn *grpc.ClientConn
	conn, err := grpc.Dial(ip+":"+strconv.Itoa(port), grpc.WithInsecure())
	if err != nil {
		return nil, errors.Wrap(err, "Agent connection failed!")
	}
	return pb.NewNaplesSimClient(conn), nil
}

//AppClient Structure
type AppClient struct {
	service pb.AppAgentClient
}

//BringUp bring up app client
func (appClient *AppClient) BringUp(ctx context.Context, config *node.AppConfig) error {

	resp, err := appClient.service.BringUp(ctx,
		&pb.AppConfig{Name: config.Name, Registry: config.Registry})

	if err != nil {
		return errors.Wrap(err, "Agent bring up api Failed on App")
	}

	if resp.Status != pb.ApiStatus_API_STATUS_OK {
		return errors.Wrapf(err, "App Bring up failed on node : %s", resp.Response)
	}

	return nil
}

//Teardown teardown app client
func (appClient *AppClient) Teardown(ctx context.Context, cfg *node.AppConfig) error {
	resp, err := appClient.service.Teardown(ctx,
		&pb.AppConfig{Name: cfg.Name, Registry: cfg.Registry})

	if err != nil {
		return errors.Wrap(err, "Agent teardown api Failed on App")
	}

	if resp.Status != pb.ApiStatus_API_STATUS_OK {
		return errors.Wrapf(err, "App teardown failed on node : %s", resp.Response)
	}

	return nil
}

//RunCommand Run command on app client
func (appClient *AppClient) RunCommand(ctx context.Context, app string, cmd string, bg bool) ([]string, []string, int, error) {
	resp, err := appClient.service.RunCommand(ctx, &pb.Command{App: app,
		Cmd:        cmd,
		Background: bg,
		Timeout:    0})

	if err != nil {
		return nil, nil, -1, errors.Wrap(err, "Running command failed")
	}

	return strings.Split(resp.Stdout, "\n"), strings.Split(resp.Stderr, "\n"), int(resp.RetCode), nil
}

//AttachInterface Attach Interface
func (appClient *AppClient) AttachInterface(ctx context.Context, app string,
	intf *node.AppInterface) error {

	resp, err := appClient.service.AttachInterface(ctx,
		&pb.Interface{App: app,
			Name:       intf.Name,
			MacAddress: intf.MacAddress,
			Vlan:       intf.Vlan,
			IpAddress:  strings.Split(intf.IPaddress, "/")[0],
			PrefixLen:  intf.PrefixLen})

	if err != nil {
		return errors.Wrap(err, "Attach Interface failed")
	}

	if resp.Status != pb.ApiStatus_API_STATUS_OK {
		return errors.Errorf("Interface attach failed : %s ", resp.GetResponse())
	}

	return nil
}

//NaplesClient Naples client structure
type NaplesClient struct {
	simClient pb.NaplesSimClient
}

//BringUp Naples bring up
func (cli *NaplesClient) BringUp(ctx context.Context, cfg *node.NaplesSimConfig) error {
	resp, err := cli.simClient.BringUp(ctx, &pb.NaplesSimConfig{Name: cfg.Name,
		NodeID:          cfg.NodeID,
		CtrlNwIpRange:   cfg.CtrlNwIPRange,
		TunnelIpStart:   cfg.TunnelIPStart,
		TunnelInterface: cfg.TunnelInterface,
		TunnelIpAddress: cfg.TunnelIPAddress})

	if err != nil {
		return errors.Wrap(err, "GRPC failure!")
	}

	if resp.Status != pb.ApiStatus_API_STATUS_OK {
		return errors.Errorf("Naples Bring up failed: %s", resp.Response)
	}

	return nil

}

//Teardown Naples  teardown
func (*NaplesClient) Teardown(context.Context, *node.NaplesSimConfig) error {
	return nil
}

//RunCommand Naples Run Command
func (*NaplesClient) RunCommand(context.Context, string) (string, string, error) {
	return "", "", nil
}

//NewNaplesClient Get new instance of naples client
func NewNaplesClient(ip string, port int) (*NaplesClient, error) {
	naples, err := NewNaplesAgentClient(ip, port)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to connect to naples agent")
	}

	return &NaplesClient{simClient: naples}, nil
}

//NewAppClient Get new instance of app client
func NewAppClient(ip string, port int) (*AppClient, error) {
	app, err := NewAppAgentClient(ip, port)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to connect app agent")
	}

	return &AppClient{service: app}, nil

}
