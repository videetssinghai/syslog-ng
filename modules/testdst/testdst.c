#include "testdst.h"
#include "messages.h"
#include "scratch-buffers.h"
#include "libcurl.h"

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

typedef struct _TestDstDriver
{
  LogThrDestDriver super;
  LogTemplateOptions template_options;
  LogTemplate *template;
  gchar *testfile_name;
  gchar *server;
  gchar *port;
  gchar *index;
  gchar *type;
  gchar *custom_id;
  time_t suspend_until;
} TestDstDriver;

G_LOCK_DEFINE_STATIC(testdst_lock);

LogTemplateOptions *
testdst_dd_get_template_options(LogDriver *s)
{
  TestDstDriver *self = (TestDstDriver *) s;

  return &self->template_options;
}

void
testdst_dd_set_template(LogDriver *s, LogTemplate *template)
{
  TestDstDriver *self = (TestDstDriver *) s;

  log_template_unref(self->template);
  self->template = template;
}

void
testdst_dd_set_server(LogDriver *d, const gchar *server)
{
  TestDstDriver *self = (TestDstDriver *) d;

  g_free(self->server);
  self->server = g_strdup(server);
}

void
testdst_dd_set_port(LogDriver *d, const gchar *port)
{
  TestDstDriver *self = (TestDstDriver *) d;

  g_free(self->port);
  self->port = g_strdup(port);
}

void
testdst_dd_set_index(LogDriver *d, const gchar *index)
{
  TestDstDriver *self = (TestDstDriver *) d;

  g_free(self->index);
  self->index = g_strdup(index);
}

void
testdst_dd_set_type(LogDriver *d, const gchar *type)
{
  TestDstDriver *self = (TestDstDriver *) d;

  g_free(self->type);
  self->type = g_strdup(type);
}

void
testdst_dd_set_custom_id(LogDriver *d, const gchar *custom_id)
{
  TestDstDriver *self = (TestDstDriver *) d;

  g_free(self->custom_id);
  self->custom_id = g_strdup(custom_id);
}

static EVTTAG *
_evt_tag_message(const GString *msg)
{
  const int max_len = 30;

  return evt_tag_printf("message", "%.*s%s",
    (int) MIN(max_len, msg->len), msg->str,
    msg->len > max_len ? "..." : "");
}

static void
_format_message(TestDstDriver *self, LogMessage *msg, GString *formatted_message)
{
  log_template_format(self->template, msg, &self->template_options, LTZ_LOCAL, 0, NULL, formatted_message);
}

static worker_insert_result_t
_map_http_status_to_worker_status(glong http_code)
{
  worker_insert_result_t retval;

  switch (http_code/100)
  {
    case 4:
    msg_debug("curl: 4XX: msg dropped",
      evt_tag_int("status_code", http_code));
    retval = WORKER_INSERT_RESULT_DROP;
    break;
    case 5:
    msg_debug("curl: 5XX: message will be retried",
      evt_tag_int("status_code", http_code));
    retval = WORKER_INSERT_RESULT_ERROR;
    break;
    default:
    msg_debug("curl: OK status code",
      evt_tag_int("status_code", http_code));
    retval = WORKER_INSERT_RESULT_SUCCESS;
    break;
  }

  return retval;
}

static worker_insert_result_t
_insert(LogThrDestDriver *s, LogMessage *msg)
{
  TestDstDriver *self = (TestDstDriver *) s;
  
  GString *message = scratch_buffers_alloc();
  worker_insert_result_t retval;
  glong http_code = 0;

  _format_message(self, msg, message);

  msg_debug("formatted msg",evt_tag_str("msg: ",message->str));

  
  msg_debug("Posting message to Elasticsearch before",
    evt_tag_str("driver", self->super.super.super.id),
    _evt_tag_message(message));

  http_code = put(self->server, self->port, self->index, self->type, self->custom_id, message->str);
  
  retval = _map_http_status_to_worker_status(http_code);

  return retval;

}

static gchar *
_format_stats_instance(LogThrDestDriver *s)
{
  static gchar stats[1024];

  TestDstDriver *self = (TestDstDriver *) s;

  g_snprintf(stats, sizeof(stats), "http,%s", self->testfile_name);

  return stats;
}

static const gchar *
_format_persist_name(const LogPipe *s)
{
  const TestDstDriver *self = (const TestDstDriver *)s;
  static gchar persist_name[1024];

  if (s->persist_name)
    g_snprintf(persist_name, sizeof(persist_name), "http.%s", s->persist_name);
  else
    g_snprintf(persist_name, sizeof(persist_name), "http(%s)", self->server);


  msg_debug("persist_name",
    evt_tag_str("persist_name", persist_name));
  return persist_name;
}


static void
_thread_init(LogThrDestDriver *s)
{
  TestDstDriver *self = (TestDstDriver *) s;
  msg_debug("Worker thread started",
    evt_tag_str("driver", self->super.super.super.id),
    NULL);
}

static void
_thread_deinit(LogThrDestDriver *s)
{
}

static gboolean
_connect(LogThrDestDriver *s)
{
  return TRUE;
}

static void
_disconnect(LogThrDestDriver *s)
{
}

static gboolean
testdst_dd_init(LogPipe *s)
{
  TestDstDriver *self = (TestDstDriver *) s;
  GlobalConfig *cfg = log_pipe_get_config(s);
  msg_debug("testdst_dd_init called",
    evt_tag_str("driver", self->super.super.super.id),
    NULL);
  testdst_curl_init();


  log_template_options_init(&self->template_options, cfg);
  return log_threaded_dest_driver_start(s);
}

gboolean
testdst_dd_deinit(LogPipe *s)
{ 
  TestDstDriver *self = (TestDstDriver *) s;

  msg_debug("testdst_dd_deinit called",
    evt_tag_str("driver", self->super.super.super.id),
    NULL);

  return log_threaded_dest_driver_deinit_method(s);
}

static void
testdst_dd_free(LogPipe *s)
{

  TestDstDriver *self = (TestDstDriver *) s;

  msg_debug("testdst_dd_free called",
    evt_tag_str("driver", self->super.super.super.id),
    NULL);

  testdst_curl_deinit();
  
  g_free(self->server);
  g_free(self->port);
  g_free(self->type);
  g_free(self->index);
  g_free(self->custom_id);

  log_template_options_destroy(&self->template_options);
  g_free(self->testfile_name);
  log_template_unref(self->template);
  log_threaded_dest_driver_free(s);
}

LogDriver *
testdst_dd_new(gchar *testfile_name, GlobalConfig *cfg)
{
  TestDstDriver *self = g_new0(TestDstDriver, 1);

  log_threaded_dest_driver_init_instance(&self->super, cfg);
  log_template_options_defaults(&self->template_options);
  self->super.super.super.super.init = testdst_dd_init;
  self->super.super.super.super.deinit = testdst_dd_deinit;
  self->super.super.super.super.free_fn = testdst_dd_free;
  self->super.worker.thread_init = _thread_init;
  self->super.worker.thread_deinit = _thread_deinit;
  self->super.worker.connect = _connect;
  self->super.worker.disconnect = _disconnect;
  self->super.worker.insert = _insert;


  self->super.super.super.super.generate_persist_name = _format_persist_name;
  self->super.format.stats_instance = _format_stats_instance;
//  self->super.super.super.super.queue = testdst_dd_queue;
  self->testfile_name = g_strdup(testfile_name);
  return (LogDriver *)self;
}
