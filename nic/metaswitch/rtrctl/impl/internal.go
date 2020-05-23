//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package impl

import (
	"github.com/spf13/cobra"
)

var internalShowCmd = &cobra.Command{
	Use:    "internal",
	Short:  "show internal related information",
	Long:   "show internal related information",
	Args:   cobra.NoArgs,
	Hidden: true,
}

var internalEvpnShowCmd = &cobra.Command{
	Use:    "internal",
	Short:  "show internal related information",
	Long:   "show internal related information",
	Args:   cobra.NoArgs,
	Hidden: true,
}

func init() {

}
