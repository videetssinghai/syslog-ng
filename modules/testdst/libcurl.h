#ifndef LIBCURL_H_INCLUDED
#define LIBCURL_H_INCLUDED
#include "driver.h"


char *get(char *query, char *server);
char* put(char* server, char *port, char *index, char *type, char *custom_id, char *json_struct);
#endif
