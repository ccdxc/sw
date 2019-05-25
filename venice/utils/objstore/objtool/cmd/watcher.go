package cmd

import (
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/api"
)

var watchCmd = &cobra.Command{
	Use:   "watch",
	Short: "watch bucket for updates",
	Run:   watch,
}

func init() {
	rootCmd.AddCommand(watchCmd)
}

func watch(cmd *cobra.Command, args []string) {
	opts := api.ListWatchOptions{
		ObjectMeta: api.ObjectMeta{
			Tenant:    testTenant,
			Namespace: bucket,
		},
	}
	watcher, err := restClient.ObjstoreV1().Object().Watch(authCtx, &opts)
	if err != nil {
		errorExit(err, "failed to create watcher")
	}
	fmt.Printf("Starting to watch\n")
	for evt := range watcher.EventChan() {
		fmt.Printf("received event [%+v]\n", evt)
	}
}
