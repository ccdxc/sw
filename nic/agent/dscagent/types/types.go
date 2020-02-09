package types

import "github.com/gorilla/mux"

//DistributedServiceCardStatus captures the NAPLES status object
type DistributedServiceCardStatus struct {
	DSCMode             string           `json:"dsc-mode,omitempty"`
	DSCName             string           `json:"dsc-name,omitempty"`
	MgmtIP              string           `json:"mgmt-ip,omitempty"`
	MgmtIntf            string           `json:"mgmt-intf,omitempty"`
	Controllers         []string         `json:"controllers,omitempty"`
	IsConnectedToVenice bool             `json:"is-connected-to-venice"`
	DSCInterfaceIPs     []DSCInterfaceIP `json:"dsc-interfaces,omitempty"`
	DSCStaticRoutes     []DSCStaticRoute `json:"dsc-static-routes,omitempty"`
	LoopbackIP          string           `json:"loopback-ip"`
}

//DSCInterfaceIP captures the IP interface info of DSC interfaces
type DSCInterfaceIP struct {
	IfID          uint32
	DestPrefixLen uint32
	IPAddress     string
	GatewayIP     string
}

//DSCStaticRoute captures the static route info of Naples
type DSCStaticRoute struct {
	DestAddr      string
	DestPrefixLen uint32
	NextHop       string
}

// SubRouterAddFunc adds subrouters
type SubRouterAddFunc func(*mux.Router)
