#!/bin/bash
ftp -inv 192.168.100.101 <<EOF
user admin linuxpassword
cd /home/admin/ftp
pwd
dir
mget ftp_server.txt
mput ftp_client.txt
