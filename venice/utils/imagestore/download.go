package imagestore

import (
	"context"
	"fmt"
	"io"
	"os"

	"github.com/pensando/sw/venice/utils/resolver"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	imageName = "venice.tgz"
)

// DownloadNaplesImage downloads naples image from minio
func DownloadNaplesImage(ctx context.Context, resolver resolver.Interface, version string) error {
	return downloadImage(ctx, resolver, version, "naples")
}

// DownloadVeniceImage downloads a venice image from minio
func DownloadVeniceImage(ctx context.Context, resolver resolver.Interface, version string) error {
	return downloadImage(ctx, resolver, version, "venice")
}

func downloadImage(ctx context.Context, resolver resolver.Interface, version string, imageType string) error {

	bucket := "images"
	//name := version + ".img/venice"
	//out := version + ".img"
	name := imageType + ".tgz"
	out := imageType + ".tgz"

	if resolver == nil {
		log.Errorf("Resolver cannot be null")
		return fmt.Errorf("Resolver cannot be null")
	}
	if version == "" {
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
	log.Infof("Got image [%v] of size [%d]", name, totsize)
	return nil
}
