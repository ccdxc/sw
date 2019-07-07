package imagestore

import (
	"context"
	"fmt"
	"io"
	"os"

	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	veniceImageName   = "venice.tgz"
	naplesImageName   = "naples_fw.tar"
	veniceOSImageName = "venice_appl_os.tgz"
	metadataName      = "metadata.json"
	bucketName        = "images"
)

// DownloadNaplesImage downloads naples image from minio
func DownloadNaplesImage(ctx context.Context, resolver resolver.Interface, version string, destFileName string) error {
	if version == "" {
		log.Errorf("Version is needed to download a naples image from objstore")
		return fmt.Errorf("Version is needed to download a naples image from objstore")
	}

	objectStoreFileName := "Naples/" + version + "_img/" + naplesImageName
	return downloadImage(ctx, resolver, objectStoreFileName, destFileName)
}

// DownloadVeniceImage downloads a venice image from minio
func DownloadVeniceImage(ctx context.Context, resolver resolver.Interface, version string) error {

	if version == "" {
		log.Errorf("Version is needed to download a venice image from objstore")
		return fmt.Errorf("Version is needed to download a venice image from objstore")
	}

	objectStoreFileName := "Venice/" + version + "_img/" + veniceImageName
	return downloadImage(ctx, resolver, objectStoreFileName, veniceImageName)
}

// DownloadMetadataFile downloads a metadata.json from minio
func DownloadMetadataFile(ctx context.Context, resolver resolver.Interface, version string) error {

	if version == "" {
		log.Errorf("Version is needed to download a metadata.json from objstore")
		return fmt.Errorf("Version is needed to download a metadata.json from objstore")
	}

	objectStoreFileName := "Bundle/" + version + "_img/" + metadataName
	return downloadImage(ctx, resolver, objectStoreFileName, metadataName)
}

// DownloadVeniceOSImage downloads a venice appliance OS image from minio
func DownloadVeniceOSImage(ctx context.Context, resolver resolver.Interface, version string) error {

	if version == "" {
		log.Errorf("Version is needed to download a venice appliance image from objstore")
		return fmt.Errorf("Version is needed to download a venice appliance image from objstore")
	}

	objectStoreFileName := "veniceOS/" + version + "_img/" + veniceOSImageName
	return downloadImage(ctx, resolver, objectStoreFileName, veniceOSImageName)
}

func downloadImage(ctx context.Context, resolver resolver.Interface, name string, outFile string) error {

	bucket := bucketName

	if resolver == nil {
		log.Errorf("Resolver cannot be null")
		return fmt.Errorf("Resolver cannot be null")
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
		return fmt.Errorf("Image doesnt exist in objectstore")
	}
	defer fr.Close()

	of, err := os.Create(outFile)
	if err != nil {
		log.Errorf("Could not create output file [%s](%s)", outFile, err)
		return fmt.Errorf("Could not create output file [%s](%s)", outFile, err)
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
