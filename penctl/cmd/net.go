//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/utils/apigen/validators"
)

var naplesIP string

func pickNetwork(cmd *cobra.Command, args []string) error {
	naplesIP = "192.168.30.12"
	if !impl.IPAddr(naplesIP) {
		panic(errors.New("Not valid Naples IP"))
	}
	if verbose {
		fmt.Println("TODO: Namespace stuff here")
	}
	return nil
}
