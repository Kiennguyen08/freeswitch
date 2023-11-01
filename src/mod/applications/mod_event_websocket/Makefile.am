include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_event_websocket

mod_LTLIBRARIES = mod_event_websocket.la
mod_event_websocket_la_SOURCES  = mod_event_websocket.c
mod_event_websocket_la_CFLAGS   = $(AM_CFLAGS)
mod_event_websocket_la_LIBADD   = $(switch_builddir)/libfreeswitch.la
mod_event_websocket_la_LDFLAGS  = -avoid-version -module -no-undefined -shared
