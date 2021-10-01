# Copyright 2021-2023 MarcosHCK
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

#
# Some variables
#

PACKAGE_NAME=@PACKAGE_NAME@
PACKAGE_RELEASE=@PACKAGE_RELEASE@
PACKAGE_VERSION_MAYOR=@PACKAGE_VERSION_MAYOR@
PACKAGE_VERSION_MINOR=@PACKAGE_VERSION_MINOR@
PACKAGE_VERSION_MICRO=@PACKAGE_VERSION_MICRO@
PACKAGE_VERSION_BUILD=@PACKAGE_VERSION_BUILD@

EXTRA_DIST=
VOID=

#
# *.dasc.c to *.c make target
#

%.c: %.dasc.c
	$(DYNASM) -D fastcall_style=\"${fastcall_style}\" -o $(@:.dynasm.c=.c) $<

#
# C headers
#

EXTRA_DIST+=\
	jit.h \
	overwrites.h \
	$(VOID)

#
# DynASM source files
#

EXTRA_DIST+=\
	pipeline_x86_64.dasc.c \
	$(VOID)

#
# Library
#

noinst_LTLIBRARIES=libjit.la

libjit_la_SOURCES=\
	pipeline_${host_cpu}.c \
	pipeline_helper.c \
	$(VOID)

libjit_la_CFLAGS=\
	$(GIO_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBSOUP_CFLAGS) \
	$(LIBAKASHIC_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(SDL2_CFLAGS) \
	$(SDL2_IMG_CFLAGS) \
	$(SDL2_TTF_CFLAGS) \
	$(GLEW_CFLAGS) \
	$(CGLM_CFLAGS) \
	$(LUA_CFLAGS) \
	-DFASTCALL_STYLE=\"${fastcall_style}\" \
	-DSCHEMASDIR=\"${schemasdir}\" \
	-DGFXDIR=\"${gfxdir}\" \
	-DPKGDATADIR=\"${pkgdatadir}\" \
	-DPKGLIBEXECDIR=\"${pkglibexecdir}\" \
	-DABSTOPBUILDDIR=\"${abs_top_builddir}\" \
	-DGAPPNAME=\"org.hck.${PACKAGE_NAME}\" \
	-I${top_builddir}/build/ \
	$(VOID)

libjit_la_LDFLAGS=\
	-static \
	$(VOID)
