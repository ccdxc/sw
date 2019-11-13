package main

import (
	"archive/tar"
	"compress/gzip"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"runtime"
	"strings"
	"time"

	"github.com/minio/minio-go"
	"github.com/sirupsen/logrus"
	"github.com/urfave/cli"

	"github.com/pensando/sw/asset-build/asset"
)

// NOTE: This app is going to replace asset-upload, which limits to dedicated bucket name and file name

func main() {
	app := cli.NewApp()
	app.Version = "2019-09-26"
	app.Email = "test-infra@pensando.io"
	app.Usage = "asset uploader for pensando software team"
	app.ArgsUsage = "[bucket-name] [dir-name] [version] [filename to upload]"

	app.Flags = []cli.Flag{
		cli.StringFlag{
			Name:   "assets-server-colo, ac",
			Value:  asset.EndpointColo,
			EnvVar: "ASSETS_HOST_COLO",
			Usage:  "host name of colo asset server. if hostname is NULL, it means not push to colo",
		},
		cli.StringFlag{
			Name:   "assets-server-hq, ah",
			Value:  asset.EndpointHQ,
			EnvVar: "ASSETS_HOST_HQ",
			Usage:  "host name of hq asset server. if hostname is NULL, it means not push to hq",
		},
		cli.StringFlag{
			Name:  "remote-name",
			Usage: "new file name at remote asset server",
		},
	}

	app.Action = action

	if err := app.Run(os.Args); err != nil {
		logrus.Errorf("Error: %v: please see `%s help`", err, os.Args[0])
		os.Exit(1)
	}
}

func action(ctx *cli.Context) error {
	if len(ctx.Args()) != 4 {
		return errors.New("invalid arguments: please seek help")
	}

	for _, arg := range ctx.Args() {
		if arg == "" {
			return errors.New("invalid empty argument")
		}
	}

	bucket := ctx.Args()[0]
	dirName, version, filename := ctx.Args()[1], ctx.Args()[2], ctx.Args()[3]
	remote := []string{}
	if ctx.GlobalString("assets-server-colo") != "NULL" {
		remote = append(remote, ctx.GlobalString("assets-server-colo"))
	}
	if ctx.GlobalString("assets-server-hq") != "NULL" {
		assetServerHQ := ctx.GlobalString("assets-server-hq")
		if bucket == "builds" {
			// because builds is an NFS mount point at HQ, move one level up
			// also use different minio server due to minio limitation
			bucket = dirName
			dirName = version
			version = ""
			assetServerHQ = "assets-hq.pensando.io:9001"
		}
		remote = append(remote, assetServerHQ)
	}

	fi, err := os.Stat(filename)
	if err != nil {
		return err
	}

	if fi.IsDir() {
		assetPath := ""
		if ctx.String("remote-name") != "" {
			assetPath = path.Join(dirName, version, ctx.String("remote-name"))
		} else {
			assetPath = path.Join(dirName, version, filename) + ".tar.gz"
		}
		return uploadDir(bucket, filename, assetPath, remote)
	}

	return uploadFile(bucket, filename, path.Join(dirName, version, filename), remote, fi)
}

func uploadFile(bucket, localFile, remoteFile string, remoteHosts []string, fi os.FileInfo) error {
	size := fi.Size()
	if !fi.Mode().IsRegular() {
		var err error
		localFile, size, err = mktemp(localFile)
		if err != nil {
			return err
		}
		defer os.Remove(localFile)
	}

	f, err := os.Open(localFile)
	if err != nil {
		return err
	}
	defer f.Close()

	for _, ep := range remoteHosts {
		logrus.Infof("Uploading %s %f MB to %s...", localFile, float64(size>>10)/float64(1024), ep)

		if _, err := f.Seek(0, io.SeekStart); err != nil {
			return err
		}
		if err := upload(bucket, remoteFile, ep, f); err != nil {
			logrus.Errorf("Failed to push to %s: %v", ep, err)
		}
	}
	return nil
}

// skip .git and submodule .git directories.
// TODO: add more skipped condition if needed
func skipDir(name string) bool {
	return strings.HasPrefix(name, ".git")
}

func getTotalFilesCount(localPath string) (int, int, error) {
	var totalFiles, totalDirs int
	err := filepath.Walk(localPath, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if info.IsDir() {
			totalDirs++
			return nil
		}
		totalFiles++
		return nil
	})
	return totalDirs, totalFiles, err
}

