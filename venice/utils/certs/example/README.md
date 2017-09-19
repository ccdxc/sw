The example directory has 
    * Utility for creating CA certificates, CSRs and signing them with CA Cert
    * Sample Server app using its own keys (server.crt, server.key)
    * Sample Client app using the client key (client.key,client.crt)

To bootstrap, compile the sources as below:

```
go build ./rpc-tls-client.go
go build ./rpc-tls-server.go
go build ./certs_example.go
```

Create certificates as below:

```
mkdir certs
./certs_example -genrsa -out certs/ca-key.pem
./certs_example -selfsign -cakey certs/ca-key.pem -out certs/ca.crt
./certs_example -genrsa -out certs/server.key
./certs_example -csr -cakey certs/server.key -out certs/server.csr
./certs_example --signcsr -in certs/server.csr -cakey certs/ca-key.pem -cacert certs/ca.crt -out certs/server.crt
./certs_example -genrsa -out certs/client.key
./certs_example -csr -cakey certs/client.key -out certs/client.csr
./certs_example --signcsr -in certs/client.csr -cakey certs/ca-key.pem -cacert certs/ca.crt -out certs/client.crt
```

Finally we can run the server and client on two different windows.
We should see the output similar to below:
```
$ ./rpc-tls-server
2017/02/27 11:15:55 server: listening
2017/02/27 11:15:59 server: accepted from 127.0.0.1:49344
2017/02/27 11:15:59 Received "twenty-three", its length is 12
2017/02/27 11:15:59 server: conn: closed
^C
```

```
$ ./rpc-tls-client
2017/02/27 11:15:59 client: connected to:  127.0.0.1:8000
2017/02/27 11:15:59 Returned result is 12
```
