SUBDIRS = include extensions data po help

ACLOCAL_AMFLAGS = -I m4

NULL =

EXTRA_DIST =			\
	autogen.sh		\
	COPYING.README		\
	ChangeLog		\
	ChangeLog-20070912	\
	po/ChangeLog.pre-2-24	\
	help/ChangeLog.pre-2-24 \
	HACKING			\
	README 			\
	TODO 			\
	ephy-extension.symbols	\
	gnome-doc-utils.make	\
	tools/copy-template	\
	doc/writing-epiphany-extensions.xml	\
	$(NULL)

DISTCLEANFILES = 		\
	gnome-doc-utils.make	\
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-scrollkeeper		\
	--disable-schemas-install

ChangeLog:
	if test -f $(top_srcdir)/.git/HEAD; then \
		git log --stat > $@; \
	fi

.PHONY: ChangeLog
