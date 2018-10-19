package main

import (
	"fmt"
	"net"
)

func main() {

	err := externalIP()
	if err != nil {
		fmt.Println(err)
	}

}

func externalIP() error {
	ifaces, err := net.Interfaces()
	if err != nil {
		return err
	}
	for _, iface := range ifaces {
		addrs, err := iface.Addrs()
		if err != nil {
			return err
		}
		for _, addr := range addrs {
			var ip net.IP
			switch v := addr.(type) {
			case *net.IPNet:
				ip = v.IP
			case *net.IPAddr:
				ip = v.IP
			}
			ip = ip.To4()
			if ip == nil {
				continue // not an ipv4 address
			}
			fmt.Println(iface.Name + ":" + ip.String())
		}
	}
	return nil
}
