module_LTLIBRARIES      += modules/testdst/libtestdst.la
modules_testdst_libtestdst_la_SOURCES = \
modules/testdst/testdst.c     \
  modules/testdst/testdst.h     \
  modules/testdst/testdst-grammar.y       \
  modules/testdst/testdst-parser.c        \
  modules/testdst/testdst-parser.h        \
  modules/testdst/testdst-plugin.c

BUILT_SOURCES       +=      \
  modules/testdst/testdst-grammar.y       \
  modules/testdst/testdst-grammar.c       \
  modules/testdst/testdst-grammar.h

EXTRA_DIST        +=      \
  modules/testdst/testdst-grammar.ym

modules_testdst_libtestdst_la_CPPFLAGS  =     \
  $(AM_CPPFLAGS)            \
  -I$(top_srcdir)/modules/testdst        \
  -I$(top_builddir)/modules/testdst
modules_testdst_libtestdst_la_LIBADD  = $(MODULE_DEPS_LIBS)
modules_testdst_libtestdst_la_LDFLAGS = $(MODULE_LDFLAGS)
modules_testdst_libtestdst_la_DEPENDENCIES= $(MODULE_DEPS_LIBS)

modules/testdst modules/testdst/ mod-testdst: modules/testdst/libtestdst.la
.PHONY: modules/testdst/ mod-testdst

