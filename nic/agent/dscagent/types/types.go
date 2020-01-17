package types

import "github.com/gorilla/mux"

//DistributedServiceCardStatus captures the NAPLES status object
type DistributedServiceCardStatus struct {
	DSCMode             string   `json:"dsc-mode,omitempty"`
	DSCName             string   `json:"dsc-name,omitempty"`
	MgmtIP              string   `json:"mgmt-ip,omitempty"`
	MgmtIntf            string   `json:"mgmt-intf,omitempty"`
	Controllers         []string `json:"controllers,omitempty"`
	IsConnectedToVenice bool     `json:"is-connected-to-venice"`
}

// SubRouterAddFunc adds subrouters
type SubRouterAddFunc func(*mux.Router)
