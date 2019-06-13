package ipif

import "github.com/krolaw/dhcp4"

const (
	//NaplesInbandInterface is the inband interface
	NaplesInbandInterface = "bond0"

	// NaplesOOBInterface is the out of band interface
	NaplesOOBInterface = "oob_mnic0"

	// NaplesMockInterface is a dummy interface used for tests
	NaplesMockInterface = "mock"

	// PensandoIdentifier identifies Pensando vendor options
	PensandoIdentifier = "Pensando"

	// FRUFilePath captures the location of the fru.json
	FRUFilePath = "/tmp/fru.json"
)

var (
	// PensandoDHCPRequestOption constructs dhcp vendor option for pensando
	PensandoDHCPRequestOption = dhcp4.Option{
		Code:  dhcp4.OptionVendorClassIdentifier,
		Value: []byte(PensandoIdentifier),
	}
)
