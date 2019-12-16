// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package utils

import (
	"context"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"os/exec"
	"path"
	"runtime"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/venice/cmd/credentials"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	elasticImage        = "docker.elastic.co/elasticsearch/elasticsearch:6.3.2"
	elasticClusterImage = "registry.test.pensando.io:5000/elasticsearch-cluster:v0.13"
	elasticHost         = "127.0.0.1"

	helpMsgOnce sync.Once
)

// CreateElasticClient helper function to create elastic client
func CreateElasticClient(elasticsearchAddr string, resolverClient resolver.Interface, logger log.Logger, signer certs.CSRSigner, trustRoots []*x509.Certificate) (elastic.ESClient, error) {
	esClient, err := createElasticClient(elasticsearchAddr, resolverClient, logger, signer, trustRoots)
	if err != nil {
		return nil, err
	}

	// check cluster health
	if !IsElasticClusterHealthy(esClient) {
		return nil, fmt.Errorf("elastic cluster not healthy")
	}

	return esClient, nil
}

// StartElasticsearch starts elasticsearch service
func StartElasticsearch(name, dir string, signer certs.CSRSigner, trustRoots []*x509.Certificate) (string, string, error) {
	setMaxMapCount()
	var err error

	log.Info("starting elasticsearch ...")

	if utils.IsEmpty(dir) {
		dir, err = ioutil.TempDir("/tmp", fmt.Sprintf("%s-elastic-test", name))
		if err != nil {
			return "", "", fmt.Errorf("failed to create temp dir, err: %v", err)
		}
		os.Chmod(dir, 0777) // override default permissions to allow cleanup
	}

	elasticDir := dir
	logDir := path.Join(dir, "log")
	if err := os.MkdirAll(logDir, 0777); err != nil {
		return "", "", fmt.Errorf("failed to create log dir, err: %v", err)
	}

	var authDir string
	if signer != nil {
		log.Infof("setting up TLS")
		var err error
		authDir, err = ioutil.TempDir(elasticDir, "auth")
		if err != nil {
			os.RemoveAll(elasticDir)
			return "", "", fmt.Errorf("failed to create auth dir, err: %v", err)
		}
		os.Chmod(authDir, 0777)

		if err := credentials.GenElasticHTTPSAuth("localhost", authDir, signer, trustRoots); err != nil {
			os.RemoveAll(elasticDir)
			return "", "", fmt.Errorf("error creating credentials in dir %s: err: %v", authDir, err)
		}
	}

	// same port needs to be exposed outside as inside to make sure underlying sniffer works given that the
	// test is run outside the elasticsearch container.
	for port := 6000; port < 7000; port++ {
		// If we have a CSRSigner we generate credentials and start the Venice-specific container with Elastic + TLS plugin
		// otherwise we just start the stock Elastic container without auth
		var cmd []string
		if signer != nil {
			cmd = []string{
				"run", "--rm", "-d", "-p", fmt.Sprintf("%d:%d", port, port),
				fmt.Sprintf("--name=%s", name),
				"-e", fmt.Sprintf("cluster.name=%s", name),
				"-e", "ES_JAVA_OPTS=-Xms2g -Xmx2g",
				"-e", fmt.Sprintf("http.port=%d", port),
				"-e", fmt.Sprintf("http.publish_host=%s", elasticHost),
				"-v", fmt.Sprintf("%s:/usr/share/elasticsearch/config/auth-node:ro", authDir),
				"-v", fmt.Sprintf("%s:/usr/share/elasticsearch/config/auth-https:ro", authDir),
				"-v", fmt.Sprintf("%s:/usr/share/elasticsearch/data", elasticDir),
				"-v", fmt.Sprintf("%s:/var/log/pensando/elastic", logDir),
				elasticClusterImage}
		} else {
			cmd = []string{
				"run", "--rm", "-d", "-p", fmt.Sprintf("%d:%d", port, port),
				fmt.Sprintf("--name=%s", name),
				"-e", fmt.Sprintf("cluster.name=%s", name),
				"-e", "xpack.security.enabled=false",
				"-e", "xpack.monitoring.enabled=false",
				"-e", "xpack.graph.enabled=false",
				"-e", "xpack.watcher.enabled=false",
				"-e", "xpack.logstash.enabled=false",
				"-e", "xpack.ml.enabled=false",
				"-e", "ES_JAVA_OPTS=-Xms2g -Xmx2g",
				"-e", fmt.Sprintf("http.port=%d", port),
				"-e", fmt.Sprintf("http.publish_host=%s", elasticHost),
				"-v", fmt.Sprintf("%s:/var/log/pensando/elastic", logDir),
				"-v", fmt.Sprintf("%s:/usr/share/elasticsearch/data", elasticDir),
				elasticImage}
		}

		fmt.Println("running command:", cmd)

		// run the command
		out, err := exec.Command("docker", cmd...).CombinedOutput()

		// stop and retry if a container with the same name exists already
		if strings.Contains(string(out), "Conflict") {
			log.Errorf("conflicting names, retrying")
			StopElasticsearch(name, authDir)
			continue
		}

		// retry with a different port
		if strings.Contains(string(out), "port is already allocated") {
			log.Errorf("port(%d) already allocated, retrying", port)
			continue
		}

		if err != nil {
			os.RemoveAll(elasticDir)
			return "", "", fmt.Errorf("%s, err: %v", out, err)
		}

		elasticAddr := fmt.Sprintf("%s:%d", elasticHost, port)
		log.Infof("started elasticsearch: %s", elasticAddr)

		return elasticAddr, elasticDir, nil
	}

	os.RemoveAll(elasticDir)
	return "", "", fmt.Errorf("exhausted all the ports from 6000-6999, failed to start elasticsearch")
}

