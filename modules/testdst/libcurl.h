#ifndef LIBCURL_H_INCLUDED
#define LIBCURL_H_INCLUDED
#include "driver.h"
#include <curl/curl.h>

typedef struct _Testdst_Curl
{
  CURL *curl;
  CURLcode result;

} Testdst_Curl;


glong _put_elasticsearch(Testdst_Curl *self, gchar *server, gchar *port, gchar *index, gchar *type, gchar *custom_id,
                         gchar *json_struct);
void testdst_curl_deinit(Testdst_Curl *self);
Testdst_Curl *tesdst_curl_create(void);
#endif
