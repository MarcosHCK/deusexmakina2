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

# Where assets goes (data comes in a different package)
assetsdir=${pkgdatadir}/assets/

# Keep sync with gfx/Makefile.am
gfxdir=${pkgdatadir}/gfx/

# Keep sync with gir/Makefile.am
girdir=${pkgdatadir}/gir/

# Keep sync with settings/Makefile.am
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
# C headers
#

EXTRA_DIST+=\
	ds_saves.h \
	ds_world.h \
	$(VOID)

#
# Binaries and libraries
#

pkglib_LTLIBRARIES=libdeus2simulator.la
libdeus2simulator_la_SOURCES=\
	ds_saves.vala \
	ds_world.vala \
	$(VOID)
libdeus2simulator_la_CFLAGS=\
	$(CGLM_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIR_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GMODULE_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(LIBAKASHIC_CFLAGS) \
	$(LIBSOUP_CFLAGS) \
	$(LUA_CFLAGS) \
	$(LIBLUAD2_CFLAGS) \
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
	-fstack-protector-strong \
	-funwind-tables \
	-Winline \
	-Wpacked \
	-Wimplicit-fallthrough \
	$(VOID)
libdeus2simulator_la_LDFLAGS=\
	-rpath ${pkglibdir} \
	-release ${PACKAGE_RELEASE} \
	-version-info ${PACKAGE_VERSION_MAYOR}:${PACKAGE_VERSION_MINOR}:${PACKAGE_VERSION_MICRO} \
	$(VOID)
libdeus2simulator_la_LIBADD=\
	$(CGLM_LIBS) \
	$(GIO_LIBS) \
	$(GIR_LIBS) \
	$(GLIB_LIBS) \
	$(GMODULE_LIBS) \
	$(GOBJECT_LIBS) \
	$(LIBAKASHIC_LIBS) \
	$(LIBSOUP_LIBS) \
	$(LUA_LIBS) \
	$(LIBLUAD2_CFLAGS) \
	$(VOID)

#
# Vala
#

VALA_PKG=\
	gio-2.0 \
	glib-2.0 \
	gmodule-2.0 \
	gobject-2.0 \
	gobject-introspection-1.0 \
	$(VOID)

VALA_VAPIS=\
	config \
	dds \
	deus \
	$(VOID)

AM_VALAFLAGS=\
	--vapidir=../vapi/ \
	--hide-internal \
	--abi-stability \
	-D DEBUG=${DEBUG} \
	-D DEVELOPER=${DEVELOPER} \
	$(addprefix --pkg , ${VALA_PKG}) \
	$(addprefix --pkg , ${VALA_VAPIS}) \
	$(VOID)
