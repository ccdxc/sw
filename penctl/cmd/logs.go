//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/globals"
)

var logsShowCmd = &cobra.Command{
	Use:   "logs",
	Short: "Show logs from Distributed Service Card",
	Long:  "\n------------------------------\n Show Module Logs From Distributed Service Card \n------------------------------\n",
	RunE:  logsShowCmdHandler,
	Args:  logsShowCmdArgsValidator,
}

var module string

func init() {
	showCmd.AddCommand(logsShowCmd)

	str := "\t\t\tValid modules are:\n"
	str += "\t\t\t\t" + strings.TrimPrefix(globals.Nmd, "pen-") + "\n"
	str += "\t\t\t\t" + strings.TrimPrefix(globals.Netagent, "pen-") + "\n"
	str += "\t\t\t\t" + strings.TrimPrefix(globals.Tmagent, "pen-") + "\n"
	str += "\t\t\t\t" + "pciemgrd" + "\n"
	logsShowCmd.Flags().StringVarP(&module, "module", "m", "", "Module to show logs for\n"+str)
	logsShowCmd.MarkFlagRequired("module")
}

func logsShowCmdHandler(cmd *cobra.Command, args []string) error {
	var moduleVal string
	logDir := "monitoring/v1/naples/logs/pensando/"
	switch module {
	case strings.TrimPrefix(globals.Nmd, "pen-"):
		moduleVal = globals.Nmd
	case strings.TrimPrefix(globals.Netagent, "pen-"):
		moduleVal = globals.Netagent
	case strings.TrimPrefix(globals.Tmagent, "pen-"):
		moduleVal = globals.Tmagent
	case "pciemgrd":
		moduleVal = "pciemgrd"
		logDir = "monitoring/v1/naples/obfl/"
	}
	moduleVal += ".log"
	resp, err := restGet(logDir + moduleVal)
	if err != nil {
		return err
	}

	fmt.Println(string(resp))
	return nil
}

func logsShowCmdArgsValidator(cmd *cobra.Command, args []string) error {
	if strings.Compare(module, strings.TrimPrefix(globals.Nmd, "pen-")) != 0 &&
		strings.Compare(module, strings.TrimPrefix(globals.Netagent, "pen-")) != 0 &&
		strings.Compare(module, strings.TrimPrefix(globals.Tmagent, "pen-")) != 0 &&
		strings.Compare(module, "pciemgrd") != 0 {
		if cmd.Flags().Changed("module") {
			str := "Not valid module: " + module + "\n"
			return errors.New(str)
		}
	}
	return nil
}
