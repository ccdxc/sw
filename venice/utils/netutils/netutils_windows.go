package netutils

import (
	"errors"
	"net"
	"time"
)

func ARPSendGratuitous(intf *net.Interface, ip net.IP, count int, interval time.Duration) error {
	return errors.New("Gratuitous ARP not implemented for Windows")
}
