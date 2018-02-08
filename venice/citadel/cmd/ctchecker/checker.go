// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"strings"
	"time"

	client "github.com/influxdata/influxdb/client/v2"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// createDatabase creates a database
func createDatabase(url, database string) error {
	var err error
	hostURLs := strings.Split(url, ",")
	for _, hurl := range hostURLs {
		var resp string
		requrl := hurl + fmt.Sprintf("/create?db=%s", database)
		err = netutils.HTTPPost(requrl, "", &resp)
		if err == nil {
			return nil
		}
	}

	return err
}

// writePoints writes points to citadel using inflxdb client
func writePoints(url string, bp client.BatchPoints) error {
	var err error
	hostURLs := strings.Split(url, ",")
	for _, hurl := range hostURLs {
		// Create a new HTTPClient
		c, cerr := client.NewHTTPClient(client.HTTPConfig{
			Addr: hurl,
		})
		if cerr != nil {
			log.Fatal(cerr)
		}
		defer c.Close()

		log.Debugf("Writing points: %+v", bp.Points())

		// Write the batch
		err = c.Write(bp)
		if err == nil {
			return nil
		}

		// retry next broker after a small delay
		time.Sleep(time.Millisecond * 100)
	}

	return err
}

// queryPoints queries citadel using influxdb client
func queryPoints(url, cmd string) (*client.Response, error) {
	var err error
	var response *client.Response
	hostURLs := strings.Split(url, ",")
	for _, hurl := range hostURLs {
		// Create a new HTTPClient
		c, cerr := client.NewHTTPClient(client.HTTPConfig{
			Addr: hurl,
		})
		if cerr != nil {
			log.Fatal(cerr)
		}
		defer c.Close()

		q := client.NewQuery(cmd, "default", "s")
		response, err = c.Query(q)
		if err == nil && response.Error() == nil {
			return response, nil
		}

		// retry next broker after a small delay
		time.Sleep(time.Millisecond * 100)
	}

	return response, err
}

// writeKv writes key-value pair to citadel
func writeKv(url, table, key, val string) error {
	var err error
	hostURLs := strings.Split(url, ",")
	for _, hurl := range hostURLs {
		var resp string
		requrl := hurl + fmt.Sprintf("/kv/put?table=%s", table)
		req := fmt.Sprintf("%s=%s", key, val)
		err = netutils.HTTPPost(requrl, req, &resp)
		if err == nil {
			return nil
		}
	}

	return err
}

func listKv(url, table string) ([]map[string]string, error) {
	var err error
	var kvlist []map[string]string
	hostURLs := strings.Split(url, ",")
	for _, hurl := range hostURLs {
		requrl := hurl + fmt.Sprintf("/kv/list?table=%s", table)
		err = netutils.HTTPGet(requrl, &kvlist)
		if err == nil {
			return kvlist, nil
		}
	}

	return kvlist, err
}

// writeTstore writes points to citadel at a specified rate
func writeTstore(url, table string, count, rate, series int) {
	fmt.Printf("Executing test writeTstore\n")
	delay := time.Second / time.Duration(rate)

	// create db
	err := createDatabase(url, "default")
	if err != nil {
		log.Fatalf("Error creating database. Err: %v", err)
	}

	// write points
	for iter := 0; iter < count; iter++ {
		select {
		case <-time.After(delay):
			// Create a new point batch
			bp, err := client.NewBatchPoints(client.BatchPointsConfig{
				Database:  "default",
				Precision: "s",
			})
			if err != nil {
				log.Fatal(err)
			}

			// Create a point and add to batch
			tags := map[string]string{
				"key1": fmt.Sprintf("key1-%d", iter),
				"key2": fmt.Sprintf("key2-%d", iter),
			}
			fields := map[string]interface{}{
				"value1": fmt.Sprintf("value1-%d", iter),
				"value2": fmt.Sprintf("value2-%d", iter),
			}

			pt, err := client.NewPoint(table, tags, fields, time.Now())
			if err != nil {
				log.Fatal(err)
			}
			bp.AddPoint(pt)

			err = writePoints(url, bp)
			if err != nil {
				log.Fatalf("Error writing points. Err: %v", err)
			}
		}
	}
}

