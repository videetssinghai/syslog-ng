#ifndef LIBCURL_H_INCLUDED
#define LIBCURL_H_INCLUDED
#include "driver.h"


char *get(char *query, char *server);
char* put(char *query, char* server, char *json_struct);
#endif
