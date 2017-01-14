package sysif

import "net"

// NodeInfo During cluster discovery this is the information that gets passed by individual CMD
// before starting the cluster formation process
type NodeInfo struct {
	ChassisSerial  string
	MachineID      string
	MemTotal       uint64
	NumCPU         int
	NumPhysicalCPU int
	NumCore        int
	CPUModelName   string
	CPUMHz         float64
	NetIntfs       []NetIntf
	Disks          []DiskInfo
}

// NetIntf - info about interfaces
type NetIntf struct {
	Name         string
	HardwareAddr net.HardwareAddr
	Flags        net.Flags
	Speed        int
	Addrs        []net.Addr
}

// DiskInfo has data for each physical disk
// TODO - Once the scheme for virtual volumes are known, filter them out
type DiskInfo struct {
	UUID   string
	Name   string
	Serial string
	Model  string
	Size   string
}
