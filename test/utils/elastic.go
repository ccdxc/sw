// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package utils

import (
	"fmt"
	"os/exec"
	"strings"

	"github.com/pensando/sw/venice/utils/log"
)

var (
	registryURL  = "registry.test.pensando.io:5000"
	elasticImage = "elasticsearch/elasticsearch:6.2.2"
	elasticHost  = "127.0.0.1"
)

// elastic util funtions

// StartElasticsearch starts elasticsearch service
func StartElasticsearch(name string) (string, error) {
	log.Info("starting elasticsearch ..")

	// set max_map_count; this is a must requirement to run elasticsearch
	// https://www.elastic.co/guide/en/elasticsearch/reference/current/vm-max-map-count.html
	if out, err := exec.Command("sysctl", "-w", "vm.max_map_count=262144").CombinedOutput(); err != nil {
		log.Errorf("failed to set max_map_count %s", out)
	}

	// same port needs to be exposed outside as inside to make sure underlying sniffer works given that the
	// test is run ousite the elasticsearch container.
	for port := 6000; port < 7000; port++ {
		cmd := []string{
			"run", "--rm", "-d", "-p", fmt.Sprintf("%d:%d", port, port),
			fmt.Sprintf("--name=%s", name),
			"-e", fmt.Sprintf("cluster.name=%s", name),
			"-e", "xpack.security.enabled=false",
			"-e", "xpack.monitoring.enabled=false",
			"-e", "xpack.graph.enabled=false",
			"-e", "xpack.watcher.enabled=false",
			"-e", "xpack.logstash.enabled=false",
			"-e", "xpack.ml.enabled=false",
			"-e", "ES_JAVA_OPTS=-Xms512m -Xmx512m",
			"-e", fmt.Sprintf("http.port=%d", port),
			"-e", fmt.Sprintf("http.publish_host=%s", elasticHost),
			fmt.Sprintf("%s/%s", registryURL, elasticImage)}

		// run the command
		out, err := exec.Command("docker", cmd...).CombinedOutput()

		// stop and retry if a container with the same name exists already
		if strings.Contains(string(out), "Conflict") {
			log.Errorf("conflicting names, retrying")
			StopElasticsearch(name)
			continue
		}

		// retry with a different port
		if strings.Contains(string(out), "port is already allocated") {
			log.Errorf("port already allocated, retrying")
			continue
		}

		if err != nil {
			return "", fmt.Errorf("%s, err: %v", out, err)
		}

		elasticAddr := fmt.Sprintf("%s:%d", elasticHost, port)
		log.Infof("started elasticsearch: %s", elasticAddr)

		return elasticAddr, nil
	}

	return "", fmt.Errorf("exhausted all the ports from 6000-6999, failed to start elasticsearch")
}

// StopElasticsearch stops elasticsearch service
func StopElasticsearch(name string) error {
	if len(strings.TrimSpace(name)) == 0 {
		return nil
	}

	log.Info("stopping elasticsearch ..")

	cmd := []string{"rm", "-f", name}

	// run the command
	out, err := exec.Command("docker", cmd...).CombinedOutput()

	if err != nil && !strings.Contains(string(out), "No such container") {
		log.Infof("docker run cmd failed, err: %+v", err)
		return fmt.Errorf("%s, err: %v", out, err)
	}

	return err
}

// GetElasticsearchAddress returns the address of elasticsearch server
func GetElasticsearchAddress(name string) (string, error) {
	if len(strings.TrimSpace(name)) == 0 {
		return "", nil
	}

	cmd := []string{"inspect", "-f", "{{range $p, $conf := .HostConfig.PortBindings}}{{range $conf}}{{println .HostPort}}{{end}}{{end}}", name}
	ports, err := exec.Command("docker", cmd...).CombinedOutput()
	if err != nil {
		return "", err
	}

	if len(strings.TrimSpace(string(ports))) == 0 {
		return "", fmt.Errorf("no ports exposed")
	}

	// it takes the first exposed port
	port := strings.Split(string(ports), "\n")[0]
	addr := fmt.Sprintf("%s:%s", elasticHost, strings.TrimSpace(port))

	log.Infof("elasticsearch address: %v", addr)

	return addr, nil
}
