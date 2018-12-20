package copier

import (
	"io/ioutil"
	"os"
	"path/filepath"

	"github.com/pkg/sftp"
	"golang.org/x/crypto/ssh"

	"github.com/pensando/sw/venice/utils/log"
)

// Copier implements all remote to/from copy functions
type Copier struct {
	SSHClientConfig *ssh.ClientConfig
	SSHClient       *ssh.Client
}

// NewCopier returns a new copier instance
func NewCopier(c *ssh.ClientConfig) *Copier {
	copier := &Copier{
		SSHClientConfig: c,
	}
	return copier
}

// NewCopierWithSSHClient returns a new copier instance
func NewCopierWithSSHClient(c *ssh.Client) *Copier {
	copier := &Copier{
		SSHClient: c,
	}
	return copier
}

// CopyTo copies the source files to the remote destination
func (c *Copier) CopyTo(ipPort, dstDir string, files []string) error {
	//fmt.Println("ANCALAGON: ", files)
	var client *ssh.Client

	if c.SSHClient == nil {
		sshClient, err := ssh.Dial("tcp", ipPort, c.SSHClientConfig)
		if sshClient == nil || err != nil {
			log.Errorf("Copier | CopyTo node111 %v failed, Err: %v", ipPort, err)
			return err
		}
		defer sshClient.Close()
		client = sshClient
	} else {
		client = c.SSHClient
	}

	sftp, err := sftp.NewClient(client)
	if sftp == nil || err != nil {
		log.Errorf("Copier | CopyTo node %v failed, Err: %v", ipPort, err)
		return err
	}
	defer sftp.Close()
	// check if dst dir exists
	if _, err := sftp.Lstat(dstDir); err != nil {
		err = sftp.Mkdir(dstDir)
		if err != nil {
			log.Errorf("Copier | CopyTo failed to creating remote directory. | Dir: %v, Node: %v, Err: %v", dstDir, ipPort, err)
		}
	}
	for _, absSrcFile := range files {
		//// Preflight checks
		if len(absSrcFile) == 0 {
			// Nothing to do here
			continue
		}
		//
		//pool.Go(func() error {
		if _, err := os.Stat(absSrcFile); err != nil {
			log.Errorf("Copier | CopyTo could not find the file %v. Err: %v", absSrcFile, err)
			return err
		}
		_, srcFile := filepath.Split(absSrcFile)
		absDstFile := filepath.Join(dstDir, srcFile)
		log.Infof("Copier | CopyTo initiating copy... | Src: %v, Dst: %v, Node: %v", absSrcFile, absDstFile, ipPort)

		// Check for the validity of source file
		out, err := ioutil.ReadFile(absSrcFile)
		if err != nil {
			log.Errorf("Copier | CopyTo could not read the source file. | SrcFile: %v, Node: %v, Err: %v", absSrcFile, ipPort, err)
			return err
		}

		f, err := sftp.Create(absDstFile)
		if err != nil {
			log.Errorf("Copier | CopyTo failed to create remote file. | File: %v, Node: %v, Err: %v", absDstFile, ipPort, err)
			return err
		}
		f.Chmod(0766)

		if _, err := f.Write(out); err != nil {
			log.Errorf("Copier | CopyTo failed to write the remote file. | File: %v, Node: %v, Err: %v", absDstFile, ipPort, err)
			return err
		}
		f.Close()
	}

	return nil

}

// CopyFrom copies the remote source files to the local destination
func (c *Copier) CopyFrom(ipPort, dstDir string, files []string) error {

	var client *ssh.Client

	if c.SSHClient == nil {
		sshclient, err := ssh.Dial("tcp", ipPort, c.SSHClientConfig)
		if sshclient == nil || err != nil {
			log.Errorf("Copier | CopyFrom node %v failed, Err: %v", ipPort, err)
			return err
		}
		defer sshclient.Close()
		client = sshclient
	} else {
		client = c.SSHClient
	}

	sftp, err := sftp.NewClient(client)
	if sftp == nil || err != nil {
		log.Errorf("Copier | CopyFrom node %v failed, Err: %v", ipPort, err)
		return err
	}
	defer sftp.Close()

	// check if dst dir exists
	if _, err := os.Lstat(dstDir); err != nil {
		err = os.Mkdir(dstDir, 0666)
		if err != nil {
			log.Errorf("Copier | CopyFrom failed to creating local directory. | Dir: %v,  Err: %v", dstDir, err)
			return err
		}
	}
	for _, srcFile := range files {

		absSrcFile, err := sftp.Open(srcFile)
		if err != nil {
			log.Errorf("Copier | CopyFrom failed to open remote file . | file: %v,  Err: %v", srcFile, err)
			return err
		}
		// Create the destination file
		_, srcFileName := filepath.Split(srcFile)
		dstFile := dstDir + "/" + srcFileName
		absDstFile, err := os.Create(dstFile)
		if err != nil {
			log.Errorf("Copier | CopyFrom failed to create local file. | Dir: %v, file : %v,  Err: %v", dstDir, dstFile, err)
			return err
		}

		log.Infof("Copier | CopyFrom initiating copy... | Src: %v, Dst: %v, Node: %v", srcFile, dstFile, ipPort)
		if _, err := absSrcFile.WriteTo(absDstFile); err != nil {
			log.Errorf("Copier | CopyFrom failed to copy local | Src: %v, Dst: %v, Node: %v", srcFile, dstFile, ipPort)
			return err
		}

		absSrcFile.Chmod(0766)
		absSrcFile.Close()
		absDstFile.Close()
	}

	return nil

}
