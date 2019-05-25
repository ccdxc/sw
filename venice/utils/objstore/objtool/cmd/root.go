package cmd

import (
	"bufio"
	"context"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"syscall"

	"github.com/spf13/cobra"
	"golang.org/x/crypto/ssh/terminal"
	"gopkg.in/yaml.v2"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
)

var (
	testUser     string
	testPassword string
	testTenant   string
	configFile   string
	uri          string
	grpcuri      string
	authzHeader  string
	restClient   apiclient.Services
	authCtx      context.Context
	userConfig   Config
	bucket       string
)

var rootCmd = &cobra.Command{
	Use:   "objtool",
	Short: "commandline to interact with the venice object store",
	Long:  `Global configuration such as user, password, URI etc are read in from a config file (default ~/.objtoolcfg.yaml) if present`,
}

// Execute is entry level function for the tool
func Execute() {
	defer cleanup()
	if err := rootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

// Config read in from file
type Config struct {
	User     string `yaml:"user"`
	Password string `yaml:"password"`
	URI      string `yaml:"uri"`
	GRPCURI  string `yaml:"gRPCuri"`
}

func init() {
	cobra.OnInitialize(initializeClient)
	rootCmd.PersistentFlags().StringVar(&configFile, "configfile", "", "location of the config file to pick global options from default is ~/.objtoolcfg.yaml")
	rootCmd.PersistentFlags().StringVar(&testUser, "user", "test", "user name for authenticating to venice")
	rootCmd.PersistentFlags().StringVar(&testPassword, "password", "", "password for authenticating to venice")
	rootCmd.PersistentFlags().StringVar(&testTenant, "tenant", globals.DefaultTenant, "tenant for the user")
	rootCmd.PersistentFlags().StringVar(&uri, "uri", "", "URI for the venice cluster")
	rootCmd.PersistentFlags().StringVar(&bucket, "bucket", "images", "bucket to operate on")
}

func errorExit(err error, format string, a ...interface{}) {
	fmt.Printf("==> FAILED : %s", fmt.Sprintf(format, a...))
	if err != nil {
		fmt.Printf("\n    --> Error: %s", err)
	}
	fmt.Printf("\n")
	os.Exit(1)
}

func cleanup() {
	if restClient != nil {
		restClient.Close()
	}
}

func initializeClient() {
	readConfigfile()
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	ctx, err := testutils.NewLoggedInContext(context.Background(), uri, userCred)
	if err != nil {
		errorExit(err, "could not login (%s)", err)
	}
	authCtx = ctx
	ok := false
	authzHeader, ok = loginctx.AuthzHeaderFromContext(ctx)
	if !ok {

		errorExit(nil, "no authorizaton header in context")
	}
	restClient, err = apiclient.NewRestAPIClient(uri)
	if err != nil {
		errorExit(err, "failed to create client")
	}
}

func readConfigfile() {
	// find config file if it exists
	cfile := configFile
	if configFile == "" {
		homedir := os.Getenv("HOME")
		cfile = filepath.Join(homedir, ".objtoolcfg.yaml")
	}
	userConfig := Config{}
	if cfile != "" {
		yfile, err := ioutil.ReadFile(cfile)
		if err == nil {
			err = yaml.Unmarshal(yfile, &userConfig)
			if err != nil {
				fmt.Printf("failed to read config file at [%s](%s)\n", configFile, err)
				os.Exit(1)
			}
		} else if configFile != "" {
			errorExit(err, "failed to open configuration file [%v]", configFile)
		}
	}

	if uri == "" {
		uri = userConfig.URI
		if uri == "" {
			errorExit(nil, "URI to reach venice required. Pass on command line or in config file")
		}
	}
	if grpcuri == "" {
		grpcuri = userConfig.GRPCURI
	}
	reader := bufio.NewReader(os.Stdin)
	if testUser == "" {
		testUser = userConfig.User
		if testUser == "" {
			fmt.Printf("Enter username: ")
			testUser, _ = reader.ReadString('\n')
			if testUser == "" {
				errorExit(nil, "user name required for authentication")
			}
		}
	}
	if testPassword == "" {
		testPassword = userConfig.Password
		if testPassword == "" {
			fmt.Printf("Enter password: ")
			paswdBytes, err := terminal.ReadPassword(int(syscall.Stdin))
			if err != nil {
				errorExit(err, "error reading password")
			}
			testPassword = string(paswdBytes)
			if testPassword == "" {
				errorExit(nil, "user name required for authentication")
			}
		}
	}
}
