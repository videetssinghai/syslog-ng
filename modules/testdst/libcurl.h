#ifndef LIBCURL_H_INCLUDED
#define LIBCURL_H_INCLUDED
#include "driver.h"


glong put(gchar* server, gchar *port, gchar *index, gchar *type, gchar *custom_id, gchar *json_struct);
void testdst_curl_init();
void testdst_curl_deinit();
#endif
