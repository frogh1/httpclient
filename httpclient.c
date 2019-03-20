
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl.h>
#include "httpclient.h"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        printf("not enough memory (realloc returned NULL)");
        exit(EXIT_FAILURE);
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int _request_set(CURL *curl,struct curl_slist *headers,char *ua, long timeout,int skip)
{
    if (skip)
    {
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    }
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    if (headers != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
    if (ua != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, ua);
    }

    return 0;
}

int _https_set(httpresponse_t *response,CURL *curl,int auth,sslkey_t *sslkey)
{
    if (auth == en_HTTPCLIENT_NOAUTH)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    else
    {
        if (sslkey == NULL)
        {
            response->success = -1;
            response->error_msg = (char *)malloc(24);
            strncpy(response->error_msg, "unset ssl key info\0", 23);
            return -1;
        }

        if (sslkey->cacert == NULL)
        { /*no need be authenticated with known CA certificates*/
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        } else {
            /*authenticated server key with CA certificates*/
            curl_easy_setopt(curl, CURLOPT_CAINFO, sslkey->cacert);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, CURL_SSLVERSION_TLSv1);
            curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
        }

        /*double  director auth,client cert info */
        if (sslkey->cert != NULL)
        {
            curl_easy_setopt(curl, CURLOPT_SSLCERT, sslkey->cert);
        }
        if (sslkey->certtype != NULL)
        {
            curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, sslkey->certtype);
        }
        if (sslkey->key != NULL)
        {
            curl_easy_setopt(curl, CURLOPT_SSLKEY, sslkey->key);
        }
        if (sslkey->keytype != NULL)
        {
            curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, sslkey->keytype);
        }
        if (sslkey->password != NULL)
        {
            curl_easy_setopt(curl, CURLOPT_SSLKEYPASSWD, sslkey->password);
        }
    }
    return 0;
}

int _request(CURL *curl,httpresponse_t *response,char *url)
{
    CURLcode res_code = -1;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1); 
    chunk.size = 0;

    //set settings
    curl_easy_setopt(curl, CURLOPT_URL, url);

    //set callbacks
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
    res_code = curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &response->status_code);

    //if reading URL was Unsuccessful
    if(res_code != CURLE_OK ){
        response->success = -1;
        response->error_msg = (char *)malloc(24);
        strncpy(response->error_msg, "Unsuccessful reading\0", 23);
    }
    else{
        //build response
        response->success = 0;

        response->content = malloc(sizeof(char) * chunk.size);
        memcpy(response->content,(char *) chunk.memory, chunk.size);
    }

    if (chunk.memory)
    {
        free(chunk.memory);
        chunk.size = 0;
    }

    return 0;
}

int _get(httpresponse_t *response,char *url,struct curl_slist *headers, char *ua, long timeout,int auth,sslkey_t *sslkey)
{
    CURL *curl;

    curl = curl_easy_init();
    if(!curl)
    {
        response->success = -1;
        response->error_msg = (char *)malloc(32);
        strncpy(response->error_msg, "Cant init cURL handler.\0", 32);
        return -1;
    }

    if (strncmp(url,"https",strlen("https")) == 0)
    {
        if (_https_set(response,curl,auth,sslkey) < 0)
        {
            return -1;
        }
    }

    _request_set(curl,headers,ua,timeout,1);
    _request(curl,response,url);

    curl_easy_cleanup(curl);

    return 0;
}

int _post(httpresponse_t *response,char *url,char *data,struct curl_slist *headers, char *ua, long timeout,int auth,sslkey_t *sslkey)
{
    CURL *curl;

    curl = curl_easy_init();
    if(!curl)
    {
        response->success = -1;
        response->error_msg = (char *)malloc(32);
        strncpy(response->error_msg, "Cant init cURL handler.\0", 32);
        return -1;
    }

    if (strncmp(url,"https",strlen("https")) == 0)
    {
        if (_https_set(response,curl,auth,sslkey) < 0)
        {
            return -1;
        }
    }

    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    _request_set(curl,headers,ua,timeout,0);
    _request(curl,response,url);

    curl_easy_cleanup(curl);

    return 0;

}


/*
 * return 0:sucess
 * 1:error
 * url:http://xxxxxx
 * ua: User Agent
 * timeout:
 * Skip:1 skip 0:noskip
 */
int httpclient_get(httpresponse_t *response,char *url,int auth,sslkey_t *sslkey)
{
    struct curl_slist *http_headers = NULL;
    return _get(response,url,http_headers,"libcurl/7.58.0", 3,auth,sslkey);

}

/*
 * return 0:sucess
 * 1:error
 * url:http://xxxxxx
 * data:post data
 * ua:User Agent
 * timeout:
 */
int httpclient_post(httpresponse_t *response,char *url,char *data,int auth,sslkey_t *sslkey)
{
    struct curl_slist *http_headers = NULL;

    return _post(response,url,data,http_headers,"libcurl/7.58.0",3,auth,sslkey);
}



int httpclient_init()
{
    int res = 0;
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
        printf("Global init curl fail.\n");
        res = -1;
    }
    return res;
}

void httpclient_release()
{
    curl_global_cleanup();
}