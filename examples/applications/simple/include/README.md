## Server and Client Certificates
   
In order to build the simple example, you'll need to convert your server and client certificates to C-header files, as follows,...
```$xslt
./scripts/include_certs.sh thingspaceserver.pem client.cert.pem client.private.key examples/applications/simple/include/
```