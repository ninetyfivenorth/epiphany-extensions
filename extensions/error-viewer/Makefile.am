SUBDIRS = mozilla

extensiondir = $(libdir)/epiphany/extensions

INCLUDES = \
	-Imozilla						\
	-I$(top_srcdir)/include					\
	$(EPIPHANY_DEPENDENCY_CFLAGS)				\
	-DSHARE_DIR=\"$(pkgdatadir)\"				\
	-DEPHY_EXTENSIONS_LOCALEDIR=\"$(datadir)/locale\"	\
	-DG_DISABLE_DEPRECATED					\
	-DGDK_DISABLE_DEPRECATED				\
	-DGTK_DISABLE_DEPRECATED				\
	-DGDK_PIXBUF_DISABLE_DEPRECATED				\
	-DGNOME_DISABLE_DEPRECATED

extension_LTLIBRARIES = liberrorviewerextension.la

liberrorviewerextension_la_SOURCES = \
	ephy-error-viewer-extension.c	\
	ephy-error-viewer-extension.h	\
	error-viewer.c			\
	error-viewer.h			\
	extension.c

liberrorviewerextension_la_LIBADD = \
	mozilla/liberrorviewermozilla.la	\
	$(MOZILLA_COMPONENT_LIBS)

glade_DATA = error-viewer.glade

gladedir = $(pkgdatadir)/glade

EXTRA_DIST = $(glade_DATA)