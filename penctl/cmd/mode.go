//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
)

var modeManagedCmd = &cobra.Command{
	Use:   "mode",
	Short: "Set Naples to Managed mode",
	Long:  "\n----------------------------\n Set NAPLES management mode \n----------------------------\n",
	RunE:  modeManagedCmdHandler,
	Args:  modeManagedCmdArgsValidator,
}

var modeManagedShowCmd = &cobra.Command{
	Use:    "mode",
	Short:  "Show mode of operation of Naples",
	Long:   "\n-------------------------------------------------------------------\n Show mode of operation of Naples - host-managed vs venice-managed \n-------------------------------------------------------------------\n",
	RunE:   modeManagedShowCmdHandler,
	Hidden: true,
}

type clusterAddresses []string

var controllers []string
var mode string
var priMac string
var hostname string
var mgmtIP string
var defaultGW string
var dnsServers []string

func init() {
	updateCmd.AddCommand(modeManagedCmd)
	showCmd.AddCommand(modeManagedShowCmd)

	modeManagedCmd.Flags().StringSliceVarP(&controllers, "controllers", "c", make([]string, 0), "List of controller IP addresses or hostnames")
	modeManagedCmd.Flags().StringVarP(&mode, "operational-mode", "o", "network", "host or network managed")
	modeManagedCmd.Flags().StringVarP(&priMac, "primary-mac", "p", "", "Primary mac")
	modeManagedCmd.Flags().StringVarP(&hostname, "hostname", "n", "", "Host name")
	modeManagedCmd.Flags().StringVarP(&mgmtIP, "mgmt-ip", "m", "", "Management IP in CIDR format")
	modeManagedCmd.Flags().StringVarP(&defaultGW, "default-gw", "g", "", "Default GW for mgmt")
	modeManagedCmd.Flags().StringSliceVarP(&dnsServers, "dns-servers", "d", make([]string, 0), "List of DNS servers")
}

func modeManagedCmdArgsValidator(cmd *cobra.Command, args []string) error {
	for _, cluster := range controllers {
		ep := strings.Split(cluster, ":")
		if vldtor.HostAddr(ep[0]) != true {
			str := "Not valid hostaddr: " + ep[0]
			return errors.New(str)
		}
		if _, err := strconv.Atoi(ep[1]); err != nil {
			str := "Not valid port number: " + ep[1]
			return errors.New(str)
		}
	}
	if strings.Compare(mode, "network") != 0 && strings.Compare(mode, "host") != 0 {
		str := "Not valid mode: " + mode
		return errors.New(str)
	}
	if !vldtor.CIDR(mgmtIP) {
		return errors.New("Not valid mgmt-ip")
	}
	if !vldtor.HostAddr(hostname) {
		return errors.New("Not valid host name")
	}
	if !vldtor.MacAddr(priMac) {
		return errors.New("Not valid mac address")
	}
	return nil
}

func modeManagedCmdHandler(cmd *cobra.Command, args []string) error {
	var modeVal nmd.MgmtMode
	if mode == "managed" {
		modeVal = nmd.MgmtMode_NETWORK
	} else {
		modeVal = nmd.MgmtMode_HOST
	}
	naplesCfg := &nmd.Naples{
		TypeMeta:   api.TypeMeta{Kind: "Naples"},
		ObjectMeta: api.ObjectMeta{Name: "NaplesConfig"},
		Spec: nmd.NaplesSpec{
			Mode:        modeVal,
			Hostname:    hostname,
			Controllers: controllers,
			IPConfig: &cluster.IPConfig{
				IPAddress:  mgmtIP,
				DefaultGW:  defaultGW,
				DNSServers: dnsServers,
			},
			PrimaryMAC: priMac,
		},
	}

	err := restPost(naplesCfg, revProxyPort, "api/v1/naples/")
	if err != nil {
		if strings.Contains(err.Error(), "EOF") {
			if verbose {
				fmt.Println("success")
			}
		} else {
			fmt.Println("Unable to set mode.")
			fmt.Println("Error:", err.Error())
		}
		return err
	}
	return nil
}

func modeManagedShowCmdHandler(cmd *cobra.Command, args []string) error {
	resp, err := restGet(revProxyPort, "api/v1/naples/")

	if err != nil {
		if verbose {
			fmt.Println(err.Error())
		}
		return err
	}
	naplesCfg := nmd.Naples{}
	json.Unmarshal(resp, &naplesCfg)
	if tabularFormat {
		fmt.Println(strings.Trim(strings.Replace(fmt.Sprintf("%+v", naplesCfg.Spec), " ", "\n", -1), "{}"))
	}
	if verbose && tabularFormat {
		fmt.Printf("%+v\n", naplesCfg.Spec)
	}
	return nil
}
