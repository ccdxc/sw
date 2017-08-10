package bookstoreGwService

import (
	"github.com/GeertJohan/go.rice/embedded"
	"time"
)

func init() {

	// define files
	file2 := &embedded.EmbeddedFile{
		Filename:    "example.swagger.json",
		FileModTime: time.Unix(1502324499, 0),
		Content:     string("{\n  \"swagger\": \"2.0\",\n  \"info\": {\n    \"title\": \"Service name\",\n    \"version\": \"version not set\"\n  },\n  \"schemes\": [\n    \"http\",\n    \"https\"\n  ],\n  \"consumes\": [\n    \"application/json\"\n  ],\n  \"produces\": [\n    \"application/json\"\n  ],\n  \"paths\": {},\n  \"definitions\": {}\n}\n"),
	}

	// define dirs
	dir1 := &embedded.EmbeddedDir{
		Filename:   "",
		DirModTime: time.Unix(1502302765, 0),
		ChildFiles: []*embedded.EmbeddedFile{
			file2, // "example.swagger.json"

		},
	}

	// link ChildDirs
	dir1.ChildDirs = []*embedded.EmbeddedDir{}

	// register embeddedBox
	embedded.RegisterEmbeddedBox(`../../../../../sw/api/generated/bookstore/swagger`, &embedded.EmbeddedBox{
		Name: `../../../../../sw/api/generated/bookstore/swagger`,
		Time: time.Unix(1502302765, 0),
		Dirs: map[string]*embedded.EmbeddedDir{
			"": dir1,
		},
		Files: map[string]*embedded.EmbeddedFile{
			"example.swagger.json": file2,
		},
	})
}
