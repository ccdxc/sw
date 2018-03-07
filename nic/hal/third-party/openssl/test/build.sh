#cd engine
#./build.sh
#cp *.so /tmp 
#cd ..

gcc -I../include -L../x86_64/lib -lpthread -ldl -lssl -lcrypto client.c -o client
#gcc -I../include -lpthread -ldl client.c ../lib/libssl.a ../lib/libcrypto.a -o client
gcc -I../include -lpthread -ldl server.c ../x86_64/lib/libssl.a ../x86_64/lib/libcrypto.a -o server
#rsync -avz . saurabh@srv20:~/work/tls18/ 
#gcc -I.././export/include -L../../export/lib -lssl -lcrypto client.c -o client
#gcc -I../../export/include -L../../export/lib -lssl -lcrypto -lpthread server.c -o server
#gcc -I../../export/include -L../../export/lib -lssl -lcrypto client.c -o client
#gcc -lssl -lcrypto client.c -o client
#gcc -I../../export/include -L../../export/lib -lssl -lcrypto tls-client.c -o tls-client
