package public

import (
	"errors"
	"fmt"
)

// NetworkSpec specifies a name an vlanID for the network
type NetworkSpec struct {
	Name   string
	VlanID int32
}

// MkNetworkName specifies the unique name for the network
func (nw NetworkSpec) MkNetworkName() string {
	return fmt.Sprintf("%s-vlan%d", nw.Name, nw.VlanID)
}

// DataNetworkSpec specifies a data network information
type DataNetworkSpec struct {
	Name           string
	SwitchIP       string
	SwitchPort     uint32
	SwitchUsername string
	SwitchPassword string
}

// ProvisionSpec is the specification for provisioning tools.
type ProvisionSpec struct {
	Username       string            `yaml:"username"`
	Password       string            `yaml:"password"`
	PrivateKeyFile string            `yaml:"private_key_file"`
	Vars           map[string]string `yaml:"vars" json:"Vars,omitempty"`
}

// Validate validates the content of the ProvisionSpec struct
func (p ProvisionSpec) Validate() error {
	if p.Username == "" {
		return errors.New("invalid username")
	}

	if p.Password == "" && p.PrivateKeyFile == "" {
		return errors.New("you must use at least one of a password or private key")
	}

	if p.Vars == nil {
		p.Vars = map[string]string{}
	}

	return nil
}

// Instance is the structure for each machine warmd allocated
type Instance struct {
	Name         string
	NodeMgmtIP   string
	Type         string
	PvtNetworks  []NetworkSpec
	DataNetworks []DataNetworkSpec
}

// WarmdEnv is the environment data for warmd data
type WarmdEnv struct {
	ID        string
	Instances []Instance
	Provision ProvisionSpec
}
