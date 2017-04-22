plugin_LTLIBRARIES = libstrapper.la

libstrapper_la_SOURCES = stpplugin.c stpplugin.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libstrapper_la_CFLAGS = $(GST_CFLAGS)
libstrapper_la_LIBADD = $(GST_LIBS)
libstrapper_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libstrapper_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = stpplugin.h
