package netutils

import (
	"net"
	"time"
)

// ARPSendGratuitous sends count number of gratuitous ARP messages via the specified interface,
// "interval" time apart.
func ARPSendGratuitous(intf *net.Interface, ip net.IP, count int, interval time.Duration) error {
	return nil
}
