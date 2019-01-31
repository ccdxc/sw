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
	return "", errors.New("Could not figure out naplesURL")
}

func pickNetwork(cmd *cobra.Command, args []string) error {
	if val, ok := os.LookupEnv("NAPLES_URL"); ok {
		for strings.HasSuffix(val, "/") {
			val = val[:len(val)-1]
		}
		naplesURL = val
	} else if cmd.Flags().Changed("localhost") {
		naplesURL = "http://127.0.0.1"
	} else {
		return errors.New("naples unreachable. please set NAPLES_URL variable to http://<naples_ip>")
	}
	naplesIP = strings.TrimPrefix(naplesURL, "http://")
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
