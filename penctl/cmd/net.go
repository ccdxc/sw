//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"os"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/globals"
)

var naplesIP string
var revProxyPort string
var naplesURL string

func getNaplesIPFromIntf(ifname string) (string, error) {
	return "169.254.0.1", nil
}

func getNaplesURL() (string, error) {
	if verbose {
		fmt.Printf("naplesURL: %s\n", naplesURL)
	}
	if naplesURL != "" {
		return naplesURL, nil
	}
	return "", errors.New("Could not figure out naplesURL")
}

func pickNetwork(cmd *cobra.Command, args []string) error {
	var err error
	if val, ok := os.LookupEnv("NAPLES_URL"); ok {
		if strings.HasSuffix(val, "/") {
			val = val[:len(val)-1]
		}
		naplesURL = val
	} else if cmd.Flags().Changed("ip") {
		naplesIP = ip
		err = nil
		naplesURL = "http://" + naplesIP
	} else if cmd.Flags().Changed("interface") {
		naplesIP, err = getNaplesIPFromIntf(intf)
		naplesURL = "http://" + naplesIP
	} else if val, ok := os.LookupEnv("PENETHDEV"); !ok {
		if verbose {
			fmt.Println("PENETHDEV flag not set")
		}
		err = errors.New("naples unreachable. please set PENETHDEV variable or use -i/--interface flag")
	} else {
		naplesIP, err = getNaplesIPFromIntf(val)
		naplesURL = "http://" + naplesIP
	}
	if cmd.Flags().Changed("localhost") {
		naplesIP = "127.0.0.1"
		naplesURL = "http://" + naplesIP
	} else if err != nil {
		if verbose {
			fmt.Printf("Could not get a valid naplesIP: %s\n", err)
		}
		return err
	}
	revProxyPort = globals.RevProxyPort
	naplesURL += ":" + revProxyPort + "/"
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
