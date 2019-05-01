package imagestore

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"os"

	"github.com/pensando/sw/venice/utils/resolver"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/rpckit"
)

func downloadMetaFile(ctx context.Context, resolver resolver.Interface, bundleVersion string) error {
	bucket := "images"
	//name := version + ".img/venice"
	//out := version + ".img"
	name := "Bundle/" + bundleVersion + "_img/metadata.json"
	out := "/tmp/metadata.json"

	log.Infof("downloadMetaFile Version [%s] metadata [%s]", bundleVersion, name)
	if resolver == nil {
		log.Errorf("Resolver cannot be null")
		return fmt.Errorf("Resolver cannot be null")
	}
	if bundleVersion == "" {
		log.Errorf("Version is needed to download a naples image from objstore")
		return fmt.Errorf("Version is needed to download a naples image from objstore")
	}

	tlsp, err := rpckit.GetDefaultTLSProvider(globals.Vos)
	if err != nil {
		log.Errorf("Error getting tls provider (%s)", err)
		return fmt.Errorf("Error getting tls provider (%s)", err)
	}

	tlsc, err := tlsp.GetClientTLSConfig(globals.Vos)
	if err != nil {
		log.Errorf("Error getting tls client (%s)", err)
		return fmt.Errorf("Error getting tls client (%s)", err)
	}
	tlsc.ServerName = globals.Vos

	client, err := objstore.NewClient("default", bucket, resolver, objstore.WithTLSConfig(tlsc))
	if err != nil {
		log.Errorf("Could not create client (%s)", err)
		return fmt.Errorf("Could not create client (%s)", err)
	}
	fr, err := client.GetObject(ctx, name)
	if err != nil {
		log.Errorf("Could not get object (%s)", err)
		return fmt.Errorf("Could not get object (%s)", err)
	}

	of, err := os.Create(out)
	if err != nil {
		log.Errorf("Could not create output file [%s](%s)", out, err)
		return fmt.Errorf("Could not create output file [%s](%s)", out, err)
	}
	defer of.Close()
	buf := make([]byte, 1024)
	totsize := 0
	for {
		n, err := fr.Read(buf)
		if err != nil && err != io.EOF {
			log.Errorf("Error while reading object (%s)", err)
			return fmt.Errorf("Error while reading object (%s)", err)
		}
		if n == 0 {
			break
		}
		totsize += n
		if _, err = of.Write(buf[:n]); err != nil {
			log.Errorf("Error writing to output file (%s)", err)
			return fmt.Errorf("Error writing to output file (%s)", err)
		}
	}
	return nil
}

// GetNaplesRolloutVersion gets Venice image version for rollout
func GetNaplesRolloutVersion(ctx context.Context, resolver resolver.Interface, bundleVersion string) (string, error) {

	out := "/tmp/metadata.json"
	err := downloadMetaFile(ctx, resolver, bundleVersion)
	if err != nil {
		return "", err
	}
	versionMap := processMetadataFile(out)
	log.Infof("VersionMap [%v] Naples version [%v]", versionMap, versionMap["Naples"]["Version"])
	return versionMap["Naples"]["Version"], nil
}

// GetVeniceRolloutVersion gets Venice image version for rollout
func GetVeniceRolloutVersion(ctx context.Context, resolver resolver.Interface, bundleVersion string) (string, error) {

	out := "/tmp/metadata.json"
	err := downloadMetaFile(ctx, resolver, bundleVersion)
	if err != nil {
		return "", err
	}
	versionMap := processMetadataFile(out)
	log.Infof("VersionMap [%v] Venice version [%v]", versionMap, versionMap["Venice"]["Version"])
	return versionMap["Venice"]["Version"], nil
}

func processMetadataFile(metadata string) map[string]map[string]string {
	versionMap := make(map[string]map[string]string)

	if _, err := os.Stat(metadata); err != nil {
		// Stat error is treated as not part of cluster.
		log.Errorf("unable to find confFile %s error: %v", metadata, err)
		return nil
	}
	var in []byte
	var err error
	if in, err = ioutil.ReadFile(metadata); err != nil {
		log.Errorf("unable to read confFile %s error: %v", metadata, err)
		return nil
	}
	if err := json.Unmarshal(in, &versionMap); err != nil {
		log.Errorf("unable to understand confFile %s error: %v", metadata, err)
		return nil
	}

	return versionMap
}
