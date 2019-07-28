package ipif

import "github.com/krolaw/dhcp4"

const (
	//NaplesInbandInterface is the inband interface
	NaplesInbandInterface = "bond0"

	// NaplesOOBInterface is the out of band interface
	NaplesOOBInterface = "oob_mnic0"

	// NaplesINB0Interface is inband interface 0
	NaplesINB0Interface = "inb_mnic0"

	// NaplesINB1Interface is inband interface 1
	NaplesINB1Interface = "inb_mnic1"

	// NaplesMockInterface is a dummy interface used for tests
	NaplesMockInterface = "mock"

	// PensandoIdentifier identifies Pensando vendor options
	PensandoIdentifier = "Pensando"

	// FRUFilePath captures the location of the fru.json
	FRUFilePath = "/tmp/fru.json"

	// IPv4OctetSize parses 4 bytes form vendor specific options as IP Addresses
	IPv4OctetSize = 4
)

var (
	// PensandoDHCPRequestOption constructs dhcp vendor option for pensando
	PensandoDHCPRequestOption = dhcp4.Option{
		Code:  dhcp4.OptionVendorClassIdentifier,
		Value: []byte(PensandoIdentifier),
	}
)
