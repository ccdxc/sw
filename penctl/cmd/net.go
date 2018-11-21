//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"net"
	"os"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/apigen/validators"
)

var naplesIP string
var revProxyPort string

func getNaplesIPFromIntf(ifname string) (string, error) {
	ifaces, err := net.Interfaces()
	if err != nil {
		return "", err
	}
	for _, iface := range ifaces {
		if verbose {
			fmt.Printf("Interface %s\n", iface.Name)
		}
		if iface.Name == ifname {
			if verbose {
				fmt.Printf("Found naples interface %s\n", iface.Name)
			}
			addrs, err := iface.Addrs()
			if err != nil {
				return "", err
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
				if verbose {
					fmt.Println(iface.Name + ":" + ip.String())
				}
				ip[3]++
				naplesIP = ip.String()
				if verbose {
					fmt.Println("Naples Ip: " + naplesIP)
				}
				return naplesIP, nil
			}
		}
	}
	return "", errors.New("Interface " + ifname + " not found")
}

func pickNetwork(cmd *cobra.Command, args []string) error {
	var err error
	if cmd.Flags().Changed("ip") {
		naplesIP = ip
		err = nil
	} else if cmd.Flags().Changed("interface") {
		naplesIP, err = getNaplesIPFromIntf(intf)
	} else if val, ok := os.LookupEnv("PENETHDEV"); !ok {
		if verbose {
			fmt.Println("PENETHDEV flag not set")
		}
		err = errors.New("naples unreachable. please set PENETHDEV variable or use -i/--interface flag")
	} else {
		naplesIP, err = getNaplesIPFromIntf(val)
	}
	if cmd.Flags().Changed("localhost") {
		naplesIP = "127.0.0.1"
	} else if err != nil {
		if verbose {
			fmt.Printf("Could not get a valid naplesIP: %s\n", err)
		}
		return err
	}
	revProxyPort = globals.RevProxyPort
	if !impl.IPAddr(naplesIP) {
		panic(errors.New("Not valid Naples IP"))
	}
	if verbose {
		fmt.Println("TODO: Namespace stuff here")
	}
	return nil
}

//TODO: Fix the username
func getNaplesUser() string {
	if mockMode {
		return "penctltestuser"
	}
	return "root"
}

//TODO: Fix the password
func getNaplesPwd() string {
	if mockMode {
		return "Pen%Ctl%Test%Pwd"
	}
	return "pen123"
}
