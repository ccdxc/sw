//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

// +build linux freebsd

package cmd

import (
	"fmt"

	"github.com/spf13/cobra"
)

func getNaplesInfo(cmd *cobra.Command, args []string) error {
	//// TODO: Return all multiple nic cards
	fmt.Println("Not implemented for linux platform")
	return nil
}
