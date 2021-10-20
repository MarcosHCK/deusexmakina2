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
# C headers
#

EXTRA_DIST+=\
	luad_core.h \
	luad_lib.h \
	$(VOID)

#
# Binaries and libraries
#

pkglib_LTLIBRARIES=libluad2.la
libluad2_la_SOURCES=\
	luad_core.c \
	luad_lib.c \
	$(VOID)
libluad2_la_CFLAGS=\
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
	$(LIBSOUP_CFLAGS) \
	$(LIBWEBP_CFLAGS) \
	$(LUA_CFLAGS) \
	$(OPENGL_CFLAGS) \
	$(SDL2_CFLAGS) \
	$(CFLAG_VISIBILITY) \
	-DASSETSDIR=\"${assetsdir}\" \
	-DGFXDIR=\"${gfxdir}\" \
	-DGIRDIR=\"${girdir}\" \
	-DSCHEMASDIR=\"${schemasdir}\" \
	-DPKGDATADIR=\"${pkgdatadir}\" \
	-DPKGLIBEXECDIR=\"${pkglibexecdir}\" \
	-DABSTOPBUILDDIR=\"${abs_top_builddir}\" \
	-DTOPBUILDDIR=\"${top_builddir}\" \
	-DGAPPNAME=\"org.hck.${PACKAGE_NAME}\" \
	-D__DUESEXMAKINA_INSIDE__=1 \
	-D__LUAD_INSIDE__=1 \
	-fstack-protector-strong \
	-funwind-tables \
	-Winline \
	-Wpacked \
	-Wimplicit-fallthrough \
	-I${top_builddir}/src/ \
	$(VOID)
libluad2_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-release ${PACKAGE_RELEASE} \
	-version-info ${PACKAGE_VERSION_MAYOR}:${PACKAGE_VERSION_MINOR}:${PACKAGE_VERSION_MICRO} \
	$(VOID)
libluad2_la_LIBADD=\
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
	$(LIBSOUP_LIBS) \
	$(LIBWEBP_LIBS) \
	$(LUA_LIBS) \
	$(OPENGL_LIBS) \
	$(SDL2_LIBS) \
	$(VOID)
