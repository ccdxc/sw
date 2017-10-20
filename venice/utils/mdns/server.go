//{C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mdns

import (
	"log"
	"os"

	"github.com/grandcat/zeroconf"

	"github.com/pensando/sw/venice/utils/sysif"
)

// Server is reference to caller of NewServer() API, needed to shutdown the instance
type server struct {
	server *zeroconf.Server
}

// Stop the server
func (s *server) Stop() {
	s.server.Shutdown()
}

// Publish updated data to the network
func (s *server) Publish(data map[string]string) {
	var txtRecords []string
	for k, v := range data {
		txtRecords = append(txtRecords, k+"="+v)
	}
	s.server.SetText(txtRecords)
}

// NewServer publishes records to network and starts mdns server which responds to queries from network
func NewServer(service, domain string, port int, data map[string]string) (Server, error) {
	s := &server{}
	var err error

	ni, err := sysif.GetNodeInfo()
	if err != nil {
		log.Printf("sysif.GetNodeInfo failed with error: %v", err)
	}
	hostname, err := os.Hostname()
	if (err != nil) || (hostname == "localhost") {
		hostname = ni.ChassisSerial + "_" + ni.MachineID
	}

	// need multiple txt records because of the DNS limit on the size of single TXT record to 256 bytes
	var txtRecords []string
	for k, v := range data {
		txtRecords = append(txtRecords, k+"="+v)
	}
	s.server, err = zeroconf.Register(hostname, service, domain, port, txtRecords, nil)

	return s, err
}
