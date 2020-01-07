//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"
	"time"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

var getSysMemCmd = &cobra.Command{
	Use:   "system-memory-usage",
	Short: "Show free/used memory on Distributed Service Card (in MB)",
	Long:  "\n-----------------------------------------\n Show Free/Used Memory on Distributed Service Card (in MB)\n-----------------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  getSysMemCmdHandler,
}

var getProcMemInfoCmd = &cobra.Command{
	Use:   "proc-meminfo",
	Short: "Check /proc/meminfo file on Distributed Service Card",
	Long:  "\n------------------------------------\n Check /proc/meminfo file on Distributed Service Card \n------------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  getProcMemInfoCmdHandler,
}

var getSystemCmd = &cobra.Command{
	Use:   "system",
	Short: "show system information",
	Long:  "\n------------------------------------\n show system information \n------------------------------------\n",
	Args:  cobra.NoArgs,
}

var getSystemStatusCmd = &cobra.Command{
	Use:   "status",
	Short: "show current system status",
	Long:  "\n------------------------------------\n show current system status \n------------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  getSystemStatusCmdCmdHandler,
}

var getSystemQueueStatsCmd = &cobra.Command{
	Use:   "queue-statistics",
	Short: "show system queue-statistics",
	Long:  "\n------------------------------------\n show system queue-statistics \n------------------------------------\n",
	Args:  cobra.NoArgs,
	Run:   getSystemQueueStatsCmdHandler,
}

func init() {
	showCmd.AddCommand(getSysMemCmd)
	showCmd.AddCommand(getProcMemInfoCmd)
	showCmd.AddCommand(getSystemCmd)
	getSystemCmd.AddCommand(getSystemStatusCmd)
	getSystemCmd.AddCommand(getSystemQueueStatsCmd)
}

func getSysMemCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "getsysmem",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

func getProcMemInfoCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "getprocmeminfo",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

func getSystemStatusCmdCmdHandler(cmd *cobra.Command, args []string) error {
	resp, err := restGet("monitoring/v1/naples/obfl/pciemgrd.log")
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

func getSystemQueueStatsCmdHandler(cmd *cobra.Command, args []string) {
	//halctlStr := "halctl show system queue-statistics "
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowsystemqueuestats",
		Opts:       "",
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}

	if len(resp) > 3 {
		s := strings.Replace(string(resp), `\n`, "\n", -1)
		fmt.Println(s)
	}

	return
}
