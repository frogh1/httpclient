/*
 * create by frog
 * depend on libcurl
 * 2019-03-17
 * */


#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

typedef struct {
    char *content;
    //char *header;
    char *error_msg;
    int  success;
    long status_code;
} httpresponse_t;

enum {
    en_HTTPCLIENT_NOAUTH = 0,
    en_HTTPCLIENT_AUTH,
    en_HTTPCLENT_AUTH_MAX
};

typedef struct {
    char *cacert;  //direction of ca cert
    char *cert;    //direction of client cert
    char *certtype; //type of client cert  "PEM" (default), "DER"和"ENG"
    char *key;      //direction of client key
    char *keytype;  //type of client key "PEM" (default), "DER"和"ENG"
    char *password; //password of client key if client key have
}sslkey_t;

/*
 * 0:success
 * -1:error
 * */
int httpclient_init();
/*
 * release resource
*/
void httpclient_release();

/*
 * return 0:sucess
 * 1:error
 * url:http://xxxxxx
 * if auth is en_HTTPCLIENT_NOAUTH,sslkey is NULL;otherwise sslkey must be set
 */
int httpclient_get(httpresponse_t *response,char *url,int auth,sslkey_t *sslkey);

/*
 * return 0:sucess
 * 1:error
 * url:http://xxxxxx
 * data:post data
 * if auth is en_HTTPCLIENT_NOAUTH,sslkey is NULL;otherwise sslkey must be set
 */
int httpclient_post(httpresponse_t *response,char *url,char *data,int auth,sslkey_t *sslkey);


#endif