func writeKstore(url, table string, count, rate int) {
	fmt.Printf("Executing test writeKstore\n")
	delay := time.Second / time.Duration(rate)

	for iter := 0; iter < count; iter++ {
		select {
		case <-time.After(delay):
			// Create a kv-pair
			err := writeKv(url, table, fmt.Sprintf("key1-%d", iter), fmt.Sprintf("value1-%d", iter))
			if err != nil {
				log.Fatalf("Error writing key-value pairs. Err: %v", err)
			}
		}
	}
}
func checkTstore(url, table string, count, series int) {
	fmt.Printf("Executing test checkTstore\n")

	resp, err := queryPoints(url, fmt.Sprintf("SELECT * from %s", table))
	if err != nil {
		log.Fatalf("Error executing query %s", err)
	}

	log.Debugf("Got response: %+v", resp)

	// check the result was what we expected
	if len(resp.Results) != 1 || len(resp.Results[0].Series) != 1 {
		log.Fatalf("Got invalid response: %+v\n", resp)
	}
	if len(resp.Results[0].Series[0].Columns) != 5 {
		log.Fatalf("Invalid number of columns %v", resp.Results[0].Series[0].Columns)
	}
	if len(resp.Results[0].Series[0].Values) != count {
		log.Fatalf("Got invalid number of values %+v", resp.Results[0].Series[0].Values)
	}

	fmt.Printf("\nSUCCESS\n")
}

func checkKstore(url, table string, count, series int) {
	fmt.Printf("Executing test checkKstore\n")

	// get from citadel
	resp, err := listKv(url, table)
	if err != nil {
		log.Fatalf("Error executing list kv %s", err)
	}

	log.Debugf("Got response: %+v", resp)

	// check the response
	if len(resp) != count {
		log.Fatalf("Got invalid response: %+v", resp)
	}

	expResp := make(map[string]string)
	for i := 0; i < count; i++ {
		expResp[fmt.Sprintf("key1-%d", i)] = fmt.Sprintf("value1-%d", i)
	}

	for i := 0; i < count; i++ {
		r := resp[i]
		key, ok := r["Key"]
		if !ok {
			log.Fatalf("Invalid key-value pair %+v", r)
		}
		val, ok := r["Value"]
		if !ok {
			log.Fatalf("Invalid key-value pair %+v", r)
		}

		exp, ok := expResp[key]
		if !ok || val != exp {
			log.Fatalf("Invalid key-value pair %+v", r)
		}
	}

	fmt.Printf("\nSUCCESS\n")
}

func main() {
	// command line flags
	var (
		kstore = flag.Bool("kstore", false, "Test kstore")
		tstore = flag.Bool("tstore", false, "Test tstore")
		write  = flag.Bool("write", false, "Write data")
		check  = flag.Bool("check", false, "Check data")
		url    = flag.String("url", "http://localhost:"+globals.CitadelHTTPPort, "Comma seperated list of urls")
		table  = flag.String("table", "test", "Table to write")
		count  = flag.Int("count", 1, "Number of values to write")
		rate   = flag.Int("rate", 1, "Rate(values per second) at which to write")
		series = flag.Int("series", 1, "Number of series to write")
	)
	flag.Parse()

	// check the arguments
	if !*kstore && !*tstore {
		log.Fatalf("Invalid arguments. -kstore or -tstore option is required")
	}
	if !*write && !*check {
		log.Fatalf("Invalid arguments: -write or -check option is required")
	}

	if *kstore {
		if *write {
			writeKstore(*url, *table, *count, *rate)
		} else if *check {
			checkKstore(*url, *table, *count, *rate)
		}
	} else if *tstore {
		if *write {
			writeTstore(*url, *table, *count, *rate, *series)
		} else if *check {
			checkTstore(*url, *table, *count, *series)
		}
	}
}
