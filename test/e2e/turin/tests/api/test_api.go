package TestApi

import (
	Infra "github.com/pensando/sw/test/utils/infra"
)

//TestInterface Interface
type TestInterface interface {
	Setup(Infra.Context) error
	Run(Infra.Context) error
	Teardown(Infra.Context) error
}

//TestManagerInterface Interface
type TestManagerInterface interface {
	GetInstance(moduleName string) TestInterface
}
