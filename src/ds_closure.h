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
#ifndef __DS_CLOSURE_INCLUDED__
#define __DS_CLOSURE_INCLUDED__
#include <glib-object.h>

typedef struct _DsClosure DsClosure;

typedef enum {
  DS_CLOSURE_FLAGS_NONE,
  DS_CLOSURE_CONSTRUCTOR,
} DsClosureFlags;

struct _DsClosure
{
  GClosure closure;
  GCallback callback;
  GVaClosureMarshal vmarshal;
  DsClosureFlags flags;
  GType return_type;
  guint n_params;
  GType* params;
};

#define DS_INVOKE_STATIC_SCOPE G_SIGNAL_TYPE_STATIC_SCOPE

#if __cplusplus
extern "C" {
#endif // __cplusplus

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_CLOSURE_INCLUDED__
