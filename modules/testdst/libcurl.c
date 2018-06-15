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


struct string {
  char *ptr;
  size_t len;
};

static void 
init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

static 
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {

  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

CURL *curl;
CURLcode res;
char *value = NULL;
struct string s;
struct curl_slist *headers = NULL;

void init() {

  init_string(&s);
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  printf("%s\n", "init called");

}

void deinit() {

  free(s.ptr);
  curl_global_cleanup();
  printf("%s\n", "deinit called");
}

static GString *
build_url(char* server, char* port, char* index, char* type, char* custom_id) {

 GString *url = g_string_sized_new(256);
 g_string_printf(url, "http://%s:%s/%s/%s/%s",server, port, index, type, custom_id);
 return url;

}

char *get(char *query, char *server) {

  // char *request = build_url(server, query);
  // init();
  // if(curl) {
  //   curl_easy_setopt(curl, CURLOPT_URL, request);
  //   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  //   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

  //   res = curl_easy_perform(curl);
  //   printf("%s\n", s.ptr);
  //   value = s.ptr;

  //   /* Check for errors */ 
  //   if(res != CURLE_OK)
  //     fprintf(stderr, "curl_easy_perform() failed: %s\n",
  //       curl_easy_strerror(res));

  //   /* always cleanup */ 
  //   curl_easy_cleanup(curl);
  // }
  // deinit();
  return value;
}

static void set_headers() {

  if(curl) {
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charsets: utf-8");
  }

}

void ir_strcpy( char *s1, const char *s2, int rb, int re )
{
  while( (rb <= re) && (*s1++ = s2[rb]) ) rb++;
  *s1 = 0;
}

char* put(char* server, char *port, char *index, char *type, char *custom_id, char *json_struct) {
  GString *request = build_url(server, port, index, type, custom_id);
  init();

  msg_debug("Inside the libcurl",
    evt_tag_str("url", request->str),
    evt_tag_str("server", server),evt_tag_str("data", json_struct));

  set_headers();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, request->str);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST"); /* !!! */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_struct); /* data goes here */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    res = curl_easy_perform(curl);
    msg_debug("request done", evt_tag_str("result",s.ptr));
    value = s.ptr;

    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(res));

    /* always cleanup */ 
    curl_easy_cleanup(curl);
    return value;
  } else {

    printf("curl not initialized correctly");

  }
  deinit();
}

// int main(int argc, char** argv)
// {

//   // char *jsonObj = "{ \"index\" : {} }\n{ \"name\" : \"videet\" }\n{ \"index\" : {} }\n{ \"name\" : \"surmeet\" }\n";
//   init();
//   FILE *fptr;

//   fptr = fopen("/home/videet/ES6-client-tool/program.txt", "a");
//   if(fptr == NULL) {
//     printf("Error!");
//   }

//   fprintf(fptr,"%s\n", "hello");

//   char *method = argv[1];
//   fprintf(fptr,"%s\n", method);

//   char *query = argv[2];
//   fprintf(fptr,"%s\n", query);

//   char *server = argv[3];
//   fprintf(fptr,"%s\n", server);
//   // printf("query: %s\n",query);
//   // printf("server: %s\n",server);

//   // char *method = "PUT";
//   // char *query = "sample/s/33";
//   // char *server = "http://localhost:9200/";

//   if(!strcasecmp(method, "GET")) {
//     get(query, server);

//   } else if(!strcasecmp(method, "PUT")) {

//     char *json_data = argv[4];
//     // ir_strcpy(json_data, argv[4], 1, strlen(argv[4])-2);
//     // char *json_data = "{ \"name\" : \"videet\" }";
//     printf("data: %s\n",json_data);

//     fprintf(fptr,"%s\n",json_data);
//     fclose(fptr);

//     put(query, server, json_data);

//   } else {

//    fprintf(fptr,"%s\n%s\n%s\n", method, query, server);
//    printf("%s\n","Please enter correct method");

//  }

//  deinit();
//  return 0;
// }