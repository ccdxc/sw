package plugins

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"

	"github.com/minio/minio-go"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/vos"
)

const (
	tarfileDir        = "/var/log/pensando"
	installerTmpDir   = globals.RuntimeDir + "/upload/"
	bundleImage       = "bundle.tar"
	imageMetaFileName = "metadata.json"
	metaReleaseDate   = "ReleaseData"
	metaVersion       = "Version"
	metaName          = "Name"
)

// ExtractImage takes a locally downloaded image and extracts the contents
func ExtractImage(filename string) error {
	var err error

	if err := os.RemoveAll(installerTmpDir); err != nil {
		log.Errorf("Error %s during removeAll of %s", err, installerTmpDir)
		return err
	}
	if err := os.MkdirAll(installerTmpDir, 0700); err != nil {
		log.Errorf("Error %s during mkdirAll of %s", err, installerTmpDir)
		return err
	}

	if _, err = exec.LookPath("tar"); err != nil {
		log.Errorf("LookPath failed during extract err %v", err)
		return err
	}
	cmd := exec.Command("tar", "-C", installerTmpDir, "-xvf", filename)
	output, err := cmd.CombinedOutput()
	if err != nil {
		log.Errorf("ExtractImage failed during extract of %s with output:%s errcode %v", filename, string(output), err)
	}
	return err
}

func getUploadBundleCbFunc(bucket string, stage vos.OperStage) vos.CallBackFunc {
	return func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		log.InfoLog("bucket", bucket, "Stage", stage, "oper", oper, "msg", "logger plugin")

		//TODO object name to be taken from objectstore.Object. in object is currently null
		fr, err := client.GetObjectWithContext(ctx, "default.images", "bundle.tar", minio.GetObjectOptions{})
		if err != nil {
			log.Errorf("Could not get object (%s)", err)
			return fmt.Errorf("Could not get object (%s)", err)
		}

		objInfo, err := fr.Stat()
		if objInfo.Key != bundleImage {
			log.Infof("Got upload call back for object %s. Returning", objInfo.Key)
			return nil
		}

		of, err := os.Create(tarfileDir + "/" + bundleImage)
		if err != nil {
			log.Errorf("Could not create output file [%s](%s)", bundleImage, err)
			return fmt.Errorf("Could not create output file [%s](%s)", bundleImage, err)
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
		log.Infof("Got image [bundle.tar] of size [%d]", totsize)

		err = ExtractImage(tarfileDir + "/" + bundleImage)
		if err != nil {
			return fmt.Errorf("ExtractImage %s  returned %v", tarfileDir+"/"+bundleImage, err)
		}

		versionMap := processMetadataFile(installerTmpDir + imageMetaFileName)
		if versionMap == nil {
			return fmt.Errorf("Failed to process metafile %s missing version info", installerTmpDir+"/"+imageMetaFileName)
		}

		for key := range versionMap {
			if err := uploadFileToObjStore(client, bucket, installerTmpDir, key, versionMap); err != nil {
				log.Errorf("Failed to upload %s. Error %+v", versionMap[key][metaName], err)
				return err
			}
			log.Infof("uploaded %s in bucket[%s]", versionMap[key][metaName], "default."+bucket)
		}
		return nil
	}
}

func uploadFileToObjStore(client vos.BackendClient, bucket string, root string, imageType string, metaMap map[string]map[string]string) error {
	var err error
	if _, err := os.Stat(root + metaMap[imageType][metaName]); err != nil {
		log.Errorf("[%s] is missing in the bundle.tar %#v", metaMap[imageType][metaName], err)
		return fmt.Errorf("[%s] is missing in the bundle.tar %#v", metaMap[imageType][metaName], err)
	}

	fileBuf, err := ioutil.ReadFile(root + metaMap[imageType][metaName])
	if err != nil {
		log.Infof("Error (%+v) reading file %s/%s", err, root, metaMap[imageType][metaName])
		return err
	}
	meta := make(map[string]string)
	meta[metaReleaseDate] = metaMap[imageType][metaReleaseDate]
	meta[metaVersion] = metaMap[imageType][metaVersion]

	_, err = client.PutObject("default."+bucket, imageType+"/"+metaMap[imageType][metaVersion]+"_img/"+metaMap[imageType][metaName], bytes.NewBuffer(fileBuf), -1, minio.PutObjectOptions{UserMetadata: meta})
	if err != nil {
		log.Errorf("UploadImage: Could not put object [%s] to datastore (%s)", metaMap[imageType][metaName], err)
		return err
	}

	return err
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
func uploadBundleImageCallbacks(instance vos.Interface) {
	instance.RegisterCb("images", vos.PostOp, vos.Upload, getUploadBundleCbFunc("images", vos.PostOp))
}
