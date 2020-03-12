package cmd

import (
	"fmt"
	"os"
	"sort"
	"strings"
	"text/tabwriter"

	cq "github.com/pensando/sw/venice/citadel/broker/continuous_query"

	"github.com/influxdata/influxdb/query"
	"github.com/spf13/cobra"

	cmd2 "github.com/pensando/sw/venice/citadel/cmd/ctchecker/cmd"
	"github.com/pensando/sw/venice/utils/netutils"
)

var measurementCmd = &cobra.Command{
	Use:   "measurement [name]",
	Short: "list  measurements in citadel",
	Long:  `list all measurements in citadel`,
	Run:   measurementCmdHandler,
}

func showMeasurements() {
	// select * from <measurement>
	w := new(tabwriter.Writer)
	// Format in tab-separated columns with a tab stop of 8.
	w.Init(os.Stdout, 0, 8, 1, '\t', 0)

	requrl := addr + "/cmd?db=" + database + "&q=show+measurements"

	var resp struct {
		Results []*query.Result `json:"results,omitempty"`
		Err     string          `json:"error,omitempty"`
	}

	err := netutils.HTTPGet(requrl, &resp)
	if err != nil {
		fmt.Println(err)
		return
	}

	tbls := []string{}

	for _, res := range resp.Results {
		// series
		for _, s := range res.Series {
			for _, vl := range s.Values {
				for _, v := range vl {
					tbls = append(tbls, v.(string))
				}
			}
		}
	}
	sort.Strings(tbls)
	for i, m := range tbls {
		fmt.Fprintf(w, "%-3d %v\n", i+1, m)
	}
	w.Flush()
}

func measurementCmdHandler(cmd *cobra.Command, args []string) {
	// show measurements
	if len(args) < 1 {
		showMeasurements()
		return
	}

	// select * from <measurement>
	w := new(tabwriter.Writer)
	// Format in tab-separated columns with a tab stop of 8.
	w.Init(os.Stdout, 0, 8, 1, '\t', 0)

	for _, m := range args {
		// check continuous query
		if cq.IsContinuousQueryMeasurement(m) {
			rp := cq.RetentionPolicyMap[strings.Split(m, "_")[1]].Name
			m = `"default"."` + rp + `"."` + m + `"`
		}

		resp, err := cmd2.QueryPoints(addr, fmt.Sprintf("SELECT * FROM %s ORDER BY time DESC", m))
		if err != nil {
			fmt.Println(err)
			return
		}

		if resp.Err != "" {
			fmt.Println(resp.Err)
			return
		}

		for _, res := range resp.Results {
			// series
			for _, s := range res.Series {
				fmt.Fprintf(w, "name: %s \n", s.Name)
				for _, c := range s.Columns {
					fmt.Fprintf(w, "%v\t", c)
				}
				fmt.Fprintln(w)

				for _, vl := range s.Values {
					for _, v := range vl {
						if v != nil {
							fmt.Fprintf(w, "%v\t", v)
						} else {
							fmt.Fprintf(w, "-\t")
						}
					}
					fmt.Fprintln(w)
				}
				fmt.Fprintln(w)
			}
		}
	}
	w.Flush()
}

var database string

func init() {
	rootCmd.AddCommand(measurementCmd)
	measurementCmd.PersistentFlags().StringVarP(&database, "database name", "d", "default", "database name to query")
}
