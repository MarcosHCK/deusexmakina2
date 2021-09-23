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

# Keep sync with settings/Makefile.am
schemasdir=${pkgdatadir}/schemas/

# Keep sync with gfx/Makefile.am
gfxdir=${pkgdatadir}/gfx/

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
	jit \
	vapi \
	$(VOID)

#
# Source files where
# at least an enum is defined
#

ENUM_FILES=\
	ds_gl.h \
	$(VOID)

ds_enums.c: $(ENUM_FILES) ds_enums.c.template
	$(GLIB_MKENUMS) --symbol-prefix "ds" \
									--identifier-prefix "Ds" \
									--template "ds_enums.c.template" \
									--output $@ \
									$(ENUM_FILES)

#
# Marshals
#

MARSHAL_CFLAGS=\
	--stdinc \
	--prototypes \
	--valist-marshallers \
	--internal \
	--prefix="ds_cclosure_marshal" \
	$(VOID)

ds_marshals.c: ds_marshals.list ds_marshals.h
	$(GLIB_GENMARSHAL) --body --include-header=config.h $(MARSHAL_CFLAGS) --output=$@ ds_marshals.list

ds_marshals.h: ds_marshals.list
	$(GLIB_GENMARSHAL) --header $(MARSHAL_CFLAGS) --output=$@ ds_marshals.list

#
# C headers
#

EXTRA_DIST+=\
	ds_application.h \
	ds_application_private.h \
	ds_callable.h \
	ds_closure.h \
	ds_dds.h \
	ds_error.h \
	ds_events.h \
	ds_gl.h \
	ds_lua.h \
	ds_luaboxed.h \
	ds_luaclass.h \
	ds_luaclosure.h \
	ds_luaenum.h \
	ds_luaobj.h \
	ds_luavala.h \
	ds_macros.h \
	ds_marshals.h \
	ds_model.h \
	ds_pipeline.h \
	ds_renderable.h \
	ds_renderer.h \
	ds_saves.h \
	ds_settings.h \
	ds_shader.h \
	ds_skybox.h \
	$(VOID)

#
# Binaries and libraries
#

bin_PROGRAMS=deusexmakina2
deusexmakina2_SOURCES=\
	ds_application.c \
	ds_application_intr.c \
	ds_callable.c \
	ds_closure.c \
	ds_dds.c \
	ds_enums.c \
	ds_error.c \
	ds_events.c \
	ds_gl.c \
	ds_lua.c \
	ds_luaboxed.c \
	ds_luaclass.c \
	ds_luaenum.c \
	ds_lualib.c \
	ds_luaobj.c \
	ds_marshals.c \
	ds_model.c \
	ds_pipeline.c \
	ds_renderable.c \
	ds_renderer.c \
	ds_saves.vala \
	ds_settings.vala \
	ds_shader.c \
	ds_skybox.c \
	ds_utils.vala \
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
	$(ASSIMP_CFLAGS) \
	-DSCHEMASDIR=\"${schemasdir}\" \
	-DGFXDIR=\"${gfxdir}\" \
	-DPKGDATADIR=\"${pkgdatadir}\" \
	-DPKGLIBEXECDIR=\"${pkglibexecdir}\" \
	-DABSTOPBUILDDIR=\"${abs_top_builddir}\" \
	-DGAPPNAME=\"org.hck.${PACKAGE_NAME}\" \
	-fstack-protector-strong \
	-funwind-tables \
	-Winline \
	-Wpacked \
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
	$(ASSIMP_LIBS) \
	jit/libjit.la \
	$(VOID)

#
# VALA
#

VALA_PKG=\
	gio-2.0 \
	gobject-2.0 \
	glib-2.0 \
	lua \
	config \
	luad \
	$(VOID)

AM_VALAFLAGS=\
	--vapidir=vapi/ \
	--hide-internal \
	$(addprefix --pkg , ${VALA_PKG}) \
	$(VOID)
