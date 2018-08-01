package TestApi

import (
	InfraApi "github.com/pensando/sw/nic/e2etests/go/infra/api"
)

//TestInterface Interface
type TestInterface interface {
	Setup(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI)
	Run(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI) bool
	Teardown(srcEp InfraApi.EndpointAPI, dstEp InfraApi.EndpointAPI)
}

//TestManagerInterface Interface
type TestManagerInterface interface {
	GetInstance(moduleName string) TestInterface
}
