// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package network is a auto generated package.
Input file: svc_network.proto
*/
package network

import (
	"context"

	"github.com/pensando/sw/api"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta

// ServiceNetworkV1Client  is the client interface for the service.
type ServiceNetworkV1Client interface {
	AutoWatchSvcNetworkV1(ctx context.Context, in *api.AggWatchOptions) (NetworkV1_AutoWatchSvcNetworkV1Client, error)

	AutoAddIPAMPolicy(ctx context.Context, t *IPAMPolicy) (*IPAMPolicy, error)
	AutoAddLbPolicy(ctx context.Context, t *LbPolicy) (*LbPolicy, error)
	AutoAddNetwork(ctx context.Context, t *Network) (*Network, error)
	AutoAddNetworkInterface(ctx context.Context, t *NetworkInterface) (*NetworkInterface, error)
	AutoAddRouteTable(ctx context.Context, t *RouteTable) (*RouteTable, error)
	AutoAddRoutingConfig(ctx context.Context, t *RoutingConfig) (*RoutingConfig, error)
	AutoAddService(ctx context.Context, t *Service) (*Service, error)
	AutoAddVirtualRouter(ctx context.Context, t *VirtualRouter) (*VirtualRouter, error)
	AutoDeleteIPAMPolicy(ctx context.Context, t *IPAMPolicy) (*IPAMPolicy, error)
	AutoDeleteLbPolicy(ctx context.Context, t *LbPolicy) (*LbPolicy, error)
	AutoDeleteNetwork(ctx context.Context, t *Network) (*Network, error)
	AutoDeleteNetworkInterface(ctx context.Context, t *NetworkInterface) (*NetworkInterface, error)
	AutoDeleteRouteTable(ctx context.Context, t *RouteTable) (*RouteTable, error)
	AutoDeleteRoutingConfig(ctx context.Context, t *RoutingConfig) (*RoutingConfig, error)
	AutoDeleteService(ctx context.Context, t *Service) (*Service, error)
	AutoDeleteVirtualRouter(ctx context.Context, t *VirtualRouter) (*VirtualRouter, error)
	AutoGetIPAMPolicy(ctx context.Context, t *IPAMPolicy) (*IPAMPolicy, error)
	AutoGetLbPolicy(ctx context.Context, t *LbPolicy) (*LbPolicy, error)
	AutoGetNetwork(ctx context.Context, t *Network) (*Network, error)
	AutoGetNetworkInterface(ctx context.Context, t *NetworkInterface) (*NetworkInterface, error)
	AutoGetRouteTable(ctx context.Context, t *RouteTable) (*RouteTable, error)
	AutoGetRoutingConfig(ctx context.Context, t *RoutingConfig) (*RoutingConfig, error)
	AutoGetService(ctx context.Context, t *Service) (*Service, error)
	AutoGetVirtualRouter(ctx context.Context, t *VirtualRouter) (*VirtualRouter, error)
	AutoLabelIPAMPolicy(ctx context.Context, t *api.Label) (*IPAMPolicy, error)
	AutoLabelLbPolicy(ctx context.Context, t *api.Label) (*LbPolicy, error)
	AutoLabelNetwork(ctx context.Context, t *api.Label) (*Network, error)
	AutoLabelNetworkInterface(ctx context.Context, t *api.Label) (*NetworkInterface, error)
	AutoLabelRouteTable(ctx context.Context, t *api.Label) (*RouteTable, error)
	AutoLabelRoutingConfig(ctx context.Context, t *api.Label) (*RoutingConfig, error)
	AutoLabelService(ctx context.Context, t *api.Label) (*Service, error)
	AutoLabelVirtualRouter(ctx context.Context, t *api.Label) (*VirtualRouter, error)
	AutoListIPAMPolicy(ctx context.Context, t *api.ListWatchOptions) (*IPAMPolicyList, error)
	AutoListLbPolicy(ctx context.Context, t *api.ListWatchOptions) (*LbPolicyList, error)
	AutoListNetwork(ctx context.Context, t *api.ListWatchOptions) (*NetworkList, error)
	AutoListNetworkInterface(ctx context.Context, t *api.ListWatchOptions) (*NetworkInterfaceList, error)
	AutoListRouteTable(ctx context.Context, t *api.ListWatchOptions) (*RouteTableList, error)
	AutoListRoutingConfig(ctx context.Context, t *api.ListWatchOptions) (*RoutingConfigList, error)
	AutoListService(ctx context.Context, t *api.ListWatchOptions) (*ServiceList, error)
	AutoListVirtualRouter(ctx context.Context, t *api.ListWatchOptions) (*VirtualRouterList, error)
	AutoUpdateIPAMPolicy(ctx context.Context, t *IPAMPolicy) (*IPAMPolicy, error)
	AutoUpdateLbPolicy(ctx context.Context, t *LbPolicy) (*LbPolicy, error)
	AutoUpdateNetwork(ctx context.Context, t *Network) (*Network, error)
	AutoUpdateNetworkInterface(ctx context.Context, t *NetworkInterface) (*NetworkInterface, error)
	AutoUpdateRouteTable(ctx context.Context, t *RouteTable) (*RouteTable, error)
	AutoUpdateRoutingConfig(ctx context.Context, t *RoutingConfig) (*RoutingConfig, error)
	AutoUpdateService(ctx context.Context, t *Service) (*Service, error)
	AutoUpdateVirtualRouter(ctx context.Context, t *VirtualRouter) (*VirtualRouter, error)

	AutoWatchNetwork(ctx context.Context, in *api.ListWatchOptions) (NetworkV1_AutoWatchNetworkClient, error)
	AutoWatchService(ctx context.Context, in *api.ListWatchOptions) (NetworkV1_AutoWatchServiceClient, error)
	AutoWatchLbPolicy(ctx context.Context, in *api.ListWatchOptions) (NetworkV1_AutoWatchLbPolicyClient, error)
	AutoWatchVirtualRouter(ctx context.Context, in *api.ListWatchOptions) (NetworkV1_AutoWatchVirtualRouterClient, error)
	AutoWatchNetworkInterface(ctx context.Context, in *api.ListWatchOptions) (NetworkV1_AutoWatchNetworkInterfaceClient, error)
	AutoWatchIPAMPolicy(ctx context.Context, in *api.ListWatchOptions) (NetworkV1_AutoWatchIPAMPolicyClient, error)
	AutoWatchRoutingConfig(ctx context.Context, in *api.ListWatchOptions) (NetworkV1_AutoWatchRoutingConfigClient, error)
	AutoWatchRouteTable(ctx context.Context, in *api.ListWatchOptions) (NetworkV1_AutoWatchRouteTableClient, error)
}

// ServiceNetworkV1Server is the server interface for the service.
type ServiceNetworkV1Server interface {
	AutoWatchSvcNetworkV1(in *api.AggWatchOptions, stream NetworkV1_AutoWatchSvcNetworkV1Server) error

	AutoAddIPAMPolicy(ctx context.Context, t IPAMPolicy) (IPAMPolicy, error)
	AutoAddLbPolicy(ctx context.Context, t LbPolicy) (LbPolicy, error)
	AutoAddNetwork(ctx context.Context, t Network) (Network, error)
	AutoAddNetworkInterface(ctx context.Context, t NetworkInterface) (NetworkInterface, error)
	AutoAddRouteTable(ctx context.Context, t RouteTable) (RouteTable, error)
	AutoAddRoutingConfig(ctx context.Context, t RoutingConfig) (RoutingConfig, error)
	AutoAddService(ctx context.Context, t Service) (Service, error)
	AutoAddVirtualRouter(ctx context.Context, t VirtualRouter) (VirtualRouter, error)
	AutoDeleteIPAMPolicy(ctx context.Context, t IPAMPolicy) (IPAMPolicy, error)
	AutoDeleteLbPolicy(ctx context.Context, t LbPolicy) (LbPolicy, error)
	AutoDeleteNetwork(ctx context.Context, t Network) (Network, error)
	AutoDeleteNetworkInterface(ctx context.Context, t NetworkInterface) (NetworkInterface, error)
	AutoDeleteRouteTable(ctx context.Context, t RouteTable) (RouteTable, error)
	AutoDeleteRoutingConfig(ctx context.Context, t RoutingConfig) (RoutingConfig, error)
	AutoDeleteService(ctx context.Context, t Service) (Service, error)
	AutoDeleteVirtualRouter(ctx context.Context, t VirtualRouter) (VirtualRouter, error)
	AutoGetIPAMPolicy(ctx context.Context, t IPAMPolicy) (IPAMPolicy, error)
	AutoGetLbPolicy(ctx context.Context, t LbPolicy) (LbPolicy, error)
	AutoGetNetwork(ctx context.Context, t Network) (Network, error)
	AutoGetNetworkInterface(ctx context.Context, t NetworkInterface) (NetworkInterface, error)
	AutoGetRouteTable(ctx context.Context, t RouteTable) (RouteTable, error)
	AutoGetRoutingConfig(ctx context.Context, t RoutingConfig) (RoutingConfig, error)
	AutoGetService(ctx context.Context, t Service) (Service, error)
	AutoGetVirtualRouter(ctx context.Context, t VirtualRouter) (VirtualRouter, error)
	AutoLabelIPAMPolicy(ctx context.Context, t api.Label) (IPAMPolicy, error)
	AutoLabelLbPolicy(ctx context.Context, t api.Label) (LbPolicy, error)
	AutoLabelNetwork(ctx context.Context, t api.Label) (Network, error)
	AutoLabelNetworkInterface(ctx context.Context, t api.Label) (NetworkInterface, error)
	AutoLabelRouteTable(ctx context.Context, t api.Label) (RouteTable, error)
	AutoLabelRoutingConfig(ctx context.Context, t api.Label) (RoutingConfig, error)
	AutoLabelService(ctx context.Context, t api.Label) (Service, error)
	AutoLabelVirtualRouter(ctx context.Context, t api.Label) (VirtualRouter, error)
	AutoListIPAMPolicy(ctx context.Context, t api.ListWatchOptions) (IPAMPolicyList, error)
	AutoListLbPolicy(ctx context.Context, t api.ListWatchOptions) (LbPolicyList, error)
	AutoListNetwork(ctx context.Context, t api.ListWatchOptions) (NetworkList, error)
	AutoListNetworkInterface(ctx context.Context, t api.ListWatchOptions) (NetworkInterfaceList, error)
	AutoListRouteTable(ctx context.Context, t api.ListWatchOptions) (RouteTableList, error)
	AutoListRoutingConfig(ctx context.Context, t api.ListWatchOptions) (RoutingConfigList, error)
	AutoListService(ctx context.Context, t api.ListWatchOptions) (ServiceList, error)
	AutoListVirtualRouter(ctx context.Context, t api.ListWatchOptions) (VirtualRouterList, error)
	AutoUpdateIPAMPolicy(ctx context.Context, t IPAMPolicy) (IPAMPolicy, error)
	AutoUpdateLbPolicy(ctx context.Context, t LbPolicy) (LbPolicy, error)
	AutoUpdateNetwork(ctx context.Context, t Network) (Network, error)
	AutoUpdateNetworkInterface(ctx context.Context, t NetworkInterface) (NetworkInterface, error)
	AutoUpdateRouteTable(ctx context.Context, t RouteTable) (RouteTable, error)
	AutoUpdateRoutingConfig(ctx context.Context, t RoutingConfig) (RoutingConfig, error)
	AutoUpdateService(ctx context.Context, t Service) (Service, error)
	AutoUpdateVirtualRouter(ctx context.Context, t VirtualRouter) (VirtualRouter, error)

	AutoWatchNetwork(in *api.ListWatchOptions, stream NetworkV1_AutoWatchNetworkServer) error
	AutoWatchService(in *api.ListWatchOptions, stream NetworkV1_AutoWatchServiceServer) error
	AutoWatchLbPolicy(in *api.ListWatchOptions, stream NetworkV1_AutoWatchLbPolicyServer) error
	AutoWatchVirtualRouter(in *api.ListWatchOptions, stream NetworkV1_AutoWatchVirtualRouterServer) error
	AutoWatchNetworkInterface(in *api.ListWatchOptions, stream NetworkV1_AutoWatchNetworkInterfaceServer) error
	AutoWatchIPAMPolicy(in *api.ListWatchOptions, stream NetworkV1_AutoWatchIPAMPolicyServer) error
	AutoWatchRoutingConfig(in *api.ListWatchOptions, stream NetworkV1_AutoWatchRoutingConfigServer) error
	AutoWatchRouteTable(in *api.ListWatchOptions, stream NetworkV1_AutoWatchRouteTableServer) error
}
