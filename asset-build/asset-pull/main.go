package main

import (
	"errors"
	"fmt"
	"io"
	"os"
	"path"
	"time"

	"github.com/minio/minio-go"
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

		name, version, filename := ctx.Args()[0], ctx.Args()[1], ctx.Args()[2]

		f, err := os.Create(filename)
		if err != nil {
			// no point retrying
			return err
		}
		defer f.Close()

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

		if _, err := io.Copy(f, obj); err != nil {
			// no point retrying
			return err
		}

		logrus.Infof("Complete!")
		return nil
	}
	return fmt.Errorf("Error pulling asset %s %s, retries exhausted", ctx.Args()[0], ctx.Args()[1])
}
