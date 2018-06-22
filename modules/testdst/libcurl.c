#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "libcurl.h"
#include "messages.h"
#include "scratch-buffers.h"
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


Testdst_Curl *
tesdst_curl_create(void)
{
  Testdst_Curl *self = g_new0(Testdst_Curl, 1);
  curl_global_init(CURL_GLOBAL_DEFAULT);
  self->curl = curl_easy_init();
  return self;
}

void
testdst_curl_deinit(Testdst_Curl *self)
{
  curl_easy_cleanup(self->curl);
  curl_global_cleanup();
}

static GString *
_build_url(gchar *server, gchar *port, gchar *index, gchar *type, gchar *custom_id)
{
  GString *url = g_string_sized_new(256);
  if (custom_id)
    g_string_printf(url, "http://%s:%s/%s/%s/%s",server, port, index, type, custom_id);
  else
    g_string_printf(url, "http://%s:%s/%s/%s",server, port, index, type);

  return url;
}

static struct curl_slist *
_get_curl_headers(Testdst_Curl *self)
{
  struct curl_slist *headers = NULL;

  if(self->curl)
    {

      headers = curl_slist_append(headers, "Accept: application/json");
      headers = curl_slist_append(headers, "Content-Type: application/json");
      headers = curl_slist_append(headers, "charsets: utf-8");

      return headers;
    }

  msg_error("Error in Headers, curl not initialized");
  return headers;
}

static size_t
_write_data(void *buffer, size_t G_GNUC_UNUSED size, size_t nmemb, void *data)
{
  GString *gbuffer = data;
  g_string_append_len(gbuffer, buffer, nmemb);

  return nmemb;
}


static void
_curl_set_opt(Testdst_Curl *self, gchar *msg, gchar *url, struct curl_slist *curl_headers)
{

  curl_easy_setopt(self->curl, CURLOPT_URL, url);
  curl_easy_setopt(self->curl, CURLOPT_HTTPHEADER, curl_headers);
  curl_easy_setopt(self->curl, CURLOPT_CUSTOMREQUEST, "POST"); /* !!! */
  curl_easy_setopt(self->curl, CURLOPT_POSTFIELDS, msg); /* data goes here */

}

glong
_put_elasticsearch(Testdst_Curl *self, gchar *server, gchar *port, gchar *index, gchar *type, gchar *custom_id,
                   gchar *json_struct)
{
  GString *request = _build_url(server, port, index, type, custom_id);
  glong response_code;
  GString *response;

  msg_debug("Inside the libcurl",
            evt_tag_str("url", request->str),
            evt_tag_str("server", server),evt_tag_str("data", json_struct));

  if(self->curl)
    {

      struct curl_slist *curl_headers = _get_curl_headers(self);
      _curl_set_opt(self, json_struct, request->str, curl_headers);

      /* for debugging */
      response = g_string_new(NULL); /* the write buffer */
      curl_easy_setopt(self->curl, CURLOPT_WRITEFUNCTION, _write_data);
      curl_easy_setopt(self->curl, CURLOPT_WRITEDATA, response);

      self->result = curl_easy_perform(self->curl);

      msg_debug("request completed", evt_tag_str("result",response->str));

      /* Check for errors */
      if(self->result != CURLE_OK)
        {
          msg_error("Error in PUT",evt_tag_str("curl_easy_perform() failed: %s\n", curl_easy_strerror(self->result)));
        }

      /* always cleanup */
      curl_slist_free_all(curl_headers);
      g_string_free(response, TRUE);

      curl_easy_getinfo (self->curl, CURLINFO_RESPONSE_CODE, &response_code);

      return response_code;
    }

}
