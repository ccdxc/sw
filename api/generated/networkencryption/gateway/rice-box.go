package networkencryptionGwService

import (
	"github.com/GeertJohan/go.rice/embedded"
	"time"
)

func init() {

	// define files
	file2 := &embedded.EmbeddedFile{
		Filename:    "networkencryption.swagger.json",
		FileModTime: time.Unix(1503808540, 0),
		Content:     string("{\n  \"swagger\": \"2.0\",\n  \"info\": {\n    \"title\": \"Service name\",\n    \"version\": \"version not set\"\n  },\n  \"schemes\": [\n    \"http\",\n    \"https\"\n  ],\n  \"consumes\": [\n    \"application/json\"\n  ],\n  \"produces\": [\n    \"application/json\"\n  ],\n  \"paths\": {},\n  \"definitions\": {}\n}\n"),
	}

	// define dirs
	dir1 := &embedded.EmbeddedDir{
		Filename:   "",
		DirModTime: time.Unix(1503808483, 0),
		ChildFiles: []*embedded.EmbeddedFile{
			file2, // "networkencryption.swagger.json"

		},
	}

	// link ChildDirs
	dir1.ChildDirs = []*embedded.EmbeddedDir{}

	// register embeddedBox
	embedded.RegisterEmbeddedBox(`../../../../../sw/api/generated/networkencryption/swagger`, &embedded.EmbeddedBox{
		Name: `../../../../../sw/api/generated/networkencryption/swagger`,
		Time: time.Unix(1503808483, 0),
		Dirs: map[string]*embedded.EmbeddedDir{
			"": dir1,
		},
		Files: map[string]*embedded.EmbeddedFile{
			"networkencryption.swagger.json": file2,
		},
	})
}
