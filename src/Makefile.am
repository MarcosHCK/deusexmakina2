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

# Keep sync with src/Makefile.am
schemasdir=${pkgdatadir}/schemas/

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
# Subdirectories
#

SUBDIRS=\
	vapi

#
# Source files where
# at least an enum is defined
#
ENUM_FILES=\
	$(VOID)

ds_enums.c: $(ENUM_FILES) ds_enums.c.template
	$(GLIB_MKENUMS) --symbol-prefix "ds" \
									--identifier-prefix "Ds" \
									--template "ds_enums.c.template" \
									--output $@ \
									$(ENUM_FILES)

#
# C headers
#

EXTRA_DIST+=\
	ds_application.h \
	ds_application_private.h \
	ds_events.h \
	ds_lua.h \
	ds_macros.h \
	ds_renderer.h \
	ds_settings.h \
	$(VOID)

#
# Binaries and libraries
#

bin_PROGRAMS=deusexmakina2
deusexmakina2_SOURCES=\
	ds_application.c \
	ds_events.c \
	ds_lua.c \
	ds_renderer.c \
	ds_settings.vala \
	$(VOID)

deusexmakina2_CFLAGS=\
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
	-DSCHEMASDIR=\"${schemasdir}\" \
	-DPKGDATADIR=\"${pkgdatadir}\" \
	-DPKGLIBEXECDIR=\"${pkglibexecdir}\" \
	-DABSTOPBUILDDIR=\"${abs_top_builddir}\" \
	-DGAPPNAME=\"org.hck.${PACKAGE_NAME}\"
	$(VOID)

deusexmakina2_LDADD=\
	$(GIO_LIBS) \
	$(GOBJECT_LIBS) \
	$(GLIB_LIBS) \
	$(LIBSOUP_LIBS) \
	$(LIBAKASHIC_LIBS) \
	$(OPENGL_LIBS) \
	$(SDL2_LIBS) \
	$(SDL2_IMG_LIBS) \
	$(SDL2_TTF_LIBS) \
	$(GLEW_LIBS) \
	$(CGLM_LIBS) \
	$(LUA_LIBS) \
	$(VOID)

#
# VALA
#

VALA_PKG=\
	gio-2.0 \
	gobject-2.0 \
	glib-2.0 \
	config \
	$(VOID)

AM_VALAFLAGS=\
	--vapidir=vapi/ \
	$(addprefix --pkg , ${VALA_PKG}) \
	$(VOID)
