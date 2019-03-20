#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "httpclient.h"


int test_get() {
    int res;

    httpresponse_t response;

    res = httpclient_init();
    if (res < 0) {
        return -1;
    }

    res = httpclient_get(&response, "https://www.sogou.com/", 0, NULL);
    if (res < 0) {
        printf("error %s\n", response.error_msg);
        return -1;
    }
    printf("success: %d\n", response.success);
    printf("   code:%d\n", response.status_code);
    printf("content:%s\n", response.content);

    if(response.content != NULL)
    {
        free(response.content);
    }
    httpclient_release();
}

int test_post(){
    int res;

    httpresponse_t response;

    res = httpclient_init();
    if (res < 0) {
        return -1;
    }

    char *data=" {\"osver\": \"CentOS release 6.4 (Final)\", \"timestamp\": 1552640722, \"os\": \"Linux\"}";

    sslkey_t sslkey;
    memset(&sslkey,0,sizeof(sslkey));

    sslkey.cacert = "/data/httpclient/ca.crt";
    sslkey.cert = "/data/httpclient/client.pem";
   // sslkey.key = "/data/httpclient/client.key";

    char *url = "https://test.com/";
    res = httpclient_post(&response,url,data,en_HTTPCLIENT_AUTH,&sslkey);
    if(res < 0)
    {
        printf("error %s\n",response.error_msg);
        return -1;
    }

    printf("success: %d\n",response.success);
    printf("   code:%d\n",response.status_code);
    printf("content:%s\n",response.content);

    if(response.content != NULL)
    {
        free(response.content);
    }

    httpclient_release();

}

int main(int argc,char *argv[])
{
    printf("--------------------------------------------------------------\n");
    test_get();
    printf("--------------------------------------------------------------\n");
    test_post();

    return 0;
}
