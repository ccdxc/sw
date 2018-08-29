package cmd

import (
	"os"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/utils/log"
)

var tsShowCmd = &cobra.Command{
	Use:   "techsupport",
	Short: "Collect technical support information",
	Long:  "Collect technical support information",
	Run:   tsShowCmdHandler,
}

// default location of the output file
var (
	outDir  = "/tmp/hal_techsupport"
	outFile = "hal-cmds.txt"
)

func init() {
	showCmd.AddCommand(tsShowCmd)
	tsShowCmd.Flags().StringVar(&outDir, "out", outDir, "output directory")
}

func tsShowCmdHandler(cmd *cobra.Command, args []string) {
	outFile = outDir + "/" + outFile

	// remove the output directory if one exists
	err := os.RemoveAll(outFile)
	if err != nil {
		log.Fatalf("Failed to remove directory %s, err : %v", outFile, err)
	}

	// create a new directory
	err = os.MkdirAll(outDir, 0777)
	if err != nil {
		log.Fatalf("Failed to create directory %s, err : %v", outDir, err)
	}

	// create the o/p file
	ofile, err := os.OpenFile(outFile, os.O_CREATE|os.O_RDWR|os.O_APPEND, 0660)
	if err != nil {
		log.Fatalf("Failed to create output file %s, err : %v", outFile, err)
	}

	ofile.WriteString("Capturing techsupport informaiton\n\n")
	ofile.WriteString("Lif information\n\n")
	handlelifDetailShowCmd(nil, ofile)
	ofile.WriteString("Interface information\n\n")
	handleIfDetailShowCmd(nil, ofile)
	ofile.WriteString("Security profile information\n\n")
	handleNwsecProfShowCmd(nil, ofile)
	ofile.WriteString("VRF information\n\n")
	handleVrfDetailShowCmd(nil, ofile)
	ofile.WriteString("L2 segment information\n\n")
	handlel2segDetailShowCmd(nil, ofile)
	ofile.WriteString("Network information\n\n")
	handleNwDetailShowCmd(nil, ofile)
	ofile.WriteString("Security policy information\n\n")
	handleNwsecPolicyShowCmd(nil, ofile)
	ofile.WriteString("Endpoint information\n\n")
	handleEpDetailShowCmd(nil, ofile)
	ofile.WriteString("Session information\n\n")
	handleSessionDetailShowCmd(nil, ofile)

	ofile.Close()
}
