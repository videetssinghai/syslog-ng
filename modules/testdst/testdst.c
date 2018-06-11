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
  LogDestDriver super;
  LogTemplateOptions template_options;
  LogTemplate *template;
  gchar *testfile_name;
  gchar *url;
  gchar *index;
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
testdst_dd_set_url(LogDriver *d, const gchar *url)
{
  TestDstDriver *self = (TestDstDriver *) d;

  g_free(self->url);
  self->url = g_strdup(url);
}

void
testdst_dd_set_index(LogDriver *d, const gchar *index)
{
  TestDstDriver *self = (TestDstDriver *) d;

  g_free(self->index);
  self->index = g_strdup(index);
}

static gboolean
_is_output_suspended(TestDstDriver *self, time_t now)
{
  if (self->suspend_until && self->suspend_until > now)
    return TRUE;
  return FALSE;
}

static void
_suspend_output(TestDstDriver *self, time_t now)
{
  GlobalConfig *cfg = log_pipe_get_config(&self->super.super.super);

  self->suspend_until = now + cfg->time_reopen;
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


static gboolean
_write_message(TestDstDriver *self, const GString *msg)
{
  int fd;
  gboolean success = FALSE;
  gint rc;

  msg_debug("Posting message to test file",
    evt_tag_str("test_file", self->testfile_name),
    evt_tag_str("driver", self->super.super.id),
    _evt_tag_message(msg));
  fd = open(self->testfile_name, O_NOCTTY | O_WRONLY | O_NONBLOCK);
  if (fd < 0)
  {
    msg_error("Error opening test file",
      evt_tag_str("test_file", self->testfile_name),
      evt_tag_str("driver", self->super.super.id),
      evt_tag_error("error"),
      _evt_tag_message(msg));
    goto exit;
  }
  msg_debug("Posting message to Elasticsearch before",
    evt_tag_str("test_file", self->testfile_name),
    evt_tag_str("driver", self->super.super.id),
    _evt_tag_message(msg));

  rc = write(fd, msg->str, msg->len);
  put(self->index,self->url, msg->str);
  
  msg_debug("Posting message to Elasticsearch",
    evt_tag_str("test_file", self->testfile_name),
    evt_tag_str("driver", self->super.super.id),
    _evt_tag_message(msg));

  if (rc < 0)
  {
    msg_error("Error writing to test file",
      evt_tag_str("test_file", self->testfile_name),
      evt_tag_str("driver", self->super.super.id),
      evt_tag_error("error"),
      _evt_tag_message(msg));
    goto exit;
  }
  else if (rc != msg->len)
  {
    msg_error("Partial write to test_file, probably the output is too much for the kernel to consume",
      evt_tag_str("test_file", self->testfile_name),
      evt_tag_str("driver", self->super.super.id),
      _evt_tag_message(msg));
    goto exit;
  }

  success = TRUE;

  exit:
  if (fd >= 0)
    close(fd);

  return success;
}

static void
testdst_dd_queue(LogPipe *s, LogMessage *msg, const LogPathOptions *path_options)
{
  TestDstDriver *self = (TestDstDriver *) s;
  GString *formatted_message = scratch_buffers_alloc();
  gboolean success;
  time_t now = msg->timestamps[LM_TS_RECVD].tv_sec;

  if (_is_output_suspended(self, now))
    goto finish;

  _format_message(self, msg, formatted_message);

  G_LOCK(testdst_lock);
  success = _write_message(self, formatted_message);
  G_UNLOCK(testdst_lock);

  if (!success)
    _suspend_output(self, now);

  finish:
  log_dest_driver_queue_method(s, msg, path_options);
}



static gboolean
testdst_dd_init(LogPipe *s)
{
  TestDstDriver *self = (TestDstDriver *) s;
  GlobalConfig *cfg = log_pipe_get_config(s);

  log_template_options_init(&self->template_options, cfg);
  return log_dest_driver_init_method(s);
}

static void
testdst_dd_free(LogPipe *s)
{
  TestDstDriver *self = (TestDstDriver *) s;

  log_template_options_destroy(&self->template_options);
  g_free(self->testfile_name);
  log_template_unref(self->template);
  log_dest_driver_free(s);
}

LogDriver *
testdst_dd_new(gchar *testfile_name, GlobalConfig *cfg)
{
  TestDstDriver *self = g_new0(TestDstDriver, 1);

  log_dest_driver_init_instance(&self->super, cfg);
  log_template_options_defaults(&self->template_options);
  self->super.super.super.init = testdst_dd_init;
  self->super.super.super.free_fn = testdst_dd_free;
  self->super.super.super.queue = testdst_dd_queue;
  self->testfile_name = g_strdup(testfile_name);
  return &self->super.super;
}
