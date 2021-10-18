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

# Keep sync with src/Makefile.am
girdir=${pkgdatadir}/gir/

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
# Source headers
#

GIR_NAMESPACE=Ds
GIR_VERSION=${PACKAGE_VERSION_MAYOR}.${PACKAGE_VERSION_MINOR}

GIR_PKG=\
	gio-2.0 \
	gmodule-2.0 \
	cglm \
	$(VOID)

GIR_INC=\
	Gio-2.0 \
	GL-1.0 \
	$(VOID)

#
# .gir file
#

${GIR_NAMESPACE}-${GIR_VERSION}.gir: sources.list ${top_srcdir}/src/libdeus2.la
	${INTROSPECTION_SCANNER} \
		--output ${GIR_NAMESPACE}-${GIR_VERSION}.gir \
		--nsversion "${GIR_VERSION}" \
		--library "${top_srcdir}/src/libdeus2.la" \
		--warn-all \
		--namespace ${GIR_NAMESPACE} \
		--cflags-begin \
			-I${top_srcdir}/src/ \
			-I${top_srcdir}/src/luad \
		--cflags-end \
		--libtool "$(LIBTOOL)" \
		--sources-top-dirs=${top_srcdir} \
		--include-first-in-src="config.h" \
		$(addprefix --pkg , ${GIR_PKG}) \
		$(addprefix --include , ${GIR_INC}) \
		--filelist sources.list \
		$(VOID)

#
# *.typelib file
#

${GIR_NAMESPACE}-${GIR_VERSION}.typelib: ${GIR_NAMESPACE}-${GIR_VERSION}.gir
	$(INTROSPECTION_COMPILER) \
		--output ${GIR_NAMESPACE}-${GIR_VERSION}.typelib \
			${GIR_NAMESPACE}-${GIR_VERSION}.gir

#
# GObject introspection data
#

gir_DATA=\
	${GIR_NAMESPACE}-${GIR_VERSION}.gir \
	${GIR_NAMESPACE}-${GIR_VERSION}.typelib \
	$(VOID)
