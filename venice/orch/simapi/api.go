package simapi

// NwIFSetReq defines a set request (i.e. create or update)
type NwIFSetReq struct {
	Name       string
	IPAddr     string
	MacAddr    string
	Vlan       string
	PortGroup  string
	WLName     string
	SmartNIC   string
	Attributes map[string]string
}

// NwIFSetResp defines a set response
type NwIFSetResp struct {
	UUID     string // UUID for the NwIF
	WlUUID   string // e.g. containerID
	IPAddr   string
	MacAddr  string
	ErrorMsg string
}

// NwIFDelResp defines a delete response
type NwIFDelResp struct {
	ErrorMsg string
}

// OrchSim defines an orch simulator interface
type OrchSim interface {
	Destroy()
	Run(string, []string, int) (string, error)
	SetHostURL(string, string) error
	CreateNwIF(string, *NwIFSetReq) (*NwIFSetResp, error)
	DeleteNwIF(string, string) *NwIFDelResp
}
