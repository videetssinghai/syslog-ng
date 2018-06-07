#ifndef TESTDST_H_INCLUDED
#define TESTDST_H_INCLUDED

#include "driver.h"


LogTemplateOptions *testdst_dd_get_template_options(LogDriver *s);
void testdst_dd_set_template(LogDriver *self, LogTemplate *template);
LogDriver *testdst_dd_new(gchar *user, GlobalConfig *cfg);
void testdst_dd_set_url(LogDriver *d, const gchar *url);
void testdst_dd_set_index(LogDriver *d, const gchar *index);

#endif
