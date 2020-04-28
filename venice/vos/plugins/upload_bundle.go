package plugins

import (
	"bytes"
	"context"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"

	minio "github.com/minio/minio-go/v6"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/vos"
)

const (
	tarfileDir        = "/var/log/pensando"
	installerTmpDir   = globals.RuntimeDir + "/upload/"
	imageMetaFileName = "metadata.json"
	metaReleaseDate   = "ReleaseData"
	metaVersion       = "Version"
	metaName          = "Name"
	veniceImageName   = "venice.tgz"
	naplesImageName   = "naples_fw.tar"
	veniceOSImageName = "venice_appl_os.tgz"
)

//getSHA256Sum computes checksum for image
func getSHA256Sum(fileName string) string {
	f, err := os.Open(fileName)
	if err != nil {
		log.Errorf("Failed to open file %s", fileName)
		return ""
	}
	defer f.Close()

	h := sha256.New()
	if _, err := io.Copy(h, f); err != nil {
		log.Errorf("Failed to copy contents of file %s", fileName)
		return ""
	}

	hash := hex.EncodeToString(h.Sum(nil))
	return hash
}

//ValidateImageBundle sanitize the uploaded image
func ValidateImageBundle() (map[string]map[string]string, error) {

	if _, err := os.Stat(installerTmpDir + "/" + imageMetaFileName); err != nil {
		log.Errorf("[%s] is missing in the bundle.tar %#v", imageMetaFileName, err)
		return nil, fmt.Errorf("Missing metadata file in the bundle")
	}
	versionMap := processMetadataFile(installerTmpDir + imageMetaFileName)
	if versionMap == nil {
		log.Errorf("Process MetadataFile Failed")
		return nil, fmt.Errorf("Processing metadata file failed")
	}

	if _, err := os.Stat(installerTmpDir + "/" + veniceImageName); err != nil {
		log.Errorf("[%s] is missing in the bundle.tar %#v", veniceImageName, err)
		return nil, fmt.Errorf("Missing venice Image in the bundle")
	}

	shaSum := getSHA256Sum(installerTmpDir + "/" + veniceImageName)
	log.Infof("shSum %v versionMap %v", string(shaSum), versionMap["Venice"]["hash"])
	if string(shaSum) != versionMap["Venice"]["hash"] {
		log.Errorf("Checksum mismatch for Venice Image")
		return nil, fmt.Errorf("Checksum mismatch of venice image")
	}

	if _, err := os.Stat(installerTmpDir + "/" + naplesImageName); err != nil {
		log.Errorf("[%s] is missing in the bundle.tar %#v", naplesImageName, err)
		return nil, fmt.Errorf("Missing naples image in the bundle")
	}
	shaSum = getSHA256Sum(installerTmpDir + "/" + naplesImageName)
	log.Infof("shSum %v versionMap %v", string(shaSum), versionMap["Naples"]["hash"])
	if string(shaSum) != versionMap["Naples"]["hash"] {
		log.Errorf("Checksum mismatch for Naples Image")
		return nil, fmt.Errorf("Checksum mismatch of naples image")
	}

	if _, err := os.Stat(installerTmpDir + "/" + veniceOSImageName); err != nil {
		log.Errorf("[%s] is missing in the bundle.tar %#v", veniceOSImageName, err)
		return nil, fmt.Errorf("Missing veniceOS image in the bundle")
	}

	shaSum = getSHA256Sum(installerTmpDir + "/" + veniceOSImageName)
	if string(shaSum) != versionMap["veniceOS"]["hash"] {
		log.Errorf("Checksum mismatch for veniceOS Image")
		return nil, fmt.Errorf("Checksum mismatch of veniceOS image")
	}

	return versionMap, nil
}

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

		fr, err := client.GetStoreObject(ctx, "default."+bucket, in.Name, minio.GetObjectOptions{})
		if err != nil {
			log.Errorf("Failed to get object [%v]", in.Name)
			return err
		}
		of, err := os.Create(tarfileDir + "/" + in.Name)
		if err != nil {
			log.Errorf("Could not create output file [%s](%s)", in.Name, err)
			return fmt.Errorf("Could not create output file [%s](%s)", in.Name, err)
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
		log.Infof("Got image [%s] of size [%d]", in.Name, totsize)

		if err := client.RemoveObject("default.images", in.Name); err != nil {
			log.Errorf("Failed to remove %s. Error %+v", in.Name, err)
			return err
		}

		err = ExtractImage(tarfileDir + "/" + in.Name)
		if err != nil {
			terr := os.Remove(tarfileDir + "/" + in.Name)
			if terr != nil {
				log.Errorf("removal of %s/%s returned %v", tarfileDir, in.Name, terr)
			}
			if rerr := client.RemoveObject("default."+bucket, in.Name); rerr != nil {
				return fmt.Errorf("Failed to extract bundle (%+v). Error while removing bundle (%+v)", err, rerr)
			}
			return fmt.Errorf("Upload Failed: ExtractImage %s/%s  returned %v", tarfileDir, in.Name, err)
		}

		err = os.Remove(tarfileDir + "/" + in.Name)
		if err != nil {
			log.Errorf("removal of %s/%s  returned %v", tarfileDir, in.Name, err)
		}

		versionMap, err := ValidateImageBundle()
		if versionMap == nil {
			if rerr := client.RemoveObject("default."+bucket, in.Name); rerr != nil {
				return fmt.Errorf("Invalid Image (%+v). Error while removing(%+v)", err, rerr)
			}
			if cerr := os.RemoveAll(installerTmpDir); cerr != nil {
				return fmt.Errorf("Invalid Image (%+v). Error %s during removeAll of %s", err, cerr, installerTmpDir)
			}
			return err
		}

		//create version object to enable deletion
		meta := make(map[string]string)

		for key := range versionMap {
			if err := uploadFileToObjStore(client, bucket, installerTmpDir, key, versionMap); err != nil {
				log.Errorf("Failed to upload %s. Error %+v", versionMap[key][metaName], err)
				if rerr := client.RemoveObject("default."+bucket, in.Name); rerr != nil {
					return fmt.Errorf("Failed to upload(%+v). Error while removing(%+v)", versionMap[key][metaName], rerr)
				}
				if cerr := os.RemoveAll(installerTmpDir); cerr != nil {
					return fmt.Errorf("Failed to upload(%+v). Error %s during removeAll of %s", versionMap[key][metaName], cerr, installerTmpDir)
				}
				return err
			}
			log.Infof("uploaded %s in bucket[%s]", versionMap[key][metaName], "default."+bucket)
		}

		_, err = client.PutObject("default."+bucket, versionMap["Bundle"][metaVersion], bytes.NewBufferString(versionMap["Bundle"][metaVersion]), -1, minio.PutObjectOptions{UserMetadata: meta})
		if err != nil {
			log.Errorf("UploadImage: Could not put object [%s] to datastore (%s)", versionMap["Bundle"][metaName], err)
			return err
		}
		if err := os.RemoveAll(installerTmpDir); err != nil {
			return fmt.Errorf("Error %s during removeAll of %s", err, installerTmpDir)
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

func getDeleteCbFunc(bucket string, stage vos.OperStage) vos.CallBackFunc {
	return func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		log.InfoLog("bucket", bucket, "Stage", stage, "oper", oper, "msg", "logger plugin")

		if in == nil {
			log.Infof("Passed in Object is empty")
			return fmt.Errorf("Passed in structure is empty")
		}
		version := in.Name
		log.Infof("Passed in object %+v.", in)

		fr, err := client.GetObjectWithContext(ctx, "default.images", "Bundle/"+version+"_img/"+imageMetaFileName, minio.GetObjectOptions{})
		if err != nil {
			log.Errorf("Could not get object (%s)", err)
			return fmt.Errorf("Could not get object (%s)", err)
		}

		objInfo, err := fr.Stat()
		if objInfo.Key != "Bundle/"+version+"_img/"+imageMetaFileName {
			log.Infof("Got delete call back for object %s. Returning", objInfo.Key)
			return nil
		}
		if err := os.RemoveAll(installerTmpDir); err != nil {
			log.Errorf("Error %s during removeAll of %s", err, installerTmpDir)
			return err
		}
		if err := os.MkdirAll(installerTmpDir, 0700); err != nil {
			log.Errorf("Error %s during mkdirAll of %s", err, installerTmpDir)
			return err
		}

		of, err := os.Create(installerTmpDir + "/" + imageMetaFileName)
		if err != nil {
			log.Errorf("Could not create output file [%s](%s)", imageMetaFileName, err)
			return fmt.Errorf("Could not create output file [%s](%s)", imageMetaFileName, err)
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
		log.Infof("Got metadata.json of size [%d]", totsize)
		versionMap := processMetadataFile(installerTmpDir + imageMetaFileName)
		if versionMap == nil {
			return fmt.Errorf("Failed to process metafile %s missing version info", installerTmpDir+"/"+imageMetaFileName)
		}

		log.Infof("Got versionMap %+v", versionMap)
		for key := range versionMap {
			if err := client.RemoveObject("default.images", key+"/"+versionMap[key][metaVersion]+"_img/"+versionMap[key][metaName]); err != nil {
				log.Errorf("Failed to remove %s. Error %+v", versionMap[key][metaName], err)
				return err
			}
			log.Infof("Removed %s in bucket[%s]", versionMap[key][metaName], "default."+bucket)
		}

		if err := os.RemoveAll(installerTmpDir); err != nil {
			return fmt.Errorf("Error %s during removeAll of %s", err, installerTmpDir)
		}

		return nil
	}
}

func uploadBundleImageCallbacks(instance vos.Interface) {
	instance.RegisterCb("images", vos.PostOp, vos.Upload, getUploadBundleCbFunc("images", vos.PostOp))
	instance.RegisterCb("images", vos.PreOp, vos.Delete, getDeleteCbFunc("images", vos.PreOp))
}
