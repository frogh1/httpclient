all:
	gcc  httpclient.c main.c -I./libcurl/inc -L./libz  -L./libcurl -L./libssl  -o httpclient -lrt  -lcurl -lssl -lcrypto -ldl -lpthread -lz

clean:
	rm -rf httpclient
