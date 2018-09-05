//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/nmd/protos"
)

var modeManagedCmd = &cobra.Command{
	Use:   "mode",
	Short: "Set Naples to Managed mode",
	Long:  "Set Naples to Venice Managed mode",
	Run:   modeManagedCmdHandler,
	Args:  modeManagedCmdArgsValidator,
}

var modeManagedShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "Show mode of operation of Naples",
	Long:  "Show mode of operation of Naples - host-managed vs venice-managed",
	Run:   modeManagedShowCmdHandler,
}

type clusterAddresses []string

var clusters []string
var mode string
var priMac string
var hostName string
var mgmtIP string

func init() {
	rootCmd.AddCommand(modeManagedCmd)
	modeManagedCmd.AddCommand(modeManagedShowCmd)

	modeManagedCmd.Flags().StringSliceVarP(&clusters, "cluster-ip", "c", make([]string, 0), "List of cluster IP addresses (Required)")
	modeManagedCmd.Flags().StringVarP(&mode, "mode", "m", "managed", "unmanaged vs managed mode")
	modeManagedCmd.Flags().StringVarP(&priMac, "primary-mac", "p", "", "Primary mac (Required)")
	modeManagedCmd.Flags().StringVarP(&hostName, "host-name", "n", "", "Host name (Required)")
	modeManagedCmd.Flags().StringVarP(&mgmtIP, "management-ip", "i", "", "Management IP (Required)")
	modeManagedCmd.MarkFlagRequired("cluster-ip")
	modeManagedCmd.MarkFlagRequired("primary-mac")
	modeManagedCmd.MarkFlagRequired("host-name")
	modeManagedCmd.MarkFlagRequired("management-ip")
}

func modeManagedCmdArgsValidator(cmd *cobra.Command, args []string) error {
	return nil
}

func modeManagedCmdHandler(cmd *cobra.Command, args []string) {
	var modeVal nmd.NaplesMode
	if mode == "managed" {
		modeVal = nmd.NaplesMode_MANAGED_MODE
	} else {
		modeVal = nmd.NaplesMode_CLASSIC_MODE
	}
	naplesCfg := &nmd.Naples{
		TypeMeta:   api.TypeMeta{Kind: "Naples"},
		ObjectMeta: api.ObjectMeta{Name: "NaplesConfig"},
		Spec: nmd.NaplesSpec{
			Mode:           modeVal,
			PrimaryMac:     priMac,
			HostName:       hostName,
			ClusterAddress: clusters,
			MgmtIp:         mgmtIP,
		},
	}

	err := restPost(naplesCfg, "http://192.168.30.12:9008/api/v1/naples/")
	if err != nil {
		if strings.Contains(err.Error(), "EOF") {
			if verbose {
				fmt.Println("success")
			}
		} else {
			if verbose {
				fmt.Println("failed")
			}
		}
	}
}

func modeManagedShowCmdHandler(cmd *cobra.Command, args []string) {
}
