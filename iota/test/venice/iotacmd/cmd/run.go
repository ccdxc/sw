package cmd

import (
	"os"

	"github.com/spf13/cobra"
)

func init() {
	rootCmd.AddCommand(runCommand)
	runCommand.PersistentFlags().BoolVar(&skipSetup, "skip-setup", false, "skips setup")
	runCommand.PersistentFlags().BoolVar(&skipInstall, "skip-install", false, "skips naples install")
	runCommand.PersistentFlags().BoolVar(&stopOnError, "stop-on-error", false, "stops on error")
	runCommand.PersistentFlags().StringVar(&suite, "suite", "", "suite to run")
	runCommand.PersistentFlags().StringVar(&focus, "focus", "", "Focus test")
}

var runCommand = &cobra.Command{
	Use:   "run",
	Short: "Run Tests from specific suite",
	Run:   runCommandAction,
}

func runCommandAction(cmd *cobra.Command, args []string) {

	os.Setenv("VENICE_DEV", "1")
	os.Setenv("JOB_ID", "1")

	st := testsuite{name: suite, path: suiteDirectory + "/" + suite, focus: focus,
		scaleData: scale}

	err := st.run(skipSetup, skipInstall, false, false, "")
	if err != nil {
		errorExit("Error running suite", nil)
	}

}
