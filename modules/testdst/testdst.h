#ifndef TESTDST_H_INCLUDED
#define TESTDST_H_INCLUDED

#include "driver.h"
#include "logthrdestdrv.h"


LogTemplateOptions *testdst_dd_get_template_options(LogDriver *s);
void testdst_dd_set_template(LogDriver *self, LogTemplate *template);
LogDriver *testdst_dd_new(GlobalConfig *cfg);
void testdst_dd_set_server(LogDriver *d, const gchar *server);
void testdst_dd_set_port(LogDriver *d, const gchar *port);
void testdst_dd_set_type(LogDriver *d, const gchar *type);
void testdst_dd_set_index(LogDriver *d, const gchar *index);
void testdst_dd_set_custom_id(LogDriver *d, const gchar *custom_id);


#endif
