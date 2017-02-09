//{C} Copyright 2017 Pensando Systems Inc. All rights reserved.
package sysif

import (
	"io/ioutil"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"github.com/c9s/goprocinfo/linux"
)

func getIntfSpeed(name string) (speed int, err error) {
	bytes, err := ioutil.ReadFile("/sys/class/net/" + name + "/speed")
	if err != nil {
		return 0, err
	}
	s := strings.TrimSpace(string(bytes))
	speed, err = strconv.Atoi(s)
	return speed, err
}

func getNetIntfs() []NetIntf {
	var allIntfs []NetIntf
	intfs, _ := net.Interfaces()
	for _, intf := range intfs {
		oneIntf := NetIntf{HardwareAddr: intf.HardwareAddr, Flags: intf.Flags, Name: intf.Name}
		if oneIntf.HardwareAddr == nil {
			continue
		}
		if (intf.Flags & net.FlagUp) == 0 {
			continue
		}
		// TODO: move this a common location and use a separate helper routine for filtering this
		if !strings.HasPrefix(intf.Name, "eth") && !strings.HasPrefix(intf.Name, "en") {
			continue
		}
		addrs, err := intf.Addrs()
		if err == nil {
			//oneIntf.Addrs = make([]net.IPNet, 0, len(addrs))
			for _, r := range addrs {
				switch v := r.(type) {
				case *net.IPNet:
					oneIntf.Addrs = append(oneIntf.Addrs, *v)
				}
			}
		}
		s, err := getIntfSpeed(oneIntf.Name)
		if err == nil {
			oneIntf.Speed = s
		}
		allIntfs = append(allIntfs, oneIntf)
	}
	return allIntfs
}

// need this separate routine to get the UUID of disks by walking /dev/disk/by-id
// returns something of the form:
/* map[
nvme0n1:nvme-INTEL_SSDPE2MX400G4_CVPD6324006H400NGN
nvme0n1p1:nvme-INTEL_SSDPE2MX400G4_CVPD6324006H400NGN-part1
sda:wwn-0x5000c5008ce63513
sda1:wwn-0x5000c5008ce63513-part1
*/
func getDiskUUIDMap() map[string]string {
	var m map[string]string
	m = make(map[string]string, 3)
	f := func(path string, info os.FileInfo, err error) error {
		if info == nil {
			return nil
		}
		if strings.HasPrefix(info.Name(), "wwn-") || strings.HasPrefix(info.Name(), "nvm") {
			p, err := filepath.EvalSymlinks(path)
			if err != nil {
				m[filepath.Base(p)] = info.Name()
			}
		}
		// some old devices dont have WWN. for them use the ata- interface. Hence add to the map only if it does not exist already
		if strings.HasPrefix(info.Name(), "ata-") {
			p, err := filepath.EvalSymlinks(path)
			d := filepath.Base(p)
			if (err == nil) && (m[d] == "") {
				m[d] = info.Name()
			}
		}

		return nil
	}
	filepath.Walk("/dev/disk/by-id", f)
	return m
}

// getDiskInfo - Merge info from lsblk as well as UUID map
func getDiskInfo() []DiskInfo {
	var Disks []DiskInfo
	uuidMap := getDiskUUIDMap()

	cmd := "lsblk -b -P -d  -o UUID,NAME,SERIAL,MODEL,SIZE,TYPE | grep disk"
	rp := regexp.MustCompile("(\\S+)=\"(.*?)\"")
	outBytes, _ := exec.Command("bash", "-c", cmd).Output()
	lines := strings.Split(strings.TrimSpace(string(outBytes)), "\n")
	for _, line := range lines {
		Disk := DiskInfo{}
		x := rp.FindAllStringSubmatch(line, -1)
		for _, v := range x {
			key := v[1]
			value := v[2]
			switch key {
			case "UUID":
				Disk.UUID = value
			case "NAME":
				Disk.Name = value
			case "SERIAL":
				Disk.Serial = value
			case "MODEL":
				Disk.Model = value
			case "SIZE":
				Disk.Size = value
			}
		}
		if Disk.UUID == "" {
			Disk.UUID = uuidMap[Disk.Name]
		}
		Disks = append(Disks, Disk)
	}
	return Disks
}

// GetNodeInfo returns info of a node before cluster formation
func GetNodeInfo() (*NodeInfo, error) {
	var info = NodeInfo{}

	meminfo, err := linux.ReadMemInfo("/proc/meminfo")
	if err != nil {
		return nil, err
	}
	info.MemTotal = meminfo.MemTotal

	cpuinfo, err := linux.ReadCPUInfo("/proc/cpuinfo")
	if err != nil {
		return nil, err
	}
	info.NumCPU = cpuinfo.NumCPU()
	info.NumCore = cpuinfo.NumCore()
	info.NumPhysicalCPU = cpuinfo.NumPhysicalCPU()
	info.CPUModelName = cpuinfo.Processors[0].ModelName
	info.CPUMHz = cpuinfo.Processors[0].MHz

	bytes, _ := ioutil.ReadFile("/sys/devices/virtual/dmi/id/chassis_serial")
	info.ChassisSerial = strings.TrimSpace(string(bytes))

	bytes, _ = ioutil.ReadFile("/etc/machine-id")
	info.MachineID = strings.TrimSpace(string(bytes))

	info.Disks = getDiskInfo()
	info.NetIntfs = getNetIntfs()

	return &info, nil
}
