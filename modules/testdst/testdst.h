#ifndef TESTDST_H_INCLUDED
#define TESTDST_H_INCLUDED

#include "driver.h"

LogTemplateOptions *testdst_dd_get_template_options(LogDriver *s);
void testdst_dd_set_template(LogDriver *self, LogTemplate *template);
LogDriver *testdst_dd_new(gchar *user, GlobalConfig *cfg);

#endif