// StopElasticsearch stops elasticsearch service
func StopElasticsearch(name, authDir string) error {
	if len(strings.TrimSpace(name)) == 0 {
		return nil
	}

	log.Info("stopping elasticsearch ...")

	if authDir != "" {
		defer certs.DeleteTLSCredentials(authDir)
		defer os.RemoveAll(authDir)
	}

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

// IsElasticClusterHealthy checks if the cluster is healthy or not
func IsElasticClusterHealthy(esClient elastic.ESClient) bool {
	healthy, err := esClient.IsClusterHealthy(context.Background())
	if err != nil {
		return false
	}

	return healthy
}

// helper function to create the client
func createElasticClient(elasticsearchAddr string, resolverClient resolver.Interface, logger log.Logger, signer certs.CSRSigner, trustRoots []*x509.Certificate) (elastic.ESClient, error) {
	var err error
	var esClient elastic.ESClient

	opts := []elastic.Option{}
	if signer != nil {
		authDir, err := ioutil.TempDir("/tmp", "elastic-client")
		if err != nil {
			return nil, fmt.Errorf("error creating temp dir for credentials: %v", err)
		}
		defer os.RemoveAll(authDir)
		err = credentials.GenElasticClientsAuth(authDir, signer, trustRoots)
		if err != nil {
			return nil, fmt.Errorf("error generating Elastic client TLS credentials: %v", err)
		}

		tlsConfig, err := certs.LoadTLSCredentials(authDir)
		if err != nil {
			return nil, fmt.Errorf("error accessing client credentials: %v", err)
		}

		tlsConfig.ServerName = globals.ElasticSearch + "-https"
		transport := &http.Transport{TLSClientConfig: tlsConfig}
		opts = append(opts, elastic.WithHTTPClient(&http.Client{Transport: transport}))
	}

	log.Infof("creating elasticsearch client using address: %v", elasticsearchAddr)

	retryInterval := 10 * time.Millisecond
	timeout := 2 * time.Minute
	for {
		select {
		case <-time.After(retryInterval):
			if esClient == nil {
				esClient, err = elastic.NewClient(elasticsearchAddr, resolverClient, logger, opts...)
			}

			// if the client is created, make sure the cluster is healthy
			if esClient != nil {
				log.Infof("created elasticsearch client")
				return esClient, nil
			}

			log.Infof("failed to create elasticsearch client, err: %v, retrying", err)
		case <-time.After(timeout):
			if err != nil {
				return nil, fmt.Errorf("failed to create elasticsearch client, err: %v", err)
			}
			return esClient, nil
		}
	}
}

func setMaxMapCount() {
	// set max_map_count; this is a must requirement to run elasticsearch
	// https://www.elastic.co/guide/en/elasticsearch/reference/current/vm-max-map-count.html
	//
	// this need to be set manually for docker for mac using the below commands:
	// $ screen ~/Library/Containers/com.docker.docker/Data/vms/0/tty
	// $ sysctl -w vm.max_map_count=262144
	//
	helpMsgOnce.Do(func() {
		if runtime.GOOS == "darwin" {
			fmt.Println("++++++ run this one time setup commands from your mac if you haven't done yet +++++++\n" +
				"screen ~/Library/Containers/com.docker.docker/Data/vms/0/tty\n" +
				"on the blank screen, press return and run: sysctl -w vm.max_map_count=262144")
			fmt.Println()
			return
		}

		fmt.Println("++++++ setting vm.max_map_count=262144 +++++++")
		out, err := exec.Command("sysctl", "-w", "vm.max_map_count=262144").CombinedOutput()
		outStr := strings.TrimSpace(string(out))
		if err != nil || outStr != "vm.max_map_count = 262144" {
			fmt.Println(fmt.Sprintf("failed to set max_map_count: %s, err: %v\n", outStr, err) +
				"run the below command manually on your machine if you haven't done yet\n" +
				"sysctl -w vm.max_map_count=262144")
			fmt.Println()
		}
	})
}
