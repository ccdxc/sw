package cmd

import (
	"fmt"
	"os"
	"text/tabwriter"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/utils/netutils"
)

var clusterCmd = &cobra.Command{
	Use:   "cluster",
	Short: "cluster details",
	Long:  `citadel cluster shard/replica details`,
	Run:   clusterCmdHandler,
}

func clusterCmdHandler(cmd *cobra.Command, args []string) {
	resp := map[string]*meta.TscaleCluster{}

	requrl := addr + "/info"
	err := netutils.HTTPGet(requrl, &resp)
	if err != nil {
		fmt.Println(err)
		return
	}

	w := new(tabwriter.Writer)
	// Format in tab-separated columns with a tab stop of 8.
	w.Init(os.Stdout, 0, 8, 1, '\t', 0)

	ctype := "tstore"
	if cl, ok := resp[ctype]; ok {
		// nodemap
		fmt.Fprintf(w, "%s-node\tstatus\tURL\tshards\tprimary-replicas\n", ctype)
		for node, st := range cl.NodeMap {
			fmt.Fprintf(w, "%v\t%v\t%v\t%v\t%v\n", node, st.NodeStatus, st.NodeURL, st.NumShards, st.NumPrimaryReplicas)
		}
		fmt.Fprintln(w)

		// shards
		fmt.Fprintf(w, "shardID\treplicaID\tstate\tnode\t\n")
		primary := func(primary bool) string {
			if primary {
				return "(P)"
			}
			return ""
		}
		for _, shard := range cl.ShardMap.Shards {
			for _, rp := range shard.Replicas {
				fmt.Fprintf(w, "%v\t%v%s\t%v\t%v\t\n", rp.ShardID, rp.ReplicaID, primary(rp.IsPrimary), rp.State, rp.NodeUUID)
			}
		}
		fmt.Fprintln(w)
	}

	w.Flush()

}

func init() {
	rootCmd.AddCommand(clusterCmd)
}
