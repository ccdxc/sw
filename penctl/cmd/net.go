//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"net"
	"os"
	"strings"
	"time"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/globals"
)

var revProxyPort string
var naplesURL string
var naplesIP string

func getNaplesURL() (string, error) {
	if verbose {
		fmt.Printf("naplesURL: %s\n", naplesURL)
	}
	if naplesURL != "" {
		return naplesURL, nil
	}
	return "", errors.New("Could not figure out dsc URL")
}

func pickNetwork(cmd *cobra.Command, args []string) error {
	if mockMode {
		return nil
	}
	if val, ok := os.LookupEnv("DSC_URL"); ok {
		for strings.HasSuffix(val, "/") {
			val = val[:len(val)-1]
		}
		naplesURL = val
	} else if val, ok := os.LookupEnv("NAPLES_URL"); ok {
		for strings.HasSuffix(val, "/") {
			val = val[:len(val)-1]
		}
		naplesURL = val
	} else if cmd.Flags().Changed("localhost") {
		naplesURL = "http://127.0.0.1"
	} else {
		return errors.New("Distributed Service Card unreachable. please set DSC_URL variable to http://<naples_ip>")
	}
	naplesIP = strings.TrimPrefix(naplesURL, "http://")
	revProxyPort = globals.AgentProxyPort
	naplesURL += ":" + revProxyPort + "/"

	return isNaplesReachable()
}

func isNaplesReachable() error {
	seconds := 5
	timeOut := time.Duration(seconds) * time.Second
	_, err := net.DialTimeout("tcp", naplesIP+":"+revProxyPort, timeOut)

	if err != nil {
		fmt.Printf("Could not reach Distributed Service Card on %s\n", naplesIP+":"+revProxyPort)
		return err
	}
	return nil
}

func isNaplesReachableOverLocalHost() error {
	seconds := 5
	timeOut := time.Duration(seconds) * time.Second
	_, err := net.DialTimeout("tcp", "127.0.0.1"+":"+revProxyPort, timeOut)

	if err != nil {
		fmt.Printf("Could not reach Distributed Service Card on %s\n", "127.0.0.1"+":"+revProxyPort)
		return err
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
