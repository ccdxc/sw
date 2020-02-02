package objects

import (
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

//Switch represents switch objet
type Switch struct {
	dataSwitch *iota.DataSwitch
}

func NewSwitch(sw *iota.DataSwitch) *Switch {
	return &Switch{dataSwitch: sw}
}

func (s *Switch) IP() string {
	return s.dataSwitch.Ip
}

// SwitchPortCollection ports
type SwitchPortCollection struct {
	CollectionCommon
	Ports []*SwitchPort
}

// SwitchPort represents a port in a data switch
type SwitchPort struct {
	sw       *Switch
	hostName string
	Port     string
}

func NewSwitchPort(hostname string, sw *Switch, port string) *SwitchPort {
	return &SwitchPort{
		Port:     port,
		sw:       sw,
		hostName: hostname,
	}
}

func (sp *SwitchPort) SwitchIP() string {
	return sp.sw.dataSwitch.Ip
}

func (sp *SwitchPort) SwitchUsernme() string {
	return sp.sw.dataSwitch.Username
}

func (sp *SwitchPort) SwitchPassword() string {
	return sp.sw.dataSwitch.Password
}

// SelectByPercentage returns a collection on switch ports based on percentage.
func (pc *SwitchPortCollection) SelectByPercentage(percent int) (*SwitchPortCollection, error) {
	if percent > 100 {
		return nil, fmt.Errorf("Invalid percentage input %v", percent)
	}

	if pc.err != nil {
		return nil, fmt.Errorf("switch port collection error (%s)", pc.err)
	}

	//Get equal share from each host to be fair
	hostPortMap := make(map[string]*[]*SwitchPort)
	for _, port := range pc.Ports {
		if _, ok := hostPortMap[port.hostName]; !ok {
			hostPortMap[port.hostName] = &[]*SwitchPort{}
		}
		swPorts, _ := hostPortMap[port.hostName]
		*swPorts = append(*swPorts, port)
	}

	ret := &SwitchPortCollection{}
	for _, ports := range hostPortMap {
		for i, port := range *ports {
			added := false
			for _, addedPort := range ret.Ports {
				if addedPort == port {
					added = true
					break
				}
			}
			if !added {
				ret.Ports = append(ret.Ports, port)
				//Break if reached limit for this host
				if (i + 1) >= len(*ports)*percent/100 {
					break
				}
			}
		}
		//If adding one more will execeed limit
		if len(ret.Ports) >= len(pc.Ports)*percent/100 {
			break
		}
	}

	if len(ret.Ports) == 0 {
		return nil, fmt.Errorf("Did not find ports matching percentage (%v)", percent)
	}
	return ret, nil
}
