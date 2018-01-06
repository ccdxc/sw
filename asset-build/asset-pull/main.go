package main

import (
	"errors"
	"io"
	"os"
	"path"

	"github.com/minio/minio-go"
	"github.com/sirupsen/logrus"
	"github.com/urfave/cli"

	"github.com/pensando/sw/asset-build/asset"
)

func main() {
	app := cli.NewApp()
	app.Version = ""
	app.Email = "erikh@pensando.io"
	app.Usage = "asset downloader for pensando software team"
	app.ArgsUsage = "[name] [version] [filename to save]"
	app.Action = action

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

	mc, err := minio.New(asset.Endpoint, asset.AccessKeyID, asset.SecretAccessKey, false)
	if err != nil {
		return err
	}

	if ok, err := mc.BucketExists(asset.RootBucket); err != nil {
		return err
	} else if !ok {
		return errors.New("bucket does not exist")
	}

	name, version, filename := ctx.Args()[0], ctx.Args()[1], ctx.Args()[2]

	f, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer f.Close()

	path := path.Join(name, version, "asset.tgz")

	stat, err := mc.StatObject(asset.RootBucket, path)
	if err != nil {
		return err
	}

	logrus.Infof("Downloading %fMB, please be patient...", float64(stat.Size)/float64(1024*1024))

	obj, err := mc.GetObject(asset.RootBucket, path)
	if err != nil {
		return err
	}

	if _, err := io.Copy(f, obj); err != nil {
		return err
	}

	logrus.Infof("Complete!")
	return nil
}
