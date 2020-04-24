package ipif

import (
	"time"

	"github.com/krolaw/dhcp4"
)

const (
	// NaplesINB0Interface is inband interface 0
	NaplesINB0Interface = "inb_mnic0"

	// NaplesINB1Interface is inband interface 1
	NaplesINB1Interface = "inb_mnic1"

	// NaplesINBPrefix is the inband interface prefix
	NaplesINBPrefix = "inb_mnic"

	// NaplesMockInterface is a dummy interface used for tests
	NaplesMockInterface = "mock"

	// ApuluINB0Interface is inband interface 0 for apulu pipeline
	ApuluINB0Interface = "dsc0"

	// ApuluINB1Interface is inband interface 1 for apulu pipeline
	ApuluINB1Interface = "dsc1"

	// PensandoIdentifier identifies Pensando vendor options
	PensandoIdentifier = "Pensando"

	// FRUFilePath captures the location of the fru.json
	FRUFilePath = "/tmp/fru.json"

	// IPv4OctetSize parses 4 bytes form vendor specific options as IP Addresses
	IPv4OctetSize = 4

	// AdmissionRetryDuration captures time to wait between dhcp and admission retry attempts
	AdmissionRetryDuration = time.Minute * 1

	// DHCPTimeout captures the timeout the dhcp client waits for the packets to be received
	DHCPTimeout = time.Minute * 1
)

var (
	// NaplesOOBInterface is the out of band interface
	NaplesOOBInterface = "oob_mnic0"

	//NaplesInbandInterface is the inband interface
	NaplesInbandInterface = "bond0"

	// PensandoDHCPRequestOption constructs dhcp vendor option for pensando
	PensandoDHCPRequestOption = dhcp4.Option{
		Code:  dhcp4.OptionVendorClassIdentifier,
		Value: []byte(PensandoIdentifier),
	}

	// DSCIfIDToInterfaceName maps the ifid to interface name
	DSCIfIDToInterfaceName = map[int8]string{
		0: NaplesInbandInterface,
	}

	// ApuluDSCIfIDToInterfaceName maps the ifid to interface name
	ApuluDSCIfIDToInterfaceName = map[int8]string{
		0: ApuluINB0Interface,
		1: ApuluINB1Interface,
	}
)

//DHCP Vendor specific suboption codes
const (
	// Venice controller coordinates
	VeniceCoordinatesSubOptID = 241

	// DSC Interface IP addresses
	DSCInterfaceIPsSubOptID = 242
)
