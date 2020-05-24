//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package impl

import (
	"fmt"
	"os"

	"github.com/spf13/cobra"
	"github.com/spf13/cobra/doc"
)

// GenDocsCmd is used to generate cobra docs
var GenDocsCmd = &cobra.Command{
	Use:    "gendocs",
	Short:  "generate cobra docs",
	Long:   "generate cobra docs",
	Args:   cobra.NoArgs,
	Hidden: true,
	RunE:   genDocsCmdHandler,
}

func init() {

}

func genDocs(rootCmd *cobra.Command) {
	os.Mkdir("./docs", os.ModePerm)
	os.Mkdir("./docs/man", os.ModePerm)
	os.Mkdir("./docs/markdown", os.ModePerm)
	os.Mkdir("./docs/rest", os.ModePerm)

	genManTreeDocs(rootCmd)
	genMarkdownDocs(rootCmd)
	genRestTreeDocs(rootCmd)
}

func genMarkdownDocs(rootCmd *cobra.Command) {
	err := doc.GenMarkdownTree(rootCmd, "./docs/markdown/")
	if err != nil {
		fmt.Println(err)
	}
}

func genRestTreeDocs(rootCmd *cobra.Command) {
	err := doc.GenReSTTree(rootCmd, "./docs/rest/")
	if err != nil {
		fmt.Println(err)
	}
}

func genManTreeDocs(rootCmd *cobra.Command) {
	header := &doc.GenManHeader{
		Title:   "MINE",
		Section: "3",
	}
	err := doc.GenManTree(rootCmd, header, "./docs/man/")
	if err != nil {
		fmt.Println(err)
	}
}

func genDocsCmdHandler(cmd *cobra.Command, args []string) error {
	genDocs(cmd.Root())
	return nil
}
