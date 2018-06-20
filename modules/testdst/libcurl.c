#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <strings.h>
#include "libcurl.h"
#include "messages.h"
#include "scratch-buffers.h"
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


struct MemoryStruct
{
  char *memory;
  size_t size;
};

typedef struct _Testdst_Curl
{
  CURL *curl;
  CURLcode res;

} Testdst_Curl; 

Testdst_Curl *self;

void
testdst_curl_init()
{
  self = g_new0(Testdst_Curl, 1);
  curl_global_init(CURL_GLOBAL_DEFAULT);
  self->curl = curl_easy_init();
}

void
testdst_curl_deinit() 
{
 curl_easy_cleanup(self->curl);
 curl_global_cleanup();
}

static GString *
build_url(char* server, char* port, char* index, char* type, char* custom_id)
{
 GString *url = g_string_sized_new(256);
 if (custom_id)
  g_string_printf(url, "http://%s:%s/%s/%s/%s",server, port, index, type, custom_id);
else
  g_string_printf(url, "http://%s:%s/%s/%s",server, port, index, type);

return url;
}

static struct curl_slist * 
_get_curl_headers() 
{
  struct curl_slist *headers = NULL;

  if(self->curl) {

    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charsets: utf-8");

    return headers;
  }

  msg_error("Error in Headers, curl not initialized");
  return headers;
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    msg_error("not enough memory (realloc returned NULL)");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}


static void 
curl_set_opt(gchar* msg, gchar* url, struct curl_slist *curl_headers)
{

  curl_easy_setopt(self->curl, CURLOPT_URL, url);
  curl_easy_setopt(self->curl, CURLOPT_HTTPHEADER, curl_headers);
  curl_easy_setopt(self->curl, CURLOPT_CUSTOMREQUEST, "POST"); /* !!! */
  curl_easy_setopt(self->curl, CURLOPT_POSTFIELDS, msg); /* data goes here */

}

glong 
put(gchar* server, gchar *port, gchar *index, gchar *type, gchar *custom_id, gchar *json_struct)
{
  GString *request = build_url(server, port, index, type, custom_id);

  struct MemoryStruct chunk;
  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
  chunk.size = 0;    /* no data at this point */ 
  glong response_code;

  msg_debug("Inside the libcurl",
    evt_tag_str("url", request->str),
    evt_tag_str("server", server),evt_tag_str("data", json_struct));

  if(self->curl) {

    struct curl_slist *curl_headers = _get_curl_headers();
    curl_set_opt(json_struct, request->str, curl_headers);

    /* for debugging */
    curl_easy_setopt(self->curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(self->curl, CURLOPT_WRITEDATA, (void *)&chunk);
    
    self->res = curl_easy_perform(self->curl);

    msg_debug("request done", evt_tag_str("result",chunk.memory));

    /* Check for errors */ 
    if(self->res != CURLE_OK)
    {
      msg_error("Error in PUT",evt_tag_str("curl_easy_perform() failed: %s\n", curl_easy_strerror(self->res)));
    }
    
    /* always cleanup */ 
    curl_slist_free_all(curl_headers);
    free(chunk.memory);

    curl_easy_getinfo (self->curl, CURLINFO_RESPONSE_CODE, &response_code);

    return response_code;
  } 

}
