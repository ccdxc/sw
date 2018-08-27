package service

import (
	"context"
)

// NaplesSimConfig structure
type NaplesSimConfig struct {
	Name            string
	NodeID          uint32
	CtrlNwIPRange   string //Control network
	TunnelIPStart   string
	TunnelInterface string
	TunnelIPAddress string
}

// NaplesSimService APIs
type NaplesSimService interface {
	BringUp(context.Context, *NaplesSimConfig) error
	Teardown(context.Context, *NaplesSimConfig) error
	RunCommand(context.Context, string) (string, string, error)
}

// AppConfig structure
type AppConfig struct {
	Name     string
	Registry string
}

// AppInterface structure
type AppInterface struct {
	Name       string
	MacAddress string
	Vlan       uint32
	IPaddress  string
	PrefixLen  uint32
}

// AppService APIs
type AppService interface {
	BringUp(context.Context, *AppConfig) error
	Teardown(context.Context, *AppConfig) error
	RunCommand(context.Context, string, string, bool) ([]string, []string, int, error)
	AttachInterface(context.Context, string, *AppInterface) error
}

// NodeServices Different types of node services.
type NodeServices struct {
	Naples NaplesSimService
	App    AppService
}
