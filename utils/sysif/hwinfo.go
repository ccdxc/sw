//{C} Copyright 2017 Pensando Systems Inc. All rights reserved.
package sysif

import (
	"bytes"
	"encoding/json"
	"fmt"
	"log"
	"net"
	"strconv"
	"strings"
)

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
	Addrs        []net.IPNet
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

// MarshallToStringMap -convert NodeInfo to strings
func MarshallToStringMap(ni *NodeInfo) (data map[string]string) {
	m := make(map[string]string)
	m["ChassisSerial"] = ni.ChassisSerial
	m["MachineID"] = ni.MachineID
	m["CPUModelName"] = ni.CPUModelName
	m["MemTotal"] = fmt.Sprintf("%d", ni.MemTotal)
	m["CPUMHz"] = fmt.Sprintf("%f", ni.CPUMHz)
	m["NumCPU"] = fmt.Sprintf("%d", ni.NumCPU)
	m["NumCore"] = fmt.Sprintf("%d", ni.NumCore)
	m["NumPhysicalCPU"] = fmt.Sprintf("%d", ni.NumPhysicalCPU)
	for index, i := range ni.NetIntfs {
		var buffer bytes.Buffer
		body, _ := json.Marshal(i)
		buffer.Write(body)
		m["NetIfs_"+fmt.Sprintf("%d", index)] = buffer.String()
	}
	for index, d := range ni.Disks {
		var buffer bytes.Buffer
		body, _ := json.Marshal(d)
		buffer.Write(body)
		m["Disks_"+fmt.Sprintf("%d", index)] = buffer.String()
	}
	log.Printf("%+v", m)
	return m
}

// UnmarshallFromStringMap - get Nodeinfo from map of strings
func UnmarshallFromStringMap(data map[string]string) NodeInfo {
	var err error

	nbd := NodeInfo{}
	for k, v := range data {
		switch k {
		case "ChassisSerial":
			nbd.ChassisSerial = v
		case "MachineID":
			nbd.MachineID = v
		case "MemTotal":
			nbd.MemTotal, _ = strconv.ParseUint(v, 0, 0)
		case "NumCPU":
			nbd.NumCPU, _ = strconv.Atoi(v)
		case "NumCore":
			nbd.NumCore, _ = strconv.Atoi(v)
		case "NumPhysicalCPU":
			nbd.NumPhysicalCPU, _ = strconv.Atoi(v)
		case "CPUMHz":
			nbd.CPUMHz, _ = strconv.ParseFloat(v, 64)
		case "CPUModelName":
			nbd.CPUModelName = v
		}
		if strings.HasPrefix(k, "Disks_") {
			var n DiskInfo
			err = json.Unmarshal([]byte(v), &n)
			if err != nil {
				log.Println(err, v)
			} else {
				nbd.Disks = append(nbd.Disks, n)
			}
		}
		if strings.HasPrefix(k, "NetIfs_") {
			var n NetIntf
			n.Addrs = make([]net.IPNet, 0, 4)
			err = json.Unmarshal([]byte(v), &n)
			if err != nil {
				log.Println(err, v)
			} else {
				nbd.NetIntfs = append(nbd.NetIntfs, n)
			}
		}
	}
	return nbd
}
