package cmd

import (
	"log"
	"strings"
	"time"

	"github.com/influxdata/influxdb/client/v2"
)

// QueryPoints queries citadel using influxdb client
func QueryPoints(url, cmd string) (*client.Response, error) {
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
