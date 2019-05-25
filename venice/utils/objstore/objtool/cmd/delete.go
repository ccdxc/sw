package cmd

import (
	"github.com/spf13/cobra"

	"github.com/pensando/sw/api"
)

var deleteCmd = &cobra.Command{
	Use:   "delete",
	Short: "delete object from bucket",
	Run:   delete,
}

func init() {
	rootCmd.AddCommand(deleteCmd)
}

func delete(cmd *cobra.Command, args []string) {

	if len(args) != 1 {
		errorExit(nil, "need one parameter : <filename>")
	}
	objMeta := api.ObjectMeta{
		Tenant:    testTenant,
		Namespace: bucket,
		Name:      args[0],
	}

	_, err := restClient.ObjstoreV1().Object().Delete(authCtx, &objMeta)
	if err != nil {
		errorExit(err, "failed to delete object bucket [%v]", bucket)
	}
	if err != nil {
		errorExit(err, "failed to delete file [%s]", args[0])
	}
}
