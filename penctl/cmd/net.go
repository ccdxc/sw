//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/apigen/validators"
)

var naplesIP string
var naplesSSHIP string
var revProxyPort string

func pickNetwork(cmd *cobra.Command, args []string) error {
	naplesIP = "127.0.0.1"
	naplesSSHIP = "192.168.68.155"
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
	return "penctltestuser"
}

//TODO: Fix the password
func getNaplesPwd() string {
	return "Pen%Ctl%Test%Pwd"
}
