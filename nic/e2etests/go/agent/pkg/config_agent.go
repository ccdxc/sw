package pkg

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"time"

	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
)

func ConfigAgent(c *Config) error {
	for _, o := range c.Objects {
		err := o.ConfigureObjects()
		if err != nil {
			return err
		}
	}
	return nil
}

func (o *Object) ConfigureObjects() error {
	var resp restapi.Response
	restURL := fmt.Sprintf("%s%s", AGENT_URL, o.RestEndpoint)
	specFile := fmt.Sprintf("%s%s", HEIMDALL_CONFIG_DIR, o.SpecFile)

	dat, err := ioutil.ReadFile(specFile)
	if err != nil {
		return err
	}

	switch o.Kind {
	case "Namespace":
		var namespaces []netproto.Namespace
		err = json.Unmarshal(dat, &namespaces)
		if err != nil {
			return err
		}

		fmt.Printf("Creating %d Namespaces...\n", o.Count)
		for _, ns := range namespaces {
			err := netutils.HTTPPost(restURL, &ns, &resp)
			if err != nil {
				o.printErr(err, resp)
				return err
			}
			time.Sleep(time.Millisecond)
		}
	case "Network":
		var networks []netproto.Network
		err = json.Unmarshal(dat, &networks)
		if err != nil {
			return err
		}

		fmt.Printf("Creating %d Networks...\n", o.Count)
		for _, nt := range networks {
			err := netutils.HTTPPost(restURL, nt, &resp)
			if err != nil {
				o.printErr(err, resp)
				return err
			}
			time.Sleep(time.Millisecond)
		}
	case "Endpoint":
		var endpoints []netproto.Endpoint
		err = json.Unmarshal(dat, &endpoints)
		if err != nil {
			return err
		}

		fmt.Printf("Creating %d Endpoints...\n", o.Count)
		for _, ep := range endpoints {
			err := netutils.HTTPPost(restURL, ep, &resp)
			if err != nil {
				o.printErr(err, resp)
				return err
			}
			time.Sleep(time.Millisecond)
		}
	case "SGPolicy":
		var sgPolicies []netproto.SGPolicy
		err = json.Unmarshal(dat, &sgPolicies)
		if err != nil {
			return err
		}

		fmt.Printf("Configuring %d SGPolicies...\n", o.Count)
		for _, ep := range sgPolicies {
			err := netutils.HTTPPost(restURL, ep, &resp)
			if err != nil {
				o.printErr(err, resp)
				return err
			}
			time.Sleep(time.Millisecond)
		}
	}

	return nil
}

func (o *Object) printErr(err error, resp restapi.Response) {
	fmt.Printf("Agent configuration failed with. Err: %v\n", err)
	fmt.Println("######### RESPONSE #########")
	b, _ := json.MarshalIndent(resp, "", "   ")
	fmt.Println(string(b))
}
