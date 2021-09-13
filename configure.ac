# Copyright 2021-2022 MarcosHCK
# This file is part of deusexmakina2.
#
# deusexmakina2 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# deusexmakina2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with deusexmakina2. If not, see <http://www.gnu.org/licenses/>.
#
AC_PREREQ([2.69])

m4_define([p_VERSION], [0.1.0.0])
m4_define([p_RELEASE], [alpha])

AC_INIT([deusexmakina2], [p_VERSION], [deusexmakina2.bugs@hcktm.com], [deusexmakina2], [https://www.hcktm.com/deusexmakina2/])
AC_CONFIG_SRCDIR([src/ds_application.c])
AC_LANG([C])

#
# Configure aclocal path
#
AC_CONFIG_MACRO_DIRS([m4])

#
# Setup default cross-compiling
# environment mechanics
#
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

#
# Automake
#
AM_INIT_AUTOMAKE

#
# Extract mayor, minor and micro version
# numbers from version string
# Also define release type
#
m4_define([vernum_pattern],[\(@<:@0-9@:>@*\)])

AC_SUBST([PACKAGE_VERSION_MAYOR])
PACKAGE_VERSION_MAYOR=$(echo "$PACKAGE_VERSION" | sed -e 's/vernum_pattern\.vernum_pattern\.vernum_pattern\.vernum_pattern/\1/')
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MAYOR], ["$PACKAGE_VERSION_MAYOR"], [Version mayor number])
AC_SUBST([PACKAGE_VERSION_MINOR])
PACKAGE_VERSION_MINOR=$(echo "$PACKAGE_VERSION" | sed -e 's/vernum_pattern\.vernum_pattern\.vernum_pattern\.vernum_pattern/\2/')
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MINOR], ["$PACKAGE_VERSION_MINOR"], [Version minor number])
AC_SUBST([PACKAGE_VERSION_MICRO])
PACKAGE_VERSION_MICRO=$(echo "$PACKAGE_VERSION" | sed -e 's/vernum_pattern\.vernum_pattern\.vernum_pattern\.vernum_pattern/\3/')
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_MICRO], ["$PACKAGE_VERSION_MICRO"], [Version micro number])
AC_SUBST([PACKAGE_VERSION_BUILD])
PACKAGE_VERSION_BUILD=$(echo "$PACKAGE_VERSION" | sed -e 's/vernum_pattern\.vernum_pattern\.vernum_pattern\.vernum_pattern/\4/')
AC_DEFINE_UNQUOTED([PACKAGE_VERSION_BUILD], ["$PACKAGE_VERSION_BUILD"], [Version build number])

AC_DEFINE([PACKAGE_RELEASE], ["p_RELEASE"], [Release tag])
AC_SUBST([PACKAGE_RELEASE], [p_RELEASE])

#
#  Checks for programs we'll
# need to build
#
AC_PROG_CC
AC_PROG_CXX
AM_PROG_VALAC
AC_PROG_INSTALL

LT_PREREQ([2.4.6])
LT_INIT
PKG_PROG_PKG_CONFIG

AC_CHECK_PROG([GLIB_MKENUMS], [glib-mkenums], [glib-mkenums], [no])
if test "x$GLIB_MKENUMS" = "xno"; then
  AC_MSG_FAILURE([glib-mkenums not found our your system])
fi
AC_CHECK_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources], [glib-compile-resources], [no])
if test "x$GLIB_COMPILE_RESOURCES" = "xno"; then
  AC_MSG_FAILURE([glib-compile-resources not found our your system])
fi
AC_CHECK_PROG([GLIB_COMPILE_SCHEMAS], [glib-compile-schemas], [glib-compile-schemas], [no])
if test "x$GLIB_COMPILE_SCHEMAS" = "xno"; then
  AC_MSG_FAILURE([glib-compile-schemas not found our your system])
fi

#
# Do you want to debug? Tell me!
#
debugging="no"
AC_ARG_ENABLE(debug,
              AS_HELP_STRING([--enable-debug], [Enable debug object output @<:@default=no@:>@]),
              [if test "x$enableval" != "xno"; then
                debugging="yes"
               fi
              ])

if test "x$debugging" = "xno"; then
  CFLAGS="-O2 $CFLAGS"
  LDFLAGS="-s $LDFLAGS"

  AC_DEFINE([DEBUG], [0], [Debugging disabled])
  AC_SUBST([DEBUG], [0])
else
  CFLAGS="-g $CFLAGS"

  AC_DEFINE([DEBUG], [1], [Debugging enabled])
  AC_SUBST([DEBUG], [1])
fi

#
# Check for libraries using pkg-config
#
PKG_CHECK_MODULES([GIO], [gio-2.0])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([LIBSOUP], [libsoup-2.4])
PKG_CHECK_MODULES([LIBAKASHIC], [libakashic])
PKG_CHECK_MODULES([OPENGL], [opengl])
PKG_CHECK_MODULES([SDL2], [sdl2])
PKG_CHECK_MODULES([SDL2_IMG], [SDL2_image])
PKG_CHECK_MODULES([SDL2_TTF], [SDL2_ttf])
PKG_CHECK_MODULES([GLEW], [glew])
PKG_CHECK_MODULES([CGLM], [cglm])
PKG_CHECK_MODULES([LUA], [luajit lua5.1 lua51])

#
# Prepare output
#
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
Makefile
src/Makefile
scripts/Makefile
scripts/build.lua
settings/Makefile
])

AC_OUTPUT
