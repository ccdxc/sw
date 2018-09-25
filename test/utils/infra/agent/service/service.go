package service

import (
	"context"
)

// NaplesSimConfig structure
type NaplesSimConfig struct {
	Name            string
	NodeID          uint32
	CtrlNwIPRange   string //Control network
	PassThroughMode bool
	DataIntfs       []string
	WithQemu        bool
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
	OnQemu   bool
}

// AppInterface structure
type AppInterface struct {
	Name       string
	MacAddress string
	IPaddress  string
	PrefixLen  uint32
}

// AppVlanInterface structure
type AppVlanInterface struct {
	ParentIntfName      string
	ParentMacMacAddress string
	MacAddress          string
	Vlan                uint32
	IPaddress           string
	PrefixLen           uint32
}

// AppService APIs
type AppService interface {
	BringUp(context.Context, *AppConfig) error
	Teardown(context.Context, *AppConfig) error
	RunCommand(context.Context, string, string, bool) ([]string, []string, int, error)
	AttachInterface(context.Context, string, *AppInterface) error
	AddVlanInterface(context.Context, string, *AppVlanInterface) error
}

// QemuConfig structure
type QemuConfig struct {
	Name   string
	NodeID uint32
	Image  string
}

// QemuService APIs
type QemuService interface {
	BringUp(context.Context, *QemuConfig) error
	Teardown(context.Context, *QemuConfig) error
	RunCommand(context.Context, string) (string, string, error)
}

// NodeServices Different types of node services.
type NodeServices struct {
	Naples NaplesSimService
	App    AppService
	Qemu   QemuService
}
