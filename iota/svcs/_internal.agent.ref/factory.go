package agent

import (
	grpc "github.com/pensando/sw/iota/svcs/agent/grpc/client"
	"github.com/pensando/sw/iota/svcs/agent/service"
)

//InitNodeService When we support differnt type of services for each agent
// Have to enhance it then
func InitNodeService(ip string, port int) (*service.NodeServices, error) {

	naples, err := grpc.NewNaplesClient(ip, port)
	if err != nil {
		return nil, err
	}
	app, err := grpc.NewAppClient(ip, port)
	if err != nil {
		return nil, err
	}
	qemu, err := grpc.NewQemuClient(ip, port)
	if err != nil {
		return nil, err
	}

	venice, err := grpc.NewVeniceClient(ip, port)
	if err != nil {
		return nil, err
	}

	return &service.NodeServices{Naples: naples, App: app, Qemu: qemu, Venice: venice}, nil
}
