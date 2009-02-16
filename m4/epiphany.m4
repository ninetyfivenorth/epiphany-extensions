## epiphany.m4 - Macros for building epiphany extensions. -*-Shell-script-*-
## Copyright © Crispin Flowerday <crispin@gnome.org>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
##
## As a special exception to the GNU General Public License, if you
## distribute this file as part of a program that contains a
## configuration script generated by Autoconf, you may include it under
## the same distribution terms that you use for the rest of that program.


# Usage:
#   EPIPHANY_EXTENSION_INIT(ephy-api-version, [minimum-ephy-version])
#
# Side effects:
#   EPIPHANY_EXTENSIONS_DIR is set to the directory to install extensions in
#   EPIPHANY_EXTENSION_RULE can be used to translate .ephy-extension files
#   EPIPHANY_HAS_PYTHON is set if the epiphany is compiled with python support
AC_DEFUN([EPIPHANY_EXTENSION_INIT],
[
dnl Check we have an api-version
ifelse([$1], [], [AC_MSG_ERROR([Required Epiphany API version not supplied])], [])

_epiphany_api_version=$1

AC_MSG_CHECKING([whether epiphany-$_epiphany_api_version is available])
PKG_CHECK_EXISTS([epiphany-$_epiphany_api_version], [result=yes],[result=no]) 
AC_MSG_RESULT([$result])

if test $result = "no" ; then
    AC_MSG_ERROR([Epiphany API version $_epiphany_api_version is required

This API version is used in the $_epiphany_api_version stable series, and
the preceeding development series. Please ensure you have the appropriate
Epiphany version installed.])
fi

dnl Check the minimum version
ifelse([$2], [], [],
[
  AC_MSG_CHECKING([whether minimum epiphany version $2 is available])
  PKG_CHECK_EXISTS([epiphany-$_epiphany_api_version >= $2], [result=yes], [result=no])
  AC_MSG_RESULT([$result])

  if test $result = "no" ; then
      AC_MSG_ERROR([The minimum epiphany version supported by this application is $2.

Please upgrade to at least $2. If you have compiled a later version it is
possible pkg-config isn't finding it, in which case set the PKG_CONFIG_PATH
environment variable.])
  fi
])

dnl set the EPIPHANY_EXTENSIONS_DIR variable
EPIPHANY_EXTENSIONS_DIR="`$PKG_CONFIG --variable=extensionsdir epiphany-$_epiphany_api_version`"
AC_SUBST([EPIPHANY_EXTENSIONS_DIR])

dnl Look to see if epiphany has python support
AC_MSG_CHECKING([whether epiphany has python support])
_epiphany_features="`$PKG_CONFIG --variable=features epiphany-$_epiphany_api_version`"
if echo "$_epiphany_features" | egrep '(^| )(python)($| )' > /dev/null; then
    result=yes
    EPIPHANY_HAS_PYTHON=1
    AC_SUBST([EPIPHANY_HAS_PYTHON])
else
    result=no
fi
AC_MSG_RESULT([$result])

dnl Add a rule that auto-translates the .ephy-extension files
dnl (like the INTLTOOL_XML_RULE does for xml files)
EPIPHANY_EXTENSION_RULE='%.ephy-extension: %.ephy-extension.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< [$]@'

AC_SUBST(EPIPHANY_EXTENSION_RULE)

dnl Icon dir
EPIPHANY_ICONDIR="$($PKG_CONFIG --variable=icondir epiphany-$EPIPHANY_API_VERSION)"
AC_SUBST(EPIPHANY_ICONDIR)

])
