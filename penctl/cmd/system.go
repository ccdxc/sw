//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"
	"time"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

var getSysMemCmd = &cobra.Command{
	Use:   "system-memory-usage",
	Short: "Show free/used memory on Naples (in MB)",
	Long:  "\n-----------------------------------------\n Show Free/Used Memory on Naples (in MB)\n-----------------------------------------\n",
	RunE:  getSysMemCmdHandler,
}

var getProcMemInfoCmd = &cobra.Command{
	Use:   "proc-meminfo",
	Short: "Check /proc/meminfo file on Naples",
	Long:  "\n------------------------------------\n Check /proc/meminfo file on Naples \n------------------------------------\n",
	RunE:  getProcMemInfoCmdHandler,
}

var getSystemCmd = &cobra.Command{
	Use:   "system",
	Short: "show system information",
	Long:  "\n------------------------------------\n show system information \n------------------------------------\n",
}

var getSystemStatusCmd = &cobra.Command{
	Use:   "status",
	Short: "Show current system status",
	Long:  "\n------------------------------------\n Show current system status \n------------------------------------\n",
	RunE:  getSystemStatusCmdCmdHandler,
}

func init() {
	showCmd.AddCommand(getSysMemCmd)
	showCmd.AddCommand(getProcMemInfoCmd)
	showCmd.AddCommand(getSystemCmd)
	getSystemCmd.AddCommand(getSystemStatusCmd)

}

func getSysMemCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "free",
		Opts:       strings.Join([]string{"-m"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func getProcMemInfoCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "cat",
		Opts:       strings.Join([]string{"/proc/meminfo"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func getSystemStatusCmdCmdHandler(cmd *cobra.Command, args []string) error {
	resp, err := restGet(revProxyPort, "monitoring/v1/naples/logs/pensando/pciemgrd.log")
	if err != nil {
		return err
	}

	var pciemgrd, port0 string
	var pyr, phr, pmin, psec, pmilsec int
	var eyr, ehr, emin, esec, emilsec int
	var starttime, endtime time.Duration
	for _, line := range strings.Split(string(resp), "\n") {
		if strings.Contains(line, "pciemgrd ready") {
			fmt.Sscanf(line, "[%d-%d:%d:%d.%d]%s", &pyr, &phr, &pmin, &psec, &pmilsec, &pciemgrd)
			starttime = time.Duration(phr)*time.Hour +
				time.Duration(pmin)*time.Minute +
				time.Duration(psec)*time.Second +
				time.Duration(pmilsec)*time.Millisecond
		} else if strings.Contains(line, "port 0 ready") {
			fmt.Sscanf(line, "[%d-%d:%d:%d.%d]%s", &eyr, &ehr, &emin, &esec, &emilsec, &port0)
			endtime = time.Duration(ehr)*time.Hour +
				time.Duration(emin)*time.Minute +
				time.Duration(esec)*time.Second +
				time.Duration(emilsec)*time.Millisecond
		}
	}
	pcietime := endtime - starttime
	fmt.Printf("The PCIE up time duration is %d:%d:%d.%d\n",
		int64(pcietime/time.Hour),
		int64(pcietime/time.Minute),
		int64(pcietime/time.Second),
		int64(pcietime/time.Millisecond))
	return nil
}
