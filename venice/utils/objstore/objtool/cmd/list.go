package cmd

import (
	"encoding/json"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/api"
)

var listCmd = &cobra.Command{
	Use:   "list",
	Short: "list objects in bucket",
	Run:   list,
}

func init() {
	rootCmd.AddCommand(listCmd)
}

func list(cmd *cobra.Command, args []string) {
	opts := api.ListWatchOptions{
		ObjectMeta: api.ObjectMeta{
			Tenant:    testTenant,
			Namespace: bucket,
		},
	}
	list, err := restClient.ObjstoreV1().Object().List(authCtx, &opts)
	if err != nil {
		errorExit(err, "failed to list bucket [%v]", bucket)
	}
	buf, err := json.MarshalIndent(list, "", "  ")
	if err != nil {
		errorExit(err, "failed to parse list")
	}
	fmt.Printf("%s\n", buf)
}
