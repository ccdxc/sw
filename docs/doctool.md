Doctool usage
===============

Doctool generates user documentation page(s) as static HTML using handwritten markdown files, generated helper files (generated from proto) and a config.yaml file

The Configuration YAML is passed in as paramenter to doctool. It defines a base path and a list of the files.
The basepath is relative to the GOPATH for the workspace and files are evaluated as GOPATH+baspath+filepath.
Each file entry has an "input" (a markdown file) and "output" (a static HTML file) a "tag" (optional) field
and a "embed" (optional) field. 

The input and output fields are self evident. The "tag" field adds a tag which can be used in the "embed" field.

Sample YAML file
-----------------
````yaml
 basepath: "src/github.com/pensando/sw"
 files:
       - input: "api/docs/md/index.md"
         output: "api/docs/index.html"
         embed:
           - "APIREFS"
           - "OBJURIS"
       - input: "api/generated/swagger/md/index.md"
         output: "api/docs/generated/swaggeruri.html"
       - input: "api/generated/apiref.md"
         tag: "APIREFS"
       - input: "api/generated/objuris.md"
         tag: "OBJURIS"
````

Usage of the Embed Field
------------------------
The Embed field is used to embed content into a input file. For example, there might be portions of the input
document that need to have content that is auto-generated. Embedding content needs the following
1. a tag on the file containing the content to be embedded
2. an entry in the embed clause of the target file that will embed the content
3. an [://embed] #(-TAG-) line on its own in the target file at the position where the embedded content is to be inserted

In the example below contents of file tagged with "APIREFS" is inserted into the input file.

````markdown
    [//embed]: #(-APIREFS-)
````

Tags are unique and each tag can only be applied to one file, but can be used in multiple embed clauses.
