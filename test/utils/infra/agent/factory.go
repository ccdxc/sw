package agent

import (
	grpc "github.com/pensando/sw/test/utils/infra/agent/grpc/client"
	"github.com/pensando/sw/test/utils/infra/agent/service"
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
	return &service.NodeServices{Naples: naples, App: app}, nil
}
