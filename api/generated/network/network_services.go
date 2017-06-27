/*
Package network is a auto generated package.
Input file: protos/network.proto
*/
package network

import (
	"context"
)

type ServiceTenantV1 interface {
	GetTenantList(ctx context.Context, t TenantList) (TenantList, error)
	TenantOper(ctx context.Context, t Tenant) (Tenant, error)
}

type ServiceNetworkV1 interface {
	GetNetworkList(ctx context.Context, t NetworkList) (NetworkList, error)
	NetworkOper(ctx context.Context, t Network) (Network, error)
}

type ServiceSecurityGroupV1 interface {
	GetSecurityGroupList(ctx context.Context, t SecurityGroupList) (SecurityGroupList, error)
	SecurityGroupOper(ctx context.Context, t SecurityGroup) (SecurityGroup, error)
}

type ServiceSgpolicyV1 interface {
	GetSgpolicyList(ctx context.Context, t SgpolicyList) (SgpolicyList, error)
	SgpolicyOper(ctx context.Context, t Sgpolicy) (Sgpolicy, error)
}

type ServiceServiceV1 interface {
	GetServiceList(ctx context.Context, t ServiceList) (ServiceList, error)
	ServiceOper(ctx context.Context, t Service) (Service, error)
}

type ServiceLbPolicyV1 interface {
	GetLbPolicyList(ctx context.Context, t LbPolicyList) (LbPolicyList, error)
	LbPolicyOper(ctx context.Context, t LbPolicy) (LbPolicy, error)
}

type ServiceEndpointV1 interface {
	GetEndpointList(ctx context.Context, t EndpointList) (EndpointList, error)
	EndpointOper(ctx context.Context, t Endpoint) (Endpoint, error)
}