func walkAndTar(localPath string, totalDirs, totalFiles int, tw *tar.Writer, gzw *gzip.Writer, pw *io.PipeWriter) {
	defer func() {
		if err := tw.Close(); err != nil {
			logrus.Errorf("failed to close tar writer: %v", err)
		}
		if err := gzw.Close(); err != nil {
			logrus.Errorf("failed to close gzip writer: %v", err)
		}
		if err := pw.Close(); err != nil {
			logrus.Errorf("failed to close pipe writer: %v", err)
		}
	}()

	var processedDirs, processedFiles int
	var processedBytes int64
	done := make(chan struct{})
	defer close(done)
	go func() {
		start := time.Now()

		spinner := []string{"-", "/", "-", "\\"}
		i := 0
		for {
			after := time.After(time.Second)
			select {
			case <-done:
				fmt.Printf("\n%d bytes compressed!\n", processedBytes)
				return
			case <-after:
				fmt.Printf("\r[elapsed: %v | files: %d%% - %d(%d) | dirs: %d%% - %d(%d) | bytes: %d] %s", time.Since(start),
					int(100.0*processedFiles/totalFiles), processedFiles, totalFiles,
					int(100.0*processedDirs/totalDirs), processedDirs, totalDirs,
					processedBytes, spinner[i%len(spinner)])
				i++
			}
		}
	}()

	err := filepath.Walk(localPath, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		link := ""
		if info.IsDir() {
			processedDirs++
			return nil
		} else if info.Mode()&os.ModeSymlink == os.ModeSymlink {
			if link, err = os.Readlink(path); err != nil {
				return err
			}
		}

		header, err := tar.FileInfoHeader(info, link)
		if err != nil {
			return err
		}

		header.Name = path
		err = tw.WriteHeader(header)
		if err != nil {
			return errors.New("tar " + path + " got " + err.Error())
		}

		processedFiles++

		if !info.Mode().IsRegular() { //nothing more to do for non-regular
			return nil
		}

		file, err := os.Open(path)
		if err != nil {
			return err
		}
		defer file.Close()

		size, err := io.Copy(tw, file)
		if err != nil {
			return errors.New("copy " + path + " got " + err.Error())
		}
		processedBytes += size
		return nil
	})
	if err != nil {
		logrus.Errorf("while walking directory, got %v", err)
	}
}

func uploadDir(bucket, localPath, remotePath string, remoteHosts []string) error {
	totalDirs, totalFiles, err := getTotalFilesCount(localPath)
	if err != nil {
		return err
	}

	for _, ep := range remoteHosts {
		logrus.Infof("Uploading (total %d files / %d dirs) under %s to %s@%s", totalFiles, totalDirs, localPath, remotePath, ep)
		pr, pw := io.Pipe()
		defer func() {
			if err := pr.Close(); err != nil {
				logrus.Errorf("failed to close pipe reader: %v", err)
			}
		}()
		gzw := gzip.NewWriter(pw)
		tw := tar.NewWriter(gzw)
		go walkAndTar(localPath, totalDirs, totalFiles, tw, gzw, pw)

		err := upload(bucket, remotePath, ep, pr)
		if err != nil {
			logrus.Errorf("failed to upload: %v", err)
		}
	}

	return nil
}

func upload(bucket, assetPath, endpoint string, reader io.Reader) error {
	mc, err := minio.New(endpoint, asset.AccessKeyID, asset.SecretAccessKey, false)
	if err != nil {
		return err
	}

	if ok, err := mc.BucketExists(bucket); err != nil {
		return err
	} else if !ok {
		if err := mc.MakeBucket(bucket, ""); err != nil {
			return err
		}
	}

	doneCh := make(chan struct{})
	list := mc.ListObjects(bucket, assetPath, false, doneCh)
	_, ok := <-list
	if ok {
		return errors.New("file exists, exiting")
	}
	close(doneCh)

	n, err := mc.PutObject(bucket, assetPath, reader, -1, minio.PutObjectOptions{NumThreads: uint(runtime.NumCPU() * 2)})
	if err != nil {
		return err
	}

	logrus.Infof("%d bytes uploaded!", n)
	return nil
}

func mktemp(filename string) (string, int64, error) {
	f, err := os.Open(filename)
	if err != nil {
		return "", 0, err
	}
	defer f.Close()

	tmp, err := ioutil.TempFile("", "assets-upload")
	if err != nil {
		return "", 0, err
	}
	defer tmp.Close()

	size, err := io.Copy(tmp, f)
	return tmp.Name(), size, err
}
