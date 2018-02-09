package tcpproxy

import (
	"net"
	"time"
)

// TCPProxy is a proxy implementation at tcp layer
type TCPProxy interface {
	// Target tcp address for this proxy. This can be changed while the proxy is
	// running and is effective for all future connection attempts
	Target(addr string)

	// Run() starts running the proxy listening at the specified address
	// If the IP field of laddr is nil or an unspecified IP address,
	// Run listens on all available unicast and anycast IP addresses
	// of the local system.
	// If the Port field of laddr is 0, a port number is automatically
	// chosen.
	Run(laddr string) error

	// Stop the proxy
	Stop()

	// Addr returns the local address on which the proxy is listening to requests
	// typically needed when address is not specified as part of Run()
	Addr() net.Addr

	// DialTimeout is used as timeout while connecting to Target during establishment of new
	// connection. Default is same as timeout specified in net.Dial()
	// Applications may want to set this to lower value to fail fast
	DialTimeout(timeout time.Duration)
}
