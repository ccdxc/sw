
# Object Store 
```
handles cluster retry
streaming ops                      minio specific ops
can support other back-ends        status checking
                                   meta data handling           open source client
objstore-client-------------------> minio-client -------------> minio-go-sdk --------> minio-server

```


## Client APIs:

```go
// PutObject uploads an object to the object store
PutObject(ctx context.Context, objectName string, reader io.Reader, userMeta map[string]string) (int64, error)

// PutStreamObject uploads stream of objects to the object store
// each write()  uploads a new object
// caller must close() after write() to close the stream
PutStreamObject(ctx context.Context, objectName string, metaData map[string]string) (io.WriteCloser, error)

// GetObject gets the object from the object store
// the caller must close() the reader after read()
GetObject(ctx context.Context, objectName string) (io.ReadCloser, error)

// GetStreamObjectAtOffset reads an object uploaded by PutStreamObject()
// caller must close() after read()
GetStreamObjectAtOffset(ctx context.Context, objectName string, offset int) (io.ReadCloser, error)

// StatObject returns object information
StatObject(objectName string) (*ObjectStats, error)

// ListObjects lists all objects starting with the prefix
ListObjects(prefix string) ([]string, error)

// RemoveObjects removes all objects starting with the prefix
RemoveObjects(prefix string) error
```

## Example
```go
// client
client, err := objstore.NewClient("default", "pktcap", resolverClient)
...
```

```go
// putobj
buffer := bytes.bytes.NewBuffer(data)
meta := map[string]string {
                    "key": "value",
        }
n, err := client.PutObject(context.Background(), "pktcap1", buffer, meta)

```
```go
// getobj
reader, err := client.GetObject(context.Background(), "image1")
data, err := ioutil.ReadAll(reader)
	
```

```go
// putstreamobj
iowr, err := client.PutStreamObject(context.Background(), "pktcap1", meta)
...
// retrieve data and write 
n, err := iowr.Write(data)
...
iowr.Close()
	
```


