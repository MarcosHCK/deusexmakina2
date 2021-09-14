/*  Copyright 2021-2022 MarcosHCK
 *  This file is part of deusexmakina2.
 *
 *  deusexmakina2 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  deusexmakina2 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with deusexmakina2.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __DS_PIPELINE_PRIVATE_INCLUDED__
#define __DS_PIPELINE_PRIVATE_INCLUDED__

#include <ds_macros.h>
#include <ds_pipeline.h>
#include <GL/glew.h>

/*
 * Some macros
 *
 */

#define DS_DEFINE_SNIPPET(name) \
void \
_ds_##name##_init (); \
void \
_ds_##name##_fini ();

#define DS_SNIPPET_POINTER(name) \
((gconstpointer) _ds_##name##_init)

#define DS_SNIPPET_SIZE(name) \
((gsize) (_ds_##name##_fini - _ds_##name##_init))

#endif // __DS_PIPELINE_PRIVATE_INCLUDED__
