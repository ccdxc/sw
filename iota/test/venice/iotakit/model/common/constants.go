package common

//ModelType type
type ModelType int

const (
	//DefaultModel for GS
	DefaultModel ModelType = 0
	//VcenterModel for Vcenter
	VcenterModel = 1

	//CloudModel for equnix
	CloudModel = 2

	//BaseNetModel for classic
	BaseNetModel = 3
)

//NetworkType networkTypes
type NetworkType int

const (
	//DefaultNetworkType
	DefaultNetworkType NetworkType = 0
	//VmotionNetworkType vmotion
	VmotionNetworkType NetworkType = 1
)

//NetworkSpec to ceate
type NetworkSpec struct {
	Name   string
	Switch string
	Nodes  []string
	NwType NetworkType
}

//MockVeniceURL mock venice url
const MockVeniceURL = "localhost:9443"
// Vmotion subnet used for vmotion
const VmotionSubnet = "169.254.0"
