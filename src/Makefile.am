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

# Where assets goes (data comes in a different package)
assetsdir=${pkgdatadir}/assets/

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
	ds_dds_fmt.h \
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
	--prototypes \
	--valist-marshallers \
	--internal \
	--prefix="ds_cclosure_marshal" \
	$(VOID)

ds_marshals.c: ds_marshals.list ds_marshals.h
	$(MKMARSHAL) --body --include-header=config.h $(MARSHAL_CFLAGS) --output=$@ ds_marshals.list

ds_marshals.h: ds_marshals.list
	$(MKMARSHAL) --header $(MARSHAL_CFLAGS) --output=$@ ds_marshals.list

ds_marshals.vapi: ds_marshals.list ds_marshals.h ds_marshals.c
	$(MKMARSHAL) --vapi --include-header=ds_marshals.h $(MARSHAL_CFLAGS) --output=$@ ds_marshals.list

#
# C headers
#

EXTRA_DIST+=\
	ds_application.h \
	ds_callable.h \
	ds_closure.h \
	ds_dds.h \
	ds_error.h \
	ds_events.h \
	ds_folder_provider.h \
	ds_font.h \
	ds_gl.h \
	ds_looper.h \
	ds_lua.h \
	ds_luaboxed.h \
	ds_luaclass.h \
	ds_luaclosure.h \
	ds_luaenum.h \
	ds_luagir.h \
	ds_luaobj.h \
	ds_luaqdata.h \
	ds_luavala.h \
	ds_macros.h \
	ds_marshals.h \
	ds_model.h \
	ds_model_private.h \
	ds_mvpholder.h \
	ds_pipeline.h \
	ds_renderable.h \
	ds_renderer.h \
	ds_saves.h \
	ds_settings.h \
	ds_shader.h \
	ds_skybox.h \
	ds_text.h \
	$(VOID)

#
# Binaries and libraries
#

bin_PROGRAMS=deusexmakina2
deusexmakina2_SOURCES=\
	ds_application.c \
	ds_basedirs.vala \
	ds_callable.c \
	ds_closure.c \
	ds_dds.vala \
	ds_enums.c \
	ds_error.c \
	ds_events.c \
	ds_folder_provider.vala \
	ds_font.c \
	ds_font_cache.c \
	ds_gl.c \
	ds_icu.cpp \
	ds_looper.vala \
	ds_lua.c \
	ds_luaboxed.c \
	ds_luaclass.c \
	ds_luaenum.c \
	ds_luagir.vala \
	ds_lualib.c \
	ds_luaobj.c \
	ds_luaqdata.c \
	ds_marshals.c \
	ds_marshals.vapi \
	ds_model.c \
	ds_model_buf.c \
	ds_model_imp.c \
	ds_model_tex.c \
	ds_model_single.c \
	ds_mvpholder.c \
	ds_pipeline.c \
	ds_renderable.c \
	ds_renderer.c \
	ds_saves.vala \
	ds_settings.vala \
	ds_shader.c \
	ds_shader_cache.c \
	ds_skybox.c \
	ds_text.c \
	ds_utils.vala \
	$(VOID)

deusexmakina2_CFLAGS=\
	$(ASSIMP_CFLAGS) \
	$(CGLM_CFLAGS) \
	$(FREETYPE_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIR_CFLAGS) \
	$(GLEW_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GMODULE_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(LIBAKASHIC_CFLAGS) \
	$(LIBICU_CFLAGS) \
	$(LIBSOUP_CFLAGS) \
	$(LIBWEBP_CFLAGS) \
	$(LUA_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(SDL2_CFLAGS) \
	-DASSETSDIR=\"${assetsdir}\" \
	-DGFXDIR=\"${gfxdir}\" \
	-DSCHEMASDIR=\"${schemasdir}\" \
	-DPKGDATADIR=\"${pkgdatadir}\" \
	-DPKGLIBEXECDIR=\"${pkglibexecdir}\" \
	-DABSTOPBUILDDIR=\"${abs_top_builddir}\" \
	-DGAPPNAME=\"org.hck.${PACKAGE_NAME}\" \
	-fstack-protector-strong \
	-funwind-tables \
	-Winline \
	-Wpacked \
	$(STB_CFLAGS) \
	$(VOID)

deusexmakina2_CXXFLAGS= \
	$(deusexmakina2_CFLAGS) \
	$(VOID)

deusexmakina2_LDADD=\
	$(ASSIMP_LIBS) \
	$(CGLM_LIBS) \
	$(FREETYPE_LIBS) \
	$(GIO_LIBS) \
	$(GIR_LIBS) \
	$(GLEW_LIBS) \
	$(GLIB_LIBS) \
	$(GMODULE_LIBS) \
	$(GOBJECT_LIBS) \
	$(LIBAKASHIC_LIBS) \
	$(LIBICU_LIBS) \
	$(LIBSOUP_LIBS) \
	$(LIBWEBP_LIBS) \
	$(LUA_LIBS) \
	$(OPENGL_LIBS) \
	$(SDL2_LIBS) \
	jit/libjit.la \
	$(STB_LIBS) \
	$(VOID)

#
# VALA
#

VALA_PKG=\
	gio-2.0 \
	glib-2.0 \
	gmodule-2.0 \
	gobject-2.0 \
	gobject-introspection-1.0 \
	$(VOID)

VALA_VAPIS=\
	closure \
	config \
	dds \
	lua \
	luad \
	model \
	$(VOID)

AM_VALAFLAGS=\
	--vapidir=vapi/ \
	--hide-internal \
	$(addprefix --pkg , ${VALA_PKG}) \
	$(addprefix --pkg , ${VALA_VAPIS}) \
	$(VOID)
