package main

import (
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path"
	"time"

	minio "github.com/minio/minio-go/v6"
	"github.com/sirupsen/logrus"
	"github.com/urfave/cli"

	"github.com/pensando/sw/asset-build/asset"
)

var (
	maxRetries    = 10
	retryInterval = 5 * time.Second
)

func main() {
	app := cli.NewApp()
	app.Version = ""
	app.Email = "erikh@pensando.io"
	app.Usage = "asset downloader for pensando software team"
	app.ArgsUsage = "[name] [version] [filename to save]"
	app.Action = action
	app.Flags = []cli.Flag{
		cli.StringFlag{
			Name:   "assets-server, a",
			Value:  asset.Endpoint,
			EnvVar: "ASSETS_HOST",
		},
	}

	if err := app.Run(os.Args); err != nil {
		logrus.Errorf("Error: %v: please see `%s help`", err, os.Args[0])
		os.Exit(1)
	}
}

func action(ctx *cli.Context) error {
	if len(ctx.Args()) != 3 {
		return errors.New("invalid arguments: please seek help")
	}

	for _, arg := range ctx.Args() {
		if arg == "" {
			return errors.New("invalid empty argument")
		}
	}

	name, version, filename := ctx.Args()[0], ctx.Args()[1], ctx.Args()[2]
	f, err := os.Create(filename)
	if err != nil {
		// no point retrying
		return err
	}
	defer f.Close()

	for i := 0; i < maxRetries; i++ {
		mc, err := minio.New(ctx.GlobalString("assets-server"), asset.AccessKeyID, asset.SecretAccessKey, false)
		if err != nil {
			logrus.Errorf("Error creating minio client: %v", err)
			time.Sleep(retryInterval)
			continue
		}

		if ok, err := mc.BucketExists(asset.RootBucket); err != nil {
			logrus.Errorf("Error checking minio bucket: %v", err)
			time.Sleep(retryInterval)
			continue
		} else if !ok {
			// no point retrying
			return errors.New("bucket does not exist")
		}

		path := path.Join(name, version, "asset.tgz")

		stat, err := mc.StatObject(asset.RootBucket, path, minio.StatObjectOptions{})
		if err != nil {
			logrus.Errorf("Error checking minio object: %v", err)
			time.Sleep(retryInterval)
			continue
		}

		logrus.Infof("Downloading %fMB, please be patient...", float64(stat.Size)/float64(1024*1024))

		obj, err := mc.GetObject(asset.RootBucket, path, minio.GetObjectOptions{})
		if err != nil {
			logrus.Errorf("Error getting minio object: %v", err)
			time.Sleep(retryInterval)
			continue
		}

		// GetObject only gets a handle to the object in the form of a Reader;
		// the actual downloading happens when Copy reads from the Reader,
		// so Copy can fail if the download is interrupted.
		// To make the operation retriable, we need to make sure that we don't
		// write anything to output file unless the download is succesfull,
		// otherwise the caller may start consuming it.
		// In order to do that, we first download the asset to a temporary
		// file and then copy it to output when it is complete.
		tmpFile, err := ioutil.TempFile("", fmt.Sprintf("asset-pull-%s", name))
		if err != nil {
			// no point retrying
			return fmt.Errorf("Error creating temporary file: %v", err)
		}

		cleanupTmpFile := func() {
			tmpFile.Close()
			os.RemoveAll(tmpFile.Name())
		}

		// copy to temporary file
		if _, err := io.Copy(tmpFile, obj); err != nil {
			logrus.Errorf("Error downloading minio object: %v", err)
			cleanupTmpFile()
			time.Sleep(retryInterval)
			continue
		}

		// copy from tmpFile to output and clean up
		tmpFile.Seek(0, io.SeekStart)
		_, err = io.Copy(f, tmpFile)
		cleanupTmpFile()
		if err != nil {
			// no point retrying
			return err
		}

		logrus.Infof("Complete!")
		return nil
	}
	return fmt.Errorf("Error pulling asset %s %s, retries exhausted", ctx.Args()[0], ctx.Args()[1])
}
