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
)

//MockVeniceURL mock venice url
const MockVeniceURL = "localhost:9443"
