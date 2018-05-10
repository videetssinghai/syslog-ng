#include "testdst.h"
#include "messages.h"
#include "scratch-buffers.h"

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

/* 1not sure if should be included */
#include "plugin.h"
#include "messages.h"

typedef struct _TestDstDriver
{
  LogDestDriver super;
  LogTemplateOptions template_options;
  LogTemplate *template;
  gchar *testfile_name;
  time_t suspend_until;
} TestDstDriver;

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

static EVTTAG *
_evt_tag_message(const GString *msg)
{
  const int max_len = 30;

  return evt_tag_printf("message", "%.*s%s",
                        (int) MIN(max_len, msg->len), msg->str,
                        msg->len > max_len ? "..." : "");
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

  rc = write(fd, msg->str, msg->len);
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
  self->testfile_name = g_strdup(testfile_name);
  return &self->super.super;
}
