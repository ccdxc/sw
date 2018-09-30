package client

import (
	"context"
	"strconv"
	"strings"

	"github.com/pkg/errors"
	"google.golang.org/grpc"

	pb "github.com/pensando/sw/iota/svcs/agent/grpc/api/pb"
	node "github.com/pensando/sw/iota/svcs/agent/service"
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

//NewVeniceAgentClient returns instance of new app agent client.
func NewVeniceAgentClient(ip string, port int) (pb.VeniceClient, error) {
	var conn *grpc.ClientConn
	conn, err := grpc.Dial(ip+":"+strconv.Itoa(port), grpc.WithInsecure())
	if err != nil {
		return nil, errors.Wrap(err, "Agent connection failed!")
	}
	return pb.NewVeniceClient(conn), nil
}

//NewQemuAgentClient returns instance of new qemu agent client.
func NewQemuAgentClient(ip string, port int) (pb.QemuClient, error) {
	var conn *grpc.ClientConn
	conn, err := grpc.Dial(ip+":"+strconv.Itoa(port), grpc.WithInsecure())
	if err != nil {
		return nil, errors.Wrap(err, "Agent connection failed!")
	}
	return pb.NewQemuClient(conn), nil
}

//AppClient Structure
type AppClient struct {
	service pb.AppAgentClient
}

//BringUp bring up app client
func (appClient *AppClient) BringUp(ctx context.Context, config *node.AppConfig) error {

	resp, err := appClient.service.BringUp(ctx,
		&pb.AppConfig{Name: config.Name, Registry: config.Registry, OnQemu: config.OnQemu})

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

//AddVlanInterface Attach Interface
func (appClient *AppClient) AddVlanInterface(ctx context.Context, app string,
	intf *node.AppVlanInterface) error {

	resp, err := appClient.service.AddVlanInterface(ctx,
		&pb.VlanInterface{App: app,
			ParentIntfName:   intf.ParentIntfName,
			ParentMacAddress: intf.ParentMacMacAddress,
			MacAddress:       intf.MacAddress,
			Vlan:             intf.Vlan,
			IpAddress:        strings.Split(intf.IPaddress, "/")[0],
			PrefixLen:        intf.PrefixLen})

	if err != nil {
		return errors.Wrap(err, "Add vlan Interface failed")
	}

	if resp.Status != pb.ApiStatus_API_STATUS_OK {
		return errors.Errorf("Add vlan Interface failed : %s ", resp.GetResponse())
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
		ControlIntf:     cfg.CtrlIntf,
		ControlIP:       cfg.CtrlIP,
		DataIntfs:       cfg.DataIntfs,
		DataIPs:         cfg.DataIPs,
		VeniceIPs:       cfg.VeniceIPs,
		PassThroughMode: cfg.PassThroughMode,
		WithQemu:        cfg.WithQemu})

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
func (cli *NaplesClient) RunCommand(ctx context.Context, cmd string) (string, string, error) {
	resp, err := cli.simClient.RunCommand(ctx, &pb.Command{
		Cmd:        cmd,
		Background: false,
		Timeout:    0})

	if err != nil {
		return "", "", errors.Wrap(err, "GRPC failure!")
	}

	if resp.Status != pb.ApiStatus_API_STATUS_OK {
		return "", "", errors.Errorf("Naples Run command up failed: %d", resp.RetCode)
	}

	return resp.GetStdout(), resp.GetStderr(), nil
}

//NewNaplesClient Get new instance of naples client
func NewNaplesClient(ip string, port int) (*NaplesClient, error) {
	naples, err := NewNaplesAgentClient(ip, port)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to connect to naples agent")
	}

	return &NaplesClient{simClient: naples}, nil
}

//QemuClient Naples client structure
type QemuClient struct {
	qemuClient pb.QemuClient
}

//BringUp Naples bring up
func (cli *QemuClient) BringUp(ctx context.Context, cfg *node.QemuConfig) error {
	resp, err := cli.qemuClient.BringUp(ctx, &pb.QemuConfig{Name: cfg.Name,
		Image: cfg.Image})

	if err != nil {
		return errors.Wrap(err, "GRPC failure!")
	}

	if resp.Status != pb.ApiStatus_API_STATUS_OK {
		return errors.Errorf("Naples Bring up failed: %s", resp.Response)
	}

	return nil

}

//Teardown Naples  teardown
func (*QemuClient) Teardown(context.Context, *node.QemuConfig) error {
	return nil
}

//RunCommand Naples Run Command
func (*QemuClient) RunCommand(context.Context, string) (string, string, error) {
	return "", "", nil
}

//VeniceClient Naples client structure
type VeniceClient struct {
	veniceClient pb.VeniceClient
}

//BringUp Naples bring up
func (cli *VeniceClient) BringUp(ctx context.Context, cfg *node.VeniceConfig) error {
	resp, err := cli.veniceClient.BringUp(ctx, &pb.VeniceConfig{Name: cfg.Name,
		ControlIP:   cfg.CtrlIP,
		ControlIntf: cfg.CtrlIntf})

	if err != nil {
		return errors.Wrap(err, "GRPC failure!")
	}

	if resp.Status != pb.ApiStatus_API_STATUS_OK {
		return errors.Errorf("Naples Bring up failed: %s", resp.Response)
	}

	return nil

}

//Teardown Naples  teardown
func (*VeniceClient) Teardown(context.Context, *node.VeniceConfig) error {
	return nil
}

//RunCommand Naples Run Command
func (*VeniceClient) RunCommand(context.Context, string) (string, string, error) {
	return "", "", nil
}

//NewQemuClient Get new instance of naples client
func NewQemuClient(ip string, port int) (*QemuClient, error) {
	qemuCli, err := NewQemuAgentClient(ip, port)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to connect to qemu client")
	}

	return &QemuClient{qemuClient: qemuCli}, nil
}

//NewVeniceClient Get new instance of naples client
func NewVeniceClient(ip string, port int) (*VeniceClient, error) {
	veniceCli, err := NewVeniceAgentClient(ip, port)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to connect to venice client")
	}

	return &VeniceClient{veniceClient: veniceCli}, nil
}

//NewAppClient Get new instance of app client
func NewAppClient(ip string, port int) (*AppClient, error) {
	app, err := NewAppAgentClient(ip, port)
	if err != nil {
		return nil, errors.Wrap(err, "Failed to connect app agent")
	}

	return &AppClient{service: app}, nil

}
